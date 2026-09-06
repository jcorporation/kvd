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

struct t_kvd_data {
    struct mg_str value;
    struct mg_str content_type;
    time_t created;
    time_t modified;
};

enum kvd_result {
    KVD_UPDATED = 200,
    KVD_CREATED = 201,
};

rax *kvd_store_init(void);
void kvd_store_free(rax *rt);

struct t_kvd_data *kvd_store_get(rax *kvd_store, const struct mg_str *key, bool silent);
void kvd_store_delete(rax *kvd_store, const struct mg_str *key);
enum kvd_result kvd_store_put(rax *kvd_store, const struct mg_str *key, const struct mg_str *value, const struct mg_str *content_type);

bool kvd_store_read(rax *kvd_store);
bool kvd_store_persist(rax *kvd_store);

#endif
