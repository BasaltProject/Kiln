//! Value builder and reader over the byte format of basalt_values.h. The writer fills a buffer you provide, or grows a
//! heap one when you give it an allocator; the reader works in place on a validated document and never traps.
const std = @import("std");

pub const max_bytes: usize = 1 << 20;
pub const max_depth: usize = 16;
pub const max_nodes: usize = 65536;
pub const max_key_bytes: usize = 256;

const tag_null: u8 = 0;
const tag_false: u8 = 1;
const tag_true: u8 = 2;
const tag_int: u8 = 3;
const tag_float: u8 = 4;
const tag_string: u8 = 5;
const tag_bytes: u8 = 6;
const tag_list: u8 = 7;
const tag_map: u8 = 8;

pub const Kind = enum { null, bool, int, float, string, bytes, list, map };

pub const WriteError = error{ Invalid, Overflow };

pub const Writer = struct {
    buf: []u8,
    len: usize = 0,
    nodes: usize = 0,
    depth: usize = 0,
    failure: ?WriteError = null,
    allocator: ?std.mem.Allocator = null,
    start: [max_depth + 1]usize = undefined,
    count: [max_depth + 1]usize = undefined,
    is_map: [max_depth + 1]bool = undefined,
    pending_key: [max_depth + 1]bool = undefined,

    /// A writer over `buffer`; it fails with `error.Overflow` rather than grow.
    pub fn init(buffer: []u8) Writer {
        return .{ .buf = buffer };
    }

    /// A writer that grows on the heap; call `deinit` to free it.
    pub fn initAlloc(allocator: std.mem.Allocator) Writer {
        return .{ .buf = &.{}, .allocator = allocator };
    }

    pub fn deinit(self: *Writer) void {
        if (self.allocator) |a| {
            if (self.buf.len > 0) a.free(self.buf);
        }
        self.buf = &.{};
        self.len = 0;
    }

    pub fn ok(self: *const Writer) bool {
        return self.failure == null;
    }

    fn fail(self: *Writer, reason: WriteError) void {
        if (self.failure == null) self.failure = reason;
    }

    fn reserve(self: *Writer, extra: usize) bool {
        if (self.failure != null) return false;
        if (extra > max_bytes or self.len > max_bytes - extra) {
            self.fail(error.Invalid);
            return false;
        }
        const need = self.len + extra;
        if (need <= self.buf.len) return true;
        const a = self.allocator orelse {
            self.fail(error.Overflow);
            return false;
        };
        var size: usize = if (self.buf.len == 0) 64 else self.buf.len * 2;
        while (size < need) size *= 2;
        const grown = a.alloc(u8, size) catch {
            self.fail(error.Overflow);
            return false;
        };
        var i: usize = 0;
        while (i < self.len) : (i += 1) grown[i] = self.buf[i];
        if (self.buf.len > 0) a.free(self.buf);
        self.buf = grown;
        return true;
    }

    fn byte(self: *Writer, b: u8) void {
        if (self.reserve(1)) {
            self.buf[self.len] = b;
            self.len += 1;
        }
    }

    fn leb(self: *Writer, value: u64) void {
        var v = value;
        while (v >= 0x80) {
            self.byte(@as(u8, @truncate(v & 0x7F)) | 0x80);
            v >>= 7;
        }
        self.byte(@truncate(v));
    }

    fn raw(self: *Writer, data: []const u8) void {
        if (self.reserve(data.len)) {
            for (data, 0..) |b, i| self.buf[self.len + i] = b;
            self.len += data.len;
        }
    }

    fn begin(self: *Writer) bool {
        if (self.failure != null) return false;
        if (self.depth == 0) {
            if (self.nodes > 0) {
                self.fail(error.Invalid);
                return false;
            }
        } else {
            const top = self.depth - 1;
            if (self.is_map[top]) {
                if (!self.pending_key[top]) {
                    self.fail(error.Invalid);
                    return false;
                }
                self.pending_key[top] = false;
            } else {
                self.count[top] += 1;
            }
        }
        self.nodes += 1;
        if (self.depth > max_depth or self.nodes > max_nodes) {
            self.fail(error.Invalid);
            return false;
        }
        return true;
    }

    pub fn nullValue(self: *Writer) void {
        if (self.begin()) self.byte(tag_null);
    }

    pub fn boolean(self: *Writer, value: bool) void {
        if (self.begin()) self.byte(if (value) tag_true else tag_false);
    }

    pub fn int(self: *Writer, value: i64) void {
        if (!self.begin()) return;
        self.byte(tag_int);
        const bits: u64 = @bitCast(value);
        const sign: u64 = @bitCast(value >> 63);
        self.leb((bits << 1) ^ sign);
    }

    pub fn float(self: *Writer, value: f64) void {
        if (!self.begin()) return;
        self.byte(tag_float);
        const bits: u64 = @bitCast(value);
        var shift: u6 = 0;
        while (true) {
            self.byte(@truncate(bits >> shift));
            if (shift == 56) break;
            shift += 8;
        }
    }

    fn blob(self: *Writer, tag: u8, data: []const u8) void {
        if (!self.begin()) return;
        self.byte(tag);
        self.leb(data.len);
        self.raw(data);
    }

    pub fn string(self: *Writer, text: []const u8) void {
        self.blob(tag_string, text);
    }

    pub fn bytes(self: *Writer, data: []const u8) void {
        self.blob(tag_bytes, data);
    }

    fn open(self: *Writer, map: bool) void {
        if (!self.begin()) return;
        self.byte(if (map) tag_map else tag_list);
        if (self.failure != null) return;
        self.start[self.depth] = self.len;
        self.count[self.depth] = 0;
        self.is_map[self.depth] = map;
        self.pending_key[self.depth] = false;
        self.depth += 1;
    }

    pub fn beginList(self: *Writer) void {
        self.open(false);
    }

    pub fn beginMap(self: *Writer) void {
        self.open(true);
    }

    /// Closes the innermost list or map, inserting its count in front of the items.
    pub fn end(self: *Writer) void {
        if (self.failure != null) return;
        if (self.depth == 0 or (self.is_map[self.depth - 1] and self.pending_key[self.depth - 1])) {
            self.fail(error.Invalid);
            return;
        }
        const top = self.depth - 1;
        var size: usize = 1;
        var v: u64 = self.count[top];
        while (v >= 0x80) : (v >>= 7) size += 1;
        if (!self.reserve(size)) return;
        var i = self.len;
        while (i > self.start[top]) {
            i -= 1;
            self.buf[i + size] = self.buf[i];
        }
        var c: u64 = self.count[top];
        var at = self.start[top];
        while (c >= 0x80) : (c >>= 7) {
            self.buf[at] = @as(u8, @truncate(c & 0x7F)) | 0x80;
            at += 1;
        }
        self.buf[at] = @truncate(c);
        self.len += size;
        self.depth = top;
    }

    /// The next key of the open map; the value follows.
    pub fn key(self: *Writer, name: []const u8) void {
        if (self.failure != null) return;
        if (self.depth == 0 or !self.is_map[self.depth - 1] or self.pending_key[self.depth - 1] or name.len > max_key_bytes) {
            self.fail(error.Invalid);
            return;
        }
        self.leb(name.len);
        self.raw(name);
        self.count[self.depth - 1] += 1;
        self.pending_key[self.depth - 1] = true;
    }

    /// The finished document, valid until the writer is reused or freed.
    pub fn finish(self: *const Writer) WriteError![]const u8 {
        if (self.failure) |f| return f;
        if (self.depth != 0 or self.nodes == 0) return error.Invalid;
        return self.buf[0..self.len];
    }
};

