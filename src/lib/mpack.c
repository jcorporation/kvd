/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2018-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Helper functions for mpack
 */

#include "src/lib/mpack.h"

#include "src/lib/log.h"

/**
 * Log handler for mpack read errors
 * @param tree mpack tree object (unused)
 * @param error error object
 */
void log_mpack_node_error(mpack_tree_t *tree, mpack_error_t error) {
    (void) tree;
    KVD_LOG_ERROR("mpack error: %s", mpack_error_to_string(error));
}

/**
 * Log handler for mpack write errors
 * @param writer mpack writer object (not used)
 * @param error error object
 */
void log_mpack_write_error(mpack_writer_t *writer, mpack_error_t error) {
    (void) writer;
    KVD_LOG_ERROR("mpack error: %s", mpack_error_to_string(error));
}
