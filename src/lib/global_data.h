/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Custom global data structure
 */

#ifndef KVD_LIB_GLOBAL_DATA_H
#define KVD_LIB_GLOBAL_DATA_H

#include "dist/mongoose/mongoose.h"
#include "src/kvd_store.h"
#include "src/lib/config.h"

extern struct t_global_data *global_data;

/**
 * Global data structure
 */
struct t_global_data {
    struct mg_mgr *mg_mgr;         //!< Mongoose mgr instance
    unsigned long listening_id;    //!< Mongoose listening id for wakeup
    struct t_config *config;       //!< Pointer to config
    struct t_kvd_store kvd_store;  //!< Key/value store struct
};

#endif
