/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief KV store implementation
 */

#include "compile_time.h"

#include "src/kvd_store.h"

#include "dist/mpack/mpack.h"
#include "src/lib/filehandler.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/lib/mpack.h"

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

struct t_kvd_data *kvd_store_get(rax *kvd_store, const struct mg_str *key, bool silent) {
    void *data;
    if (raxFind(kvd_store, (unsigned char *)key->buf, key->len, &data) == 1) {
        // Key was found
        if (silent == false) {
            KVD_LOG_DEBUG("Key \"%.*s\" was found.", (int)key->len, key->buf);
        }
        return (struct t_kvd_data *)data;
    }
    if (silent == false) {
        KVD_LOG_WARN("Key \"%.*s\" not found.", (int)key->len, key->buf);
    }
    return NULL;
}

void kvd_store_delete(rax *kvd_store, const struct mg_str *key) {
    void *data;
    if (raxRemove(kvd_store, (unsigned char *)key->buf, key->len, &data) == 1) {
        kvd_data_free((struct t_kvd_data *)data);
        KVD_LOG_DEBUG("Key \"%.*s\" deleted.", (int)key->len, key->buf);
        return;
    }
    KVD_LOG_WARN("Key \"%.*s\" not found for deletion.", (int)key->len, key->buf);
}

enum kvd_result kvd_store_put(rax *kvd_store, const struct mg_str *key, const struct mg_str *value, const struct mg_str *content_type) {
    struct t_kvd_data *data = kvd_store_get(kvd_store, key, true);
    if (data == NULL) {
        // Create
        KVD_LOG_DEBUG("Inserting key \"%.*s\"", (int)key->len, key->buf);
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
    KVD_LOG_DEBUG("Updating key \"%.*s\"", (int)key->len, key->buf);
    data->modified = time(NULL);
    free(data->value.buf);
    data->value.len = value->len;
    data->value.buf = my_memcpy(value->buf, value->len);
    return KVD_UPDATED;
}

bool kvd_store_read(rax *kvd_store) {
    if (is_file(KVD_STORE_FILENAME) == false) {
        return false;
    }

    mpack_tree_t tree;
    mpack_tree_init_filename(&tree, KVD_STORE_FILENAME, 0);
    mpack_tree_set_error_handler(&tree, log_mpack_node_error);
    mpack_tree_parse(&tree);
    mpack_node_t root = mpack_tree_root(&tree);

    // Check for expected disc format version
    int kvd_store_version = mpack_node_int(mpack_node_map_cstr(root, "version"));
    if (kvd_store_version != KVD_STORE_VERSION) {
        mpack_tree_destroy(&tree);
        KVD_LOG_WARN("Unexpected kvd store version");
        rm_file(KVD_STORE_FILENAME);
        return false;
    }

    // Read keys
    mpack_node_t keys_node = mpack_node_map_cstr(root, "keys");
    size_t len = mpack_node_array_length(keys_node);
    for (size_t i = 0; i < len; i++) {
        mpack_node_t node = mpack_node_array_at(keys_node, i);
        struct t_kvd_data *data = malloc_assert(sizeof(struct t_kvd_data));
        char *key = mpack_node_cstr_alloc(mpack_node_map_cstr(node, "key"), MAX_KEY_SIZE);

        mpack_node_t value_node = mpack_node_map_cstr(node, "value");
        data->value.buf = mpack_node_data_alloc(value_node, MAX_VALUE_SIZE);
        data->value.len = mpack_node_data_len(value_node);

        mpack_node_t content_type_node = mpack_node_map_cstr(node, "content_type");
        data->content_type.buf = mpack_node_data_alloc(content_type_node, MAX_VALUE_SIZE);
        data->content_type.len = mpack_node_data_len(content_type_node);

        data->created = (time_t)mpack_node_i64(mpack_node_map_cstr(node, "created"));
        data->modified = (time_t)mpack_node_i64(mpack_node_map_cstr(node, "modified"));

        if (raxTryInsert(kvd_store, (unsigned char *)key, strlen(key), data, NULL) == 0) {
            KVD_LOG_ERROR("Duplicate key in kvd store file found: %s", key);
            kvd_data_free(data);
        }
    }

    // clean up and check for errors
    bool rc = mpack_tree_destroy(&tree) != mpack_ok
        ? false
        : true;
    if (rc == false) {
        KVD_LOG_ERROR("Reading kvd store from disc failed");
        rm_file(KVD_STORE_FILENAME);
    }
    else {
        KVD_LOG_INFO("Read %" PRIu64 " key(s) from disc", kvd_store->numele);
    }
    return rc;
}

bool kvd_store_persist(rax *kvd_store) {
    KVD_LOG_INFO("Persisting kvd store");
    // Open tmp file
    char filepath[] = "kvd_store.mpackXXXXXX";
    FILE *fp = open_tmp_file(filepath);
    if (fp == NULL) {
        return false;
    }

    // Init mpack
    mpack_writer_t writer;
    mpack_writer_init_stdfile(&writer, fp, true);
    mpack_writer_set_error_handler(&writer, log_mpack_write_error);
    mpack_build_map(&writer);
    mpack_write_kv(&writer, "version", KVD_STORE_VERSION);
    mpack_write_cstr(&writer, "keys");
    mpack_start_array(&writer, (uint32_t)kvd_store->numele);
    raxIterator iter;
    raxStart(&iter, kvd_store);
    raxSeek(&iter, "^", NULL, 0);
    while (raxNext(&iter)) {
        const struct t_kvd_data *data = (struct t_kvd_data *)iter.data;
        mpack_build_map(&writer);
        mpack_write_cstr(&writer, "key");
        mpack_write_str(&writer, (char *)iter.key, (unsigned)iter.key_len);
        mpack_write_cstr(&writer, "value");
        mpack_write_str(&writer, data->value.buf, (unsigned)data->value.len);
        mpack_write_cstr(&writer, "content_type");
        mpack_write_str(&writer, data->content_type.buf, (unsigned)data->content_type.len);
        mpack_write_cstr(&writer, "created");
        mpack_write_i64(&writer, (int64_t)data->created);
        mpack_write_cstr(&writer, "modified");
        mpack_write_i64(&writer,(int64_t)data->modified);
        mpack_complete_map(&writer);
    }
    raxStop(&iter);
    mpack_finish_array(&writer);
    mpack_complete_map(&writer);
    // finish writing
    bool rc = mpack_writer_destroy(&writer) != mpack_ok
        ? false
        : true;
    if (rc == false) {
        errno = 0;
        rm_file(filepath);
        KVD_LOG_ERROR("An error occurred encoding the data");
        return false;
    }
    // rename tmp file
    errno = 0;
    if (rename(filepath, KVD_STORE_FILENAME) == -1) {
        KVD_LOG_ERROR("Rename file from \"%s\" to \"%s\" failed", filepath, KVD_STORE_FILENAME);
        KVD_LOG_ERRNO(errno);
        rm_file(filepath);
        rc = false;
    }
    return rc;
}

// Private functions

static void kvd_data_free(struct t_kvd_data *data) {
    free(data->value.buf);
    free(data->content_type.buf);
    free(data);
}
