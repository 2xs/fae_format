/*******************************************************************************/
/*  © Université de Lille, The Pip Development Team (2015-2025)                */
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

/**
 * @file stdriot.c
 *
 * This file is the counterpart of xipfs definitions, such
 * as exec_ctx_t or syscall_index_t.
 *
 * @warning THIS FILE MUST REMAIN SYNCHRONIZED with xipfs, otherwise crashes and UB are to
 * be expected.
 */

#include <stdarg.h>
#include <errno.h>

#include "crt0_ctx.h"
#include "xipfs_crt0_ctx_data.h"
#include "stdriot.h"

/**
 * @warning The order of the members in the enumeration must
 * remain synchronized with the order of the members of the same
 * enumeration declared in caller site (xipfs.h's one).
 *
 * @brief An enumeration describing the index of functions.
 * @remark Enumeration members are explicitly set to be able to
 * to check easily syscalls IDs.
 * @see xipfs/include/xipfs.h
 */
typedef enum xipfs_syscall_e {
    XIPFS_SYSCALL_EXIT               = 0,
    XIPFS_SYSCALL_VPRINTF            = 1,
    XIPFS_SYSCALL_GET_TEMP           = 2,
    XIPFS_SYSCALL_ISPRINT            = 3,
    XIPFS_SYSCALL_STRTOL             = 4,
    XIPFS_SYSCALL_GET_LED            = 5,
    XIPFS_SYSCALL_SET_LED            = 6,
    XIPFS_SYSCALL_COPY_FILE          = 7,
    XIPFS_SYSCALL_GET_FILE_SIZE      = 8,
    XIPFS_SYSCALL_MEMSET             = 9,
    XIPFS_SYSCALL_MEMCMP             = 10,
    XIPFS_SYSCALL_STRCMP             = 11,
    XIPFS_SYSCALL_STRNCMP            = 12,

    /* VFS */
    XIPFS_SYSCALL_VFS_OPEN           = 13,
    XIPFS_SYSCALL_VFS_CLOSE          = 14,
    XIPFS_SYSCALL_VFS_LSEEK          = 15,
    XIPFS_SYSCALL_VFS_WRITE          = 16,
    XIPFS_SYSCALL_VFS_READ           = 17,
    XIPFS_SYSCALL_VFS_READLINE       = 18,
    XIPFS_SYSCALL_VFS_STAT           = 19,
    XIPFS_SYSCALL_VFS_FSTAT          = 20,
    XIPFS_SYSCALL_VFS_STATVFS        = 21,
    XIPFS_SYSCALL_VFS_FSTATVFS       = 22,
    XIPFS_SYSCALL_VFS_RENAME         = 23,
    XIPFS_SYSCALL_VFS_NORMALIZE_PATH = 24,
    XIPFS_SYSCALL_VFS_FSYNC          = 25,
    XIPFS_SYSCALL_VFS_FCNTL          = 26,
    XIPFS_SYSCALL_VFS_MKDIR          = 27,

    XIPFS_SYSCALL_VSNPRINTF          = 28,

    /* This value must remain the last in the enum declaration */
    XIPFS_SYSCALL_MAX
} xipfs_syscall_t;

typedef int (*xipfs_syscall_exit_t)(int status);
typedef int (*xipfs_syscall_vprintf_t)(const char * restrict format, va_list ap);
typedef int (*xipfs_syscall_vsnprintf_t)(char * restrict str, size_t size,
                                         const char * restrict format, va_list ap);
typedef int (*xipfs_syscall_get_temp_t)(void);
typedef int (*xipfs_syscall_isprint_t)(int character);
typedef long (*xipfs_syscall_strtol_t)(
    const char *str, char **endptr, int base);
typedef int (*xipfs_syscall_get_led_t)(int pos);
typedef int (*xipfs_syscall_set_led_t)(int pos, int val);
typedef ssize_t (*xipfs_syscall_copy_file_t)(
    const char *name, void *buf, size_t nbyte);
typedef int (*xipfs_syscall_get_file_size_t)(
    const char *name, size_t *size);
