/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Custom global data structure
 */

#ifndef KVD_GLOBAL_DATA_H
#define KVD_GLOBAL_DATA_H

#include "dist/mongoose/mongoose.h"
#include "dist/rax/rax.h"
#include "src/lib/config.h"

extern struct t_global_data *global_data;

struct t_global_data {
    struct mg_mgr *mg_mgr;       //!< Mongoose mgr instance
    unsigned long listening_id;  //!< Mongoose listening id for wakeup
    struct t_config *config;     //!< Pointer to config
    rax *kvd_store;              //!< The KV store
    time_t kvd_store_mtime;      //!< Last update time of the KV store
};

struct t_mg_user_data *mg_user_data_new(struct t_config *config);
void mg_user_data_free(struct t_mg_user_data *mg_user_data);

#endif