fn readLeb(b: []const u8, pos: *usize) ?u64 {
    var result: u64 = 0;
    var shift: u32 = 0;
    while (shift < 70) : (shift += 7) {
        if (pos.* >= b.len) return null;
        const byte = b[pos.*];
        pos.* += 1;
        if (shift == 63 and byte > 1) return null;
        result |= @as(u64, byte & 0x7F) << @as(u6, @truncate(shift));
        if (byte & 0x80 == 0) return result;
    }
    return null;
}

/// Steps over one value of a validated document.
fn skip(b: []const u8, start: usize) ?usize {
    if (start >= b.len) return null;
    var pos = start + 1;
    switch (b[start]) {
        tag_int => {
            _ = readLeb(b, &pos) orelse return null;
            return pos;
        },
        tag_float => return if (b.len - pos >= 8) pos + 8 else null,
        tag_string, tag_bytes => {
            const n = readLeb(b, &pos) orelse return null;
            return if (n <= b.len - pos) pos + @as(usize, @intCast(n)) else null;
        },
        tag_list, tag_map => {
            var n = readLeb(b, &pos) orelse return null;
            while (n > 0) : (n -= 1) {
                if (b[start] == tag_map) {
                    const k = readLeb(b, &pos) orelse return null;
                    if (k > b.len - pos) return null;
                    pos += @as(usize, @intCast(k));
                }
                pos = skip(b, pos) orelse return null;
            }
            return pos;
        },
        else => return pos,
    }
}

