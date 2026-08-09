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

#ifndef STDRIOT_H
#define STDRIOT_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <stdarg.h>

extern int printf(const char * format, ...);

extern int get_temp(void);

extern int isprint(int character);

extern long strtol(const char *str, char **endptr, int base);

extern int get_led(int pos);

extern int set_led(int pos, int val);

extern ssize_t copy_file(const char *name, void *buf, size_t nbyte);

extern int get_file_size(const char *name, size_t *size);

extern void *memset(void *m, int c, size_t n);

extern size_t strlen(const char *s);

extern int vsnprintf(char *buffer, size_t buffer_bytesize, const char *format, va_list ap);

static inline int snprintf(char *buffer, size_t buffer_size, const char *format, ...) {
    va_list ap;

    va_start(ap, format);

    int res = vsnprintf(buffer, buffer_size, format, ap);

    va_end(ap);

    return res;
}

typedef enum scribe_code_e {
    SCRIBE_CODE_OK = 0,

    SCRIBE_CODE_NOT_INITIALIZED,
    SCRIBE_CODE_ALREADY_INITIALIZED,

    SCRIBE_CODE_NULL_SINKS,
    SCRIBE_CODE_INVALID_SINKS_COUNT,

    SCRIBE_CODE_NULL_SINK,
    SCRIBE_CODE_NULL_SINK_CLASS,
    SCRIBE_CODE_INTERNAL_INVALID_METHODS_OFFSETS,
    SCRIBE_CODE_NULL_SINK_CLASS_METHOD,

    SCRIBE_CODE_INVALID_SINK_STATE,
    SCRIBE_CODE_PREPARATION_FAILURE,

    SCRIBE_CODE_NO_DATA,
    SCRIBE_CODE_INVALID_DATA_BYTESIZE,
    SCRIBE_CODE_COMMIT_FAILURE,

    SCRIBE_CODE_WRITE_FAILURE,

    SCRIBE_CODE_FIRST = SCRIBE_CODE_OK,
    SCRIBE_CODE_LAST  = SCRIBE_CODE_WRITE_FAILURE,
} scribe_code_t;

#define SCRIBE_CODE_COUNT ((size_t)( SCRIBE_CODE_LAST - SCRIBE_CODE_FIRST + 1 ))

extern scribe_code_t scribe_write(const void *data, size_t bytesize);

#endif /* STDRIOT_H */
