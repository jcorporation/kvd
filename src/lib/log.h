/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

#ifndef KVD_LOG_H
#define KVD_LOG_H

#include <stdbool.h>
#include <syslog.h>

/**
 * Macros for logging
 */
#define KVD_LOG_EMERG(...) kvd_log(LOG_EMERG, __VA_ARGS__)
#define KVD_LOG_ALERT(...) kvd_log(LOG_ALERT, __VA_ARGS__)
#define KVD_LOG_CRIT(...) kvd_log(LOG_CRIT, __VA_ARGS__)
#define KVD_LOG_ERROR(...) kvd_log(LOG_ERR, __VA_ARGS__)
#define KVD_LOG_WARN(...) kvd_log(LOG_WARNING, __VA_ARGS__)
#define KVD_LOG_NOTICE(...) kvd_log(LOG_NOTICE, __VA_ARGS__)
#define KVD_LOG_INFO(...) kvd_log(LOG_INFO, __VA_ARGS__)
#define KVD_LOG_DEBUG(...) kvd_log(LOG_DEBUG, __VA_ARGS__)
#define KVD_LOG_ERRNO(ERRNUM) kvd_log_errno(ERRNUM)

/**
 * Global log variables
 */
extern _Atomic int loglevel;
extern bool log_on_tty;

const char *get_loglevel_name(int level);
void set_loglevel(int level);

void kvd_log_errno(int errnum);
void kvd_log(int level, const char *fmt, ...)
    __attribute__ ((format (printf, 2, 3)));

#endif