typedef void *(*xipfs_syscall_memset_t)(void *m, int c, size_t n);
typedef int (*xipfs_syscall_memcmp_t)(const void *s1, const void *s2, size_t n);
typedef int (*xipfs_syscall_strcmp_t)(const char *s1, const char *s2);
typedef int (*xipfs_syscall_strncmp_t)(const char *s1, const char *s2, size_t n);

/* VFS */
typedef int (*xipfs_syscall_vfs_open_t)(const char *name, int flags, mode_t mode);
typedef int (*xipfs_syscall_vfs_close_t)(int fd);
typedef off_t (*xipfs_syscall_vfs_lseek_t)(int fd, off_t off, int whence);
typedef ssize_t (*xipfs_syscall_vfs_write_t)(int fd, const void *src, size_t count);
typedef ssize_t (*xipfs_syscall_vfs_read_t)(int fd, void *dest, size_t count);
typedef ssize_t (*xipfs_syscall_vfs_readline_t)(int fd, char *dest, size_t count);
typedef int (*xipfs_syscall_vfs_stat_t)(const char *restrict path, struct stat *restrict buf);
typedef int (*xipfs_syscall_vfs_fstat_t)(int fd, struct stat *buf);
typedef int (*xipfs_syscall_vfs_statvfs_t)(const char *restrict path, struct statvfs *restrict buf);
typedef int (*xipfs_syscall_vfs_fstatvfs_t)(int fd, struct statvfs *buf);
typedef int (*xipfs_syscall_vfs_rename_t)(const char *from_path, const char *to_path);
typedef int (*xipfs_syscall_vfs_normalize_path_t)(char *buf, const char *path, size_t buflen);
typedef int (*xipfs_syscall_vfs_fsync_t)(int fd);
typedef int (*xipfs_syscall_vfs_fcntl_t)(int fd, int cmd, int arg);
typedef int (*xipfs_syscall_vfs_mkdir_t)(const char *name, mode_t mode);

/*
 * Global variable
 */

/**
 * @internal
 *
 * @brief true if the call is a safe one, false otherwise
 *
 * @see xipfs/src/file.c
 */
static unsigned char is_safe_call;

/**
 * @internal
 *
 * @brief A pointer to the xipfs syscall table
 *
 * @see xipfs/src/file.c
 */
static void **xipfs_syscall_table;

/**
 * @internal
 *
 * @brief a pointer to RIOT's got
 */
static const void *previous_got = NULL;

static const void *current_got = NULL;

static inline void set_r10(const void *ptr) {
    __asm__ volatile ("mov sl, %0" :: "r"(ptr));
}

/**
 * @brief Wrapper that branches to the xipfs_exit(3) function
 *
 * @param status The exit status of the program
 *
 * @see xipfs/src/file.c
 */
static void exit(int status)
{
    xipfs_syscall_exit_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_EXIT];
    set_r10(previous_got);
    func(status);
    /* Should never be reached */
    for(;;) {}
}

int vprintf(const char * restrict format, va_list ap) {
    int res;
    xipfs_syscall_vprintf_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VPRINTF];
    set_r10(previous_got);
    res = func(format, ap);
    set_r10(current_got);

    return res;
}

int printf(const char * format, ...)
{
    int res;
    va_list ap;

    va_start(ap, format);

    res = vprintf(format, ap);

    va_end(ap);

    return res;
}

int vsnprintf(char *restrict str, size_t size, const char *restrict format, va_list ap) {
    int res;
    xipfs_syscall_vsnprintf_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VSNPRINTF];
    set_r10(previous_got);
    res = func(str, size, format, ap);
    set_r10(current_got);

    va_end(ap);

    return res;
}

int snprintf(char *restrict str, size_t size, const char *restrict format, ...) {
    int res;
    va_list ap;

    va_start(ap, format);

    res = vsnprintf(str, size, format, ap);

    va_end(ap);

    return res;
}

int get_temp(void) {
    int res;
    xipfs_syscall_get_temp_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_GET_TEMP];
    set_r10(previous_got);
    res  = func();
    set_r10(current_got);

    return res;
}

int isprint(int character) {
    int res;
    xipfs_syscall_isprint_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_ISPRINT];
    set_r10(previous_got);
    res  = func(character);
    set_r10(current_got);


    return res;
}

