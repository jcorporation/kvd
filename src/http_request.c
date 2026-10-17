/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief HTTP Request event handler
 */

#include "compile_time.h"

#include "src/http_request.h"

#include "dist/mongoose/mongoose.h"
#include "src/kvd_rest.h"
#include "src/lib/global_data.h"
#include "src/lib/http.h"
#include "src/lib/log.h"
#include "src/rrdcached.h"

/**
 * Central HTTP request event handler
 * @param nc mongoose connection
 * @param ev_data event data
 */
void request_handler(struct mg_connection *nc, void *ev_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    KVD_LOG_DEBUG("Rest: HTTP request %.*s %.*s",
        (int)hm->method.len, hm->method.buf,
        (int)hm->uri.len, hm->uri.buf
    );

    // Connection handling
    nc->data[0] = 'C';
    if (mg_strcmp(hm->proto, mg_str("HTTP/1.1")) == 1) {
        struct mg_str *connection_hdr = mg_http_get_header(hm, "Connection");
        if (connection_hdr != NULL &&
            mg_strcasecmp(*connection_hdr, mg_str("keep-alive")) == 0)
        {
            nc->data[0] = 'K';
        }
    }

    // Enforce size limit of 1 MB
    if (hm->body.len > MAX_VALUE_SIZE) {
        mg_http_reply(nc, HTTP_CONTENT_TOO_LARGE, HTTP_HEADER_JSON,
            "{\"error\":\"Value too large, max. size is %d byte\"}",
            MAX_VALUE_SIZE
        );
        KVD_LOG_ERROR("Value too large.");
        http_handle_connection_close(nc);
        return;
    }

    // Match uri
    struct mg_str caps[2];
    if (mg_match(hm->uri, mg_str("/kv1/"), NULL)) {
        request_kvd_list(nc);
        http_handle_connection_close(nc);
        return;
    }
    if (mg_match(hm->uri, mg_str("/kv1/#"), caps)) {
        if (caps[0].len == 0) {
            mg_http_reply(nc, HTTP_BAD_REQUEST, HTTP_HEADER_JSON, "{\"error\":\"Key could not be empty.\"}");
            http_handle_connection_close(nc);
            return;
        }
        if (caps[0].len > MAX_KEY_SIZE) {
            mg_http_reply(nc, HTTP_BAD_REQUEST, HTTP_HEADER_JSON, "{\"error\":\"Key is too long.\"}");
            http_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("GET")) == 0) {
            request_kvd_get(nc, &caps[0]);
            http_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("DELETE")) == 0) {
            request_kvd_delete(nc, &caps[0]);
            http_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
            request_kvd_options(nc, &caps[0]);
            http_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("PUT")) == 0 ||
            mg_strcmp(hm->method, mg_str("POST")) == 0)
        {
            request_kvd_put(nc, &caps[0], hm);
            http_handle_connection_close(nc);
            return;
        }
    }
    else if (mg_match(hm->uri, mg_str("/rrdcached/"), caps)) {
        if (global_data->config->rrdcached_uri[0] == '\0') {
            mg_http_reply(nc, HTTP_NOT_IMPLEMENTED, HTTP_HEADER_JSON, "{\"error\":\"Not RRD Cached URI configured\"}");
            http_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("POST")) == 0) {
            struct t_rrdcached_data *data = rrdcached_data_new(nc, &hm->body);
            if (send_data_to_rrdcached(nc->mgr, data) == false) {
                mg_http_reply(nc, HTTP_NOT_IMPLEMENTED, HTTP_HEADER_JSON, "{\"error\":\"Failure creating RRD Cached connection\"}");
                http_handle_connection_close(nc);
            }
            return;
        }
    }
    // Other methods are not implemented
    // Request was not handled
    mg_http_reply(nc, HTTP_NOT_IMPLEMENTED, HTTP_HEADER_JSON, "{\"error\":\"Not implemented\"}");
    http_handle_connection_close(nc);
}
