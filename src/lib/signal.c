/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Signal handling
 */

#include "src/lib/signal.h"

#include "dist/mongoose/mongoose.h"
#include "src/lib/global_data.h"
#include "src/lib/log.h"

#include <string.h>

// Global variables
sig_atomic_t s_signal_received;  //!< Signal received indicator

// Private definitions
static void signal_handler(int signo);

// Public functions

/**
 * Installs a minimal signal handler for SIGTERM, SIGINT, and SIGHUP.
 * @return true on success, else false
 */
bool signal_init(void) {
    // Install signal handlers for SIGTERM, SIGINT, and SIGHUP
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler

    // Ignore SIGPIPE to prevent termination when writing to closed sockets
    struct sigaction sa_ignore;
    memset(&sa_ignore, 0, sizeof(sa_ignore));
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = SA_RESTART; // Restart functions if interrupted by handler

    if (sigaction(SIGTERM, &sa, NULL) == -1 ||
        sigaction(SIGINT, &sa, NULL) == -1 ||
        sigaction(SIGHUP, &sa, NULL) == -1 ||
        sigaction(SIGPIPE, &sa_ignore, NULL) == -1)
    {
        KVD_LOG_ERROR("Installing signal handler failed");
        return false;
    }

    return true;
}

// Private functions

/**
 * Minimal signal handler that wke ups the mongoose loop
 * @param signo signal number
 */
static void signal_handler(int signo) {
    switch(signo) {
        case SIGTERM:
        case SIGINT:
            s_signal_received = 1;
            mg_wakeup(global_data->mg_mgr, global_data->listening_id, "X", 1);
            break;
        case SIGHUP:
            mg_wakeup(global_data->mg_mgr, global_data->listening_id, "W", 1);
            break;
        default:
            // Ignore
            break;
    }
}
