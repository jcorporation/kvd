/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Send data to rrdcached
 */

#ifndef KVD_RRDCACHED_H
#define KVD_RRDCACHED_H

#include "dist/mongoose/mongoose.h"

/**
 * Struct for mongoose connection data for RRD Cached
 */
struct t_rrdcached_data {
    struct mg_str data;             //!< Data to send to RRD Cached
    struct mg_connection *rest_nc;  //!< REST-API Connection to send response
    int32_t lines_expected;        //!< Number of expected lines
    int32_t lines_received;        //!< Number of lines read
};

struct t_rrdcached_data *rrdcached_data_new(struct mg_connection *nc, struct mg_str *value);
void rrdcached_data_free(struct t_rrdcached_data *data);
bool send_data_to_rrdcached(struct mg_mgr *mgr, struct t_rrdcached_data *data);

#endif
