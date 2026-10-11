/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief REST API implementation
 */

#ifndef KVD_REST_H
#define KVD_REST_H

#include "dist/mongoose/mongoose.h"

enum {
    HTTP_OK = 200,
    HTTP_CREATED = 201,
    HTTP_BAD_REQUEST = 400,
    HTTP_NOT_FOUND = 404,
    HTTP_CONTENT_TOO_LARGE = 413,
    HTTP_NOT_IMPLEMENTED = 501,
    HTTP_BAD_GATEWAY = 502,
    HTTP_INSUFFICIENT_STORAGE = 507,
};

#define HTTP_HEADER_JSON "Content-Type: application/json\r\n"
#define HTTP_HEADER_PLAIN "Content-Type: text/plain\r\n"

void rest_api_handler(struct mg_connection *nc, void *ev_data);
void rest_handle_connection_close(struct mg_connection *nc);

#endif
