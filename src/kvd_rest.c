/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief REST API for kvd
 */

#include "compile_time.h"

#include "src/kvd_rest.h"

#include "kvd_store.h"
#include "src/kvd_store.h"
#include "src/lib/global_data.h"
#include "src/lib/http.h"
#include "src/lib/log.h"

void request_kvd_list(struct mg_connection *nc) {
    raxIterator iter;
    raxStart(&iter, global_data->kvd_store.db);
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

void request_kvd_get(struct mg_connection *nc, const struct mg_str *key) {
    struct t_kvd_data *data = kvd_store_get(&global_data->kvd_store, key, false);
    if (data == NULL) {
        mg_http_reply(nc, HTTP_NOT_FOUND, HTTP_HEADER_JSON, "{\"error\":\"Key not found\"}");
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

void request_kvd_delete(struct mg_connection *nc, const struct mg_str *key) {
    kvd_store_delete(&global_data->kvd_store, key);
    mg_http_reply(nc, HTTP_OK, HTTP_HEADER_JSON, "{\"result\":\"OK\"}");
}

void request_kvd_options(struct mg_connection *nc, const struct mg_str *key) {
    struct t_kvd_data *data = kvd_store_get(&global_data->kvd_store, key, false);
    if (data == NULL) {
        mg_http_reply(nc, HTTP_NOT_FOUND, HTTP_HEADER_JSON, "{\"error\":\"Key not found\"}");
        return;
    }
    mg_http_reply(nc, HTTP_OK, HTTP_HEADER_JSON,
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

void request_kvd_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm) {
    if (global_data->kvd_store.db->numele > MAX_KEYS) {
        mg_http_reply(nc, HTTP_INSUFFICIENT_STORAGE, HTTP_HEADER_JSON, "{\"error\":\"Too many keys.\"}");
        KVD_LOG_ERROR("Too many keys");
        return;
    }

    const struct mg_str *content_type = mg_http_get_header(hm, "content-type");
    if (content_type == NULL) {
        mg_http_reply(nc, HTTP_BAD_REQUEST, HTTP_HEADER_JSON, "{\"error\":\"Content-Type header not found\"}");
        KVD_LOG_ERROR("Content-Type header not found");
        return;
    }

    enum kvd_result result = kvd_store_put(&global_data->kvd_store, key, &hm->body, content_type);
    mg_http_reply(nc, (int)result, HTTP_HEADER_JSON, "{\"result\":\"OK\"}");
}
