#include <basalt/server.h>
#include <basalt/server/core.hpp>
#include <basalt/server/entities.h>
// Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options,
// queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for
// snapshot).
#pragma once

namespace basalt::server::entities {
using namespace basalt::server::core;

inline int of_player(int who) { return bs_entity_of_player(who); }

inline int player(int entity) { return bs_entity_player(entity); }

inline int release(int entity) { return bs_entity_release(entity); }

inline int alive(int entity) { return bs_entity_alive(entity); }

inline int id(int entity) { return bs_entity_id(entity); }

inline int by_id(const char* world, int number) { return bs_entity_by_id(world, number); }

inline int by_uuid(const char* uuid) { return bs_entity_by_uuid(uuid); }

inline int query(const char* world, const char* filter, int filter_length, int* out, int capacity) {
    return bs_entity_query(world, filter, filter_length, out, capacity);
}

inline int types(char* out, int capacity) { return bs_entity_types(out, capacity); }

inline int type_attributes(const char* kind, char* out, int capacity) {
    return bs_entity_type_attributes(kind, out, capacity);
}

inline int spawn(const char* world, const char* kind, double x, double y, double z, const char* opts, int opts_length) {
    return bs_entity_spawn(world, kind, x, y, z, opts, opts_length);
}

inline int spawn_nbt(const char* world, const char* nbt, int nbt_length, const char* cause) {
    return bs_entity_spawn_nbt(world, nbt, nbt_length, cause);
}

inline int drop_item(const char* world, const char* item, int item_length, double x, double y, double z, double vx,
                     double vy, double vz) {
    return bs_entity_drop_item(world, item, item_length, x, y, z, vx, vy, vz);
}

inline int remove(int entity) { return bs_entity_remove(entity); }

inline int remove_uuid(const char* world, const char* uuid) { return bs_entity_remove_uuid(world, uuid); }

inline int position(int entity, double* out) { return bs_entity_position(entity, out); }

inline int teleport(int entity, double x, double y, double z, const char* opts, int opts_length) {
    return bs_entity_teleport(entity, x, y, z, opts, opts_length);
}

inline int velocity(int entity, double* out) { return bs_entity_velocity(entity, out); }

inline int set_velocity(int entity, double x, double y, double z) { return bs_entity_set_velocity(entity, x, y, z); }

inline int rotation(int entity, double* out) { return bs_entity_rotation(entity, out); }

inline int set_rotation(int entity, double yaw, double pitch) { return bs_entity_set_rotation(entity, yaw, pitch); }

inline int get(int entity, const char* name, char* out, int capacity) {
    return bs_entity_get(entity, name, out, capacity);
}

inline int set(int entity, const char* name, const char* value, int value_length) {
    return bs_entity_set(entity, name, value, value_length);
}

inline int call(int entity, const char* op, const char* args, int args_length, char* out, int capacity) {
    return bs_entity_call(entity, op, args, args_length, out, capacity);
}

inline int properties(int entity, char* out, int capacity) { return bs_entity_properties(entity, out, capacity); }

inline int operations(int entity, char* out, int capacity) { return bs_entity_operations(entity, out, capacity); }

inline int snapshot(int entity, char* out, int capacity) { return bs_entity_snapshot(entity, out, capacity); }

inline int restore(int entity, const char* nbt, int nbt_length) { return bs_entity_restore(entity, nbt, nbt_length); }

inline int spawn_rules(const char* world, const char* rules, int rules_length, char* out, int capacity) {
    return bs_entity_spawn_rules(world, rules, rules_length, out, capacity);
}

inline int mechanic(const char* name, const char* args, int args_length, char* out, int capacity) {
    return bs_entity_mechanic(name, args, args_length, out, capacity);
}

inline int event_entity(int ctx) { return bs_entity_event_entity(ctx); }

inline int ai_answer(int ctx, int result) { return bs_entity_ai_answer(ctx, result); }

} // namespace basalt::server::entities
