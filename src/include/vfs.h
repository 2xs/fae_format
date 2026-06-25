/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2026)                */
/*                                                                             */
/*  This software is a computer program whose purpose is to run a minimal,     */
/*  hypervisor relying on proven properties such as memory isolation.          */
/*                                                                             */
/*  This software is governed by the CeCILL license under French law and       */
/*  abiding by the rules of distribution of free software.  You can  use,      */
/*  modify and/ or redistribute the software under the terms of the CeCILL     */
/*  license as circulated by CEA, CNRS and INRIA at the following URL          */
/*  "http://www.cecill.info".                                                  */
/*                                                                             */
/*  As a counterpart to the access to the source code and  rights to copy,     */
/*  modify and redistribute granted by the license, users are provided only    */
/*  with a limited warranty  and the software's author,  the holder of the     */
/*  economic rights,  and the successive licensors  have only  limited         */
/*  liability.                                                                 */
/*                                                                             */
/*  In this respect, the user's attention is drawn to the risks associated     */
/*  with loading,  using,  modifying and/or developing or reproducing the      */
/*  software by the user in light of its specific status of free software,     */
/*  that may mean  that it is complicated to manipulate,  and  that  also      */
/*  therefore means  that it is reserved for developers  and  experienced      */
/*  professionals having in-depth computer knowledge. Users are therefore      */
/*  encouraged to load and test the software's suitability as regards their    */
/*  requirements in conditions enabling the security of their systems and/or   */
/*  data to be ensured and,  more generally, to use and operate it in the      */
/*  same conditions as regards security.                                       */
/*                                                                             */
/*  The fact that you are presently reading this means that you have had       */
/*  knowledge of the CeCILL license and that you accept its terms.             */
/*******************************************************************************/

#ifndef VFS_H
#define VFS_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h> /* fsblkcnt_t, fsfilcnt_t, off_t */

/**
 * @def XIPFS_MAX_OPEN_DESC
 *
 * @brief The maximum number of opened descriptors
 *
 * @warning This definition MUST be kept synchronized with definitions in either xipfs_config.h or
 * xipfs.h when compiling RIOT
 */
#define XIPFS_MAX_OPEN_DESC (16)

/**
 * On baremetal toolchains, struct statvfs is not defined.
 * Please see https://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/statvfs.h.html
 */
struct statvfs {
    unsigned long f_bsize;   /**< File system block size. */
    unsigned long f_frsize;  /**< Fundamental file system block size. */
    fsblkcnt_t f_blocks;     /**< Total number of blocks on file system in
                                  units of @c f_frsize. */
    fsblkcnt_t f_bfree;      /**< Total number of free blocks. */
    fsblkcnt_t f_bavail;     /**< Number of free blocks available to
                                  non-privileged process. */
    fsfilcnt_t f_files;      /**< Total number of file serial numbers. */
    fsfilcnt_t f_ffree;      /**< Total number of free file serial numbers. */
    fsfilcnt_t f_favail;     /**< Number of file serial numbers available to
                                  non-privileged process. */

    unsigned long f_fsid;    /**< File system ID. */
    unsigned long f_flag;    /**< Bit mask of f_flag values. */
    unsigned long f_namemax; /**< Maximum filename length. */
};

int vfs_open(const char *name, int flags, mode_t mode);
int open(const char *name, int flags, ...);
int vfs_close(int fd);
int close(int fd);

off_t vfs_lseek(int fd, off_t off, int whence);
off_t lseek(int fd, off_t off, int whence);

ssize_t vfs_write(int fd, const void *src, size_t count);
ssize_t write(int fd, const void *src, size_t count);
ssize_t vfs_read(int fd, void *dest, size_t count);
ssize_t read(int fd, void *dest, size_t count);
/*
 * `man readline`.
 * Regular standard c/GNU library's `readline` function returns a
 * malloc'ed string, which must be freed by callers.
 * Because we do not have an allocator right now, only vfs_readline
 * is available.
 */
ssize_t vfs_readline(int fd, char *dest, size_t count);

int vfs_stat(const char *restrict path, struct stat *restrict buf);
int stat(const char *restrict path, struct stat *restrict buf);
int vfs_fstat(int fd, struct stat *buf);
int fstat(int fd, struct stat *buf);
int vfs_statvfs(const char *restrict path, struct statvfs *restrict buf);
int statvfs(const char *restrict path, struct statvfs *restrict buf);
int vfs_fstatvfs(int fd, struct statvfs *buf);
int fstatvfs(int fd, struct statvfs *buf);

int vfs_rename(const char *from_path, const char *to_path);
int rename(const char *from_path, const char *to_path);

/* To the best of our knowledge, there is no path normalization function
 * available in standard c/GNU library */
int vfs_normalize_path(char *buf, const char *path, size_t buflen);

int vfs_fsync(int fd);
int fsync(int fd);

int vfs_fcntl(int fd, int cmd, int arg);
int fcntl(int fd, int cmd, ...);

int vfs_mkdir(const char *name, mode_t mode);
int mkdir(const char *name, mode_t mode);

#ifdef NO

/* Because of vfs_DIR & vfs_dirent_t RIOT special structures */
int vfs_opendir(vfs_DIR *dirp, const char *dirname);
int vfs_readdir(vfs_DIR *dirp, vfs_dirent_t *entry);
int vfs_closedir(vfs_DIR *dirp);
int vfs_dstatvfs(vfs_DIR *dirp, struct statvfs *buf);
bool vfs_iterate_mount_dirs(vfs_DIR *dir);

/* Don't know */
int vfs_mount(vfs_mount_t *mountp);
int vfs_umount(vfs_mount_t *mountp, bool force);
ssize_t vfs_write_iol(int fd, const iolist_t *iolist);

/* Because of mountpoint execution_mutex */
int vfs_format(vfs_mount_t *mountp);
int vfs_rmdir(const char *name);
int vfs_unlink(const char *name);

/* AUTOMOUNT */
int vfs_unmount_by_path(const char *path, bool force);
int vfs_mount_by_path(const char *path);
int vfs_format_by_path(const char *path);

int vfs_bind(int fd, int flags, const vfs_file_ops_t *f_op, void *private_data);
const vfs_file_t *vfs_file_get(int fd);
int vfs_sysop_stat_from_fstat(vfs_mount_t *mountp,
        const char *restrict path,
        struct stat *restrict buf);
#endif

#endif /* VFS_H */
