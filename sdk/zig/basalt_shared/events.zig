//! Event listeners and the fields of the event being handled.
const core = @import("core.zig");
const raw = @import("raw/core.zig");

pub fn on(event: []const u8, priority: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) core.Status.invalid else raw.event_on(event.ptr, core.len(event), priority, token);
}
pub fn playerOf(ctx: core.Ctx) core.Player {
    return raw.event_player(ctx);
}
pub fn fieldStr(ctx: core.Ctx, field: []const u8, out: []u8) ?[]const u8 {
    return core.view(out, raw.event_field_str(ctx, field.ptr, core.len(field), out.ptr, core.cap(out)));
}
pub fn fieldI64(ctx: core.Ctx, field: []const u8) ?i64 {
    var value: i64 = 0;
    return if (raw.event_field_i64(ctx, field.ptr, core.len(field), &value) == 0) value else null;
}
pub fn cancel(ctx: core.Ctx, cancelled: bool) i32 {
    return raw.event_cancel(ctx, @intFromBool(cancelled));
}
