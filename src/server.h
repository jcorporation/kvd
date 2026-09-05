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
#include "src/lib/mg_user_data.h"

#include <stdbool.h>

bool mongoose_init(struct mg_mgr *mgr, struct t_config *config, struct t_mg_user_data *mg_user_data);
void mongoose_loop(struct mg_mgr *mgr);
void mongoose_free(struct mg_mgr *mgr);

#endif
