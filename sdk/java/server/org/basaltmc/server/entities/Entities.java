package org.basaltmc.server.entities;
import org.basaltmc.server.internal.*;
import org.basaltmc.server.core.*;

import org.teavm.interop.Address;

/** Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot). */
public final class Entities {
    private Entities() {}

    public static int ofPlayer(int who) {
        return RawEntities.rawEntityOfPlayer(who);
    }

    public static int player(int entity) {
        return RawEntities.rawEntityPlayer(entity);
    }

    public static int release(int entity) {
        return RawEntities.rawEntityRelease(entity);
    }

    public static int alive(int entity) {
        return RawEntities.rawEntityAlive(entity);
    }

    public static int id(int entity) {
        return RawEntities.rawEntityId(entity);
    }

    public static int byId(String world, int number) {
        byte[] worldBytes = Native.utf8(world);
        return RawEntities.rawEntityById(Native.at(worldBytes), worldBytes.length, number);
    }

    public static int byUuid(String uuid) {
        byte[] uuidBytes = Native.utf8(uuid);
        return RawEntities.rawEntityByUuid(Native.at(uuidBytes), uuidBytes.length);
    }

    public static int query(String world, byte[] filter, int[] out) {
        byte[] worldBytes = Native.utf8(world);
        return RawEntities.rawEntityQuery(Native.at(worldBytes), worldBytes.length, Native.at(filter), filter.length, Address.ofData(out), out.length);
    }

    public static int types(byte[] out) {
        return RawEntities.rawEntityTypes(Native.at(out), out.length);
    }

    public static int typeAttributes(String kind, byte[] out) {
        byte[] kindBytes = Native.utf8(kind);
        return RawEntities.rawEntityTypeAttributes(Native.at(kindBytes), kindBytes.length, Native.at(out), out.length);
    }

    public static int spawn(String world, String kind, double x, double y, double z, byte[] opts) {
        byte[] worldBytes = Native.utf8(world);
        byte[] kindBytes = Native.utf8(kind);
        return RawEntities.rawEntitySpawn(Native.at(worldBytes), worldBytes.length, Native.at(kindBytes), kindBytes.length, x, y, z, Native.at(opts), opts.length);
    }

    public static int spawnNbt(String world, byte[] nbt, String cause) {
        byte[] worldBytes = Native.utf8(world);
        byte[] causeBytes = Native.utf8(cause);
        return RawEntities.rawEntitySpawnNbt(Native.at(worldBytes), worldBytes.length, Native.at(nbt), nbt.length, Native.at(causeBytes), causeBytes.length);
    }

    public static int dropItem(String world, byte[] item, double x, double y, double z, double vx, double vy, double vz) {
        byte[] worldBytes = Native.utf8(world);
        return RawEntities.rawEntityDropItem(Native.at(worldBytes), worldBytes.length, Native.at(item), item.length, x, y, z, vx, vy, vz);
    }

    public static int remove(int entity) {
        return RawEntities.rawEntityRemove(entity);
    }

    public static int removeUuid(String world, String uuid) {
        byte[] worldBytes = Native.utf8(world);
        byte[] uuidBytes = Native.utf8(uuid);
        return RawEntities.rawEntityRemoveUuid(Native.at(worldBytes), worldBytes.length, Native.at(uuidBytes), uuidBytes.length);
    }

    public static int position(int entity, double[] out) {
        return RawEntities.rawEntityPosition(entity, Address.ofData(out));
    }

    public static int teleport(int entity, double x, double y, double z, byte[] opts) {
        return RawEntities.rawEntityTeleport(entity, x, y, z, Native.at(opts), opts.length);
    }

    public static int velocity(int entity, double[] out) {
        return RawEntities.rawEntityVelocity(entity, Address.ofData(out));
    }

    public static int setVelocity(int entity, double x, double y, double z) {
        return RawEntities.rawEntitySetVelocity(entity, x, y, z);
    }

    public static int rotation(int entity, double[] out) {
        return RawEntities.rawEntityRotation(entity, Address.ofData(out));
    }

    public static int setRotation(int entity, double yaw, double pitch) {
        return RawEntities.rawEntitySetRotation(entity, yaw, pitch);
    }

    public static int get(int entity, String name, byte[] out) {
        byte[] nameBytes = Native.utf8(name);
        return RawEntities.rawEntityGet(entity, Native.at(nameBytes), nameBytes.length, Native.at(out), out.length);
    }

    public static int set(int entity, String name, byte[] value) {
        byte[] nameBytes = Native.utf8(name);
        return RawEntities.rawEntitySet(entity, Native.at(nameBytes), nameBytes.length, Native.at(value), value.length);
    }

    public static int call(int entity, String op, byte[] args, byte[] out) {
        byte[] opBytes = Native.utf8(op);
        return RawEntities.rawEntityCall(entity, Native.at(opBytes), opBytes.length, Native.at(args), args.length, Native.at(out), out.length);
    }

    public static int properties(int entity, byte[] out) {
        return RawEntities.rawEntityProperties(entity, Native.at(out), out.length);
    }

    public static int operations(int entity, byte[] out) {
        return RawEntities.rawEntityOperations(entity, Native.at(out), out.length);
    }

    public static int snapshot(int entity, byte[] out) {
        return RawEntities.rawEntitySnapshot(entity, Native.at(out), out.length);
    }

    public static int restore(int entity, byte[] nbt) {
        return RawEntities.rawEntityRestore(entity, Native.at(nbt), nbt.length);
    }

    public static int spawnRules(String world, byte[] rules, byte[] out) {
        byte[] worldBytes = Native.utf8(world);
        return RawEntities.rawEntitySpawnRules(Native.at(worldBytes), worldBytes.length, Native.at(rules), rules.length, Native.at(out), out.length);
    }

    public static int mechanic(String name, byte[] args, byte[] out) {
        byte[] nameBytes = Native.utf8(name);
        return RawEntities.rawEntityMechanic(Native.at(nameBytes), nameBytes.length, Native.at(args), args.length, Native.at(out), out.length);
    }

    public static int eventEntity(int ctx) {
        return RawEntities.rawEntityEventEntity(ctx);
    }

    public static int aiAnswer(int ctx, int result) {
        return RawEntities.rawEntityAiAnswer(ctx, result);
    }

}
