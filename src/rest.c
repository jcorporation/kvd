/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief REST API implementation
 */

#include "compile_time.h"

#include "src/rest.h"

#include "dist/mongoose/mongoose.h"
#include "kvd_store.h"
#include "src/kvd_store.h"
#include "src/lib/log.h"
#include "src/lib/mg_user_data.h"

// Private functions
static void rest_handle_connection_close(struct mg_connection *nc);
static void rest_get(struct mg_connection *nc, const struct mg_str *key);
static void rest_delete(struct mg_connection *nc, const struct mg_str *key);
static void rest_options(struct mg_connection *nc, const struct mg_str *key);
static void rest_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm);

static const char *response_headers = "Content-Type: application/json\r\n";

enum {
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    BAD_REQUEST = 400,
    HTTP_NOT_FOUND = 404,
    CONTENT_TOO_LARGE = 413,
    NOT_IMPLEMENTED = 501,
    INSUFFICIENT_STORAGE = 507,
};

// Public functions

/**
 * Central REST event handler
 * @param nc mongoose connection
 * @param ev connection event
 * @param ev_data event data
 */
void http_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    if (ev == MG_EV_OPEN) {
        nc->data[0] = '-'; // connection header
    }
    else if (ev == MG_EV_HTTP_MSG) {
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
            mg_http_reply(nc, CONTENT_TOO_LARGE, response_headers, 
                "{\"error\":\"Content too large, max. size is %d byte\"}",
                MAX_VALUE_SIZE
            );
            rest_handle_connection_close(nc);
            return;
        }

        // Match uri
        struct mg_str caps[3];  // Two wildcard symbols '*' plus 1
        if (mg_match(hm->uri, mg_str("/kv1/#"), caps)) {
            if (mg_strcmp(hm->method, mg_str("GET")) == 0) {
                rest_get(nc, &caps[1]);
                rest_handle_connection_close(nc);
                return;
            }
            if (mg_strcmp(hm->method, mg_str("DELETE")) == 0) {
                rest_delete(nc, &caps[1]);
                rest_handle_connection_close(nc);
                return;
            }
            if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
                rest_options(nc, &caps[1]);
                rest_handle_connection_close(nc);
                return;
            }
            if (mg_strcmp(hm->method, mg_str("PUT")) == 0 ||
                mg_strcmp(hm->method, mg_str("POST")) == 0)
            {
                rest_put(nc, &caps[1], hm);
                rest_handle_connection_close(nc);
                return;
            }
        }
        // Other methods are not implemented
        // Request was not handled
        mg_http_reply(nc, NOT_IMPLEMENTED, response_headers, "{\"error\":\"Not implemented\"}");
        rest_handle_connection_close(nc);
    }
}

// Private functions

static void rest_get(struct mg_connection *nc, const struct mg_str *key) {
    const struct t_mg_user_data *mg_user_data = (struct t_mg_user_data *)nc->mgr->userdata;

    struct t_kvd_data *data = kvd_store_get(mg_user_data->kvd_store, key);
    if (data == NULL) {
        mg_http_reply(nc, HTTP_NOT_FOUND, response_headers, "{\"error\":\"Key not found\"}");
        return;
    }

    char *content_type_header = mg_mprintf("Content-type: %.*s\r\n"
        "X-KVD-Created: %lld\r\n"
        "X-KVD-Modified: %lld\r\n",
        (int)data->content_type.len, data->content_type.buf,
        (long long)data->created,
        (long long)data->modified
    );
    mg_http_reply(nc, HTTP_OK, content_type_header, "%.*s", (int)data->value.len, data->value.buf);
    free(content_type_header);
}

static void rest_delete(struct mg_connection *nc, const struct mg_str *key) {
    const struct t_mg_user_data *mg_user_data = (struct t_mg_user_data *)nc->mgr->userdata;
    kvd_store_delete(mg_user_data->kvd_store, key);
    mg_http_reply(nc, HTTP_OK, response_headers, "{\"result\":\"OK\"}");
}

static void rest_options(struct mg_connection *nc, const struct mg_str *key) {
    const struct t_mg_user_data *mg_user_data = (struct t_mg_user_data *)nc->mgr->userdata;
    struct t_kvd_data *data = kvd_store_get(mg_user_data->kvd_store, key);
    if (data == NULL) {
        mg_http_reply(nc, HTTP_NOT_FOUND, response_headers, "{\"error\":\"Key not found\"}");
        return;
    }
    mg_http_reply(nc, HTTP_OK, response_headers,
        "{%m:{"
            "%m: %m,"
            "%m: %lld,"
            "%m: %lld"
        "}}",
        MG_ESC("result"),
        MG_ESC("content-type"), mg_print_esc, (int)data->content_type.len, data->content_type.buf,
        MG_ESC("created"), (long long)data->created,
        MG_ESC("modified"), (long long)data->modified
    );
}

static void rest_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm) {
    const struct t_mg_user_data *mg_user_data = (struct t_mg_user_data *)nc->mgr->userdata;

    if (mg_user_data->kvd_store->numele > MAX_KEYS) {
        mg_http_reply(nc, INSUFFICIENT_STORAGE, response_headers, "{\"error\":\"Too many keys.\"}");
        return;
    }

    const struct mg_str *content_type = mg_http_get_header(hm, "content-type");
    if (content_type == NULL) {
        mg_http_reply(nc, BAD_REQUEST, response_headers, "{\"error\":\"Content-Type not found\"}");
        return;
    }

    enum kvd_result result = kvd_store_put(mg_user_data->kvd_store, key, &hm->body, content_type);
    mg_http_reply(nc, (int)result, response_headers, "{\"result\":\"OK\"}");
}

/**
 * Drains the connection if connection is set to close
 * @param nc mongoose connection
 */
static void rest_handle_connection_close(struct mg_connection *nc) {
    if (nc->data[0] == 'C') {
        nc->is_draining = 1;
    }
    nc->is_resp = 0;
}
