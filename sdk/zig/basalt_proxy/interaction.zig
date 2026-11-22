//! Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
//! operations. Observed events and completions fill your buffer with an encoded value for `values.Value.open`; the
//! result is the length the full answer needs, or a negative code.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
const raw = @import("raw/proxy_interaction.zig").raw;
const raw_values = @import("basalt_shared").raw_values;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

/// Observes player.resource_pack: the value holds player, player_name, id and status.
pub fn watchResourcePacks(handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    const name = "player.resource_pack";
    return if (token < 0) -3 else raw.interaction_watch(name, len(name), token);
}

/// Observes player.settings: the value holds player, player_name and settings.
pub fn watchSettings(handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    const name = "player.settings";
    return if (token < 0) -3 else raw.interaction_watch(name, len(name), token);
}

/// The encoded value of an observed event or a completion.
pub fn valueResult(ctx: core.Ctx, out: []u8) i32 {
    return raw_values.value_result(ctx, out.ptr, cap(out));
}

pub const Button = struct { label: []const u8, command: []const u8 };

/// kind is "notice", "confirmation" or "multi_action"; body may be empty.
pub fn dialogShow(player: core.Player, kind: []const u8, title: []const u8, body: []const u8, buttons: []const Button, closable: bool) i32 {
    var buffer: [6144]u8 = undefined;
    var w = values.Writer.init(&buffer);
    w.beginMap();
    w.key("kind");
    w.string(kind);
    w.key("title");
    w.string(title);
    if (body.len > 0) {
        w.key("body");
        w.string(body);
    }
    w.key("buttons");
    w.beginList();
    for (buttons) |button| {
        w.beginMap();
        w.key("label");
        w.string(button.label);
        w.key("command");
        w.string(button.command);
        w.end();
    }
    w.end();
    w.key("can_close_with_escape");
    w.boolean(closable);
    w.end();
    const doc = w.finish() catch return -3;
    return raw.dialog_show(player, doc.ptr, len(doc));
}

pub fn dialogClear(player: core.Player) i32 {
    return raw.dialog_clear(player);
}

/// Pushes a pack; hash, prompt and id may be empty. The pack id text goes to `out`, the result is its length.
pub fn resourcePackPush(player: core.Player, url: []const u8, hash: []const u8, required: bool, prompt: []const u8, id: []const u8, out: []u8) i32 {
    var buffer: [2048]u8 = undefined;
    var w = values.Writer.init(&buffer);
    w.beginMap();
    w.key("url");
    w.string(url);
    if (hash.len > 0) {
        w.key("hash");
        w.string(hash);
    }
    w.key("required");
    w.boolean(required);
    if (prompt.len > 0) {
        w.key("prompt");
        w.string(prompt);
    }
    if (id.len > 0) {
        w.key("id");
        w.string(id);
    }
    w.end();
    const doc = w.finish() catch return -3;
    return raw.resource_pack_push(player, doc.ptr, len(doc), out.ptr, cap(out));
}

pub fn resourcePackRemove(player: core.Player, id: []const u8) i32 {
    return raw.resource_pack_remove(player, id.ptr, len(id));
}

pub fn resourcePackClear(player: core.Player) i32 {
    return raw.resource_pack_clear(player);
}

pub fn cookieSet(player: core.Player, key: []const u8, data: []const u8) i32 {
    return raw.cookie_set(player, key.ptr, len(key), data.ptr, len(data));
}

/// Returns the request handle; the handler reads the cookie with `cookieResult`.
pub fn cookieRequest(player: core.Player, key: []const u8, handler: core.Handler) i32 {
    const token = core.tokenFor(handler);
    return if (token < 0) -3 else raw.cookie_request(player, key.ptr, len(key), token);
}

/// Copies the cookie of a completion into `out` and returns its length, or a negative code when the client had none.
pub fn cookieResult(ctx: core.Ctx, out: []u8) i32 {
    var scratch: [5200]u8 = undefined;
    const n = valueResult(ctx, &scratch);
    if (n < 0) return n;
    if (n > scratch.len) return -3;
    const root = values.Value.open(scratch[0..@intCast(n)]) orelse return -3;
    if (root.isNull()) return -1;
    const data = root.asBytes() orelse return -3;
    if (data.len > out.len) return -3;
    @memcpy(out[0..data.len], data);
    return @intCast(data.len);
}

pub fn requestCancel(request: i32) i32 {
    return raw_values.request_cancel(request);
}

pub fn playerTransfer(player: core.Player, host: []const u8, port: i32) i32 {
    return raw.player_transfer(player, host.ptr, len(host), port);
}

pub fn playersByUuid(uuid: []const u8) core.Player {
    return raw.players_by_uuid(uuid.ptr, len(uuid));
}

/// Players on the proxy, or on `server` when it is not empty.
pub fn playersCount(server: []const u8) i32 {
    return raw.players_count(server.ptr, len(server));
}

pub fn playersBroadcast(text: []const u8, args: ?*const core.Args) i32 {
    const packed_args: [*]const u8 = if (args) |a| (if (a.count == 0) text.ptr else &a.buf) else text.ptr;
    const packed_len: i32 = if (args) |a| (if (a.count == 0) 0 else @intCast(a.used)) else 0;
    return raw.players_broadcast(text.ptr, len(text), packed_args, packed_len);
}

pub fn playerAddress(player: core.Player, out: []u8) i32 {
    return raw.player_address(player, out.ptr, cap(out));
}

pub fn playerPort(player: core.Player) i32 {
    return raw.player_port(player);
}

/// The client settings as an encoded map.
pub fn playerSettings(player: core.Player, out: []u8) i32 {
    return raw.player_settings(player, out.ptr, cap(out));
}

pub fn playerDataGetF64(player: core.Player, key: []const u8, fallback: f64) f64 {
    return raw.player_data_get_f64(player, key.ptr, len(key), fallback);
}

pub fn playerDataSetF64(player: core.Player, key: []const u8, value: f64) i32 {
    return raw.player_data_set_f64(player, key.ptr, len(key), value);
}
