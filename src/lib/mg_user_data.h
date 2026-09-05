/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Custom mongoose data structure
 */

#ifndef KVD_MG_USER_DATA_H
#define KVD_MG_USER_DATA_H

#include "dist/rax/rax.h"
#include "src/lib/config.h"

/**
 * Struct for mg_mgr userdata
 */
struct t_mg_user_data {
    struct t_config *config;  //!< Pointer to config
    rax *kvd_store;           // The KV store
};

struct t_mg_user_data *mg_user_data_new(struct t_config *config);
void mg_user_data_free(struct t_mg_user_data *mg_user_data);

#endif
