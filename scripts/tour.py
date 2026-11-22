"""The worlds tour: one list of calls, rendered as source in each example language (used by gen-worlds.py)."""
from pathlib import Path


class Doc:
    def __init__(self, tree):
        self.tree = tree


class Bytes:
    def __init__(self, data):
        self.data = list(data)


class Zeros:
    def __init__(self, n):
        self.n = n


class Var:
    def __init__(self, name):
        self.name = name


class Keep:
    def __init__(self, name, size):
        self.name, self.size = name, size


class Use:
    def __init__(self, name):
        self.name = name


class Out:
    def __init__(self, n):
        self.n = n


class Cb:
    def __init__(self, name="done"):
        self.name = name


W, P = Var("w"), Var("p")
NBT = Bytes([10, 0, 0, 0])
ENDS = ()


def call(name, *args, assign=None):
    return ("call", name, list(args), assign)


STATIC_ZERO_STATES = Zeros(4096)

TOUR = [
    # worlds and properties
    call("world_by_name", "world", assign="w"),
    call("world_by_name", "nope"),
    call("world_default"),
    call("world_list", Out(8)),
    call("world_name", W, Out(128)),
    call("world_name", 999, Out(128)),
    call("world_get", W, "time", Out(128)),
    call("world_get", W, "nope", Out(128)),
    call("world_set", W, "day_time", Doc(1000)),
    call("world_set", W, "nope", Doc(1)),
    call("world_rule_get", W, "doDaylightCycle", Out(128)),
    call("world_rule_get", W, "nope", Out(128)),
    call("world_rule_set", W, "doDaylightCycle", "false"),
    call("world_at", W, 1.5, 1.5, Cb(), assign="rq"),
    call("request_cancel", Var("rq")),
    call("world_at", W, 2.5, 2.5, Cb()),
    # blocks
    call("world_block_set", W, 0, 64, 0, "minecraft:stone", 0),
    call("world_block_set", W, 0, 64, 0, "nope:block", 0),
    call("world_block_get", W, 0, 64, 0, 0, Out(128)),
    call("world_state_get", W, 0, 64, 0, 0),
    call("world_state_set", W, 1, 64, 0, 3, 0),
    call("world_state_get", 999, 0, 64, 0, 0),
    call("world_states_read", W, [0, 64, 0, 1, 64, 0], Out(2), 0),
    call("world_states_write", W, [2, 64, 0, 3, 64, 0], [5, 5], 0),
    call("world_fill", W, 2, 64, 2, 5, 65, 5, 5, 0),
    call("world_fill", 999, 0, 0, 0, 1, 1, 1, 1, 0),
    call("world_section_read", W, 0, 4, 0, 0, Out(4096)),
    call("world_section_write", W, 0, 4, 0, Zeros(4096), 0),
    call("world_section_write", W, 0, 200, 0, Zeros(4096), 0),
    call("world_chunk_read", W, 0, 0, 0, Out(8)),
    call("world_chunk_apply", W, 0, 0, [4], Zeros(4096), 0, Out(4096)),
    call("world_changes_apply", W, [0, 64, 0], [5], 0),
    call("world_blocks_settle", W, [0, 64, 0], [0], 0),
    call("world_blocks_notify", W, [0, 64, 0], [1, 64, 0]),
    call("world_blocks_reshape", W, [0, 64, 0], 0),
    call("world_state_valid", W, 0, 64, 0, 1),
    call("world_tick_schedule", W, 0, 64, 0, 5, 0),
    call("world_tick_pending", W, 0, 64, 0),
    call("world_state_set", W, 3, 64, 0, 6, 0),
    call("world_block_entity_edit", W, 1, 3, 64, 0, NBT),
    call("world_block_entity_get", W, 3, 64, 0, Out(256)),
    call("world_block_entity_get", W, 9, 64, 9, Out(256)),
    call("world_block_entities", W, 0, 0, Out(8)),
    call("world_data_set", W, 0, 0, 0, 0, NBT),
    call("world_data_get", W, 0, 0, 0, 0, Out(256)),
    call("world_data_get", W, 1, 9, 0, 9, Out(256)),
    # chunks
    call("world_chunk_status", W, 0, 0),
    call("world_chunk_status", W, 9, 9),
    call("world_chunk_load", W, 3, 3, 8),
    call("world_chunk_unload", W, 3, 3),
    call("world_chunk_request", W, 4, 4, 8, 0, Cb()),
    call("world_chunks_loaded", W, Out(16)),
    call("world_chunk_hold", W, "tour", [4, 4]),
    call("world_chunk_held", W, "tour", Out(8)),
    call("world_chunk_release", W, "tour", [4, 4]),
    call("world_chunks_resend", W, [0, 0]),
    call("world_stored_put", W, 7, 7, NBT),
    call("world_stored_get", W, 7, 7, Out(256)),
    call("world_stored_get", W, 8, 8, Out(256)),
    call("world_stored_list", W, 0, 0, Out(8)),
    # light and biomes
    call("world_light_at", W, 0, 64, 0, 0),
    call("world_light_read", W, 0, 4, 0, 1, 1, Out(2048)),
    call("world_light_write", W, 0, 4, 0, 1, Zeros(2048)),
    call("world_light_write", W, 0, 4, 0, 1, NBT),
    call("world_light_check", W, [0, 0], Cb()),
    call("world_biome_get", W, 0, 64, 0, Out(128)),
    call("world_biome_set", W, 0, 64, 0, "minecraft:desert"),
    call("world_biome_set", W, 0, 64, 0, "nope:biome"),
    call("world_biomes_read", W, 0, 0, Out(8)),
    call("world_biomes_write", W, 0, 0, [1, 1, 1, 1]),
    # effects and border
    call("world_sound", W, "minecraft:block.note_block.bell", "block", 0.5, 64.0, 0.5, 1.0, 1.0, 0),
    call("world_sound", W, "", "block", 0.5, 64.0, 0.5, 1.0, 1.0, 0),
    call("world_particle", W, Doc({"type": "minecraft:flame", "x": 0.5, "y": 65.0, "z": 0.5, "count": 3})),
    call("world_particle", W, Doc({"count": 1})),
    call("world_level_event", W, 1000, 0, 64, 0, 0, 0),
    call("world_block_event", W, 0, 64, 0, 1, 2),
    call("world_explode", W, 0.5, 64.0, 0.5, 0.0, 0, 0),
    call("world_lightning", W, 0.5, 64.0, 0.5),
    call("world_game_event", W, "minecraft:block_change", 0.5, 64.0, 0.5, ""),
    call("world_game_event", W, "", 0.5, 64.0, 0.5, ""),
    call("world_player_effect", P, "sound_at", Doc(["minecraft:block.note_block.bell", "block", 0.5, 64.0, 0.5, 1.0, 1.0, 0]), Out(64)),
    call("world_player_effect", P, "nope", Doc([]), Out(64)),
    call("world_border_get", W, Out(512)),
    call("world_border_set", W, Doc({"size": 100.0})),
    call("world_border_set", W, Doc({"size": -1.0, "seconds": -1.0})),
    # long tail
    call("world_call", W, "ops", Doc({}), Out(1024)),
    call("world_call", W, "highest", Doc({"x": 2, "z": 2}), Out(256)),
    call("world_call", W, "nope", Doc({}), Out(256)),
    # structures
    call("structure_capture", W, 0, 64, 0, 3, 2, 3, 0, Keep("s1", 2048)),
    call("structure_put", "tour:a", Use("s1"), 0),
    call("structure_put", "tour:b", NBT, 0),
    call("structure_list", Out(256)),
    call("structure_get", "tour:a", Out(256)),
    call("structure_get", "tour:zzz", Out(256)),
    call("structure_file", "tour:a", Out(128)),
    call("structure_scan", Use("s1"), Doc({"x": 0, "y": 64, "z": 0}), Out(1024)),
    call("structure_place", Use("s1"), Doc({"world": "world", "x": 8, "y": 64, "z": 8})),
    call("structure_remove", "tour:a", 0),
    call("structure_remove", "tour:a", 0),
    # noise
    call("noise_query", Doc({"op": "shape", "settings": "minecraft:overworld", "seed": 0}), Out(256)),
    call("noise_query", Doc({"op": "nope"}), Out(256)),
    call("noise_create", Doc({"id": "minecraft:temperature", "seed": 0}), assign="nz"),
    call("noise_sample", Var("nz"), 0.0, 0.0, 0.0, Out(0)),
    call("noise_info", Var("nz"), Out(128)),
    call("noise_release", Var("nz")),
    call("noise_release", Var("nz")),
    # tables and data fixing
    call("block_state_id", "minecraft:stone"),
    call("block_state_id", "nope:block"),
    call("block_state_text", 1, Out(128)),
    call("worlds_data", "tables", Doc({}), Out(1024)),
    call("worlds_data", "blocks", Doc({}), Out(512)),
    call("worlds_data", "nope", Doc({}), Out(64)),
    call("datafix_call", "types", Doc({}), Out(128)),
    call("datafix_call", "supports", Doc({"type": "chunk"}), Out(64)),
    call("datafix_call", "steps", Doc({"type": "chunk"}), Out(256)),
    call("datafix_call", "missing", Doc({"type": "chunk", "from": 100, "to": 200}), Out(64)),
    call("datafix_call", "legacy_state", Doc({"id": 16}), Out(128)),
    call("datafix_call", "modern_state", Doc({"id": 16}), Out(128)),
    call("datafix_call", "upgrade", Doc({"type": "chunk", "nbt": Bytes([10, 0, 0, 0]), "from": 100, "to": 200}), Out(128)),
    call("datafix_call", "nope", Doc({}), Out(64)),
    call("world_at", W, 3.5, 3.5, Cb("later")),
]

