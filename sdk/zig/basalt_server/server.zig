//! Server area for Zig: permissions, dynamic commands, broadcast, plugin files and storage, services and the
//! scheduler completions. Slices over the raw imports; reads answer a value or an error, writes the status as `i32`.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
const raw = @import("raw/server.zig").raw;
const raw_values = @import("basalt_shared").raw_values;

pub const Error = values.CallError;
pub const buffer_size: usize = 1024;
pub const broadcast_players: i32 = 1;
pub const broadcast_console: i32 = 2;
pub const provide_check: i32 = 1;
pub const provide_meta: i32 = 2;

const error_invalid: i32 = -3;
const none = [_]u8{};

fn n(s: []const u8) i32 {
    return @intCast(s.len);
}

fn view(out: []u8, length: i32) ?[]const u8 {
    if (length < 0) return null;
    return out[0..@min(@as(usize, @intCast(length)), out.len)];
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

/// The negative `BASALT_ERR_*` status an `Error` stands for.
pub fn status(err: Error) i32 {
    return switch (err) {
        error.Absent => -1,
        error.Stale => -2,
        error.Invalid => -3,
        error.Denied => -4,
        else => -5,
    };
}

const Call = struct {
    a: []const u8 = "",
    b: []const u8 = "",
    id: i32 = 0,

    fn result(self: Call, out: [*]u8, cap: i32) i32 {
        return raw_values.value_result(self.id, out, cap);
    }
    fn info(_: Call, out: [*]u8, cap: i32) i32 {
        return raw.server_info(out, cap);
    }
    fn serverCall(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.server_call(self.a.ptr, n(self.a), self.b.ptr, n(self.b), out, cap);
    }
    fn storeGet(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.store_get(self.a.ptr, n(self.a), out, cap);
    }
    fn dataRead(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.data_read(self.a.ptr, n(self.a), out, cap);
    }
    fn config(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.config_value(self.a.ptr, n(self.a), out, cap);
    }
    fn list(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.files_list(self.a.ptr, n(self.a), out, cap);
    }
    fn stat(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.files_stat(self.a.ptr, n(self.a), out, cap);
    }
    fn walk(self: Call, out: [*]u8, cap: i32) i32 {
        return raw.files_walk(self.a.ptr, n(self.a), out, cap);
    }
};

fn read(call: Call, comptime func: anytype, buffer: []u8) Error!values.Value {
    return values.callValue(call, func, buffer, buffer[0..0]);
}

pub fn permissionCheck(subject: []const u8, node: []const u8) i32 {
    return raw.permission_check(subject.ptr, n(subject), node.ptr, n(node));
}

pub fn permissionHas(player: core.Player, node: []const u8) i32 {
    return raw.permission_has(player, node.ptr, n(node));
}

pub fn permissionGrant(player: core.Player, node: []const u8) i32 {
    return raw.permission_set(player, node.ptr, n(node), 1);
}

pub fn permissionDeny(player: core.Player, node: []const u8) i32 {
    return raw.permission_set(player, node.ptr, n(node), 0);
}

pub fn permissionDrop(player: core.Player, node: []const u8) i32 {
    return raw.permission_set(player, node.ptr, n(node), -1);
}

pub fn permissionMeta(subject: []const u8, key: []const u8, out: []u8) ?[]const u8 {
    return view(out, raw.permission_meta(subject.ptr, n(subject), key.ptr, n(key), out.ptr, n(out)));
}

pub fn permissionProvide(priority: i32, kinds: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.permission_provide(priority, kinds, token);
}

pub fn permissionAnswer(ctx: core.Ctx, answer: i32) i32 {
    return raw.permission_answer(ctx, answer);
}

pub fn permissionAnswerMeta(ctx: core.Ctx, text: []const u8) i32 {
    return raw.permission_answer_meta(ctx, text.ptr, n(text));
}

/// The node (or meta key) a provider callback is asked about: `field` is "kind" or "node".
pub fn permissionQuery(ctx: core.Ctx, field: []const u8, buffer: []u8) ?[]const u8 {
    const doc = result(ctx, buffer) catch return null;
    return (doc.find(field) orelse return null).asString();
}

pub fn command(spec: []const u8, permission: []const u8, handler: core.Handler, complete: ?core.Handler) i32 {
    const token = core.tokenFor(handler);
    const complete_token: i32 = if (complete) |c| core.tokenFor(c) else -1;
    if (token < 0 or (complete != null and complete_token < 0)) return error_invalid;
    return raw.command_register_ex(spec.ptr, n(spec), permission.ptr, n(permission), token, complete_token);
}

pub fn commandAlias(name: []const u8, alias: []const u8) i32 {
    return raw.command_alias(name.ptr, n(name), alias.ptr, n(alias));
}

pub fn commandUnregister(name: []const u8) i32 {
    return raw.command_unregister(name.ptr, n(name));
}

/// Runs a command line as the console (player 0) or as `player`.
pub fn commandRun(line: []const u8, player: core.Player) i32 {
    return raw.command_run(line.ptr, n(line), player);
}

pub fn completeInput(ctx: core.Ctx, out: []u8) ?[]const u8 {
    return view(out, raw.command_complete_input(ctx, out.ptr, n(out)));
}

pub fn completeAdd(ctx: core.Ctx, candidate: []const u8) i32 {
    return raw.command_complete_add(ctx, candidate.ptr, n(candidate));
}

pub fn commandArgF64(ctx: core.Ctx, name: []const u8) ?f64 {
    var out: f64 = 0;
    return if (raw.command_arg_f64(ctx, name.ptr, n(name), @ptrCast(&out)) < 0) null else out;
}

pub fn commandArgBool(ctx: core.Ctx, name: []const u8) ?bool {
    var out: i32 = 0;
    return if (raw.command_arg_bool(ctx, name.ptr, n(name), @ptrCast(&out)) < 0) null else out != 0;
}

/// `text` may be a "@key" lang key; `args` may be null.
pub fn broadcast(text: []const u8, args: ?*const core.Args, flags: i32) i32 {
    return raw.server_broadcast(text.ptr, n(text), argsPtr(args), argsLen(args), flags);
}

pub fn placeholderFill(text: []const u8, player: core.Player, out: []u8) ?[]const u8 {
    return view(out, raw.placeholder_fill(text.ptr, n(text), player, out.ptr, n(out)));
}

pub fn info(buffer: []u8) Error!values.Value {
    return read(.{}, Call.info, buffer);
}

/// `args` is a finished value document, empty for null.
pub fn serverCall(op: []const u8, args: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = op, .b = args }, Call.serverCall, buffer);
}

