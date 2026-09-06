/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief REST API implementation
 */

#include "compile_time.h"

#include "src/rest.h"

#include "dist/mongoose/mongoose.h"
#include "kvd_store.h"
#include "src/kvd_store.h"
#include "src/lib/global_data.h"
#include "src/lib/log.h"

// Private functions
static void rest_list(struct mg_connection *nc);
static void rest_get(struct mg_connection *nc, const struct mg_str *key);
static void rest_delete(struct mg_connection *nc, const struct mg_str *key);
static void rest_options(struct mg_connection *nc, const struct mg_str *key);
static void rest_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm);
static void rest_handle_connection_close(struct mg_connection *nc);

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
 * @param ev_data event data
 */
void rest_api_handler(struct mg_connection *nc, void *ev_data) {
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
            "{\"error\":\"Value too large, max. size is %d byte\"}",
            MAX_VALUE_SIZE
        );
        KVD_LOG_ERROR("Value too large.");
        rest_handle_connection_close(nc);
        return;
    }

    // Match uri
    struct mg_str caps[2];
    if (mg_match(hm->uri, mg_str("/kv1/"), NULL)) {
        rest_list(nc);
        rest_handle_connection_close(nc);
        return;
    }
    if (mg_match(hm->uri, mg_str("/kv1/#"), caps)) {
        if (caps[0].len == 0) {
            mg_http_reply(nc, BAD_REQUEST, response_headers, "{\"error\":\"Key could not be empty.\"}");
            rest_handle_connection_close(nc);
            return;
        }
        if (caps[0].len > MAX_KEY_SIZE) {
            mg_http_reply(nc, BAD_REQUEST, response_headers, "{\"error\":\"Key is too long.\"}");
            rest_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("GET")) == 0) {
            rest_get(nc, &caps[0]);
            rest_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("DELETE")) == 0) {
            rest_delete(nc, &caps[0]);
            rest_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("OPTIONS")) == 0) {
            rest_options(nc, &caps[0]);
            rest_handle_connection_close(nc);
            return;
        }
        if (mg_strcmp(hm->method, mg_str("PUT")) == 0 ||
            mg_strcmp(hm->method, mg_str("POST")) == 0)
        {
            rest_put(nc, &caps[0], hm);
            rest_handle_connection_close(nc);
            return;
        }
    }
    // Other methods are not implemented
    // Request was not handled
    mg_http_reply(nc, NOT_IMPLEMENTED, response_headers, "{\"error\":\"Not implemented\"}");
    rest_handle_connection_close(nc);
}

// Private functions

static void rest_list(struct mg_connection *nc) {
    raxIterator iter;
    raxStart(&iter, global_data->kvd_store);
    raxSeek(&iter, "^", NULL, 0);
    size_t i = 0;

    mg_printf(nc, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
    mg_http_printf_chunk(nc, "{\"result\":{");
    while (raxNext(&iter)) {
        const struct t_kvd_data *data = (struct t_kvd_data *)iter.data;
        mg_http_printf_chunk(nc, "%s%m: {"
                "%m: %m,"
                "%m: %lld,"
                "%m: %lld,"
                "%m: %lld"
            "}",
            (i == 0 ? "" : "," ),
            mg_print_esc, (int)iter.key_len, iter.key,
            MG_ESC("content-type"), mg_print_esc, (int)data->content_type.len, data->content_type.buf,
            MG_ESC("created"), (long long)data->created,
            MG_ESC("modified"), (long long)data->modified,
            MG_ESC("size"), (long long)data->value.len
        );
        i++;
    }
    raxStop(&iter);
    mg_http_printf_chunk(nc, "}}");
    mg_http_printf_chunk(nc, "");
}

static void rest_get(struct mg_connection *nc, const struct mg_str *key) {
    struct t_kvd_data *data = kvd_store_get(global_data->kvd_store, key, false);
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
    kvd_store_delete(global_data->kvd_store, key);
    mg_http_reply(nc, HTTP_OK, response_headers, "{\"result\":\"OK\"}");
}

static void rest_options(struct mg_connection *nc, const struct mg_str *key) {
    struct t_kvd_data *data = kvd_store_get(global_data->kvd_store, key, false);
    if (data == NULL) {
        mg_http_reply(nc, HTTP_NOT_FOUND, response_headers, "{\"error\":\"Key not found\"}");
        return;
    }
    mg_http_reply(nc, HTTP_OK, response_headers,
        "{%m:{"
            "%m: %m,"
            "%m: %lld,"
            "%m: %lld,"
            "%m: %lld"
        "}}",
        MG_ESC("result"),
        MG_ESC("content-type"), mg_print_esc, (int)data->content_type.len, data->content_type.buf,
        MG_ESC("created"), (long long)data->created,
        MG_ESC("modified"), (long long)data->modified,
        MG_ESC("size"), (long long)data->value.len
    );
}

static void rest_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm) {
    if (global_data->kvd_store->numele > MAX_KEYS) {
        mg_http_reply(nc, INSUFFICIENT_STORAGE, response_headers, "{\"error\":\"Too many keys.\"}");
        KVD_LOG_ERROR("Too many keys");
        return;
    }

    const struct mg_str *content_type = mg_http_get_header(hm, "content-type");
    if (content_type == NULL) {
        mg_http_reply(nc, BAD_REQUEST, response_headers, "{\"error\":\"Content-Type header not found\"}");
        KVD_LOG_ERROR("Content-Type header not found");
        return;
    }

    enum kvd_result result = kvd_store_put(global_data->kvd_store, key, &hm->body, content_type);
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