long strtol(const char *str, char **endptr, int base) {
    long res;
    xipfs_syscall_strtol_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_STRTOL];
    set_r10(previous_got);
    res  = func(str, endptr, base);
    set_r10(current_got);

    return res;
}

int get_led(int pos) {
    int res;
    xipfs_syscall_get_led_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_GET_LED];
    set_r10(previous_got);
    res  = func(pos);
    set_r10(current_got);

    return res;
}

int set_led(int pos, int val) {
    int res;
    xipfs_syscall_set_led_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_SET_LED];
    set_r10(previous_got);
    res  = func(pos, val);
    set_r10(current_got);


    return res;
}

ssize_t copy_file(const char *name, void *buf, size_t nbyte) {
    ssize_t res;
    xipfs_syscall_copy_file_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_COPY_FILE];
    set_r10(previous_got);
    res  = func(name, buf, nbyte);
    set_r10(current_got);


    return res;
}

int get_file_size(const char *name, size_t *size) {
    int res;
    xipfs_syscall_get_file_size_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_GET_FILE_SIZE];
    set_r10(previous_got);
    res  = func(name, size);
    set_r10(current_got);

    return res;
}

void *memset(void *m, int c, size_t n) {
    void *res;
    xipfs_syscall_memset_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_MEMSET];
    set_r10(previous_got);
    res  = func(m, c, n);
    set_r10(current_got);

    return res;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    int res;
    xipfs_syscall_memcmp_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_MEMCMP];
    set_r10(previous_got);
    res  = func(s1, s2, n);
    set_r10(current_got);

    return res;
}

int strcmp(const char *s1, const char *s2) {
    int res;
    xipfs_syscall_strcmp_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_STRCMP];
    set_r10(previous_got);
    res  = func(s1, s2);
    set_r10(current_got);

    return res;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    int res;
    xipfs_syscall_strncmp_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_STRNCMP];
    set_r10(previous_got);
    res  = func(s1, s2, n);
    set_r10(current_got);

    return res;
}

/* VFS */
int vfs_open(const char *name, int flags, mode_t mode) {
    int res;
    xipfs_syscall_vfs_open_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_OPEN];
    set_r10(previous_got);
    res = func(name, flags, mode);
    set_r10(current_got);

    return res;
}

int open(const char *name, int flags, ...) {
    mode_t mode = 0;

    if ( ((flags & O_CREAT) != 0)
#ifdef O_TMPFILE
        || ((flags & O_TMPFILE) != 0)
#endif
    ) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    return vfs_open(name, flags, mode);
}

int vfs_close(int fd) {
    int res;
    xipfs_syscall_vfs_close_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_CLOSE];
    set_r10(previous_got);
    res = func(fd);
    set_r10(current_got);

    return res;
}

int close(int fd) {
    return vfs_close(fd);
}

off_t vfs_lseek(int fd, off_t off, int whence) {
    off_t res;
    xipfs_syscall_vfs_lseek_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_LSEEK];
    set_r10(previous_got);
    res = func(fd, off, whence);
    set_r10(current_got);

    return res;
}

off_t lseek(int fd, off_t off, int whence) {
    return vfs_lseek(fd, off, whence);
}

ssize_t vfs_write(int fd, const void *src, size_t count) {
    ssize_t res;
    xipfs_syscall_vfs_write_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_WRITE];
    set_r10(previous_got);
    res = func(fd, src, count);
    set_r10(current_got);

    return res;
}

ssize_t write(int fd, const void *src, size_t count) {
    return vfs_write(fd, src, count);
}

ssize_t vfs_read(int fd, void *dest, size_t count) {
    ssize_t res;
    xipfs_syscall_vfs_read_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_READ];
    set_r10(previous_got);
    res = func(fd, dest, count);
    set_r10(current_got);

    return res;
}

ssize_t read(int fd, void *dest, size_t count) {
    return vfs_read(fd, dest, count);
}

ssize_t vfs_readline(int fd, char *dest, size_t count) {
    ssize_t res;
    xipfs_syscall_vfs_readline_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_READLINE];
    set_r10(previous_got);
    res = func(fd, dest, count);
    set_r10(current_got);

    return res;
}

