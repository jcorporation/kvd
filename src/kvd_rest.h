/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief REST API for kvd
 */

#ifndef KVD_KVD_REST_H
#define KVD_KVD_REST_H

#include "dist/mongoose/mongoose.h"

void request_kvd_list(struct mg_connection *nc);
void request_kvd_get(struct mg_connection *nc, const struct mg_str *key);
void request_kvd_delete(struct mg_connection *nc, const struct mg_str *key);
void request_kvd_options(struct mg_connection *nc, const struct mg_str *key);
void request_kvd_put(struct mg_connection *nc, const struct mg_str *key, struct mg_http_message *hm);

#endif
