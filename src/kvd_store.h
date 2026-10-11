/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief KV store implementation
 */

#ifndef KVD_STORE_H
#define KVD_STORE_H

#include "dist/mongoose/mongoose.h"
#include "dist/rax/rax.h"

#include <time.h>

/**
 * Key/value store
 */
struct t_kvd_store {
    rax *db;       //!< Radix tree for the data
    time_t mtime;  //!< Last update time
};

/**
 * KVD data struct
 */
struct t_kvd_data {
    struct mg_str value;         //!< Value
    struct mg_str content_type;  //!< Content-Type
    time_t created;              //!< Unix timestamp of creation time
    time_t modified;             //!< Unix timestamp of last modified time
};

enum kvd_result {
    KVD_UPDATED = 200,      //!< OK
    KVD_CREATED = 201,      //!< Created
    KVD_NOT_MODIFIED = 304  //!< Not Modified
};

void kvd_store_init(struct t_kvd_store *kvd_store);
void kvd_store_free(struct t_kvd_store *kvd_store);

struct t_kvd_data *kvd_store_get(struct t_kvd_store *kvd_store, const struct mg_str *key, bool silent);
void kvd_store_delete(struct t_kvd_store *kvd_store, const struct mg_str *key);
enum kvd_result kvd_store_put(struct t_kvd_store *kvd_store, const struct mg_str *key, const struct mg_str *value, const struct mg_str *content_type);

bool kvd_store_read(struct t_kvd_store *kvd_store);
bool kvd_store_persist(struct t_kvd_store *kvd_store);

#endif
