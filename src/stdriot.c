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

#include "crt0_ctx.h"
#include "xipfs_crt0_ctx_data.h"
#include "stdriot.h"


/**
 * @warning The order of the members in the enumeration must
 * remain synchronized with the order of the members of the same
 * enumeration declared in caller site (xipfs.h's one).
 *
 * @brief An enumeration describing the index of functions.
 *
 * @see xipfs/include/xipfs.h
 */
typedef enum xipfs_syscall_e {
    XIPFS_SYSCALL_EXIT,
    XIPFS_SYSCALL_VPRINTF,
    XIPFS_SYSCALL_GET_TEMP,
    XIPFS_SYSCALL_ISPRINT,
    XIPFS_SYSCALL_STRTOL,
    XIPFS_SYSCALL_GET_LED,
    XIPFS_SYSCALL_SET_LED,
    XIPFS_SYSCALL_COPY_FILE,
    XIPFS_SYSCALL_GET_FILE_SIZE,
    XIPFS_SYSCALL_MEMSET,
    XIPFS_SYSCALL_STRLEN,
    XIPFS_SYSCALL_VSNPRINTF,
    XIPFS_SYSCALL_SCRIBE_WRITE,
    XIPFS_SYSCALL_MAX
} xipfs_syscall_t;

typedef int (*xipfs_syscall_exit_t)(int status);
typedef int (*xipfs_syscall_vprintf_t)(const char *format, va_list ap);
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
typedef size_t (*xipfs_syscall_strlen_t)(const char *s);
typedef int (*xipfs_syscall_vsnprintf_t)(char *buffer, size_t buffer_bytesize,
                                         const char *format, va_list va);
typedef scribe_code_t (*xipfs_syscall_scribe_write_t)(const void *data, size_t bytesize);

/**
 * @internal
 *
 * @def XIPFS_SVC_NUMBER
 *
 * The Supervisor Virtual Call number through which SVCs are performed.
 *
 * @warning Must be synchronized with xipfs' one
 *
 * @see xipfs/src/file.c
 */
// #define XIPFS_SVC_NUMBER 3

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
    set_r10(current_got);
}

int printf(const char * format, ...)
{
    int res;
    va_list ap;
    xipfs_syscall_vprintf_t func;

    va_start(ap, format);

    func = xipfs_syscall_table[XIPFS_SYSCALL_VPRINTF];
    set_r10(previous_got);
    res = func(format, ap);
    set_r10(current_got);

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


size_t strlen(const char *s) {
    size_t res;
    xipfs_syscall_strlen_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_STRLEN];
    set_r10(previous_got);
    res  = func(s);
    set_r10(current_got);

    return res;
}

int vsnprintf(char *buffer, size_t buffer_bytesize, const char *format, va_list ap) {
    int res;
    xipfs_syscall_vsnprintf_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_VSNPRINTF];
    set_r10(previous_got);
    res  = func(buffer, buffer_bytesize, format, ap);
    set_r10(current_got);

    return res;
}

scribe_code_t scribe_write(const void *data, size_t bytesize) {
    scribe_code_t res;
    xipfs_syscall_scribe_write_t func;

    func = xipfs_syscall_table[XIPFS_SYSCALL_SCRIBE_WRITE];
    set_r10(previous_got);
    res  = func(data, bytesize);
    set_r10(current_got);

    return res;
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
