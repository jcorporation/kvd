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

void rest_api_handler(struct mg_connection *nc, void *ev_data);

#endif