TOUR_B = [
    call("world_create", Doc({"name": "tour"}), assign="w2"),
    call("world_at", Var("w2"), 0.5, 0.5, Cb("later2")),
]

TOUR_C = [
    call("world_create", Doc({"name": "tour"})),
    call("world_unload", "nope", 0),
    call("world_unload", "tour", 0),
    call("world_delete", "tour"),
]

HANDLES = ["w", "p", "w2", "rq", "nz"]


def collect(tour):
    keeps = {}
    for stmt in tour:
        for a in stmt[2]:
            if isinstance(a, Keep):
                keeps[a.name] = a.size
    return keeps


def tree_ops(node):
    """The writer operations of a document tree: ('map',) ('list',) ('end',) ('key', k) ('int', v) ..."""
    if isinstance(node, dict):
        ops = [("map",)]
        for k, v in node.items():
            ops.append(("key", k))
            ops += tree_ops(v)
        return ops + [("end",)]
    if isinstance(node, list):
        ops = [("list",)]
        for v in node:
            ops += tree_ops(v)
        return ops + [("end",)]
    if isinstance(node, bool):
        return [("bool", node)]
    if isinstance(node, int):
        return [("int", node)]
    if isinstance(node, float):
        return [("float", node)]
    if isinstance(node, Bytes):
        return [("bytes", node.data)]
    return [("str", node)]


