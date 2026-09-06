/*
 SPDX-License-Identifier: GPL-3.0-or-later
 (c) 2026 Juergen Mang <mail@jcgames.de>
 https://github.com/jcorporation/kvd
*/

/*! \file
 * \brief File related functions
 */

#include "src/lib/filehandler.h"

#include "src/lib/log.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Opens a temporary file for write using mkstemp
 * @param filepath filepath to open, e.g. /tmp/test.XXXXXX
 *                 XXXXXX is replaced with a random string
 * @return FILE pointer
 */
FILE *open_tmp_file(char *filepath) {
    errno = 0;
    int fd = mkstemp(filepath);
    if (fd < 0) {
        KVD_LOG_ERROR("Can not open tmp file descriptor \"%s\" for write", filepath);
        KVD_LOG_ERRNO(errno);
        return NULL;
    }
    errno = 0;
    FILE *fp = fdopen(fd, "w");
    if (fp == NULL) {
        KVD_LOG_ERROR("Can not open tmp file \"%s\" for write", filepath);
        KVD_LOG_ERRNO(errno);
        close(fd);
    }
    return fp;
}

/**
 * Checks if a path is a regular file
 * @param file_name file path to check
 * @return true if it is a regular file, else false
 */
bool is_file(const char *file_name) {
    struct stat status;
    errno = 0;
    if (lstat(file_name, &status) != 0) {
        KVD_LOG_ERROR("Error getting status for \"%s\"", file_name);
        KVD_LOG_ERRNO(errno);
        return false;
    }
    return S_ISREG(status.st_mode);
}

/**
 * Renames a file. src and dst must be in the same filesystem.
 * @param src source filename
 * @param dst destination filename
 * @return true on success, else false
 */
bool rename_file(const char *src, const char *dst) {
    if (is_file(src) == false) {
        return false;
    }
    if (rename(src, dst) == -1) {
        KVD_LOG_ERROR("Rename file from \"%s\" to \"%s\" failed", src, dst);
        KVD_LOG_ERRNO(errno);
        return false;
    }
    return true;
}

/**
 * Removes a file and reports all errors
 * @param filepath filepath to remove
 * @return true on success else false
 */
bool rm_file(const char *filepath) {
    if (is_file(filepath) == false) {
        return false;
    }
    errno = 0;
    if (unlink(filepath) != 0) {
        KVD_LOG_ERROR("Error removing file \"%s\"", filepath);
        KVD_LOG_ERRNO(errno);
        return false;
    }
    return true;
}

/**
 * Returns the modification time of a file
 * @param filepath filepath
 * @return time_t modification time
 */
time_t get_mtime(const char *filepath) {
    // Verify with lstat() to prevent TOCTTOU attacks and symlink following
    struct stat sb;
    errno = 0;
    if (lstat(filepath, &sb) != 0) {
        // File disappeared or is inaccessible, skip it
        KVD_LOG_ERROR("Error getting mtime for \"%s\", file not accessible", filepath);
        KVD_LOG_ERRNO(errno);
        return 0;
    }
    if (S_ISREG(sb.st_mode) == false) {
        // File type changed or is a symlink/directory, skip it
        KVD_LOG_ERROR("Error getting mtime for \"%s\", file is not a regular file", filepath);
        KVD_LOG_ERRNO(errno);
        return 0;
    }
    return sb.st_mtime;
}
