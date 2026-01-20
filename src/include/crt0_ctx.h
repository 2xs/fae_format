#ifndef __FAE_CRT0_CONTEXT_H__
#define __FAE_CRT0_CONTEXT_H__

/**
 * @brief Data structure that describes the memory layout
 * required by the CRT0 to execute the relocatable binary
 * @warning MUST REMAIN SYNCHRONIZED with xipfs's file definition.
 */
typedef struct crt0_ctx_s {
    /*
     * Start address of the binary in the NVM
     */
    void *bin_base;
    /**
     * Start address of the available free RAM
     */
    void *ram_start;
    /**
     * End address of the available free RAM
     */
    void *ram_end;
    /**
     * Start address of the free NVM
     */
    void *nvm_start;
    /**
     * End address of the free NVM
     */
    void *nvm_end;
    /**
     * Arguments passed to the relocatable binary.
     * This is up to caller and callee to agree on a common arguments type.
     */
    void *argv;
} crt0_ctx_t;

typedef void (*crt0_entrypoint_t)(crt0_ctx_t *ctx);

#endif /*  __FAE_CRT0_CONTEXT_H__ */