def stride_of(name):
    return 3 if name in ("positions", "sources") else 2 if name == "chunks" else 1


def isnum(kind):
    return kind in ("i32", "i64", "f32", "f64")


class TL:
    explicit_counts = False
    use_clamp = "kl"

    def arr_len(self, v, name):
        return len(v) // stride_of(name) if isinstance(v, list) else v.n

    def scalar(self, kind, v):
        if isinstance(v, Var):
            return v.name
        if kind in ("f32", "f64"):
            return self.fl(kind, float(v))
        return self.il(kind, int(v))

    def il(self, kind, v):
        return str(v)

    def fl(self, kind, v):
        return repr(v)

    def render(self, tour, plans):
        keeps = collect(tour)
        later = self.later_fn(self.body(TOUR_B, plans, keeps, 1000))
        second = self.later_fn(self.body(TOUR_C, plans, keeps, 2000))
        later = second.replace("later", "later2").replace("Later", "Later2") + later
        return (self.head().replace("//LATER\n", later) + self.decls(keeps) + self.body(tour, plans, keeps, 0)
                + self.tail(keeps))

    def hname(self, text, v):
        return text.replace("done", v.name).replace("Done", v.name.capitalize())

    def body(self, tour, plans, keeps, base):
        body = []
        for idx0, (_, name, args, assign) in enumerate(tour):
            idx = base + idx0
            imp, owner, items = plans[name]
            pub = name if owner == "worlds" else "world_" + name
            pre, parts, ai, arr, keep = [], [], 0, None, None
            for kind, pname, extra in items:
                if kind == "count":
                    if self.explicit_counts:
                        parts.append(str(arr))
                    continue
                v = args[ai]
                ai += 1
                if kind == "ints" and arr is None:
                    arr = self.arr_len(v, pname)
                if kind == "out" and isinstance(v, Keep):
                    keep = v
                if isnum(kind):
                    parts.append(self.scalar(kind, v))
                    continue
                p, e = self.arg(kind, pname, extra, v, idx * 10 + len(parts), keeps)
                if kind == "handler":
                    e = self.hname(e, v)
                pre += p
                parts.append(e)
            target = self.call(pub, name, parts)
            if assign:
                stmt = self.assign(assign, target)
            elif keep is not None:
                stmt = self.assign(keep.name + "n", target)
            else:
                stmt = self.discard(target)
            body.append(self.block(pre, stmt))
        return "".join(body)

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "    {\n" + "".join("        %s\n" % l for l in lines) + "    }\n"

    def assign(self, var, target):
        return "%s = %s;" % (var, target)

    def discard(self, target):
        return target + ";"


# ----------------------------------------------------------------------------------------------------------- C++

class TCpp(TL):
    key = "cpp"
    path = "src/worlds/worlds.cpp"
    explicit_counts = True

    def il(self, kind, v):
        return "%dLL" % v if kind == "i64" else str(v)

    def fl(self, kind, v):
        return repr(v) + ("f" if kind == "f32" else "")

    def call(self, pub, name, parts):
        return "basalt::server::worlds::%s(%s)" % (pub, ", ".join(parts))

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                pre = ["basalt::server::values::ValueBuffer<512> d%d;" % n]
                chain = "d%d" % n
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".map()", "list": ".list()", "end": ".end()"}.get(t, "")
                    if t == "key":
                        chain += '.key("%s")' % op[1]
                    elif t == "int":
                        chain += ".integer(%d)" % op[1]
                    elif t == "float":
                        chain += ".number(%r)" % op[1]
                    elif t == "bool":
                        chain += ".boolean(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.string("%s")' % op[1]
                    elif t == "bytes":
                        pre.append("static const char blob%d[] = {%s};" % (n, ", ".join(map(str, op[1]))))
                        chain += ".bytes(blob%d, %d)" % (n, len(op[1]))
                return pre + [chain + ";"], "d%d.data(), d%d.finish()" % (n, n)
            if isinstance(v, Use):
                return [], "%s, kl(%sn, %d)" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return ["char z%d[%d] = {};" % (n, v.n)], "z%d, %d" % (n, v.n)
            return (["static const char b%d[] = {%s};" % (n, ", ".join(map(str, v.data)))], "b%d, %d" % (n, len(v.data)))
        if kind == "out":
            if isinstance(v, Keep):
                return [], "%s, %d" % (v.name, v.size)
            return ["char o%d[%d];" % (n, max(v.n, 1))], "o%d, %d" % (n, v.n)
        if kind == "ints":
            if isinstance(v, Zeros):
                return ["int32_t i%d[%d] = {};" % (n, v.n)], "i%d" % n
            return ["static const int32_t i%d[] = {%s};" % (n, ", ".join(map(str, v)))], "i%d" % n
        if kind == "intsout":
            return ["int32_t o%d[%d] = {};" % (n, v.n)], "o%d" % n + (", %d" % v.n if extra else "")
        if kind == "f64out":
            return ["double f%d = 0;" % n], "&f%d" % n
        return [], "done"

    def later_fn(self, body):
        return "void later(basalt::server::core::Ctx) {\n    int32_t w2 = 0;\n" + body + "    static_cast<void>(w2);\n}\n\n"

    def head(self):
        return '''#include "worlds/worlds.hpp"

#include "common/text.hpp"

namespace worlds {

namespace {

int32_t kl(int32_t length, int32_t capacity) { return length > 0 && length <= capacity ? length : 0; }

void done(basalt::server::core::Ctx ctx) {
    char out[16];
    basalt::server::worlds::world_value_result(ctx, out, 16);
}

//LATER
void tour(basalt::server::core::Ctx) {
    basalt::server::worlds::chunk_coord(-17);
    basalt::server::worlds::block_distance(0, 0, 0, 3, 4, 0);
    int32_t pos[3] = {1, 2, 3};
    basalt::server::worlds::pos_offset(pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = "    int32_t w = 0, p = basalt::server::players::player_by_name(\"Steve\"), w2 = 0, rq = 0, nz = 0;\n"
        for k, s in keeps.items():
            text += "    char %s[%d] = {};\n    int32_t %sn = 0;\n" % (k, s, k)
        return text

    def tail(self, keeps):
        return '''    static_cast<void>(w2);
    static_cast<void>(rq);
    static_cast<void>(nz);
}

} // namespace

