/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Configuration handling
 */

#include "compile_time.h"
#include "src/lib/config.h"

#include "src/lib/mem.h"

#include <string.h>

/**
 * Mallocs and initializes the config struct
 * @return config pointer to config struct
 */
struct t_config *kvd_config_new(void) {
    struct t_config *config = malloc_assert(sizeof(struct t_config));
    config->listen_uri = strdup(LISTEN_URI);
    config->workdir = strdup(WORKDIR);
    return config;
}

/**
 * Frees the config struct
 * @param config pointer to config struct
 */
void kvd_config_free(struct t_config *config) {
    FREE_PTR(config->listen_uri);
    FREE_PTR(config->workdir);
    FREE_PTR(config);
}
