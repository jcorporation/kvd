/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief Command line options handling
 */

#ifndef KVD_SIGNAL_H
#define KVD_SIGNAL_H

#include <signal.h>
#include <stdbool.h>

extern sig_atomic_t s_signal_received;

bool signal_init(void);

#endif
