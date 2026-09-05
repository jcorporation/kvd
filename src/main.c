/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2025 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

#include "compile_time.h"
#include "dist/mongoose/mongoose.h"
#include "src/kvd_store.h"
#include "src/lib/config.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"
#include "src/lib/mg_user_data.h"
#include "src/lib/options.h"
#include "src/server.h"

#include <errno.h>

/**
 * The main function.
 * @param argc number of command line arguments
 * @param argv char array of the command line arguments
 * @return 0 on success
 */
int main(int argc, char **argv) {
    //set logging states
    log_on_tty = isatty(fileno(stdout));
    #ifdef KVD_DEBUG
        set_loglevel(LOG_DEBUG);
        KVD_LOG_NOTICE("Main: Debug build is running");
    #else
        set_loglevel(LOG_NOTICE);
    #endif

    //central data structures
    rax *kvd_store = kvd_store_init();
    struct t_mg_user_data *mg_user_data = NULL;
    struct mg_mgr *mgr = NULL;
    int rc = EXIT_FAILURE;
    struct t_config *config = kvd_config_new();

    //command line option
    enum handle_options_rc options_rc = handle_options(config, argc, argv);
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
    if (chdir(config->workdir) != 0) {
        KVD_LOG_ERROR("Main: Can not change directory to \"%s\"", config->workdir);
        KVD_LOG_ERRNO(errno);
        goto cleanup;
    }

    KVD_LOG_NOTICE("Main: Starting KVD %s", KVD_VERSION);
    KVD_LOG_INFO("Main: Mongoose %s", MG_VERSION);

    // Init HTTP server
    mg_user_data = mg_user_data_new(config);
    mg_user_data->kvd_store = kvd_store;
    mgr = malloc_assert(sizeof(struct mg_mgr));
    if (mongoose_init(mgr, config, mg_user_data) == false) {
        goto cleanup;
    }

    //Run the udpserver
    mongoose_loop(mgr);
    rc = EXIT_SUCCESS;

    //Try to cleanup all
    cleanup:

    kvd_config_free(config);
    kvd_store_free(kvd_store);

    if (mgr != NULL) {
        mongoose_free(mgr);
    }
    if (mg_user_data != NULL) {
        mg_user_data_free(mg_user_data);
    }
    if (rc == EXIT_SUCCESS) {
        printf("Main: Exiting gracefully\n");
    }
    else {
        printf("Main: Exiting erroneous\n");
    }

    return rc;
}
