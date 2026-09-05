/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief Custom mongoose data structure
 */

#include "src/lib/mg_user_data.h"

#include "src/lib/mem.h"

#include <string.h>

/**
 * Mallocs and initializes the t_mg_user_data struct
 * @param config Pointer to static config
 * @return struct t_mg_user_data*
 */
struct t_mg_user_data *mg_user_data_new(struct t_config *config) {
    struct t_mg_user_data *mg_user_data = malloc_assert(sizeof(struct t_mg_user_data));
    mg_user_data->config = config;
    mg_user_data->kvd_store = NULL;
    return mg_user_data;
}

/**
 * Frees the members of mg_user_data struct and the struct itself
 * @param mg_user_data pointer to mg_user_data struct
 */
void mg_user_data_free(struct t_mg_user_data *mg_user_data) {
    FREE_PTR(mg_user_data);
}
