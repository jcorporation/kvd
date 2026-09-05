/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Log implementation
 */

#include "compile_time.h"
#include "src/lib/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/**
 * Global variables
 */

/**
 * Loglevel
 */
_Atomic int loglevel;

/**
 * Log goes to tty?
 */
bool log_on_tty;

/**
 * Maps loglevels to names
 */
static const char *loglevel_names[8] = {
    [LOG_EMERG] = "EMERG",
    [LOG_ALERT] = "ALERT",
    [LOG_CRIT] = "CRITICAL",
    [LOG_ERR] = "ERROR",
    [LOG_WARNING] = "WARN",
    [LOG_NOTICE] = "NOTICE",
    [LOG_INFO] = "INFO",
    [LOG_DEBUG] = "DEBUG"
};

/**
 * Maps loglevels to terminal colors
 */
static const char *loglevel_colors[8] = {
    [LOG_EMERG] = "\033[0;31m",
    [LOG_ALERT] = "\033[0;31m",
    [LOG_CRIT] = "\033[0;31m",
    [LOG_ERR] = "\033[0;31m",
    [LOG_WARNING] = "\033[0;33m",
    [LOG_NOTICE] = "",
    [LOG_INFO] = "",
    [LOG_DEBUG] = "\033[0;32m"
};

/**
 * Returns the name of the loglevel
 * @param level 
 * @return the loglevel name or empty string if loglevel is invalid
 */
const char *get_loglevel_name(int level) {
    if (level > LOGLEVEL_MAX ||
        level < LOGLEVEL_MIN)
    {
        return "";
    }
    return loglevel_names[level];
}

/**
 * Sets the loglevel
 * @param level loglevel to set
 */
void set_loglevel(int level) {
    if (level > LOGLEVEL_MAX) {
        level = LOG_DEBUG;
    }
    else if (level < LOGLEVEL_MIN) {
        level = LOG_EMERG;
    }
    KVD_LOG_NOTICE("Setting loglevel to %s", loglevel_names[level]);
    loglevel = level;
}

/**
 * Logs the errno string
 * This function should be called by the suitable macro
 * @param errnum errno
 */
void kvd_log_errno(int errnum) {
    if (errnum == 0) {
        //do not log success
        return;
    }
    char err_text[256];
    int rc = strerror_r(errnum, err_text, 256);
    const char *err_str = rc == 0
        ? err_text
        : "Unknown error";
    kvd_log(LOG_ERR, "%s", err_str);
}

/**
 * Logs the fmt string
 * This function should be called by the suitable macro
 * @param level loglevel of the message
 * @param fmt format string to print
 * @param ... arguments for the format string
 */
void kvd_log(int level, const char *fmt, ...) {
    if (level > loglevel) {
        return;
    }

    if (log_on_tty == true) {
        printf("%s", loglevel_colors[level]);
        time_t now = time(NULL);
        struct tm timeinfo;
        if (localtime_r(&now, &timeinfo) != NULL) {
            printf("%02d:%02d:%02d ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
    }
    printf("%-8s ", loglevel_names[level]);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    if (log_on_tty == true) {
        printf("%s", "\033[0m\n");
    }
    else {
        printf("%s", "\n");
    }
}
