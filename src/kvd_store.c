/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief KV store implementation
 */

#include "src/kvd_store.h"

#include "src/lib/mem.h"

// Private definitions

static void kvd_data_free(struct t_kvd_data *data);

// Public functions

rax *kvd_store_init(void) {
    return raxNew();
}

/**
 * Frees the kvd_store and its data
 * @param r rax tree to free
 */
void kvd_store_free(rax *rt) {
    raxIterator iter;
    raxStart(&iter, rt);
    raxSeek(&iter, "^", NULL, 0);
    while (raxNext(&iter)) {
        kvd_data_free((struct t_kvd_data *)iter.data);
        iter.data = NULL;
    }
    raxStop(&iter);
    raxFree(rt);
}

struct t_kvd_data *kvd_store_get(rax *kvd_store, const struct mg_str *key) {
    void *data;
    if (raxFind(kvd_store, (unsigned char *)key->buf, key->len, &data) == 1) {
        // Key was found
        return (struct t_kvd_data *)data;
    }
    return NULL;
}

void kvd_store_delete(rax *kvd_store, const struct mg_str *key) {
    void *data;
    if (raxRemove(kvd_store, (unsigned char *)key->buf, key->len, &data) == 1) {
        kvd_data_free((struct t_kvd_data *)data);
    }
}

enum kvd_result kvd_store_put(rax *kvd_store, const struct mg_str *key, const struct mg_str *value, const struct mg_str *content_type) {
    struct t_kvd_data *data = kvd_store_get(kvd_store, key);
    if (data == NULL) {
        // Create
        data = malloc_assert(sizeof(struct t_kvd_data));
        data->created = time(NULL);
        data->modified = data->created;
        data->value.len = value->len;
        data->value.buf = my_memcpy(value->buf, value->len);
        data->content_type.len = content_type->len;
        data->content_type.buf = my_memcpy(content_type->buf, content_type->len);
        raxInsert(kvd_store, (unsigned char *)key->buf, key->len, data , NULL);
        return KVD_CREATED;
    }

    // Modify
    data->modified = time(NULL);
    free(data->value.buf);
    data->value.buf = my_memcpy(value->buf, value->len);
    return KVD_UPDATED;
}

// Private functions

static void kvd_data_free(struct t_kvd_data *data) {
    free(data->value.buf);
    free(data->content_type.buf);
    free(data);
}