fn walk(b: []const u8, start: usize, depth: usize, nodes: *usize) ?usize {
    if (depth > max_depth) return null;
    nodes.* += 1;
    if (nodes.* > max_nodes or start >= b.len) return null;
    var pos = start + 1;
    switch (b[start]) {
        tag_null, tag_false, tag_true => return pos,
        tag_int => {
            _ = readLeb(b, &pos) orelse return null;
            return pos;
        },
        tag_float => return if (b.len - pos >= 8) pos + 8 else null,
        tag_string, tag_bytes => {
            const n = readLeb(b, &pos) orelse return null;
            return if (n <= b.len - pos) pos + @as(usize, @intCast(n)) else null;
        },
        tag_list => {
            const n = readLeb(b, &pos) orelse return null;
            if (n > b.len - pos) return null;
            var i: u64 = 0;
            while (i < n) : (i += 1) pos = walk(b, pos, depth + 1, nodes) orelse return null;
            return pos;
        },
        tag_map => {
            const n = readLeb(b, &pos) orelse return null;
            if (n > b.len - pos) return null;
            const first = pos;
            var i: u64 = 0;
            while (i < n) : (i += 1) {
                const klen = readLeb(b, &pos) orelse return null;
                if (klen > max_key_bytes or klen > b.len - pos) return null;
                const name = b[pos .. pos + @as(usize, @intCast(klen))];
                var scan = first;
                var j: u64 = 0;
                while (j < i) : (j += 1) {
                    const olen = readLeb(b, &scan) orelse return null;
                    const other = b[scan .. scan + @as(usize, @intCast(olen))];
                    if (std.mem.eql(u8, other, name)) return null;
                    scan = skip(b, scan + other.len) orelse return null;
                }
                pos = walk(b, pos + name.len, depth + 1, nodes) orelse return null;
            }
            return pos;
        },
        else => return null,
    }
}

pub const Entry = struct { key: []const u8, value: Value };

pub const Items = struct {
    bytes: []const u8,
    pos: usize,
    left: u64,
    is_map: bool,

    pub fn next(self: *Items) ?Value {
        const entry = self.nextEntry() orelse return null;
        return entry.value;
    }

    /// The next item; `key` is empty for a list.
    pub fn nextEntry(self: *Items) ?Entry {
        if (self.left == 0 or self.pos >= self.bytes.len) return null;
        var pos = self.pos;
        var name: []const u8 = &.{};
        if (self.is_map) {
            const klen = readLeb(self.bytes, &pos) orelse return self.stop();
            if (klen > self.bytes.len - pos) return self.stop();
            const n: usize = @intCast(klen);
            name = self.bytes[pos .. pos + n];
            pos += n;
        }
        const item = Value{ .bytes = self.bytes, .off = pos };
        self.pos = skip(self.bytes, pos) orelse self.bytes.len;
        self.left -= 1;
        return .{ .key = name, .value = item };
    }

    fn stop(self: *Items) ?Entry {
        self.left = 0;
        return null;
    }
};

