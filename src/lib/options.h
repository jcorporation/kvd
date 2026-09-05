/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Command line options handling
 */

#ifndef KVD_OPTIONS_H
#define KVD_OPTIONS_H

#include "src/lib/config.h"

#include <stdbool.h>

/**
 * Return codes for options handler
 */
enum handle_options_rc {
    OPTIONS_RC_INVALID = -1,
    OPTIONS_RC_OK = 0,
    OPTIONS_RC_EXIT = 1
};

enum handle_options_rc handle_options(struct t_config *config, int argc, char **argv);

#endif