void register_all() { basalt::server::commands::command("worlds", "", tour); }

} // namespace worlds
'''


# ----------------------------------------------------------------------------------------------------------- Zig

class TZig(TL):
    key = "zig"
    path = "src/worlds/worlds.zig"

    def call(self, pub, name, parts):
        return "worlds.%s(%s)" % (camelcase(pub), ", ".join(parts))

    def discard(self, target):
        return "_ = %s;" % target

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                pre = ["var dbuf%d: [512]u8 = undefined;" % n, "var d%d = values.Writer.init(&dbuf%d);" % (n, n)]
                for op in tree_ops(v.tree):
                    t = op[0]
                    pre.append({"map": "d%d.beginMap();" % n, "list": "d%d.beginList();" % n, "end": "d%d.end();" % n}.get(t, "") or
                               {"key": 'd%d.key("%s");' % (n, op[1]) if t == "key" else "",
                                "int": "d%d.int(%s);" % (n, op[1]) if t == "int" else "",
                                "float": "d%d.float(%r);" % (n, op[1]) if t == "float" else "",
                                "bool": "d%d.boolean(%s);" % (n, "true" if t == "bool" and op[1] else "false") if t == "bool" else "",
                                "str": 'd%d.string("%s");' % (n, op[1]) if t == "str" else "",
                                "bytes": "d%d.bytes(&[_]u8{%s});" % (n, ", ".join(map(str, op[1]))) if t == "bytes" else ""}[t])
                pre.append("const d%dd = d%d.finish() catch dbuf%d[0..0];" % (n, n, n))
                return pre, "d%dd" % n
            if isinstance(v, Use):
                return [], "%s[0..kl(%sn, %d)]" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return ["const z%d: [%d]u8 = @splat(0);" % (n, v.n)], "&z%d" % n
            return [], "&[_]u8{%s}" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], "&%s" % v.name
            return ["var o%d: [%d]u8 = @splat(0);" % (n, max(v.n, 1))], "&o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return ["const ia%d: [%d]i32 = @splat(0);" % (n, v.n)], "&ia%d" % n
            return [], "&[_]i32{%s}" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["var o%d: [%d]i32 = @splat(0);" % (n, v.n)], "&o%d" % n
        if kind == "f64out":
            return ["var fl%d: f64 = 0;" % n], "&fl%d" % n
        return [], "done"

    def later_fn(self, body):
        return "fn later(_: basalt.core.Ctx) void {\n    var w2: i32 = 0;\n" + body + "    _ = &w2;\n}\n\n"

    def head(self):
        return '''const basalt = @import("basalt_server");
const worlds = basalt.worlds;
const values = basalt.values;

fn kl(length: i32, capacity: i32) usize {
    return if (length > 0 and length <= capacity) @intCast(length) else 0;
}

fn done(ctx: basalt.core.Ctx) void {
    var out: [16]u8 = @splat(0);
    _ = worlds.worldValueResult(ctx, &out);
}

