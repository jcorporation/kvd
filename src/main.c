/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

#include "compile_time.h"
#include "dist/mongoose/mongoose.h"
#include "src/kvd_store.h"
#include "src/lib/config.h"
#include "src/lib/global_data.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/lib/options.h"
#include "src/lib/signal.h"
#include "src/server.h"

#include <errno.h>

/**
 * The main function.
 * @param argc number of command line arguments
 * @param argv char array of the command line arguments
 * @return 0 on success
 */
int main(int argc, char **argv) {
    // Set initial states
    log_on_tty = isatty(fileno(stdout));
    #ifdef KVD_DEBUG
        set_loglevel(LOG_DEBUG);
        KVD_LOG_NOTICE("Main: Debug build is running");
    #else
        set_loglevel(LOG_NOTICE);
    #endif

    int rc = EXIT_FAILURE;
    s_signal_received = 0;
    signal_init();

    // Central data structures
    global_data = malloc_assert(sizeof(struct t_global_data));
    global_data->kvd_store = NULL;
    global_data->config = kvd_config_new();
    global_data->mg_mgr = NULL;
    global_data->listening_id = 0;

    //command line option
    enum handle_options_rc options_rc = handle_options(global_data->config, argc, argv);
    switch(options_rc) {
        case OPTIONS_RC_INVALID:
            //invalid option or error
            loglevel = LOG_ERR;
            goto cleanup;
        case OPTIONS_RC_EXIT:
            //valid option and exit
            loglevel = LOG_ERR;
            rc = EXIT_SUCCESS;
            goto cleanup;
        case OPTIONS_RC_OK:
            //continue
            break;
    }

    //set output buffers
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0 ||
        setvbuf(stderr, NULL, _IOLBF, 0) != 0)
    {
        KVD_LOG_EMERG("Main: Could not set stdout and stderr buffer");
        goto cleanup;
    }

    // Go into workdir
    errno = 0;
    if (chdir(global_data->config->workdir) != 0) {
        KVD_LOG_ERROR("Main: Can not change directory to \"%s\"", global_data->config->workdir);
        KVD_LOG_ERRNO(errno);
        goto cleanup;
    }

    KVD_LOG_NOTICE("Starting KVD %s", KVD_VERSION);
    KVD_LOG_INFO("Mongoose %s", MG_VERSION);

    // Init HTTP server
    global_data->mg_mgr = malloc_assert(sizeof(struct mg_mgr));
    if (mongoose_init(global_data->mg_mgr, global_data->config) == false) {
        goto cleanup;
    }

    // Init kvd store
    global_data->kvd_store = kvd_store_init();

    //Run the server
    while (s_signal_received == 0) {
        // Webserver polling
        mg_mgr_poll(global_data->mg_mgr, -1);
    }
    KVD_LOG_INFO("Server: Stopping");
    rc = EXIT_SUCCESS;

    //Try to cleanup all
    cleanup:

    kvd_config_free(global_data->config);
    if (global_data->kvd_store != NULL) {
        kvd_store_free(global_data->kvd_store);
    }
    if (global_data->mg_mgr != NULL) {
        mongoose_free(global_data->mg_mgr);
    }
    if (global_data != NULL) {
        free(global_data);
    }
    if (rc == EXIT_SUCCESS) {
        printf("Main: Exiting gracefully\n");
    }
    else {
        printf("Main: Exiting erroneous\n");
    }

    return rc;
}
