//! Events area for Zig: slices over the raw imports, documents as `values.Value`. Statuses are the negative
//! `BASALT_ERR_*` codes as `i32`; reads answer a value or an error.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
const raw = @import("raw/events.zig").raw;
const raw_values = @import("basalt_shared").raw_values;
const shared = @import("basalt_shared").events;

pub const on = shared.on;
pub const playerOf = shared.playerOf;
pub const fieldStr = shared.fieldStr;
pub const fieldI64 = shared.fieldI64;
pub const cancel = shared.cancel;

pub const Error = values.CallError;
pub const post: i32 = 1;
pub const cancelled: i32 = 2;

const none = [_]u8{};

fn n(s: []const u8) i32 {
    return @intCast(s.len);
}

fn view(out: []u8, length: i32) ?[]const u8 {
    if (length < 0) return null;
    const used: usize = @intCast(length);
    if (used > out.len) return null;
    return out[0..used];
}

fn argsPtr(args: ?*const core.Args) [*]const u8 {
    if (args) |a| {
        if (a.ptr()) |p| return p;
    }
    return &none;
}

fn argsLen(args: ?*const core.Args) i32 {
    return if (args) |a| a.size() else 0;
}

/// Listens to `name`; `filters` is a finished map document or empty.
pub fn onEx(name: []const u8, priority: i32, flags: i32, filters: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    if (token < 0) return -3;
    return raw.event_on_ex(name.ptr, n(name), priority, flags, filters.ptr, n(filters), token);
}

pub fn eventName(ctx: core.Ctx, out: []u8) ?[]const u8 {
    return view(out, raw.event_name(ctx, out.ptr, n(out)));
}

const FieldCall = struct {
    id: i32,
    name: []const u8,
    fn event(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw.event_get(self.id, self.name.ptr, n(self.name), out, cap);
    }
    fn player(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw.player_get(self.id, self.name.ptr, n(self.name), out, cap);
    }
    fn describe(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw.event_describe(self.name.ptr, n(self.name), out, cap);
    }
    fn fields(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw.event_fields(self.id, out, cap);
    }
    fn payload(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw.event_payload(self.id, out, cap);
    }
    fn names(self: FieldCall, out: [*]u8, cap: i32) i32 {
        _ = self;
        return raw.event_names(out, cap);
    }
    fn describePlayer(self: FieldCall, out: [*]u8, cap: i32) i32 {
        _ = self;
        return raw.player_describe(out, cap);
    }
    fn result(self: FieldCall, out: [*]u8, cap: i32) i32 {
        return raw_values.value_result(self.id, out, cap);
    }
};

pub fn eventGet(ctx: core.Ctx, field: []const u8, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = ctx, .name = field }, FieldCall.event, buffer, buffer[0..0]);
}

pub fn eventSet(ctx: core.Ctx, field: []const u8, value: []const u8) i32 {
    return raw.event_set(ctx, field.ptr, n(field), value.ptr, n(value));
}

pub fn eventFields(ctx: core.Ctx, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = ctx, .name = "" }, FieldCall.fields, buffer, buffer[0..0]);
}

pub fn eventPayload(ctx: core.Ctx, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = ctx, .name = "" }, FieldCall.payload, buffer, buffer[0..0]);
}

pub fn eventDescribe(name: []const u8, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = 0, .name = name }, FieldCall.describe, buffer, buffer[0..0]);
}

pub fn eventNames(buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = 0, .name = "" }, FieldCall.names, buffer, buffer[0..0]);
}

pub fn eventCancelled(ctx: core.Ctx) i32 {
    return raw.event_cancelled(ctx);
}

pub fn eventCancelWith(ctx: core.Ctx, text: []const u8, args: ?*const core.Args) i32 {
    return raw.event_cancel_with(ctx, text.ptr, n(text), argsPtr(args), argsLen(args));
}

pub fn eventCancelMessage(ctx: core.Ctx, out: []u8) ?[]const u8 {
    return view(out, raw.event_cancel_message(ctx, out.ptr, n(out)));
}

pub fn eventIsPost(ctx: core.Ctx) i32 {
    return raw.event_is_post(ctx);
}