pub fn profileLookup(name: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.profile_lookup(name.ptr, n(name), token);
}

/// The value a completion callback (a wait, a lookup, a service call or a service handler) carries.
pub fn result(ctx: core.Ctx, buffer: []u8) Error!values.Value {
    return read(.{ .id = ctx }, Call.result, buffer);
}

pub fn fileRead(path: []const u8, out: []u8) i32 {
    return raw.files_read(path.ptr, n(path), out.ptr, n(out));
}

pub fn fileWrite(path: []const u8, data: []const u8) i32 {
    return raw.files_write(path.ptr, n(path), data.ptr, n(data));
}

pub fn fileExists(path: []const u8) bool {
    return raw.files_exists(path.ptr, n(path)) == 1;
}

pub fn fileDelete(path: []const u8, recursive: bool) i32 {
    return raw.files_delete(path.ptr, n(path), @intFromBool(recursive));
}

pub fn fileList(prefix: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = prefix }, Call.list, buffer);
}

pub fn fileStat(path: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = path }, Call.stat, buffer);
}

pub fn fileWalk(path: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = path }, Call.walk, buffer);
}

pub fn storeSetInt(key: []const u8, value: i64) i32 {
    var doc: [16]u8 = undefined;
    var writer = values.Writer.init(&doc);
    writer.int(value);
    const bytes = writer.finish() catch return error_invalid;
    return raw.store_set(key.ptr, n(key), bytes.ptr, n(bytes));
}

pub fn storeSetStr(key: []const u8, value: []const u8) i32 {
    var doc: [256]u8 = undefined;
    var writer = values.Writer.init(&doc);
    writer.string(value);
    const bytes = writer.finish() catch return error_invalid;
    return raw.store_set(key.ptr, n(key), bytes.ptr, n(bytes));
}

pub fn storeGet(key: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = key }, Call.storeGet, buffer);
}

pub fn dataWrite(name: []const u8, doc: []const u8) i32 {
    return raw.data_write(name.ptr, n(name), doc.ptr, n(doc));
}

pub fn dataRead(name: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = name }, Call.dataRead, buffer);
}

/// An empty key answers the whole config table.
pub fn configValue(key: []const u8, buffer: []u8) Error!values.Value {
    return read(.{ .a = key }, Call.config, buffer);
}

pub fn serviceRegister(name: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.service_register(name.ptr, n(name), token);
}

/// Answers the call a service handler runs for; `doc` is a finished value document, empty for null.
pub fn serviceReply(ctx: core.Ctx, doc: []const u8) i32 {
    return raw.service_reply(ctx, doc.ptr, n(doc));
}

pub fn serviceHas(name: []const u8) bool {
    return raw.service_has(name.ptr, n(name)) == 1;
}

pub fn serviceCall(name: []const u8, args: []const u8, timeout: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.service_call(name.ptr, n(name), args.ptr, n(args), timeout, token);
}

pub fn wait(ticks: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.task_wait(ticks, token);
}

pub fn at(world: []const u8, x: f64, z: f64, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) error_invalid else raw.task_at(world.ptr, n(world), x, z, token);
}
