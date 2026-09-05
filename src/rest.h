/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief REST API implementation
 */

#ifndef KVD_REST_H
#define KVD_REST_H

#include "dist/mongoose/mongoose.h"

void http_ev_handler(struct mg_connection *nc, int ev, void *ev_data);

#endif
