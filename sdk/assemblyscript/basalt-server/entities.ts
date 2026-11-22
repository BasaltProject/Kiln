// Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers.
// Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).
import { raw_entities as raw } from "./raw/entities";

export function entityOfPlayer(who: i32): i32 {
  return raw.entity_of_player(who);
}

export function entityPlayer(entity: i32): i32 {
  return raw.entity_player(entity);
}

export function entityRelease(entity: i32): i32 {
  return raw.entity_release(entity);
}

export function entityAlive(entity: i32): i32 {
  return raw.entity_alive(entity);
}

export function entityId(entity: i32): i32 {
  return raw.entity_id(entity);
}

export function entityById(world: string, number: i32): i32 {
  const worldB = String.UTF8.encode(world);
  return raw.entity_by_id(changetype<usize>(worldB), worldB.byteLength, number);
}

export function entityByUuid(uuid: string): i32 {
  const uuidB = String.UTF8.encode(uuid);
  return raw.entity_by_uuid(changetype<usize>(uuidB), uuidB.byteLength);
}

export function entityQuery(world: string, filter: Uint8Array, out: Int32Array): i32 {
  const worldB = String.UTF8.encode(world);
  return raw.entity_query(changetype<usize>(worldB), worldB.byteLength, filter.dataStart, filter.length, out.dataStart, out.length);
}

export function entityTypes(out: Uint8Array): i32 {
  return raw.entity_types(out.dataStart, out.length);
}

export function entityTypeAttributes(kind: string, out: Uint8Array): i32 {
  const kindB = String.UTF8.encode(kind);
  return raw.entity_type_attributes(changetype<usize>(kindB), kindB.byteLength, out.dataStart, out.length);
}

export function entitySpawn(world: string, kind: string, x: f64, y: f64, z: f64, opts: Uint8Array): i32 {
  const worldB = String.UTF8.encode(world);
  const kindB = String.UTF8.encode(kind);
  return raw.entity_spawn(changetype<usize>(worldB), worldB.byteLength, changetype<usize>(kindB), kindB.byteLength, x, y, z, opts.dataStart, opts.length);
}

export function entitySpawnNbt(world: string, nbt: Uint8Array, cause: string): i32 {
  const worldB = String.UTF8.encode(world);
  const causeB = String.UTF8.encode(cause);
  return raw.entity_spawn_nbt(changetype<usize>(worldB), worldB.byteLength, nbt.dataStart, nbt.length, changetype<usize>(causeB), causeB.byteLength);
}

export function entityDropItem(world: string, item: Uint8Array, x: f64, y: f64, z: f64, vx: f64, vy: f64, vz: f64): i32 {
  const worldB = String.UTF8.encode(world);
  return raw.entity_drop_item(changetype<usize>(worldB), worldB.byteLength, item.dataStart, item.length, x, y, z, vx, vy, vz);
}

export function entityRemove(entity: i32): i32 {
  return raw.entity_remove(entity);
}

export function entityRemoveUuid(world: string, uuid: string): i32 {
  const worldB = String.UTF8.encode(world);
  const uuidB = String.UTF8.encode(uuid);
  return raw.entity_remove_uuid(changetype<usize>(worldB), worldB.byteLength, changetype<usize>(uuidB), uuidB.byteLength);
}

export function entityPosition(entity: i32, out: Float64Array): i32 {
  return raw.entity_position(entity, out.dataStart);
}

export function entityTeleport(entity: i32, x: f64, y: f64, z: f64, opts: Uint8Array): i32 {
  return raw.entity_teleport(entity, x, y, z, opts.dataStart, opts.length);
}

export function entityVelocity(entity: i32, out: Float64Array): i32 {
  return raw.entity_velocity(entity, out.dataStart);
}

export function entitySetVelocity(entity: i32, x: f64, y: f64, z: f64): i32 {
  return raw.entity_set_velocity(entity, x, y, z);
}

export function entityRotation(entity: i32, out: Float64Array): i32 {
  return raw.entity_rotation(entity, out.dataStart);
}

export function entitySetRotation(entity: i32, yaw: f64, pitch: f64): i32 {
  return raw.entity_set_rotation(entity, yaw, pitch);
}

export function entityGet(entity: i32, name: string, out: Uint8Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.entity_get(entity, changetype<usize>(nameB), nameB.byteLength, out.dataStart, out.length);
}

export function entitySet(entity: i32, name: string, value: Uint8Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.entity_set(entity, changetype<usize>(nameB), nameB.byteLength, value.dataStart, value.length);
}

export function entityCall(entity: i32, op: string, args: Uint8Array, out: Uint8Array): i32 {
  const opB = String.UTF8.encode(op);
  return raw.entity_call(entity, changetype<usize>(opB), opB.byteLength, args.dataStart, args.length, out.dataStart, out.length);
}

export function entityProperties(entity: i32, out: Uint8Array): i32 {
  return raw.entity_properties(entity, out.dataStart, out.length);
}

export function entityOperations(entity: i32, out: Uint8Array): i32 {
  return raw.entity_operations(entity, out.dataStart, out.length);
}

export function entitySnapshot(entity: i32, out: Uint8Array): i32 {
  return raw.entity_snapshot(entity, out.dataStart, out.length);
}

export function entityRestore(entity: i32, nbt: Uint8Array): i32 {
  return raw.entity_restore(entity, nbt.dataStart, nbt.length);
}

export function entitySpawnRules(world: string, rules: Uint8Array, out: Uint8Array): i32 {
  const worldB = String.UTF8.encode(world);
  return raw.entity_spawn_rules(changetype<usize>(worldB), worldB.byteLength, rules.dataStart, rules.length, out.dataStart, out.length);
}

export function entityMechanic(name: string, args: Uint8Array, out: Uint8Array): i32 {
  const nameB = String.UTF8.encode(name);
  return raw.entity_mechanic(changetype<usize>(nameB), nameB.byteLength, args.dataStart, args.length, out.dataStart, out.length);
}

export function entityEventEntity(ctx: i32): i32 {
  return raw.entity_event_entity(ctx);
}

export function entityAiAnswer(ctx: i32, result: i32): i32 {
  return raw.entity_ai_answer(ctx, result);
}
