//! Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component JSON,
//! the plugin store, data by holder and waiting. Calls that give a value fill your buffer and answer an opened
//! `values.Value` over it (null when the call failed); the others answer the ABI result code.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
const raw = @import("raw/proxy_admin.zig").raw;
const raw_values = @import("basalt_shared").raw_values;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

fn opened(out: []u8, needed: i32) ?values.Value {
    if (needed < 0 or needed > out.len) return null;
    return values.Value.open(out[0..@intCast(needed)]);
}

/// OK, or INVALID with the reason written into `err` (cut at its length).
pub fn serversRegister(name: []const u8, host: []const u8, port: i32, like: []const u8, err: []u8) i32 {
    return raw.servers_register(name.ptr, len(name), host.ptr, len(host), port, like.ptr, len(like), err.ptr, cap(err));
}

pub fn serversUnregister(name: []const u8) i32 {
    return raw.servers_unregister(name.ptr, len(name));
}

/// Pings a backend; the handler reads the answer with `completionValue`. Returns the request handle.
pub fn serversPing(name: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.servers_ping(name.ptr, len(name), token);
}

/// Runs the handler after `ticks` ticks of 50 ms. Returns the request handle.
pub fn wait(ticks: i32, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.wait(ticks, token);
}

pub fn requestCancel(request: i32) i32 {
    return raw_values.request_cancel(request);
}

pub fn completionValue(ctx: core.Ctx, out: []u8) ?values.Value {
    return opened(out, raw_values.value_result(ctx, out.ptr, cap(out)));
}

/// 1 when the line named a command, 0 when not, DENIED without the capability and the operator's grant.
pub fn consoleRun(line: []const u8) i32 {
    return raw.console_run(line.ptr, len(line));
}

pub fn playerChat(player: core.Player, text: []const u8) i32 {
    return raw.player_chat(player, text.ptr, len(text));
}

pub fn playerSendJson(player: core.Player, json: []const u8) i32 {
    return raw.player_send_json(player, json.ptr, len(json));
}

pub fn playerKickJson(player: core.Player, json: []const u8) i32 {
    return raw.player_kick_json(player, json.ptr, len(json));
}

/// 1 granted, 0 denied, ABSENT undecided. `who` "" is the console; `server` may be empty.
pub fn permissionCheck(who: []const u8, node: []const u8, server: []const u8) i32 {
    var buf: [320]u8 = undefined;
    var doc: []const u8 = buf[0..0];
    if (server.len > 0) {
        var w = values.Writer.init(&buf);
        w.beginMap();
        w.key("server");
        w.string(server);
        w.end();
        doc = w.finish() catch return -3;
    }
    return raw.permission_check(who.ptr, len(who), node.ptr, len(node), doc.ptr, len(doc));
}

pub fn permissionMeta(who: []const u8, key: []const u8, out: []u8) ?[]const u8 {
    const n = raw.permission_meta(who.ptr, len(who), key.ptr, len(key), out.ptr, 0, out.ptr, cap(out));
    if (n < 0 or n > out.len) return null;
    return out[0..@intCast(n)];
}

/// Registers provider callbacks (null for none); returns the provider handle.
pub fn permissionProvide(check: ?core.Handler, meta: ?core.Handler, priority: i32) i32 {
    const check_token = core.tokenFor(check);
    const meta_token = core.tokenFor(meta);
    return if (check_token < 0 or meta_token < 0) -3 else raw.permission_provide(check_token, meta_token, priority);
}

pub fn permissionUnprovide(provider: i32) i32 {
    return raw.permission_unprovide(provider);
}

/// The question of a provider callback: a map with kind, name, uuid, console, text and context.
pub fn permissionRequest(ctx: core.Ctx, out: []u8) ?values.Value {
    return opened(out, raw.permission_request(ctx, out.ptr, cap(out)));
}

/// 1 grants, 0 denies, negative leaves it to the next provider.
pub fn permissionAnswer(ctx: core.Ctx, answer: i32) i32 {
    return raw.permission_answer(ctx, answer);
}

pub fn permissionAnswerText(ctx: core.Ctx, text: []const u8) i32 {
    return raw.permission_answer_text(ctx, text.ptr, len(text));
}

pub fn permissionChanged() i32 {
    return raw.permission_changed();
}

/// 1 grants, 0 denies, -1 removes.
pub fn permissionOverride(name: []const u8, node: []const u8, value: i32) i32 {
    return raw.permission_override(name.ptr, len(name), node.ptr, len(node), value);
}

pub fn permissionList(name: []const u8, out: []u8) ?values.Value {
    return opened(out, raw.permission_list(name.ptr, len(name), out.ptr, cap(out)));
}

pub fn groupGet(name: []const u8, out: []u8) ?[]const u8 {
    const n = raw.group_get(name.ptr, len(name), out.ptr, cap(out));
    if (n < 0 or n > out.len) return null;
    return out[0..@intCast(n)];
}

pub fn groupSet(name: []const u8, group: []const u8) i32 {
    return raw.group_set(name.ptr, len(name), group.ptr, len(group));
}

/// Reads a store key (null holder) or a player data key of `holder`; null when unset.
pub fn dataValue(holder: ?[]const u8, key: []const u8, out: []u8) ?values.Value {
    const needed = if (holder) |h|
        raw.player_data_get(h.ptr, len(h), key.ptr, len(key), out.ptr, cap(out))
    else
        raw.store_get(key.ptr, len(key), out.ptr, cap(out));
    return opened(out, needed);
}

pub fn dataSetValue(holder: ?[]const u8, key: []const u8, doc: []const u8) i32 {
    if (holder) |h| return raw.player_data_set(h.ptr, len(h), key.ptr, len(key), doc.ptr, len(doc));
    return raw.store_set(key.ptr, len(key), doc.ptr, len(doc));
}

pub fn dataSetInt(holder: ?[]const u8, key: []const u8, number: i64) i32 {
    var buf: [16]u8 = undefined;
    var w = values.Writer.init(&buf);
    w.int(number);
    return dataSetValue(holder, key, w.finish() catch return -3);
}

pub fn dataSetFloat(holder: ?[]const u8, key: []const u8, number: f64) i32 {
    var buf: [16]u8 = undefined;
    var w = values.Writer.init(&buf);
    w.float(number);
    return dataSetValue(holder, key, w.finish() catch return -3);
}

pub fn dataSetBool(holder: ?[]const u8, key: []const u8, flag: bool) i32 {
    var buf: [4]u8 = undefined;
    var w = values.Writer.init(&buf);
    w.boolean(flag);
    return dataSetValue(holder, key, w.finish() catch return -3);
}

pub fn dataSetStr(holder: ?[]const u8, key: []const u8, text: []const u8) i32 {
    var buf: [4200]u8 = undefined;
    var w = values.Writer.init(&buf);
    w.string(text);
    return dataSetValue(holder, key, w.finish() catch return -3);
}

pub fn dataRemove(holder: ?[]const u8, key: []const u8) i32 {
    var buf: [4]u8 = undefined;
    var w = values.Writer.init(&buf);
    w.nullValue();
    return dataSetValue(holder, key, w.finish() catch return -3);
}

/// The best `limit` numeric values of a store key: a list of maps with uuid, name and value.
pub fn storeTop(key: []const u8, limit: i32, out: []u8) ?values.Value {
    return opened(out, raw.store_top(key.ptr, len(key), limit, out.ptr, cap(out)));
}
