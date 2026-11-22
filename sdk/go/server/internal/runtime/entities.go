package runtime

import "unsafe"

// Entities area (abi/areas/entities.h): entity handles, spawning, motion, properties by name and goal answers. Options, queries, items and arguments are encoded values; an `out` buffer receives an encoded value (or NBT bytes for snapshot).

func entitiesBytes(data []byte) (*byte, int32) {
	if len(data) == 0 {
		return nil, 0
	}
	return unsafe.SliceData(data), int32(len(data))
}

func entitiesInts(data []int32) (*int32, int32) {
	if len(data) == 0 {
		return nil, 0
	}
	return unsafe.SliceData(data), int32(len(data))
}

func EntityOfPlayer(who int32) int32 {
	return rawEntityOfPlayer(who)
}

func EntityPlayer(entity int32) int32 {
	return rawEntityPlayer(entity)
}

func EntityRelease(entity int32) int32 {
	return rawEntityRelease(entity)
}

func EntityAlive(entity int32) int32 {
	return rawEntityAlive(entity)
}

func EntityId(entity int32) int32 {
	return rawEntityId(entity)
}

func EntityById(world string, number int32) int32 {
	worldPtr, worldLen := ptr(world)
	return rawEntityById(worldPtr, worldLen, number)
}

func EntityByUuid(uuid string) int32 {
	uuidPtr, uuidLen := ptr(uuid)
	return rawEntityByUuid(uuidPtr, uuidLen)
}

func EntityQuery(world string, filter []byte, out []int32) int32 {
	worldPtr, worldLen := ptr(world)
	filterPtr, filterLen := entitiesBytes(filter)
	outPtr, outCap := entitiesInts(out)
	return rawEntityQuery(worldPtr, worldLen, filterPtr, filterLen, outPtr, outCap)
}

func EntityTypes(out []byte) int32 {
	outPtr, outLen := entitiesBytes(out)
	return rawEntityTypes(outPtr, outLen)
}

func EntityTypeAttributes(kind string, out []byte) int32 {
	kindPtr, kindLen := ptr(kind)
	outPtr, outLen := entitiesBytes(out)
	return rawEntityTypeAttributes(kindPtr, kindLen, outPtr, outLen)
}

func EntitySpawn(world string, kind string, x float64, y float64, z float64, opts []byte) int32 {
	worldPtr, worldLen := ptr(world)
	kindPtr, kindLen := ptr(kind)
	optsPtr, optsLen := entitiesBytes(opts)
	return rawEntitySpawn(worldPtr, worldLen, kindPtr, kindLen, x, y, z, optsPtr, optsLen)
}

func EntitySpawnNbt(world string, nbt []byte, cause string) int32 {
	worldPtr, worldLen := ptr(world)
	nbtPtr, nbtLen := entitiesBytes(nbt)
	causePtr, causeLen := ptr(cause)
	return rawEntitySpawnNbt(worldPtr, worldLen, nbtPtr, nbtLen, causePtr, causeLen)
}

func EntityDropItem(world string, item []byte, x float64, y float64, z float64, vx float64, vy float64, vz float64) int32 {
	worldPtr, worldLen := ptr(world)
	itemPtr, itemLen := entitiesBytes(item)
	return rawEntityDropItem(worldPtr, worldLen, itemPtr, itemLen, x, y, z, vx, vy, vz)
}

func EntityRemove(entity int32) int32 {
	return rawEntityRemove(entity)
}

func EntityRemoveUuid(world string, uuid string) int32 {
	worldPtr, worldLen := ptr(world)
	uuidPtr, uuidLen := ptr(uuid)
	return rawEntityRemoveUuid(worldPtr, worldLen, uuidPtr, uuidLen)
}

func EntityPosition(entity int32, out *[3]float64) int32 {
	return rawEntityPosition(entity, &out[0])
}

func EntityTeleport(entity int32, x float64, y float64, z float64, opts []byte) int32 {
	optsPtr, optsLen := entitiesBytes(opts)
	return rawEntityTeleport(entity, x, y, z, optsPtr, optsLen)
}

func EntityVelocity(entity int32, out *[3]float64) int32 {
	return rawEntityVelocity(entity, &out[0])
}

func EntitySetVelocity(entity int32, x float64, y float64, z float64) int32 {
	return rawEntitySetVelocity(entity, x, y, z)
}

func EntityRotation(entity int32, out *[2]float64) int32 {
	return rawEntityRotation(entity, &out[0])
}

func EntitySetRotation(entity int32, yaw float64, pitch float64) int32 {
	return rawEntitySetRotation(entity, yaw, pitch)
}

func EntityGet(entity int32, name string, out []byte) int32 {
	namePtr, nameLen := ptr(name)
	outPtr, outLen := entitiesBytes(out)
	return rawEntityGet(entity, namePtr, nameLen, outPtr, outLen)
}

func EntitySet(entity int32, name string, value []byte) int32 {
	namePtr, nameLen := ptr(name)
	valuePtr, valueLen := entitiesBytes(value)
	return rawEntitySet(entity, namePtr, nameLen, valuePtr, valueLen)
}

func EntityCall(entity int32, op string, args []byte, out []byte) int32 {
	opPtr, opLen := ptr(op)
	argsPtr, argsLen := entitiesBytes(args)
	outPtr, outLen := entitiesBytes(out)
	return rawEntityCall(entity, opPtr, opLen, argsPtr, argsLen, outPtr, outLen)
}

func EntityProperties(entity int32, out []byte) int32 {
	outPtr, outLen := entitiesBytes(out)
	return rawEntityProperties(entity, outPtr, outLen)
}

func EntityOperations(entity int32, out []byte) int32 {
	outPtr, outLen := entitiesBytes(out)
	return rawEntityOperations(entity, outPtr, outLen)
}

func EntitySnapshot(entity int32, out []byte) int32 {
	outPtr, outLen := entitiesBytes(out)
	return rawEntitySnapshot(entity, outPtr, outLen)
}

func EntityRestore(entity int32, nbt []byte) int32 {
	nbtPtr, nbtLen := entitiesBytes(nbt)
	return rawEntityRestore(entity, nbtPtr, nbtLen)
}

func EntitySpawnRules(world string, rules []byte, out []byte) int32 {
	worldPtr, worldLen := ptr(world)
	rulesPtr, rulesLen := entitiesBytes(rules)
	outPtr, outLen := entitiesBytes(out)
	return rawEntitySpawnRules(worldPtr, worldLen, rulesPtr, rulesLen, outPtr, outLen)
}

func EntityMechanic(name string, args []byte, out []byte) int32 {
	namePtr, nameLen := ptr(name)
	argsPtr, argsLen := entitiesBytes(args)
	outPtr, outLen := entitiesBytes(out)
	return rawEntityMechanic(namePtr, nameLen, argsPtr, argsLen, outPtr, outLen)
}

func EntityEventEntity(ctx int32) int32 {
	return rawEntityEventEntity(ctx)
}

func EntityAiAnswer(ctx int32, result int32) int32 {
	return rawEntityAiAnswer(ctx, result)
}
