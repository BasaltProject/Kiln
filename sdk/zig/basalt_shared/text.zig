//! Lang keys of resources/lang rendered with named arguments.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn translate(key: []const u8, args: ?*const core.Args, out: []u8) ?[]const u8 {
    return core.view(out, raw.translate(key.ptr, core.len(key), core.argsPtr(args), core.argsLen(args), out.ptr, core.cap(out)));
}
