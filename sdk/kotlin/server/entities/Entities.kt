@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.server.entities

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

// Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers.
// Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).



fun entityOfPlayer(who: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityOfPlayer(who)
    status
}

fun entityPlayer(entity: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityPlayer(entity)
    status
}

fun entityRelease(entity: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityRelease(entity)
    status
}

fun entityAlive(entity: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityAlive(entity)
    status
}

fun entityId(entity: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityId(entity)
    status
}

fun entityById(world: String, number: Int): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val status = rawEntityById(worldB.ptr, worldB.len, number)
    status
}

fun entityByUuid(uuid: String): Int = withScopedMemoryAllocator { a ->
    val uuidB = a.str(uuid)
    val status = rawEntityByUuid(uuidB.ptr, uuidB.len)
    status
}

fun entityQuery(world: String, filter: ByteArray, out: IntArray): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val filterB = a.bytes(filter)
    val outB = a.allocate(maxOf(out.size * 4, 1))
    val status = rawEntityQuery(worldB.ptr, worldB.len, filterB.ptr, filterB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i * 4).loadInt() }
    status
}

fun entityTypes(out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityTypes(outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityTypeAttributes(kind: String, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val kindB = a.str(kind)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityTypeAttributes(kindB.ptr, kindB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entitySpawn(world: String, kind: String, x: Double, y: Double, z: Double, opts: ByteArray): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val kindB = a.str(kind)
    val optsB = a.bytes(opts)
    val status = rawEntitySpawn(worldB.ptr, worldB.len, kindB.ptr, kindB.len, x, y, z, optsB.ptr, optsB.len)
    status
}

fun entitySpawnNbt(world: String, nbt: ByteArray, cause: String): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val nbtB = a.bytes(nbt)
    val causeB = a.str(cause)
    val status = rawEntitySpawnNbt(worldB.ptr, worldB.len, nbtB.ptr, nbtB.len, causeB.ptr, causeB.len)
    status
}

fun entityDropItem(world: String, item: ByteArray, x: Double, y: Double, z: Double, vx: Double, vy: Double, vz: Double): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val itemB = a.bytes(item)
    val status = rawEntityDropItem(worldB.ptr, worldB.len, itemB.ptr, itemB.len, x, y, z, vx, vy, vz)
    status
}

fun entityRemove(entity: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityRemove(entity)
    status
}

fun entityRemoveUuid(world: String, uuid: String): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val uuidB = a.str(uuid)
    val status = rawEntityRemoveUuid(worldB.ptr, worldB.len, uuidB.ptr, uuidB.len)
    status
}

fun entityPosition(entity: Int, out: DoubleArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(24)
    val status = rawEntityPosition(entity, outB.address.toInt())
    if (status == 0) { for (i in 0 until 3) out[i] = Double.fromBits((outB + i * 8).loadLong()) }
    status
}

fun entityTeleport(entity: Int, x: Double, y: Double, z: Double, opts: ByteArray): Int = withScopedMemoryAllocator { a ->
    val optsB = a.bytes(opts)
    val status = rawEntityTeleport(entity, x, y, z, optsB.ptr, optsB.len)
    status
}

fun entityVelocity(entity: Int, out: DoubleArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(24)
    val status = rawEntityVelocity(entity, outB.address.toInt())
    if (status == 0) { for (i in 0 until 3) out[i] = Double.fromBits((outB + i * 8).loadLong()) }
    status
}

fun entitySetVelocity(entity: Int, x: Double, y: Double, z: Double): Int = withScopedMemoryAllocator { a ->
    val status = rawEntitySetVelocity(entity, x, y, z)
    status
}

fun entityRotation(entity: Int, out: DoubleArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(16)
    val status = rawEntityRotation(entity, outB.address.toInt())
    if (status == 0) { for (i in 0 until 2) out[i] = Double.fromBits((outB + i * 8).loadLong()) }
    status
}

fun entitySetRotation(entity: Int, yaw: Double, pitch: Double): Int = withScopedMemoryAllocator { a ->
    val status = rawEntitySetRotation(entity, yaw, pitch)
    status
}

fun entityGet(entity: Int, name: String, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityGet(entity, nameB.ptr, nameB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entitySet(entity: Int, name: String, value: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val valueB = a.bytes(value)
    val status = rawEntitySet(entity, nameB.ptr, nameB.len, valueB.ptr, valueB.len)
    status
}

fun entityCall(entity: Int, op: String, args: ByteArray, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val opB = a.str(op)
    val argsB = a.bytes(args)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityCall(entity, opB.ptr, opB.len, argsB.ptr, argsB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityProperties(entity: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityProperties(entity, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityOperations(entity: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityOperations(entity, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entitySnapshot(entity: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntitySnapshot(entity, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityRestore(entity: Int, nbt: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nbtB = a.bytes(nbt)
    val status = rawEntityRestore(entity, nbtB.ptr, nbtB.len)
    status
}

fun entitySpawnRules(world: String, rules: ByteArray, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val rulesB = a.bytes(rules)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntitySpawnRules(worldB.ptr, worldB.len, rulesB.ptr, rulesB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityMechanic(name: String, args: ByteArray, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val argsB = a.bytes(args)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawEntityMechanic(nameB.ptr, nameB.len, argsB.ptr, argsB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun entityEventEntity(ctx: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityEventEntity(ctx)
    status
}

fun entityAiAnswer(ctx: Int, result: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawEntityAiAnswer(ctx, result)
    status
}
