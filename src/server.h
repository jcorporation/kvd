/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief Mongoose server implementation
 */

#ifndef KVD_HTTPSERVER_H
#define KVD_HTTPSERVER_H

#include "dist/mongoose/mongoose.h"
#include "src/lib/config.h"

#include <stdbool.h>

bool mongoose_init(struct mg_mgr *mgr, struct t_config *config);
void mongoose_free(struct mg_mgr *mgr);

#endif
