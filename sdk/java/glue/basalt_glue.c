#include "basalt_abi.h"
#include "basalt_proxy_abi.h"
#include "generated/basalt_java_areas.h"

#include <stdint.h>
#include <stdio.h>

static void (*basalt_entry)(int32_t, int32_t);

void basalt_register_callback(void* entry) { basalt_entry = (void (*)(int32_t, int32_t))entry; }

#include "all.c"

extern int teavm_boot_main(int, char**);
static int booted;

BASALT_EXPORT(basalt_abi_version) int32_t basalt_abi_version(void) { return BASALT_ABI_VERSION; }

BASALT_EXPORT(basalt_init) void basalt_init(void) {
    if (!booted) {
        booted = 1;
        teavm_boot_main(0, 0);
    }
}

BASALT_EXPORT(basalt_callback) void basalt_callback(int32_t token, int32_t ctx) { basalt_entry(token, ctx); }

/* WASI calls the host does not provide, stubbed out, and the exception tag that setjmp needs. */
int __imported_wasi_snapshot_preview1_fd_seek(int fd, int64_t offset, int whence, void* out) { return 70; }
int fclose(FILE* file) { return -1; }
int close(int fd) { return -1; }
__asm__(".globl __c_longjmp\n.tagtype __c_longjmp i32\n__c_longjmp:\n");
