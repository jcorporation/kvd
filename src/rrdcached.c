/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief Send data to RRDCached
 */

#include "src/rrdcached.h"

#include "dist/mongoose/mongoose.h"
#include "src/lib/global_data.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/rest.h"
#include <stdint.h>

// Private definitions

static void ev_handler_rrdcached(struct mg_connection *nc, int ev, void *ev_data);

// Public functions

struct t_rrdcached_data *rrdcached_data_new(struct mg_connection *nc, struct mg_str *value) {
    struct t_rrdcached_data *data = malloc_assert(sizeof(struct t_rrdcached_data));
    data->lines_expected = 0;
    data->lines_received = 0;
    data->rest_nc = nc;
    data->data.len = value->len;
    data->data.buf = my_memcpy(value->buf, value->len);
    return data;
}

/**
 * Frees the t_rrdcached_data struct
 * @param data 
 */
void rrdcached_data_free(struct t_rrdcached_data *data) {
    free(data->data.buf);
    free(data);
}

/**
 * Sends the data to a RRDCached instance
 * @param mgr Mongoose mgr
 * @param data Data to send
 * @param data_len Data Length
 */
bool send_data_to_rrdcached(struct mg_mgr *mgr, struct t_rrdcached_data *data) {
    if (mg_connect(mgr, global_data->config->rrdcached_uri, ev_handler_rrdcached, data) == NULL) {
        KVD_LOG_ERROR("RRDCached: Error creating connection to: %s", global_data->config->rrdcached_uri);
        rrdcached_data_free(data);
        return false;
    }
    return true;
}

// Private functions

/**
 * Parsed the first RRD Cache response line
 * @param buf Receive buffer
 * @param len Receive length
 * @return Response code or INT32_MIN on error
 */
static int32_t parse_rrdcached_response_code(const unsigned char *buf, size_t len) {
    char response_code[12];
    size_t code_len = len < 11 ? len : 11;
    for (size_t i = 0; i < code_len; i++) {
        if (buf[i] == ' ') {
            response_code[i] = '\0';
            return (int32_t)strtol(response_code, NULL, 10);
        }
        response_code[i] = (char)buf[i];
    }
    KVD_LOG_ERROR("Unparsable RRD Cached response");
    return INT32_MIN;
}

/**
 * Central RRDCached connection handler
 * @param nc mongoose connection
 * @param ev connection event
 * @param ev_data event data
 */
static void ev_handler_rrdcached(struct mg_connection *nc, int ev, void *ev_data) {
    (void) ev_data; //unused
    if (ev == MG_EV_CONNECT) {
        KVD_LOG_DEBUG("RRDCached: Connected");
        // Send the data with a newline appended
        struct t_rrdcached_data *data =(struct t_rrdcached_data *)nc->fn_data;

        //Store pointer to rrdcached connection in rest connection
        data->rest_nc->data[1] = 'b';
        data->rest_nc->fn_data = nc;

        KVD_LOG_DEBUG("RRDCached: Sending %.*s", (int)data->data.len, data->data.buf);
        mg_printf(nc, "%.*s\n", (int)data->data.len, data->data.buf);
    }
    else if (ev == MG_EV_READ) {
        struct t_rrdcached_data *data =(struct t_rrdcached_data *)nc->fn_data;
        // First packet
        if (data->lines_expected == 0 &&
            data->lines_received == 0)
        {
            // Extract response code
            int32_t val = parse_rrdcached_response_code(nc->recv.buf, nc->recv.len);
            KVD_LOG_DEBUG("RRDCached: Response code %d", val);

            // Parsing error
            if (val == INT32_MIN) {
                mg_http_reply(data->rest_nc, HTTP_BAD_GATEWAY, HTTP_HEADER_PLAIN, "Unparsable RRD Cached response");
                rest_handle_connection_close(data->rest_nc);
                // Close connection to rrdcached
                nc->is_draining = 1;
                mg_iobuf_del(&nc->recv, 0, nc->recv.len);
                return;
            }
            // RRD Cached error
            if (val < 0) {
                KVD_LOG_ERROR("RRDCached: Error from rrdcached %d", val);
                mg_http_reply(data->rest_nc, HTTP_BAD_GATEWAY, HTTP_HEADER_PLAIN, "%.*s",
                    (int)nc->recv.len, nc->recv.buf);
                rest_handle_connection_close(data->rest_nc);
                // Close connection to rrdcached
                nc->is_draining = 1;
                mg_iobuf_del(&nc->recv, 0, nc->recv.len);
                return;
            }
            // No response lines
            if (val == 0) {
                mg_http_reply(data->rest_nc, HTTP_OK, HTTP_HEADER_PLAIN, "%.*s",
                    (int)nc->recv.len, nc->recv.buf);
                rest_handle_connection_close(data->rest_nc);
                // Close connection to rrdcached
                nc->is_draining = 1;
                mg_iobuf_del(&nc->recv, 0, nc->recv.len);
                return;
            }

            // Set expected lines count and print chunking header to REST-API connection
            data->lines_expected = val + 1;
            mg_printf(data->rest_nc, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
        }

        // Count received lines
        for (size_t i = 0; i < nc->recv.len; i++) {
            if (nc->recv.buf[i] == '\n') {
                data->lines_received++;
            }
        }

        // Move data from rrdcached to REST-API connection
        mg_http_printf_chunk(data->rest_nc, "%.*s", (int)nc->recv.len, nc->recv.buf);
        mg_iobuf_del(&nc->recv, 0, nc->recv.len);

        // Handle connection end
        if (data->lines_received == data->lines_expected) {
            // Complete RRD Cached response received, send empty chunk to http client
            mg_http_printf_chunk(data->rest_nc, "");
            rest_handle_connection_close(data->rest_nc);
            // And close rrdcached connection
            nc->is_draining = 1;
        }
    }
    else if (ev == MG_EV_ERROR) {
        KVD_LOG_ERROR("RRDCached: Connection error occurred");
    }
    else if (ev == MG_EV_CLOSE) {
        if (nc->fn_data != NULL) {
            struct t_rrdcached_data *data =(struct t_rrdcached_data *)nc->fn_data;
            KVD_LOG_DEBUG("RRDCached: Received response lines %d/%d", data->lines_received, data->lines_expected);
            rrdcached_data_free(data);
            nc->fn_data = NULL;
        }
        KVD_LOG_DEBUG("RRDCached: Connection closed");
    }
}