//LATER
fn tour(_: basalt.core.Ctx) void {
    _ = worlds.chunkCoord(-17);
    _ = worlds.blockDistance(0, 0, 0, 3, 4, 0);
    var pos = [3]i32{ 1, 2, 3 };
    worlds.posOffset(&pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = '    var w: i32 = 0;\n    var p: i32 = basalt.players.byName("Steve");\n    var w2: i32 = 0;\n    var rq: i32 = 0;\n    var nz: i32 = 0;\n'
        for k, s in keeps.items():
            text += "    var %s: [%d]u8 = @splat(0);\n    var %sn: i32 = 0;\n" % (k, s, k)
        return text

    def tail(self, keeps):
        extra = "".join("    _ = &%sn;\n    _ = &%s;\n" % (k, k) for k in keeps)
        return "    _ = &w;\n    _ = &p;\n    _ = &w2;\n    _ = &rq;\n    _ = &nz;\n" + extra + '''}

pub fn register() void {
    _ = basalt.commands.register("worlds", "", tour);
}
'''


def camelcase(n):
    w = n.split("_")
    return w[0] + "".join(x.capitalize() for x in w[1:])


def pascalcase(n):
    return "".join(x.capitalize() for x in n.split("_"))


# ---------------------------------------------------------------------------------------------------------- Rust

class TRust(TL):
    key = "rust"
    path = "src/worlds/mod.rs"

    def call(self, pub, name, parts):
        return "worlds::%s(%s)" % (pub, ", ".join(parts))

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "d%d" % n
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".begin_map()", "list": ".begin_list()", "end": ".end()"}.get(t, "")
                    if t == "key":
                        chain += '.key("%s")' % op[1]
                    elif t == "int":
                        chain += ".int(%d)" % op[1]
                    elif t == "float":
                        chain += ".float(%r)" % op[1]
                    elif t == "bool":
                        chain += ".boolean(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.string("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".bytes(&[%s])" % ", ".join("%du8" % b for b in op[1])
                return ["let mut d%d = Writer::new();" % n, chain + ";"], "d%d.finish().unwrap_or(&[])" % n
            if isinstance(v, Use):
                return [], "&%s[..kl(%sn, %d)]" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "&[0u8; %d]" % v.n
            return [], "&[%s]" % ", ".join("%du8" % b for b in v.data)
        if kind == "out":
            if isinstance(v, Keep):
                return [], "&mut %s" % v.name
            return ["let mut o%d = [0u8; %d];" % (n, max(v.n, 1))], "&mut o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "&[0i32; %d]" % v.n
            return [], "&[%s]" % ", ".join("%di32" % x for x in v)
        if kind == "intsout":
            return ["let mut o%d = [0i32; %d];" % (n, v.n)], "&mut o%d" % n
        if kind == "f64out":
            return ["let mut f%d = 0.0f64;" % n], "&mut f%d" % n
        return [], "done"

    def later_fn(self, body):
        return "fn later(_: Ctx) {\n    let mut w2: i32 = 0;\n" + body + "    let _ = w2;\n}\n\n"

    def head(self):
        return '''use basalt_server::{values::Writer, worlds};
use basalt_server::core::Ctx;

fn kl(length: i32, capacity: usize) -> usize {
    if length > 0 && (length as usize) <= capacity {
        length as usize
    } else {
        0
    }
}

fn done(ctx: Ctx) {
    let mut out = [0u8; 16];
    worlds::world_value_result(ctx, &mut out);
}

//LATER
fn tour(_: Ctx) {
    worlds::chunk_coord(-17);
    worlds::block_distance(0, 0, 0, 3, 4, 0);
    let mut pos = [1, 2, 3];
    worlds::pos_offset(&mut pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = '    let mut w: i32 = 0;\n    let p: i32 = basalt_server::players::by_name("Steve");\n    let mut w2: i32 = 0;\n    let mut rq: i32 = 0;\n    let mut nz: i32 = 0;\n'
        for k, s in keeps.items():
            text += "    let mut %s = [0u8; %d];\n    let mut %sn: i32 = 0;\n" % (k, s, k)
        return text

    def tail(self, keeps):
        return '''    let _ = (w, p, w2, rq, nz);
}

pub fn register() {
    basalt_server::commands::register("worlds", "", tour);
}
'''


# ------------------------------------------------------------------------------------------------------------ Go

class TGo(TL):
    key = "go"
    path = "internal/worlds/worlds.go"

    def call(self, pub, name, parts):
        return "sdkWorlds.%s(%s)" % (pascalcase(pub), ", ".join(parts))

    def discard(self, target):
        return target

    def assign(self, var, target):
        return "%s = %s" % (var, target)

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "sdkValues.NewValueWriterLimit(512)"
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".BeginMap()", "list": ".BeginList()", "end": ".End()"}.get(t, "")
                    if t == "key":
                        chain += '.Key("%s")' % op[1]
                    elif t == "int":
                        chain += ".Int(%d)" % op[1]
                    elif t == "float":
                        chain += ".Float(%r)" % op[1]
                    elif t == "bool":
                        chain += ".Bool(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.String("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".Bytes([]byte{%s})" % ", ".join(map(str, op[1]))
                return ["d%d, _ := %s.Finish()" % (n, chain)], "d%d" % n
            if isinstance(v, Use):
                return [], "%s[:kl(%sn, %d)]" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "make([]byte, %d)" % v.n
            return [], "[]byte{%s}" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], v.name
            return ["o%d := make([]byte, %d)" % (n, v.n)], "o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "make([]int32, %d)" % v.n
            return [], "[]int32{%s}" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["o%d := make([]int32, %d)" % (n, v.n)], "o%d" % n
        if kind == "f64out":
            return ["var f%d float64" % n], "&f%d" % n
        return [], "done"

    def later_fn(self, body):
        return "func later(sdkCore.Ctx) {\n\tvar w2 int32\n" + body + "\t_ = w2\n}\n\n"

    def head(self):
        return '''package worlds

import (
    sdkCore "basaltmc.org/sdk/server/core"
    sdkCommands "basaltmc.org/sdk/server/commands"
    sdkPlayers "basaltmc.org/sdk/server/players"
    sdkValues "basaltmc.org/sdk/server/values"
    sdkWorlds "basaltmc.org/sdk/server/worlds"
)

func kl(length, capacity int32) int32 {
	if length > 0 && length <= capacity {
		return length
	}
	return 0
}

func done(ctx sdkCore.Ctx) {
	out := make([]byte, 16)
	sdkWorlds.WorldValueResult(int32(ctx), out)
}

