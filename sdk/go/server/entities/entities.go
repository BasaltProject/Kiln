package entities

import "basaltmc.org/sdk/server/internal/runtime"

func EntityOfPlayer(who int32) int32 { return runtime.EntityOfPlayer(who) }

func EntityPlayer(entity int32) int32 { return runtime.EntityPlayer(entity) }

func EntityRelease(entity int32) int32 { return runtime.EntityRelease(entity) }

func EntityAlive(entity int32) int32 { return runtime.EntityAlive(entity) }

func EntityId(entity int32) int32 { return runtime.EntityId(entity) }

func EntityById(world string, number int32) int32 { return runtime.EntityById(world, number) }

func EntityByUuid(uuid string) int32 { return runtime.EntityByUuid(uuid) }

func EntityQuery(world string, filter []byte, out []int32) int32 {
	return runtime.EntityQuery(world, filter, out)
}

func EntityTypes(out []byte) int32 { return runtime.EntityTypes(out) }

func EntityTypeAttributes(kind string, out []byte) int32 {
	return runtime.EntityTypeAttributes(kind, out)
}

func EntitySpawn(world string, kind string, x float64, y float64, z float64, opts []byte) int32 {
	return runtime.EntitySpawn(world, kind, x, y, z, opts)
}

func EntitySpawnNbt(world string, nbt []byte, cause string) int32 {
	return runtime.EntitySpawnNbt(world, nbt, cause)
}

func EntityDropItem(world string, item []byte, x float64, y float64, z float64, vx float64, vy float64, vz float64) int32 {
	return runtime.EntityDropItem(world, item, x, y, z, vx, vy, vz)
}

func EntityRemove(entity int32) int32 { return runtime.EntityRemove(entity) }

func EntityRemoveUuid(world string, uuid string) int32 { return runtime.EntityRemoveUuid(world, uuid) }

func EntityPosition(entity int32, out *[3]float64) int32 { return runtime.EntityPosition(entity, out) }

func EntityTeleport(entity int32, x float64, y float64, z float64, opts []byte) int32 {
	return runtime.EntityTeleport(entity, x, y, z, opts)
}

func EntityVelocity(entity int32, out *[3]float64) int32 { return runtime.EntityVelocity(entity, out) }

func EntitySetVelocity(entity int32, x float64, y float64, z float64) int32 {
	return runtime.EntitySetVelocity(entity, x, y, z)
}

func EntityRotation(entity int32, out *[2]float64) int32 { return runtime.EntityRotation(entity, out) }

func EntitySetRotation(entity int32, yaw float64, pitch float64) int32 {
	return runtime.EntitySetRotation(entity, yaw, pitch)
}

func EntityGet(entity int32, name string, out []byte) int32 {
	return runtime.EntityGet(entity, name, out)
}

func EntitySet(entity int32, name string, value []byte) int32 {
	return runtime.EntitySet(entity, name, value)
}

func EntityCall(entity int32, op string, args []byte, out []byte) int32 {
	return runtime.EntityCall(entity, op, args, out)
}

func EntityProperties(entity int32, out []byte) int32 { return runtime.EntityProperties(entity, out) }

func EntityOperations(entity int32, out []byte) int32 { return runtime.EntityOperations(entity, out) }

func EntitySnapshot(entity int32, out []byte) int32 { return runtime.EntitySnapshot(entity, out) }

func EntityRestore(entity int32, nbt []byte) int32 { return runtime.EntityRestore(entity, nbt) }

func EntitySpawnRules(world string, rules []byte, out []byte) int32 {
	return runtime.EntitySpawnRules(world, rules, out)
}

func EntityMechanic(name string, args []byte, out []byte) int32 {
	return runtime.EntityMechanic(name, args, out)
}

func EntityEventEntity(ctx int32) int32 { return runtime.EntityEventEntity(ctx) }

func EntityAiAnswer(ctx int32, result int32) int32 { return runtime.EntityAiAnswer(ctx, result) }
