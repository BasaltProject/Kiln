//! Commands declared by a Brigadier-like spec, their arguments and replies.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn register(spec: []const u8, permission: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) core.Status.invalid else raw.command_register(spec.ptr, core.len(spec), permission.ptr, core.len(permission), token);
}
pub fn reply(ctx: core.Ctx, text: []const u8, args: ?*const core.Args) i32 {
    return raw.command_reply(ctx, text.ptr, core.len(text), core.argsPtr(args), core.argsLen(args));
}
pub fn argStr(ctx: core.Ctx, name: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.command_arg_str(ctx, name.ptr, core.len(name), out.ptr, core.cap(out)));
}
pub fn argI64(ctx: core.Ctx, name: []const u8) ?i64 {
    var value: i64 = 0;
    return if (raw.command_arg_i64(ctx, name.ptr, core.len(name), &value) == 0) value else null;
}
pub fn sourceName(ctx: core.Ctx, out: []u8) ?[]const u8 {
    return core.view(out, raw.command_source_name(ctx, out.ptr, core.cap(out)));
}
pub fn isConsole(ctx: core.Ctx) bool {
    return raw.command_is_console(ctx) != 0;
}