//LATER
func tour(sdkCore.Ctx) {
	sdkWorlds.ChunkCoord(-17)
	sdkWorlds.BlockDistance(0, 0, 0, 3, 4, 0)
	pos := [3]int32{1, 2, 3}
	sdkWorlds.PosOffset(&pos, 1, 1, 1)
'''

    def decls(self, keeps):
        text = '\tvar w, w2, rq, nz int32\n\tp := int32(sdkPlayers.PlayerByName("Steve"))\n'
        for k, s in keeps.items():
            text += "\t%s := make([]byte, %d)\n\tvar %sn int32\n" % (k, s, k)
        return text

    def tail(self, keeps):
        extra = "".join("\t_ = %sn\n" % k for k in keeps)
        return "\t_, _, _, _, _ = w, p, w2, rq, nz\n" + extra + "}\n\nfunc Register() { sdkCommands.Command(\"worlds\", \"\", tour) }\n"

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "\t{\n" + "".join("\t\t%s\n" % l for l in lines) + "\t}\n"


# ---------------------------------------------------------------------------------------------------------- Java

class TJava(TL):
    key = "java"
    path = "src/org/basaltmc/server/example/worlds/Worlds.java"

    def il(self, kind, v):
        return "%dL" % v if kind == "i64" else str(v)

    def fl(self, kind, v):
        return repr(v) + ("f" if kind == "f32" else "")

    def call(self, pub, name, parts):
        return "org.basaltmc.server.worlds.Worlds.%s(%s)" % (camelcase(pub), ", ".join(parts))

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "new org.basaltmc.server.values.Values.Writer(512)"
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".beginMap()", "list": ".beginList()", "end": ".end()"}.get(t, "")
                    if t == "key":
                        chain += '.key("%s")' % op[1]
                    elif t == "int":
                        chain += ".integer(%d)" % op[1]
                    elif t == "float":
                        chain += ".number(%r)" % op[1]
                    elif t == "bool":
                        chain += ".bool(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.string("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".bytes(new byte[] {%s})" % ", ".join(map(str, op[1]))
                return ["byte[] d%d = %s.finish();" % (n, chain)], "d%d" % n
            if isinstance(v, Use):
                return [], "cut(%s, kl(%sn, %d))" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "new byte[%d]" % v.n
            return [], "new byte[] {%s}" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], v.name
            return ["byte[] o%d = new byte[%d];" % (n, v.n)], "o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "new int[%d]" % v.n
            return [], "new int[] {%s}" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["int[] o%d = new int[%d];" % (n, v.n)], "o%d" % n
        if kind == "f64out":
            return ["double[] f%d = new double[1];" % n], "f%d" % n
        return [], "Worlds::done"

    def later_fn(self, body):
        return "    private static void later(int ctx) {\n        int w2 = 0;\n" + body + "        if (w2 < 0) {\n            org.basaltmc.server.log.Log.logDebug(\"unreachable\");\n        }\n    }\n\n"

    def head(self):
        return '''package org.basaltmc.server.example.worlds;

public final class Worlds {
    private Worlds() {}

    private static int kl(int length, int capacity) {
        return length > 0 && length <= capacity ? length : 0;
    }

    private static byte[] cut(byte[] data, int length) {
        byte[] out = new byte[length];
        System.arraycopy(data, 0, out, 0, length);
        return out;
    }

    private static void done(int ctx) {
        byte[] out = new byte[16];
        org.basaltmc.server.worlds.Worlds.worldValueResult(ctx, out);
    }

//LATER
    private static void tour(int ctx) {
        org.basaltmc.server.worlds.Worlds.chunkCoord(-17);
        org.basaltmc.server.worlds.Worlds.blockDistance(0, 0, 0, 3, 4, 0);
        int[] pos = {1, 2, 3};
        org.basaltmc.server.worlds.Worlds.posOffset(pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = '        int w = 0, p = org.basaltmc.server.players.Players.playerByName("Steve"), w2 = 0, rq = 0, nz = 0;\n'
        for k, s in keeps.items():
            text += "        byte[] %s = new byte[%d];\n        int %sn = 0;\n" % (k, s, k)
        return text

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "        {\n" + "".join("            %s\n" % l for l in lines) + "        }\n"

    def tail(self, keeps):
        return '''    }

    public static void register() {
        org.basaltmc.server.commands.Commands.command("worlds", "", Worlds::tour);
    }
}
'''


# -------------------------------------------------------------------------------------------------------- Kotlin

class TKotlin(TL):
    key = "kotlin"
    path = "src/org/basaltmc/server/example/worlds/Worlds.kt"

    def il(self, kind, v):
        return "%dL" % v if kind == "i64" else str(v)

    def fl(self, kind, v):
        return repr(v) + ("f" if kind == "f32" else "")

    def call(self, pub, name, parts):
        return "org.basaltmc.server.worlds.%s(%s)" % (camelcase(pub), ", ".join(parts))

    def assign(self, var, target):
        return "%s = %s" % (var, target)

    def discard(self, target):
        return target

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "org.basaltmc.server.values.ValueWriter(512)"
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".beginMap()", "list": ".beginList()", "end": ".end()"}.get(t, "")
                    if t == "key":
                        chain += '.key("%s")' % op[1]
                    elif t == "int":
                        chain += ".int(%dL)" % op[1]
                    elif t == "float":
                        chain += ".float(%r)" % op[1]
                    elif t == "bool":
                        chain += ".bool(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.string("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".bytes(byteArrayOf(%s))" % ", ".join(map(str, op[1]))
                return ["val d%d = %s.finish() ?: ByteArray(0)" % (n, chain)], "d%d" % n
            if isinstance(v, Use):
                return [], "%s.copyOf(kl(%sn, %d))" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "ByteArray(%d)" % v.n
            return [], "byteArrayOf(%s)" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], v.name
            return ["val o%d = ByteArray(%d)" % (n, v.n)], "o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "IntArray(%d)" % v.n
            return [], "intArrayOf(%s)" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["val o%d = IntArray(%d)" % (n, v.n)], "o%d" % n
        if kind == "f64out":
            return ["val f%d = DoubleArray(1)" % n], "f%d" % n
        return [], "::done"

    def later_fn(self, body):
        return "    private fun later(ctx: Ctx) {\n        var w2 = 0\n" + body + "        if (w2 < 0) org.basaltmc.server.log.logDebug(\"unreachable\")\n    }\n\n"

    def head(self):
        return '''package org.basaltmc.server.example.worlds

import org.basaltmc.server.core.Ctx

object Worlds {
    private fun kl(length: Int, capacity: Int): Int = if (length > 0 && length <= capacity) length else 0

    private fun done(ctx: Ctx) {
        val out = ByteArray(16)
        org.basaltmc.server.worlds.worldValueResult(ctx, out)
    }

//LATER
    private fun tour(ctx: Ctx) {
        org.basaltmc.server.worlds.chunkCoord(-17)
        org.basaltmc.server.worlds.blockDistance(0, 0, 0, 3, 4, 0)
        val pos = intArrayOf(1, 2, 3)
        org.basaltmc.server.worlds.posOffset(pos, 1, 1, 1)
'''

    def decls(self, keeps):
        text = '        var w = 0\n        val p = org.basaltmc.server.players.playerByName("Steve")\n        var w2 = 0\n        var rq = 0\n        var nz = 0\n'
        for k, s in keeps.items():
            text += "        val %s = ByteArray(%d)\n        var %sn = 0\n" % (k, s, k)
        return text

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "        run {\n" + "".join("            %s\n" % l for l in lines) + "        }\n"

    def tail(self, keeps):
        return '''        if (w + p + w2 + rq + nz < 0) org.basaltmc.server.log.logDebug("unreachable")
    }

    fun register() {
        org.basaltmc.server.commands.command("worlds", "", ::tour)
    }
}
'''


# -------------------------------------------------------------------------------------------------------- C#

class TCSharp(TL):
    key = "csharp"
    path = "src/Worlds/Worlds.cs"

    def il(self, kind, v):
        return "%dL" % v if kind == "i64" else str(v)

    def fl(self, kind, v):
        return repr(v) + ("f" if kind == "f32" else "")

    def call(self, pub, name, parts):
        return "SdkWorlds.%s(%s)" % (pascalcase(pub), ", ".join(parts))

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "new ValueWriter(512)"
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".BeginMap()", "list": ".BeginList()", "end": ".End()"}.get(t, "")
                    if t == "key":
                        chain += '.Key("%s")' % op[1]
                    elif t == "int":
                        chain += ".Int(%d)" % op[1]
                    elif t == "float":
                        chain += ".Float(%r)" % op[1]
                    elif t == "bool":
                        chain += ".Bool(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.String("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".Bytes(new byte[] {%s})" % ", ".join(map(str, op[1]))
                return ["byte[] d%d = %s.Finish();" % (n, chain)], "d%d" % n
            if isinstance(v, Use):
                return [], "Cut(%s, Kl(%sn, %d))" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "new byte[%d]" % v.n
            return [], "new byte[] {%s}" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], v.name
            return ["byte[] o%d = new byte[%d];" % (n, v.n)], "o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "new int[%d]" % v.n
            return [], "new int[] {%s}" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["int[] o%d = new int[%d];" % (n, v.n)], "o%d" % n
        if kind == "f64out":
            return ["double[] f%d = new double[1];" % n], "f%d" % n
        return [], "Done"

    def later_fn(self, body):
        return "    static void Later(int ctx)\n    {\n        int w2 = 0;\n" + body + "        if (w2 < 0)\n        {\n            SdkLog.LogDebug(\"unreachable\");\n        }\n    }\n\n"

    def head(self):
        return '''using Basalt.Server.Values;
using SdkWorlds = Basalt.Server.Worlds.Worlds;
using SdkPlayers = Basalt.Server.Players.Players;
using SdkCommands = Basalt.Server.Commands.Commands;
using SdkLog = Basalt.Server.Log.Logger;

namespace Basalt.Server.Example.Worlds;

public static class WorldsTour
{
    static int Kl(int length, int capacity) => length > 0 && length <= capacity ? length : 0;

    static byte[] Cut(byte[] data, int length)
    {
        byte[] result = new byte[length];
        System.Array.Copy(data, result, length);
        return result;
    }

    static void Done(int ctx)
    {
        byte[] output = new byte[16];
        SdkWorlds.WorldValueResult(ctx, output);
    }

//LATER
    static void Tour(int ctx)
    {
        SdkWorlds.ChunkCoord(-17);
        SdkWorlds.BlockDistance(0, 0, 0, 3, 4, 0);
        int[] pos = { 1, 2, 3 };
        SdkWorlds.PosOffset(pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = '        int w = 0, p = SdkPlayers.PlayerByName("Steve"), w2 = 0, rq = 0, nz = 0;\n'
        for k, s in keeps.items():
            text += "        byte[] %s = new byte[%d];\n        int %sn = 0;\n" % (k, s, k)
        return text

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "        {\n" + "".join("            %s\n" % l for l in lines) + "        }\n"

    def tail(self, keeps):
        return '''        if (w + p + w2 + rq + nz < 0)
        {
            SdkLog.LogDebug("unreachable");
        }
    }

    public static void Register() => SdkCommands.Command("worlds", "", Tour);
}
'''


# ---------------------------------------------------------------------------------------------------- AssemblyScript

class TAS(TL):
    key = "assemblyscript"
    path = "src/worlds/worlds.ts"

    def call(self, pub, name, parts):
        return "bsWorlds.%s(%s)" % (camelcase(pub), ", ".join(parts))

    def arg(self, kind, name, extra, v, n, keeps):
        if kind == "str":
            return [], '"%s"' % v
        if kind == "bytes":
            if isinstance(v, Doc):
                chain = "new bsValues.ValueWriter(512)"
                for op in tree_ops(v.tree):
                    t = op[0]
                    chain += {"map": ".beginMap()", "list": ".beginList()", "end": ".end()"}.get(t, "")
                    if t == "key":
                        chain += '.key("%s")' % op[1]
                    elif t == "int":
                        chain += ".int(%d)" % op[1]
                    elif t == "float":
                        chain += ".float(%r)" % op[1]
                    elif t == "bool":
                        chain += ".boolean(%s)" % ("true" if op[1] else "false")
                    elif t == "str":
                        chain += '.string("%s")' % op[1]
                    elif t == "bytes":
                        chain += ".bytes(bytesOf([%s]))" % ", ".join(map(str, op[1]))
                return ["const d%d = %s.finish()!;" % (n, chain)], "d%d" % n
            if isinstance(v, Use):
                return [], "%s.slice(0, kl(%sn, %d))" % (v.name, v.name, keeps[v.name])
            if isinstance(v, Zeros):
                return [], "new Uint8Array(%d)" % v.n
            return [], "bytesOf([%s])" % ", ".join(map(str, v.data))
        if kind == "out":
            if isinstance(v, Keep):
                return [], v.name
            return ["const o%d = new Uint8Array(%d);" % (n, max(v.n, 1))], "o%d" % n
        if kind == "ints":
            if isinstance(v, Zeros):
                return [], "new Int32Array(%d)" % v.n
            return [], "i32s([%s])" % ", ".join(map(str, v))
        if kind == "intsout":
            return ["const o%d = new Int32Array(%d);" % (n, max(v.n, 1))], "o%d" % n
        if kind == "f64out":
            return ["const f%d = new Float64Array(1);" % n], "f%d" % n
        return [], "done"

    def later_fn(self, body):
        return "function later(ctx: bsCore.Ctx): void {\n  let w2: i32 = 0;\n" + body + "  if (w2 < 0) bsLog.logDebug(\"unreachable\");\n}\n\n"

    def head(self):
        return '''import * as bsCore from "basalt-server/core";
