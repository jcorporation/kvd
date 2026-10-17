/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief HTTP helper library
 */

#include "src/lib/http.h"

/**
 * Drains the connection if connection is set to close
 * @param nc mongoose connection
 */
void http_handle_connection_close(struct mg_connection *nc) {
    if (nc->data[0] == 'C') {
        nc->is_draining = 1;
    }
    nc->is_resp = 0;
}
