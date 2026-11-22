//! Proxy hud area (abi/areas/proxy_hud.h): boss bars, sidebar, tab list, tab rows, server links and chat completions.
//! A null optional text leaves that part unchanged; every call answers 0 or a negative code.
const core = @import("core.zig");
const values = @import("basalt_shared").values;
const raw = @import("raw/proxy_hud.zig").raw;

const buffer_bytes = 8192;
const invalid: i32 = -3;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn strKey(w: *values.Writer, name: []const u8, text: ?[]const u8) void {
    if (text) |t| {
        w.key(name);
        w.string(t);
    }
}

pub fn clearTitle(player: core.Player, reset: bool) i32 {
    return raw.hud_clear_title(player, @intFromBool(reset));
}

pub fn tabList(player: core.Player, header: ?[]const u8, footer: ?[]const u8) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginMap();
    strKey(&w, "header", header);
    strKey(&w, "footer", footer);
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_tab_list(player, doc.ptr, len(doc));
}

pub fn bossbarShow(player: core.Player, id: []const u8, text: []const u8, progress: f64, color: []const u8, overlay: []const u8) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginMap();
    strKey(&w, "text", text);
    w.key("progress");
    w.float(progress);
    strKey(&w, "color", color);
    strKey(&w, "overlay", overlay);
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_bossbar_show(player, id.ptr, len(id), doc.ptr, len(doc));
}

pub fn bossbarHide(player: core.Player, id: []const u8) i32 {
    return raw.hud_bossbar_hide(player, id.ptr, len(id));
}

pub fn bossbarProgress(player: core.Player, id: []const u8, progress: f64) i32 {
    return raw.hud_bossbar_progress(player, id.ptr, len(id), progress);
}

pub fn bossbarTitle(player: core.Player, id: []const u8, text: []const u8) i32 {
    return raw.hud_bossbar_title(player, id.ptr, len(id), text.ptr, len(text), text.ptr, 0);
}

pub fn bossbarStyle(player: core.Player, id: []const u8, color: []const u8, overlay: []const u8) i32 {
    return raw.hud_bossbar_style(player, id.ptr, len(id), color.ptr, len(color), overlay.ptr, len(overlay));
}

pub fn sidebar(player: core.Player, title: []const u8, lines: []const []const u8) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginMap();
    strKey(&w, "title", title);
    w.key("lines");
    w.beginList();
    for (lines) |line| w.string(line);
    w.end();
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_sidebar(player, doc.ptr, len(doc));
}

pub fn sidebarClear(player: core.Player) i32 {
    return raw.hud_sidebar_clear(player);
}

pub fn tabEntry(player: core.Player, id: []const u8, name: []const u8, text: []const u8, latency: i64) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginMap();
    strKey(&w, "name", name);
    strKey(&w, "text", text);
    w.key("latency");
    w.int(latency);
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_tab_entry(player, id.ptr, len(id), doc.ptr, len(doc));
}

pub fn tabEntryRemove(player: core.Player, id: []const u8) i32 {
    return raw.hud_tab_entry_remove(player, id.ptr, len(id));
}

/// `pairs` holds label then url for each link.
pub fn serverLinks(player: core.Player, pairs: []const []const u8) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginList();
    var at: usize = 0;
    while (at + 1 < pairs.len) : (at += 2) {
        w.beginList();
        w.string(pairs[at]);
        w.string(pairs[at + 1]);
        w.end();
    }
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_server_links(player, doc.ptr, len(doc));
}

pub fn chatCompletions(player: core.Player, action: []const u8, words: []const []const u8) i32 {
    var storage: [buffer_bytes]u8 = undefined;
    var w = values.Writer.init(&storage);
    w.beginList();
    for (words) |word| w.string(word);
    w.end();
    const doc = w.finish() catch return invalid;
    return raw.hud_chat_completions(player, action.ptr, len(action), doc.ptr, len(doc));
}