import * as bsCommands from "basalt-server/commands";
import * as bsLog from "basalt-server/log";
import * as bsPlayers from "basalt-server/players";
import * as bsValues from "basalt-server/values";
import * as bsWorlds from "basalt-server/worlds";

function kl(length: i32, capacity: i32): i32 {
  return length > 0 && length <= capacity ? length : 0;
}

function bytesOf(values: i32[]): Uint8Array {
  const out = new Uint8Array(values.length);
  for (let i = 0; i < values.length; i++) out[i] = <u8>values[i];
  return out;
}

function i32s(values: i32[]): Int32Array {
  const out = new Int32Array(values.length);
  for (let i = 0; i < values.length; i++) out[i] = values[i];
  return out;
}

function done(ctx: bsCore.Ctx): void {
  bsWorlds.worldValueResult(ctx, new Uint8Array(16));
}

//LATER
function tour(ctx: bsCore.Ctx): void {
  bsWorlds.chunkCoord(-17);
  bsWorlds.blockDistance(0, 0, 0, 3, 4, 0);
  const pos = i32s([1, 2, 3]);
  bsWorlds.posOffset(pos, 1, 1, 1);
'''

    def decls(self, keeps):
        text = '  let w: i32 = 0;\n  const p: i32 = bsPlayers.playerByName("Steve");\n  let w2: i32 = 0;\n  let rq: i32 = 0;\n  let nz: i32 = 0;\n'
        for k, s in keeps.items():
            text += "  const %s = new Uint8Array(%d);\n  let %sn: i32 = 0;\n" % (k, s, k)
        return text

    def block(self, pre, stmt):
        lines = pre + [stmt]
        return "  {\n" + "".join("    %s\n" % l for l in lines) + "  }\n"

    def tail(self, keeps):
        return '''  if (w + p + w2 + rq + nz < 0) bsLog.logDebug("unreachable");
}

export function register(): void {
  bsCommands.command("worlds", "", tour);
}
'''


TOURS = [TCpp(), TZig(), TRust(), TGo(), TJava(), TKotlin(), TCSharp(), TAS()]


def write(base, imports, plan, gen):
    plans = {imp.name: (imp, owner, plan(imp)) for imp, owner in imports}
    for t in TOURS:
        dest = Path(base) / ("ServerPlugin-Example-worlds-" + t.key)
        if not dest.is_dir():
            continue
        out = dest / t.path
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(t.render(TOUR, plans))
        print("wrote", out)
