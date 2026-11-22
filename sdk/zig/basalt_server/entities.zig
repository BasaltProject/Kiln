//! Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).
const core = @import("core.zig");
const raw = @import("raw/entities.zig").raw;

fn len(s: []const u8) i32 {
    return @intCast(s.len);
}

fn cap(out: []u8) i32 {
    return @intCast(out.len);
}

pub fn ofPlayer(who: i32) i32 {
    return raw.entity_of_player(who);
}

pub fn player(entity: i32) i32 {
    return raw.entity_player(entity);
}

pub fn release(entity: i32) i32 {
    return raw.entity_release(entity);
}

pub fn alive(entity: i32) i32 {
    return raw.entity_alive(entity);
}

pub fn id(entity: i32) i32 {
    return raw.entity_id(entity);
}

pub fn byId(world: []const u8, number: i32) i32 {
    return raw.entity_by_id(world.ptr, len(world), number);
}

pub fn byUuid(uuid: []const u8) i32 {
    return raw.entity_by_uuid(uuid.ptr, len(uuid));
}

pub fn query(world: []const u8, filter: []const u8, out: []i32) i32 {
    return raw.entity_query(world.ptr, len(world), filter.ptr, len(filter), out.ptr, @intCast(out.len));
}

pub fn types(out: []u8) i32 {
    return raw.entity_types(out.ptr, cap(out));
}

pub fn typeAttributes(kind: []const u8, out: []u8) i32 {
    return raw.entity_type_attributes(kind.ptr, len(kind), out.ptr, cap(out));
}

pub fn spawn(world: []const u8, kind: []const u8, x: f64, y: f64, z: f64, opts: []const u8) i32 {
    return raw.entity_spawn(world.ptr, len(world), kind.ptr, len(kind), x, y, z, opts.ptr, len(opts));
}

pub fn spawnNbt(world: []const u8, nbt: []const u8, cause: []const u8) i32 {
    return raw.entity_spawn_nbt(world.ptr, len(world), nbt.ptr, len(nbt), cause.ptr, len(cause));
}

pub fn dropItem(world: []const u8, item: []const u8, x: f64, y: f64, z: f64, vx: f64, vy: f64, vz: f64) i32 {
    return raw.entity_drop_item(world.ptr, len(world), item.ptr, len(item), x, y, z, vx, vy, vz);
}

pub fn remove(entity: i32) i32 {
    return raw.entity_remove(entity);
}

pub fn removeUuid(world: []const u8, uuid: []const u8) i32 {
    return raw.entity_remove_uuid(world.ptr, len(world), uuid.ptr, len(uuid));
}

pub fn position(entity: i32, out: *[3]f64) i32 {
    return raw.entity_position(entity, @ptrCast(out));
}

pub fn teleport(entity: i32, x: f64, y: f64, z: f64, opts: []const u8) i32 {
    return raw.entity_teleport(entity, x, y, z, opts.ptr, len(opts));
}

pub fn velocity(entity: i32, out: *[3]f64) i32 {
    return raw.entity_velocity(entity, @ptrCast(out));
}

pub fn setVelocity(entity: i32, x: f64, y: f64, z: f64) i32 {
    return raw.entity_set_velocity(entity, x, y, z);
}

pub fn rotation(entity: i32, out: *[2]f64) i32 {
    return raw.entity_rotation(entity, @ptrCast(out));
}

pub fn setRotation(entity: i32, yaw: f64, pitch: f64) i32 {
    return raw.entity_set_rotation(entity, yaw, pitch);
}

pub fn get(entity: i32, name: []const u8, out: []u8) i32 {
    return raw.entity_get(entity, name.ptr, len(name), out.ptr, cap(out));
}

pub fn set(entity: i32, name: []const u8, value: []const u8) i32 {
    return raw.entity_set(entity, name.ptr, len(name), value.ptr, len(value));
}

pub fn call(entity: i32, op: []const u8, args: []const u8, out: []u8) i32 {
    return raw.entity_call(entity, op.ptr, len(op), args.ptr, len(args), out.ptr, cap(out));
}

pub fn properties(entity: i32, out: []u8) i32 {
    return raw.entity_properties(entity, out.ptr, cap(out));
}

pub fn operations(entity: i32, out: []u8) i32 {
    return raw.entity_operations(entity, out.ptr, cap(out));
}

pub fn snapshot(entity: i32, out: []u8) i32 {
    return raw.entity_snapshot(entity, out.ptr, cap(out));
}

pub fn restore(entity: i32, nbt: []const u8) i32 {
    return raw.entity_restore(entity, nbt.ptr, len(nbt));
}

pub fn spawnRules(world: []const u8, rules: []const u8, out: []u8) i32 {
    return raw.entity_spawn_rules(world.ptr, len(world), rules.ptr, len(rules), out.ptr, cap(out));
}

pub fn mechanic(name: []const u8, args: []const u8, out: []u8) i32 {
    return raw.entity_mechanic(name.ptr, len(name), args.ptr, len(args), out.ptr, cap(out));
}

pub fn eventEntity(ctx: i32) i32 {
    return raw.entity_event_entity(ctx);
}

pub fn aiAnswer(ctx: i32, result: i32) i32 {
    return raw.entity_ai_answer(ctx, result);
}
