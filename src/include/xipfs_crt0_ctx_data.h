#ifndef __XIPFS_CRT0_CONTEXT_DATA_H__
#define __XIPFS_CRT0_CONTEXT_DATA_H__

/**
 * @brief XIPFS Max Command Line arguments count.
 * @warning MUST REMAIN SYNCHRONIZED with xipfs's file definition.
 */
#define XIPFS_EXEC_ARGC_MAX (64)

typedef struct xipfs_crt0_ctx_data_s {
    /*
     * Start address of the file in NVM,
     * which is the text segment of the xipfs file.
     */
    void *file_base;
    /*
     * true if the context is executed in user mode with configured MPU regions,
     * false otherwise
     */
    unsigned char is_safe_call;
    /*
     * Table of function pointers for functions used by stdriot.
     */
    void **syscall_table;
    /*
     * Number of arguments passed to the relocatable binary.
     */
    int argc;
    /*
     * Arguments passed to the relocatable binary.
     */
    char *argv[XIPFS_EXEC_ARGC_MAX];
    /*
     * RIOT's Global Offset Table
     */
    const void *former_got;
    /*
     * Relocatable binary's current got
     */
    const void *current_got;

} xipfs_crt0_ctx_data_t;

#endif /* __XIPFS_CRT0_CONTEXT_DATA_H__ */
