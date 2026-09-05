/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Central KVD configuration definitions
 */

#ifndef KVD_CONFIG_H
#define KVD_CONFIG_H

/**
 * Static KVD configuration
 */
struct t_config {
    char *listen_uri;      //!< Listening URI for REST
    char *workdir;         //!< Working directory
};

struct t_config *kvd_config_new(void);
void kvd_config_free(struct t_config *config);

#endif
