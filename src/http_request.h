/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief HTTP Request event handler
 */

#ifndef KVD_HTTP_REQUEST_H
#define KVD_HTTP_REQUEST_H

#include "dist/mongoose/mongoose.h"

void request_handler(struct mg_connection *nc, void *ev_data);

#endif