/// A position in a validated document.
pub const Value = struct {
    bytes: []const u8,
    off: usize,

    /// Validates the whole document (tags, limits, duplicate keys, trailing bytes); null when malformed.
    pub fn open(document: []const u8) ?Value {
        if (document.len == 0 or document.len > max_bytes) return null;
        var nodes: usize = 0;
        const end = walk(document, 0, 0, &nodes) orelse return null;
        if (end != document.len) return null;
        return .{ .bytes = document, .off = 0 };
    }

    pub fn kind(self: Value) Kind {
        return switch (self.bytes[self.off]) {
            tag_false, tag_true => .bool,
            tag_int => .int,
            tag_float => .float,
            tag_string => .string,
            tag_bytes => .bytes,
            tag_list => .list,
            tag_map => .map,
            else => .null,
        };
    }

    pub fn isNull(self: Value) bool {
        return self.kind() == .null;
    }

    pub fn asBool(self: Value) ?bool {
        if (self.kind() != .bool) return null;
        return self.bytes[self.off] == tag_true;
    }

    pub fn asInt(self: Value) ?i64 {
        if (self.kind() != .int) return null;
        var pos = self.off + 1;
        const raw = readLeb(self.bytes, &pos) orelse return null;
        const magnitude: u64 = raw >> 1;
        const sign: u64 = 0 -% (raw & 1);
        return @bitCast(magnitude ^ sign);
    }

    pub fn asFloat(self: Value) ?f64 {
        if (self.kind() != .float or self.bytes.len - self.off < 9) return null;
        var bits: u64 = 0;
        var i: u6 = 0;
        while (true) {
            bits |= @as(u64, self.bytes[self.off + 1 + i / 8]) << i;
            if (i == 56) break;
            i += 8;
        }
        return @bitCast(bits);
    }

    fn payload(self: Value, expected: Kind) ?[]const u8 {
        if (self.kind() != expected) return null;
        var pos = self.off + 1;
        const n = readLeb(self.bytes, &pos) orelse return null;
        if (n > self.bytes.len - pos) return null;
        return self.bytes[pos .. pos + @as(usize, @intCast(n))];
    }

    /// The UTF-8 payload of a string, borrowed from the document.
    pub fn asString(self: Value) ?[]const u8 {
        return self.payload(.string);
    }

    pub fn asBytes(self: Value) ?[]const u8 {
        return self.payload(.bytes);
    }

    /// Items of a list or entries of a map; null for any other value.
    pub fn count(self: Value) ?usize {
        const k = self.kind();
        if (k != .list and k != .map) return null;
        var pos = self.off + 1;
        const n = readLeb(self.bytes, &pos) orelse return null;
        return @intCast(n);
    }

    pub fn items(self: Value) Items {
        const k = self.kind();
        var pos = self.off + 1;
        var left: u64 = 0;
        if (k == .list or k == .map) left = readLeb(self.bytes, &pos) orelse 0;
        return .{ .bytes = self.bytes, .pos = pos, .left = left, .is_map = k == .map };
    }

    pub fn at(self: Value, index: usize) ?Value {
        if (self.kind() != .list) return null;
        var it = self.items();
        var i: usize = 0;
        while (it.next()) |item| : (i += 1) {
            if (i == index) return item;
        }
        return null;
    }

    pub fn find(self: Value, name: []const u8) ?Value {
        if (self.kind() != .map) return null;
        var it = self.items();
        while (it.nextEntry()) |entry| {
            if (std.mem.eql(u8, entry.key, name)) return entry.value;
        }
        return null;
    }
};

pub const CallError = error{ Absent, Stale, Invalid, Denied, Failed, TooLarge };

fn callFailure(code: i32) CallError {
    return switch (code) {
        -1 => error.Absent,
        -2 => error.Stale,
        -3 => error.Invalid,
        -4 => error.Denied,
        else => error.Failed,
    };
}

/// Calls `func(context, out, cap)` (an import that answers with the length it needs) with `buffer`, and once more
/// with `spare` when the answer did not fit. Returns the answer, a slice of whichever buffer holds it.
pub fn callOut(context: anytype, comptime func: anytype, buffer: []u8, spare: []u8) CallError![]const u8 {
    var n = func(context, buffer.ptr, @as(i32, @intCast(buffer.len)));
    if (n < 0) return callFailure(n);
    if (@as(usize, @intCast(n)) <= buffer.len) return buffer[0..@intCast(n)];
    if (@as(usize, @intCast(n)) > spare.len) return error.TooLarge;
    n = func(context, spare.ptr, @as(i32, @intCast(spare.len)));
    if (n < 0) return callFailure(n);
    if (@as(usize, @intCast(n)) > spare.len) return error.TooLarge;
    return spare[0..@intCast(n)];
}

/// The same with a heap retry: when the answer needs more than `buffer`, allocates exactly that much and calls
/// again. The caller frees the result when it is not a slice of `buffer`.
pub fn callOutAlloc(allocator: std.mem.Allocator, context: anytype, comptime func: anytype, buffer: []u8) (CallError || error{OutOfMemory})![]const u8 {
    const n = func(context, buffer.ptr, @as(i32, @intCast(buffer.len)));
    if (n < 0) return callFailure(n);
    const need: usize = @intCast(n);
    if (need <= buffer.len) return buffer[0..need];
    if (need > max_bytes) return error.TooLarge;
    const grown = try allocator.alloc(u8, need);
    errdefer allocator.free(grown);
    const again = func(context, grown.ptr, @as(i32, @intCast(grown.len)));
    if (again < 0) return callFailure(again);
    if (@as(usize, @intCast(again)) > grown.len) return error.TooLarge;
    return grown[0..@intCast(again)];
}

/// `callOut` followed by `Value.open`.
pub fn callValue(context: anytype, comptime func: anytype, buffer: []u8, spare: []u8) CallError!Value {
    const document = try callOut(context, func, buffer, spare);
    return Value.open(document) orelse error.Invalid;
}
