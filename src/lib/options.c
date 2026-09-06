/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Command line options handling
 */

#include "compile_time.h"
#include "src/lib/options.h"

#include "src/lib/config.h"
#include "src/lib/log.h"
#include "src/lib/mem.h"

#include <getopt.h>
#include <stdio.h>
#include <string.h>

/**
 * Options definitions
 */
static struct option long_options[] = {
    {"help",      no_argument,       0, 'h'},
    {"listen",    required_argument, 0, 'l'},
    {"loglevel",  required_argument, 0, 'o'},
    {"version",   no_argument,       0, 'v'},
    {"workdir",   required_argument, 0, 'w'}
};

/**
 * Prints the command line usage information
 * @param config pointer to config struct
 * @param cmd argv[0] from main function
 */
static void print_usage(struct t_config *config, const char *cmd) {
    printf("\nUsage: %s [OPTION]...\n\n"
        "KVD %s\n"
        "(c) 2026 Juergen Mang <mail@jcgames.de>\n"
        "https://github.com/jcorporation/kvd\n\n"
        "Options:\n"
        "  -h, --help              Displays this help\n"
        "  -l, --listen <uri>      Listen URI for REST-API (default: %s)\n"
        "  -o, --loglevel <level>  Syslog loglevel (default: 5 - NOTICE)\n"
        "  -v, --version           Displays this help\n"
        "  -w, --workdir <folder>  Working directory (default: %s)\n\n",
        cmd, KVD_VERSION, config->listen_uri, config->workdir);
}

/**
 * Handles the command line arguments
 * @param config pointer to config struct
 * @param argc from main function
 * @param argv from main function
 * @return OPTIONS_RC_INVALID on error
 *         OPTIONS_RC_EXIT if KVD should exit
 *         OPTIONS_RC_OK if arguments are parsed successfully
 */
enum handle_options_rc handle_options(struct t_config *config, int argc, char **argv) {
    int n = 0;
    int option_index = 0;
    while ((n = getopt_long(argc, argv, "l:o:vhw:", long_options, &option_index)) != -1) {
        switch(n) {
            case 'l':
                FREE_PTR(config->listen_uri);
                config->listen_uri = my_strdup(optarg, strlen(optarg));
                break;
            case 'o':
                set_loglevel((int)strtol(optarg, NULL, 10));
                break;
            case 'v':
            case 'h':
                print_usage(config, argv[0]);
                return OPTIONS_RC_EXIT;
            case 'w':
                FREE_PTR(config->workdir);
                config->workdir = my_strdup(optarg, strlen(optarg));
                break;
            default:
                print_usage(config, argv[0]);
                return OPTIONS_RC_INVALID;
        }
    }
    return OPTIONS_RC_OK;
}
