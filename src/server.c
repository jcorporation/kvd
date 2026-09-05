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
#include "src/lib/global_data.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/rest.h"

#include <stdbool.h>

// Private definitions

static void http_ev_handler(struct mg_connection *nc, int ev, void *ev_data);
static void mongoose_log(char ch, void *param);
static void handle_wakeup(struct mg_connection *nc, struct mg_str *data);

// Public functions

/**
 * Initializes the udpserver
 * @param mgr Already allocated Mongoose mgr
 * @param config Pointer to KVD config
 * @return true on success, else false
 */
bool mongoose_init(struct mg_mgr *mgr, struct t_config *config) {
    // Set mongoose loglevel to error
    mg_log_set(1);
    mg_log_set_fn(mongoose_log, NULL);
    // Init mongoose mgr
    mg_mgr_init(mgr);

    // Listener for the REST API
    struct mg_connection *nc_rest = mg_http_listen(mgr, config->listen_uri, http_ev_handler, NULL);
    if (nc_rest == NULL) {
        KVD_LOG_EMERG("Can't bind to %s", config->listen_uri);
        return false;
    }
    KVD_LOG_NOTICE("Listening on %s for REST requests", config->listen_uri);

    // Initialize the wakeup scheme
    if (mg_wakeup_init(mgr) == false) {
        KVD_LOG_EMERG("Failure initializing webserver wakeup scheme");
        mongoose_free(mgr);
        return NULL;
    }

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
 * Central mongoose event handler
 * @param nc mongoose connection
 * @param ev connection event
 * @param ev_data event data
 */
static void http_ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
    switch(ev) {
        case MG_EV_WAKEUP: {
            struct mg_str *data = (struct mg_str *) ev_data;
            handle_wakeup(nc, data);
            break;
        }
        case MG_EV_OPEN: {
            nc->data[0] = '-'; // connection header
            if (nc->is_listening) {
                KVD_LOG_DEBUG("Main listening connection opened");
                global_data->listening_id = nc->id;
            }
            break;
        }
        case MG_EV_HTTP_MSG: {
            rest_api_handler(nc, ev_data);
            break;
        }
        default: {
            // Ignore other events
            break;
        }
    }
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

/**
 * Handles the data from the MG_EV_WAKEUP event
 * @param nc Mongoose connection
 * @param data Received data
 */
static void handle_wakeup(struct mg_connection *nc, struct mg_str *data) {
    (void) nc;
    switch(data->buf[0]) {
        case 'X':
            // Main loop ends
            KVD_LOG_DEBUG("Wakeup mongoose");
            break;
        case 'W':
            // Persist data to disk
            KVD_LOG_DEBUG("SIGHUP received");
            break;
        default:
            KVD_LOG_ERROR("Unhandled wakeup data received");
    }
}