pub fn eventEmit(name: []const u8, value: []const u8) i32 {
    return raw.event_emit(name.ptr, n(name), value.ptr, n(value));
}

pub fn playerByUuid(uuid: []const u8) core.Player {
    return raw.player_by_uuid(uuid.ptr, n(uuid));
}

pub fn playerUuid(player: core.Player, out: []u8) ?[]const u8 {
    return view(out, raw.player_uuid(player, out.ptr, n(out)));
}

pub fn playerGet(player: core.Player, name: []const u8, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = player, .name = name }, FieldCall.player, buffer, buffer[0..0]);
}

pub fn playerSet(player: core.Player, name: []const u8, value: []const u8) i32 {
    return raw.player_set(player, name.ptr, n(name), value.ptr, n(value));
}

const OpCall = struct {
    player: i32,
    op: []const u8,
    args: []const u8,
    fn call(self: OpCall, out: [*]u8, cap: i32) i32 {
        return raw.player_call(self.player, self.op.ptr, n(self.op), self.args.ptr, n(self.args), out, cap);
    }
};

/// Runs a named operation; `args` is a finished list document or empty.
pub fn playerCall(player: core.Player, op: []const u8, args: []const u8, buffer: []u8) Error!values.Value {
    return values.callValue(OpCall{ .player = player, .op = op, .args = args }, OpCall.call, buffer, buffer[0..0]);
}

pub fn playerDescribe(buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = 0, .name = "" }, FieldCall.describePlayer, buffer, buffer[0..0]);
}

pub fn playerHealth(player: core.Player) ?f64 {
    var health: f64 = 0;
    return if (raw.player_health(player, @ptrCast(&health)) < 0) null else health;
}

pub fn playerSetHealth(player: core.Player, health: f64) i32 {
    return raw.player_set_health(player, health);
}

pub fn playerGamemode(player: core.Player, out: []u8) ?[]const u8 {
    return view(out, raw.player_gamemode(player, out.ptr, n(out)));
}

pub fn playerSetGamemode(player: core.Player, mode: []const u8) i32 {
    return raw.player_set_gamemode(player, mode.ptr, n(mode));
}

pub fn playerTeleport(player: core.Player, world: []const u8, x: f64, y: f64, z: f64, yaw: f32, pitch: f32) i32 {
    return raw.player_teleport(player, world.ptr, n(world), x, y, z, yaw, pitch);
}

pub fn playerTitle(player: core.Player, title: []const u8, subtitle: []const u8, fade_in: i32, stay: i32, fade_out: i32, args: ?*const core.Args) i32 {
    return raw.player_title(player, title.ptr, n(title), subtitle.ptr, n(subtitle), fade_in, stay, fade_out, argsPtr(args), argsLen(args));
}

pub fn playerKick(player: core.Player, text: []const u8, args: ?*const core.Args) i32 {
    return raw.player_kick(player, text.ptr, n(text), argsPtr(args), argsLen(args));
}

pub fn playerEffect(player: core.Player, effect: []const u8, ticks: i32, amplifier: i32) i32 {
    return raw.player_effect(player, effect.ptr, n(effect), ticks, amplifier);
}

pub fn playerOpenInventory(player: core.Player, kind: []const u8, spec: []const u8) i32 {
    return raw.player_open_inventory(player, kind.ptr, n(kind), spec.ptr, n(spec));
}

pub fn playerCloseWindow(player: core.Player) i32 {
    return raw.player_close_window(player);
}

/// Shows a form (a finished document); the handler runs once with the answer, read with `completionValue`.
pub fn formOpen(player: core.Player, form: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    if (token < 0) return -3;
    return raw.form_open(player, form.ptr, n(form), token);
}

pub fn formClose(player: core.Player) i32 {
    return raw.form_close(player);
}

pub fn dialogShow(player: core.Player, dialog: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    if (token < 0) return -3;
    return raw.dialog_show(player, dialog.ptr, n(dialog), token);
}

pub fn dialogClear(player: core.Player) i32 {
    return raw.dialog_clear(player);
}

pub fn completionValue(ctx: core.Ctx, buffer: []u8) Error!values.Value {
    return values.callValue(FieldCall{ .id = ctx, .name = "" }, FieldCall.result, buffer, buffer[0..0]);
}
