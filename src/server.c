/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kebacc
*/

/*! \file
 * \brief Mongoose server implementation
 */

#include "src/server.h"

#include "dist/mongoose/mongoose.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/lib/mg_user_data.h"
#include "src/rest.h"

#include <stdbool.h>

// Private definitions

static void mongoose_log(char ch, void *param);

// Public functions

/**
 * Initializes the udpserver
 * @param mgr Already allocated Mongoose mgr
 * @param config Pointer to KVD config
 * @param mg_user_data t_mg_user_data to set
 * @return true on success, else false
 */
bool mongoose_init(struct mg_mgr *mgr, struct t_config *config, struct t_mg_user_data *mg_user_data) {
    // Set mongoose loglevel to error
    mg_log_set(1);
    mg_log_set_fn(mongoose_log, NULL);
    // Init mongoose mgr
    mg_mgr_init(mgr);
    mgr->userdata = mg_user_data;

    // Listener for the REST API
    struct mg_connection *nc_rest = mg_http_listen(mgr, config->listen_uri, http_ev_handler, NULL);
    if (nc_rest == NULL) {
        KVD_LOG_EMERG("Server: Can't bind to %s", config->listen_uri);
        return false;
    }
    KVD_LOG_NOTICE("Server: Listening on %s for REST requests", config->listen_uri);
    return true;
}

/**
 * Frees the mongoose mgr
 * @param mgr mongoose mgr to free
 */
void mongoose_free(struct mg_mgr *mgr) {
    mg_mgr_free(mgr);
    FREE_PTR(mgr);
}

/**
 * HTTP Server loop
 * @pram arg_mgr void pointer to mongoose mgr
 */
void mongoose_loop(struct mg_mgr *mgr) {
    //struct t_mg_user_data *mg_user_data = (struct t_mg_user_data *) mgr->userdata;
    //struct t_config *config = mg_user_data->config;
    for (;;) {
        // Webserver polling
        mg_mgr_poll(mgr, -1);
    }
    KVD_LOG_INFO("Server: Stopping");
}

/**
 * Mongoose logging function
 * @param ch character to log
 * @param param
 */
static void mongoose_log(char ch, void *param) {
    static char buf[256];
    static size_t len;
    buf[len++] = ch;
    if (ch == '\n' ||
        len >= sizeof(buf))
    {
        if (ch == '\n') {
            // Remove newline
            len--;
        }
        // We log only mongoose errors
        KVD_LOG_ERROR("%.*s", (int) len, buf); // Send logs
        len = 0;
    }
    (void)param;
}
