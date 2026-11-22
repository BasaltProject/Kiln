#ifndef BASALT_SERVER_ENTITIES_H
#define BASALT_SERVER_ENTITIES_H

#include "basalt/server/core.h"
#include "basalt/server/values.h"
#include "areas/entities.h"

/* Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot). */

static inline int bs_entity_length(const char *text) {
    int n = 0;
    while (text != 0 && text[n]) {
        ++n;
    }
    return n;
}

static inline int bs_entity_of_player(int32_t who) {
    return basalt_entity_of_player(who);
}

static inline int bs_entity_player(int32_t entity) {
    return basalt_entity_player(entity);
}

static inline int bs_entity_release(int32_t entity) {
    return basalt_entity_release(entity);
}

static inline int bs_entity_alive(int32_t entity) {
    return basalt_entity_alive(entity);
}

static inline int bs_entity_id(int32_t entity) {
    return basalt_entity_id(entity);
}

static inline int bs_entity_by_id(const char *world, int32_t number) {
    return basalt_entity_by_id(world, bs_entity_length(world), number);
}

static inline int bs_entity_by_uuid(const char *uuid) {
    return basalt_entity_by_uuid(uuid, bs_entity_length(uuid));
}

static inline int bs_entity_query(const char *world, const char *filter, int filter_length, int32_t *out, int cap) {
    return basalt_entity_query(world, bs_entity_length(world), filter, filter_length, out, cap);
}

static inline int bs_entity_types(char *out, int cap) {
    return basalt_entity_types(out, cap);
}

static inline int bs_entity_type_attributes(const char *kind, char *out, int cap) {
    return basalt_entity_type_attributes(kind, bs_entity_length(kind), out, cap);
}

static inline int bs_entity_spawn(const char *world, const char *kind, double x, double y, double z, const char *opts, int opts_length) {
    return basalt_entity_spawn(world, bs_entity_length(world), kind, bs_entity_length(kind), x, y, z, opts, opts_length);
}

static inline int bs_entity_spawn_nbt(const char *world, const char *nbt, int nbt_length, const char *cause) {
    return basalt_entity_spawn_nbt(world, bs_entity_length(world), nbt, nbt_length, cause, bs_entity_length(cause));
}

static inline int bs_entity_drop_item(const char *world, const char *item, int item_length, double x, double y, double z, double vx, double vy, double vz) {
    return basalt_entity_drop_item(world, bs_entity_length(world), item, item_length, x, y, z, vx, vy, vz);
}

static inline int bs_entity_remove(int32_t entity) {
    return basalt_entity_remove(entity);
}

static inline int bs_entity_remove_uuid(const char *world, const char *uuid) {
    return basalt_entity_remove_uuid(world, bs_entity_length(world), uuid, bs_entity_length(uuid));
}

static inline int bs_entity_position(int32_t entity, double *out) {
    return basalt_entity_position(entity, out);
}

static inline int bs_entity_teleport(int32_t entity, double x, double y, double z, const char *opts, int opts_length) {
    return basalt_entity_teleport(entity, x, y, z, opts, opts_length);
}

static inline int bs_entity_velocity(int32_t entity, double *out) {
    return basalt_entity_velocity(entity, out);
}

static inline int bs_entity_set_velocity(int32_t entity, double x, double y, double z) {
    return basalt_entity_set_velocity(entity, x, y, z);
}

static inline int bs_entity_rotation(int32_t entity, double *out) {
    return basalt_entity_rotation(entity, out);
}

static inline int bs_entity_set_rotation(int32_t entity, double yaw, double pitch) {
    return basalt_entity_set_rotation(entity, yaw, pitch);
}

static inline int bs_entity_get(int32_t entity, const char *name, char *out, int cap) {
    return basalt_entity_get(entity, name, bs_entity_length(name), out, cap);
}

static inline int bs_entity_set(int32_t entity, const char *name, const char *value, int value_length) {
    return basalt_entity_set(entity, name, bs_entity_length(name), value, value_length);
}

static inline int bs_entity_call(int32_t entity, const char *op, const char *args, int args_length, char *out, int cap) {
    return basalt_entity_call(entity, op, bs_entity_length(op), args, args_length, out, cap);
}

static inline int bs_entity_properties(int32_t entity, char *out, int cap) {
    return basalt_entity_properties(entity, out, cap);
}

static inline int bs_entity_operations(int32_t entity, char *out, int cap) {
    return basalt_entity_operations(entity, out, cap);
}

static inline int bs_entity_snapshot(int32_t entity, char *out, int cap) {
    return basalt_entity_snapshot(entity, out, cap);
}

static inline int bs_entity_restore(int32_t entity, const char *nbt, int nbt_length) {
    return basalt_entity_restore(entity, nbt, nbt_length);
}

static inline int bs_entity_spawn_rules(const char *world, const char *rules, int rules_length, char *out, int cap) {
    return basalt_entity_spawn_rules(world, bs_entity_length(world), rules, rules_length, out, cap);
}

static inline int bs_entity_mechanic(const char *name, const char *args, int args_length, char *out, int cap) {
    return basalt_entity_mechanic(name, bs_entity_length(name), args, args_length, out, cap);
}

static inline int bs_entity_event_entity(int32_t ctx) {
    return basalt_entity_event_entity(ctx);
}

static inline int bs_entity_ai_answer(int32_t ctx, int32_t result) {
    return basalt_entity_ai_answer(ctx, result);
}

#endif
