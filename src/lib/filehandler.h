/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

#ifndef KVD_FILE_H
#define KVD_FILE_H

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

FILE *open_tmp_file(char *filepath);
bool is_file(const char *file_name);
bool rename_file(const char *src, const char *dst);
bool rm_file(const char *filepath);
time_t get_mtime(const char *filepath);

#endif
