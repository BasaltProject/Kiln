//! Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).
#![allow(dead_code)]

use crate::raw::entities as raw;
use crate::core::p;

pub fn of_player(who: i32) -> i32 {
    unsafe { raw::entity_of_player(who) }
}

pub fn player(entity: i32) -> i32 {
    unsafe { raw::entity_player(entity) }
}

pub fn release(entity: i32) -> i32 {
    unsafe { raw::entity_release(entity) }
}

pub fn alive(entity: i32) -> i32 {
    unsafe { raw::entity_alive(entity) }
}

pub fn id(entity: i32) -> i32 {
    unsafe { raw::entity_id(entity) }
}

pub fn by_id(world: &str, number: i32) -> i32 {
    let (world_ptr, world_len) = p(world);
    unsafe { raw::entity_by_id(world_ptr, world_len, number) }
}

pub fn by_uuid(uuid: &str) -> i32 {
    let (uuid_ptr, uuid_len) = p(uuid);
    unsafe { raw::entity_by_uuid(uuid_ptr, uuid_len) }
}

pub fn query(world: &str, filter: &[u8], out: &mut [i32]) -> i32 {
    let (world_ptr, world_len) = p(world);
    unsafe { raw::entity_query(world_ptr, world_len, filter.as_ptr(), filter.len() as i32, out.as_mut_ptr(), out.len() as i32) }
}

pub fn types(out: &mut [u8]) -> i32 {
    unsafe { raw::entity_types(out.as_mut_ptr(), out.len() as i32) }
}

pub fn type_attributes(kind: &str, out: &mut [u8]) -> i32 {
    let (kind_ptr, kind_len) = p(kind);
    unsafe { raw::entity_type_attributes(kind_ptr, kind_len, out.as_mut_ptr(), out.len() as i32) }
}

pub fn spawn(world: &str, kind: &str, x: f64, y: f64, z: f64, opts: &[u8]) -> i32 {
    let (world_ptr, world_len) = p(world);
    let (kind_ptr, kind_len) = p(kind);
    unsafe { raw::entity_spawn(world_ptr, world_len, kind_ptr, kind_len, x, y, z, opts.as_ptr(), opts.len() as i32) }
}

pub fn spawn_nbt(world: &str, nbt: &[u8], cause: &str) -> i32 {
    let (world_ptr, world_len) = p(world);
    let (cause_ptr, cause_len) = p(cause);
    unsafe { raw::entity_spawn_nbt(world_ptr, world_len, nbt.as_ptr(), nbt.len() as i32, cause_ptr, cause_len) }
}

pub fn drop_item(world: &str, item: &[u8], x: f64, y: f64, z: f64, vx: f64, vy: f64, vz: f64) -> i32 {
    let (world_ptr, world_len) = p(world);
    unsafe { raw::entity_drop_item(world_ptr, world_len, item.as_ptr(), item.len() as i32, x, y, z, vx, vy, vz) }
}

pub fn remove(entity: i32) -> i32 {
    unsafe { raw::entity_remove(entity) }
}

pub fn remove_uuid(world: &str, uuid: &str) -> i32 {
    let (world_ptr, world_len) = p(world);
    let (uuid_ptr, uuid_len) = p(uuid);
    unsafe { raw::entity_remove_uuid(world_ptr, world_len, uuid_ptr, uuid_len) }
}

pub fn position(entity: i32, out: &mut [f64; 3]) -> i32 {
    unsafe { raw::entity_position(entity, out.as_mut_ptr()) }
}

pub fn teleport(entity: i32, x: f64, y: f64, z: f64, opts: &[u8]) -> i32 {
    unsafe { raw::entity_teleport(entity, x, y, z, opts.as_ptr(), opts.len() as i32) }
}

pub fn velocity(entity: i32, out: &mut [f64; 3]) -> i32 {
    unsafe { raw::entity_velocity(entity, out.as_mut_ptr()) }
}

pub fn set_velocity(entity: i32, x: f64, y: f64, z: f64) -> i32 {
    unsafe { raw::entity_set_velocity(entity, x, y, z) }
}

pub fn rotation(entity: i32, out: &mut [f64; 2]) -> i32 {
    unsafe { raw::entity_rotation(entity, out.as_mut_ptr()) }
}

pub fn set_rotation(entity: i32, yaw: f64, pitch: f64) -> i32 {
    unsafe { raw::entity_set_rotation(entity, yaw, pitch) }
}

pub fn get(entity: i32, name: &str, out: &mut [u8]) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::entity_get(entity, name_ptr, name_len, out.as_mut_ptr(), out.len() as i32) }
}

pub fn set(entity: i32, name: &str, value: &[u8]) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::entity_set(entity, name_ptr, name_len, value.as_ptr(), value.len() as i32) }
}

pub fn call(entity: i32, op: &str, args: &[u8], out: &mut [u8]) -> i32 {
    let (op_ptr, op_len) = p(op);
    unsafe { raw::entity_call(entity, op_ptr, op_len, args.as_ptr(), args.len() as i32, out.as_mut_ptr(), out.len() as i32) }
}

pub fn properties(entity: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::entity_properties(entity, out.as_mut_ptr(), out.len() as i32) }
}

pub fn operations(entity: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::entity_operations(entity, out.as_mut_ptr(), out.len() as i32) }
}

pub fn snapshot(entity: i32, out: &mut [u8]) -> i32 {
    unsafe { raw::entity_snapshot(entity, out.as_mut_ptr(), out.len() as i32) }
}

pub fn restore(entity: i32, nbt: &[u8]) -> i32 {
    unsafe { raw::entity_restore(entity, nbt.as_ptr(), nbt.len() as i32) }
}

pub fn spawn_rules(world: &str, rules: &[u8], out: &mut [u8]) -> i32 {
    let (world_ptr, world_len) = p(world);
    unsafe { raw::entity_spawn_rules(world_ptr, world_len, rules.as_ptr(), rules.len() as i32, out.as_mut_ptr(), out.len() as i32) }
}

pub fn mechanic(name: &str, args: &[u8], out: &mut [u8]) -> i32 {
    let (name_ptr, name_len) = p(name);
    unsafe { raw::entity_mechanic(name_ptr, name_len, args.as_ptr(), args.len() as i32, out.as_mut_ptr(), out.len() as i32) }
}

pub fn event_entity(ctx: i32) -> i32 {
    unsafe { raw::entity_event_entity(ctx) }
}

pub fn ai_answer(ctx: i32, result: i32) -> i32 {
    unsafe { raw::entity_ai_answer(ctx, result) }
}