int vfs_stat(const char *restrict path, struct stat *restrict buf) {
    int res;
    xipfs_syscall_vfs_stat_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_STAT];
    set_r10(previous_got);
    res = func(path, buf);
    set_r10(current_got);

    return res;
}

int stat(const char *restrict path, struct stat *restrict buf) {
    return vfs_stat(path, buf);
}

int vfs_fstat(int fd, struct stat *buf) {
    int res;
    xipfs_syscall_vfs_fstat_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_FSTAT];
    set_r10(previous_got);
    res = func(fd, buf);
    set_r10(current_got);

    return res;
}

int fstat(int fd, struct stat *buf) {
    return vfs_fstat(fd, buf);
}

int vfs_statvfs(const char *restrict path, struct statvfs *restrict buf) {
    int res;
    xipfs_syscall_vfs_statvfs_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_STATVFS];
    set_r10(previous_got);
    res = func(path, buf);
    set_r10(current_got);

    return res;
}

int statvfs(const char *restrict path, struct statvfs *restrict buf) {
    return vfs_statvfs(path, buf);
}

int vfs_fstatvfs(int fd, struct statvfs *buf) {
    int res;
    xipfs_syscall_vfs_fstatvfs_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_FSTATVFS];
    set_r10(previous_got);
    res = func(fd, buf);
    set_r10(current_got);

    return res;
}

int fstatvfs(int fd, struct statvfs *buf) {
    return vfs_fstatvfs(fd, buf);
}

int vfs_rename(const char *from_path, const char *to_path) {
    int res;
    xipfs_syscall_vfs_rename_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_RENAME];
    set_r10(previous_got);
    res = func(from_path, to_path);
    set_r10(current_got);

    return res;
}

int rename(const char *from_path, const char *to_path) {
    return vfs_rename(from_path, to_path);
}

int vfs_normalize_path(char *buf, const char *path, size_t buflen) {
    int res;
    xipfs_syscall_vfs_normalize_path_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_NORMALIZE_PATH];
    set_r10(previous_got);
    res = func(buf, path, buflen);
    set_r10(current_got);

    return res;
}

int vfs_fsync(int fd) {
    int res;
    xipfs_syscall_vfs_fsync_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_FSYNC];
    set_r10(previous_got);
    res = func(fd);
    set_r10(current_got);

    return res;
}

int fsync(int fd) {
    return vfs_fsync(fd);
}

int vfs_fcntl(int fd, int cmd, int arg) {
    int res;
    xipfs_syscall_vfs_fcntl_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_FCNTL];
    set_r10(previous_got);
    res = func(fd, cmd, arg);
    set_r10(current_got);

    return res;
}

int fcntl(int fd, int cmd, ...) {
    if (cmd != F_GETFL) {
        return -ENOTSUP;
    }

    return vfs_fcntl(fd, cmd, 0);
}

int vfs_mkdir(const char *name, mode_t mode) {
    int res;
    xipfs_syscall_vfs_mkdir_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VFS_MKDIR];
    set_r10(previous_got);
    res = func(name, mode);
    set_r10(current_got);

    return res;
}

int mkdir(const char *name, mode_t mode) {
    return vfs_mkdir(name, mode);
}

/**
 * @internal
 *
 * @brief The function to which CRT0 branches after the
 * executable has been relocated
 */
int start(crt0_ctx_t *crt0_ctx)
{
    int status, argc;
    char **argv;
    xipfs_crt0_ctx_data_t *xipfs_crt0_ctx_data =
        (xipfs_crt0_ctx_data_t *)crt0_ctx->argv;

    previous_got = xipfs_crt0_ctx_data->former_got;
    current_got  = xipfs_crt0_ctx_data->current_got;

    /* Are we executing a safe exec call ? */
    is_safe_call = xipfs_crt0_ctx_data->is_safe_call;

    /* initialize syscall table pointer */
    xipfs_syscall_table = xipfs_crt0_ctx_data->syscall_table;

    /* initialize the arguments passed to the program */
    argc = xipfs_crt0_ctx_data->argc;
    argv = xipfs_crt0_ctx_data->argv;

    /* branch to the main() function of the program */
    extern int main(int argc, char **argv);
    status = main(argc, argv);

    /* exit the program */
    exit(status);

    /* should never be reached */
    for (;;);
}
