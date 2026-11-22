// Defines the WASI imports the host does not provide, so the linker keeps them out of the module.
#include <stdint.h>

int32_t __imported_wasi_snapshot_preview1_fd_close(int32_t fd) { (void)fd; return 8; }
int32_t __imported_wasi_snapshot_preview1_fd_fdstat_get(int32_t fd, int32_t out) { (void)fd; (void)out; return 8; }
int32_t __imported_wasi_snapshot_preview1_fd_prestat_get(int32_t fd, int32_t out) { (void)fd; (void)out; return 8; }
int32_t __imported_wasi_snapshot_preview1_fd_prestat_dir_name(int32_t fd, int32_t path, int32_t length) {
    (void)fd; (void)path; (void)length;
    return 8;
}
int32_t __imported_wasi_snapshot_preview1_fd_seek(int32_t fd, int64_t offset, int32_t whence, int32_t out) {
    (void)fd; (void)offset; (void)whence; (void)out;
    return 8;
}
int32_t __imported_wasi_snapshot_preview1_poll_oneoff(int32_t in, int32_t out, int32_t count, int32_t events) {
    (void)in; (void)out; (void)count; (void)events;
    return 52;
}
int32_t __imported_wasi_snapshot_preview1_sched_yield(void) { return 0; }
