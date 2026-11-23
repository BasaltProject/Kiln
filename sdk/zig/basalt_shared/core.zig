//! Handles, callbacks, entry points and the slice helpers that every area of both profiles shares.
pub const Player = i32;
pub const Ctx = i32;
pub const Handler = *const fn (ctx: Ctx) void;

pub const Level = struct {
    pub const debug: i32 = 0;
    pub const info: i32 = 1;
    pub const warn: i32 = 2;
    pub const err: i32 = 3;
};

pub const Priority = struct {
    pub const lowest: i32 = 0;
    pub const low: i32 = 1;
    pub const normal: i32 = 2;
    pub const high: i32 = 3;
    pub const highest: i32 = 4;
};

/// `BASALT_OK` and the negative `BASALT_ERR_*` results of the ABI.
pub const Status = struct {
    pub const ok: i32 = 0;
    pub const absent: i32 = -1;
    pub const stale: i32 = -2;
    pub const invalid: i32 = -3;
    pub const denied: i32 = -4;
};

var handlers: [1024]?Handler = @splat(null);
var handler_count: usize = 0;

pub fn tokenFor(handler: ?Handler) i32 {
    const h = handler orelse return 0;
    if (handler_count >= handlers.len) return -1;
    handlers[handler_count] = h;
    handler_count += 1;
    return @intCast(handler_count);
}

fn callback(token: i32, ctx: i32) callconv(.c) void {
    if (token > 0 and @as(usize, @intCast(token)) <= handler_count) {
        if (handlers[@intCast(token - 1)]) |handler| handler(ctx);
    }
}

fn abiVersion() callconv(.c) i32 {
    return 1;
}

/// Exports the module entry points; Kiln calls this from the generated root with the plugin's `init`.
pub fn install(comptime init: fn () void) void {
    const entry = struct {
        fn run() callconv(.c) void {
            init();
        }
    };
    @export(&abiVersion, .{ .name = "basalt_abi_version" });
    @export(&entry.run, .{ .name = "basalt_init" });
    @export(&callback, .{ .name = "basalt_callback" });
}

pub fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

pub fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

/// Handles `players.online` and `servers.players` can read when the caller's array is smaller than the answer.
pub const spill_handles = 1024;

/// The text a call wrote into `out`, or null on an error or when the answer did not fit (nothing was written then).
pub fn view(out: []u8, n: i32) ?[]const u8 {
    if (n < 0) return null;
    const used: usize = @intCast(n);
    if (used > out.len) return null;
    return out[0..used];
}

/// Named arguments for text: add pairs, then pass the list to `players.send`, `text.translate` or `commands.reply`.
pub const Args = struct {
    buf: [1024]u8 = undefined,
    used: usize = 4,
    count: u32 = 0,

    pub fn init() Args {
        var args = Args{};
        args.writeU32(0, 0);
        return args;
    }

    fn writeU32(self: *Args, at: usize, value: u32) void {
        inline for (0..4) |i| self.buf[at + i] = @truncate(value >> (8 * i));
    }

    pub fn add(self: *Args, key: []const u8, value: []const u8) *Args {
        if (self.used + 8 + key.len + value.len > self.buf.len) return self;
        self.writeU32(self.used, @intCast(key.len));
        @memcpy(self.buf[self.used + 4 ..][0..key.len], key);
        self.used += 4 + key.len;
        self.writeU32(self.used, @intCast(value.len));
        @memcpy(self.buf[self.used + 4 ..][0..value.len], value);
        self.used += 4 + value.len;
        self.count += 1;
        self.writeU32(0, self.count);
        return self;
    }

    pub fn ptr(self: *const Args) ?[*]const u8 {
        return if (self.count == 0) null else &self.buf;
    }

    pub fn size(self: *const Args) i32 {
        return if (self.count == 0) 0 else @intCast(self.used);
    }
};

pub fn argsPtr(args: ?*const Args) ?[*]const u8 {
    return if (args) |a| a.ptr() else null;
}

pub fn argsLen(args: ?*const Args) i32 {
    return if (args) |a| a.size() else 0;
}

/// Writes the decimal text of `value` into `out` and returns it.
pub fn formatI64(value: i64, out: []u8) []const u8 {
    var digits: [24]u8 = undefined;
    var n: usize = 0;
    var magnitude: u64 = if (value < 0) 0 -% @as(u64, @bitCast(value)) else @intCast(value);
    while (true) {
        digits[n] = @intCast('0' + magnitude % 10);
        n += 1;
        magnitude /= 10;
        if (magnitude == 0) break;
    }
    var used: usize = 0;
    if (value < 0) {
        out[0] = '-';
        used = 1;
    }
    while (n > 0) {
        n -= 1;
        out[used] = digits[n];
        used += 1;
    }
    return out[0..used];
}
