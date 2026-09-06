/*
 SPDX-License-Identifier: GPL-3.0-or-later
 myMPD (c) 2018-2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/mympd
*/

/*! \file
 * \brief Helper functions for mpack
 */

#ifndef KVD_MPACK_H
#define KVD_MPACK_H

#include "dist/mpack/mpack.h"

void log_mpack_node_error(mpack_tree_t *tree, mpack_error_t error);
void log_mpack_write_error(mpack_writer_t *writer, mpack_error_t error);

#endif
