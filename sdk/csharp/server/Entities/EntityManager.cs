using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
using static Basalt.Server.Internal.Native;

namespace Basalt.Server.Entities;

// Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers.
// Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).

public static unsafe class EntityManager
{
    public static int OfPlayer(int who)
    {
        return RawEntities.EntityOfPlayer(who);
    }

    public static int Player(int entity)
    {
        return RawEntities.EntityPlayer(entity);
    }

    public static int Release(int entity)
    {
        return RawEntities.EntityRelease(entity);
    }

    public static int Alive(int entity)
    {
        return RawEntities.EntityAlive(entity);
    }

    public static int Id(int entity)
    {
        return RawEntities.EntityId(entity);
    }

    public static int ById(string world, int number)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        int status;
        fixed (byte* worldP = worldB)
            status = RawEntities.EntityById(worldP, worldB.Length, number);
        return status;
    }

    public static int ByUuid(string uuid)
    {
        byte[] uuidB = Encoding.UTF8.GetBytes(uuid);
        int status;
        fixed (byte* uuidP = uuidB)
            status = RawEntities.EntityByUuid(uuidP, uuidB.Length);
        return status;
    }

    public static int Query(string world, byte[] filter, int[] result)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* filterP = filter)
                fixed (int* resultP = result)
                    status = RawEntities.EntityQuery(worldP, worldB.Length, filterP, filter.Length, resultP, result.Length);
        return status;
    }

    public static int Types(byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawEntities.EntityTypes(resultP, result.Length);
        return status;
    }

    public static int TypeAttributes(string kind, byte[] result)
    {
        byte[] kindB = Encoding.UTF8.GetBytes(kind);
        int status;
        fixed (byte* kindP = kindB)
            fixed (byte* resultP = result)
                status = RawEntities.EntityTypeAttributes(kindP, kindB.Length, resultP, result.Length);
        return status;
    }

    public static int Spawn(string world, string kind, double x, double y, double z, byte[] opts)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        byte[] kindB = Encoding.UTF8.GetBytes(kind);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* kindP = kindB)
                fixed (byte* optsP = opts)
                    status = RawEntities.EntitySpawn(worldP, worldB.Length, kindP, kindB.Length, x, y, z, optsP, opts.Length);
        return status;
    }

    public static int SpawnNbt(string world, byte[] nbt, string cause)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        byte[] causeB = Encoding.UTF8.GetBytes(cause);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* nbtP = nbt)
                fixed (byte* causeP = causeB)
                    status = RawEntities.EntitySpawnNbt(worldP, worldB.Length, nbtP, nbt.Length, causeP, causeB.Length);
        return status;
    }

    public static int DropItem(string world, byte[] item, double x, double y, double z, double vx, double vy, double vz)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* itemP = item)
                status = RawEntities.EntityDropItem(worldP, worldB.Length, itemP, item.Length, x, y, z, vx, vy, vz);
        return status;
    }

    public static int Remove(int entity)
    {
        return RawEntities.EntityRemove(entity);
    }

    public static int RemoveUuid(string world, string uuid)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        byte[] uuidB = Encoding.UTF8.GetBytes(uuid);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* uuidP = uuidB)
                status = RawEntities.EntityRemoveUuid(worldP, worldB.Length, uuidP, uuidB.Length);
        return status;
    }

    public static int Position(int entity, double[] result)
    {
        int status;
        fixed (double* resultP = result)
            status = RawEntities.EntityPosition(entity, resultP);
        return status;
    }

    public static int Teleport(int entity, double x, double y, double z, byte[] opts)
    {
        int status;
        fixed (byte* optsP = opts)
            status = RawEntities.EntityTeleport(entity, x, y, z, optsP, opts.Length);
        return status;
    }

    public static int Velocity(int entity, double[] result)
    {
        int status;
        fixed (double* resultP = result)
            status = RawEntities.EntityVelocity(entity, resultP);
        return status;
    }

    public static int SetVelocity(int entity, double x, double y, double z)
    {
        return RawEntities.EntitySetVelocity(entity, x, y, z);
    }

    public static int Rotation(int entity, double[] result)
    {
        int status;
        fixed (double* resultP = result)
            status = RawEntities.EntityRotation(entity, resultP);
        return status;
    }

    public static int SetRotation(int entity, double yaw, double pitch)
    {
        return RawEntities.EntitySetRotation(entity, yaw, pitch);
    }

    public static int Get(int entity, string name, byte[] result)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            fixed (byte* resultP = result)
                status = RawEntities.EntityGet(entity, nameP, nameB.Length, resultP, result.Length);
        return status;
    }

    public static int Set(int entity, string name, byte[] value)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            fixed (byte* valueP = value)
                status = RawEntities.EntitySet(entity, nameP, nameB.Length, valueP, value.Length);
        return status;
    }

    public static int Call(int entity, string op, byte[] args, byte[] result)
    {
        byte[] opB = Encoding.UTF8.GetBytes(op);
        int status;
        fixed (byte* opP = opB)
            fixed (byte* argsP = args)
                fixed (byte* resultP = result)
                    status = RawEntities.EntityCall(entity, opP, opB.Length, argsP, args.Length, resultP, result.Length);
        return status;
    }

    public static int Properties(int entity, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawEntities.EntityProperties(entity, resultP, result.Length);
        return status;
    }

    public static int Operations(int entity, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawEntities.EntityOperations(entity, resultP, result.Length);
        return status;
    }

    public static int Snapshot(int entity, byte[] result)
    {
        int status;
        fixed (byte* resultP = result)
            status = RawEntities.EntitySnapshot(entity, resultP, result.Length);
        return status;
    }

    public static int Restore(int entity, byte[] nbt)
    {
        int status;
        fixed (byte* nbtP = nbt)
            status = RawEntities.EntityRestore(entity, nbtP, nbt.Length);
        return status;
    }

    public static int SpawnRules(string world, byte[] rules, byte[] result)
    {
        byte[] worldB = Encoding.UTF8.GetBytes(world);
        int status;
        fixed (byte* worldP = worldB)
            fixed (byte* rulesP = rules)
                fixed (byte* resultP = result)
                    status = RawEntities.EntitySpawnRules(worldP, worldB.Length, rulesP, rules.Length, resultP, result.Length);
        return status;
    }

    public static int Mechanic(string name, byte[] args, byte[] result)
    {
        byte[] nameB = Encoding.UTF8.GetBytes(name);
        int status;
        fixed (byte* nameP = nameB)
            fixed (byte* argsP = args)
                fixed (byte* resultP = result)
                    status = RawEntities.EntityMechanic(nameP, nameB.Length, argsP, args.Length, resultP, result.Length);
        return status;
    }

    public static int EventEntity(int ctx)
    {
        return RawEntities.EntityEventEntity(ctx);
    }

    public static int AiAnswer(int ctx, int result)
    {
        return RawEntities.EntityAiAnswer(ctx, result);
    }

}
