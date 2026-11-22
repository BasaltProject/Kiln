package worlds

import "basaltmc.org/sdk/server/internal/runtime"

const WorldLoad = runtime.WorldLoad

const WorldNotify = runtime.WorldNotify

const WorldResend = runtime.WorldResend

const WorldGenerate = runtime.WorldGenerate

func ChunkCoord(block int32) int32 { return runtime.ChunkCoord(block) }

func BlockDistance(x1, y1, z1, x2, y2, z2 int32) float64 {
	return runtime.BlockDistance(x1, y1, z1, x2, y2, z2)
}

func PosOffset(pos *[3]int32, dx, dy, dz int32) { runtime.PosOffset(pos, dx, dy, dz) }

func WorldByName(name string) int32 { return runtime.WorldByName(name) }

func WorldDefault() int32 { return runtime.WorldDefault() }

func WorldList(buffer []int32) int32 { return runtime.WorldList(buffer) }

func WorldName(world int32, buffer []byte) int32 { return runtime.WorldName(world, buffer) }

func WorldGet(world int32, key string, buffer []byte) int32 {
	return runtime.WorldGet(world, key, buffer)
}

func WorldSet(world int32, key string, value []byte) int32 {
	return runtime.WorldSet(world, key, value)
}

func WorldRuleGet(world int32, name string, buffer []byte) int32 {
	return runtime.WorldRuleGet(world, name, buffer)
}

func WorldRuleSet(world int32, name string, value string) int32 {
	return runtime.WorldRuleSet(world, name, value)
}

func WorldCreate(spec []byte) int32 { return runtime.WorldCreate(spec) }

func WorldUnload(name string, flags int32) int32 { return runtime.WorldUnload(name, flags) }

func WorldDelete(name string) int32 { return runtime.WorldDelete(name) }

func WorldAt(world int32, x float64, z float64, done runtime.Handler) int32 {
	return runtime.WorldAt(world, x, z, done)
}

func WorldBlockGet(world int32, x int32, y int32, z int32, flags int32, buffer []byte) int32 {
	return runtime.WorldBlockGet(world, x, y, z, flags, buffer)
}

func WorldBlockSet(world int32, x int32, y int32, z int32, block string, flags int32) int32 {
	return runtime.WorldBlockSet(world, x, y, z, block, flags)
}

func WorldStateGet(world int32, x int32, y int32, z int32, flags int32) int32 {
	return runtime.WorldStateGet(world, x, y, z, flags)
}

func WorldStateSet(world int32, x int32, y int32, z int32, state int32, flags int32) int32 {
	return runtime.WorldStateSet(world, x, y, z, state, flags)
}

func WorldStatesRead(world int32, positions []int32, buffer []int32, flags int32) int32 {
	return runtime.WorldStatesRead(world, positions, buffer, flags)
}

func WorldStatesWrite(world int32, positions []int32, states []int32, flags int32) int32 {
	return runtime.WorldStatesWrite(world, positions, states, flags)
}

func WorldFill(world int32, x1 int32, y1 int32, z1 int32, x2 int32, y2 int32, z2 int32, state int32, flags int32) int32 {
	return runtime.WorldFill(world, x1, y1, z1, x2, y2, z2, state, flags)
}

func WorldSectionRead(world int32, chunk_x int32, section_y int32, chunk_z int32, flags int32, buffer []int32) int32 {
	return runtime.WorldSectionRead(world, chunk_x, section_y, chunk_z, flags, buffer)
}

func WorldSectionWrite(world int32, chunk_x int32, section_y int32, chunk_z int32, states []int32, flags int32) int32 {
	return runtime.WorldSectionWrite(world, chunk_x, section_y, chunk_z, states, flags)
}

func WorldChunkRead(world int32, chunk_x int32, chunk_z int32, flags int32, buffer []int32) int32 {
	return runtime.WorldChunkRead(world, chunk_x, chunk_z, flags, buffer)
}

func WorldChunkApply(world int32, chunk_x int32, chunk_z int32, section_ys []int32, states []int32, flags int32, previous []int32) int32 {
	return runtime.WorldChunkApply(world, chunk_x, chunk_z, section_ys, states, flags, previous)
}

func WorldChangesApply(world int32, positions []int32, states []int32, effects int32) int32 {
	return runtime.WorldChangesApply(world, positions, states, effects)
}

func WorldBlocksSettle(world int32, positions []int32, previous []int32, effects int32) int32 {
	return runtime.WorldBlocksSettle(world, positions, previous, effects)
}

func WorldBlocksNotify(world int32, positions []int32, sources []int32) int32 {
	return runtime.WorldBlocksNotify(world, positions, sources)
}

func WorldBlocksReshape(world int32, positions []int32, flags int32) int32 {
	return runtime.WorldBlocksReshape(world, positions, flags)
}

func WorldStateValid(world int32, x int32, y int32, z int32, state int32) int32 {
	return runtime.WorldStateValid(world, x, y, z, state)
}

func WorldTickSchedule(world int32, x int32, y int32, z int32, delay int32, flags int32) int32 {
	return runtime.WorldTickSchedule(world, x, y, z, delay, flags)
}

func WorldTickPending(world int32, x int32, y int32, z int32) int32 {
	return runtime.WorldTickPending(world, x, y, z)
}

func WorldBlockEntityGet(world int32, x int32, y int32, z int32, buffer []byte) int32 {
	return runtime.WorldBlockEntityGet(world, x, y, z, buffer)
}

func WorldBlockEntityEdit(world int32, op int32, x int32, y int32, z int32, nbt []byte) int32 {
	return runtime.WorldBlockEntityEdit(world, op, x, y, z, nbt)
}

func WorldBlockEntities(world int32, chunk_x int32, chunk_z int32, buffer []int32) int32 {
	return runtime.WorldBlockEntities(world, chunk_x, chunk_z, buffer)
}

func WorldDataGet(world int32, scope int32, x int32, y int32, z int32, buffer []byte) int32 {
	return runtime.WorldDataGet(world, scope, x, y, z, buffer)
}

func WorldDataSet(world int32, scope int32, x int32, y int32, z int32, nbt []byte) int32 {
	return runtime.WorldDataSet(world, scope, x, y, z, nbt)
}

func WorldChunkStatus(world int32, chunk_x int32, chunk_z int32) int32 {
	return runtime.WorldChunkStatus(world, chunk_x, chunk_z)
}

func WorldChunkLoad(world int32, chunk_x int32, chunk_z int32, flags int32) int32 {
	return runtime.WorldChunkLoad(world, chunk_x, chunk_z, flags)
}

func WorldChunkUnload(world int32, chunk_x int32, chunk_z int32) int32 {
	return runtime.WorldChunkUnload(world, chunk_x, chunk_z)
}

func WorldChunkRequest(world int32, chunk_x int32, chunk_z int32, flags int32, priority int32, done runtime.Handler) int32 {
	return runtime.WorldChunkRequest(world, chunk_x, chunk_z, flags, priority, done)
}

func WorldChunksLoaded(world int32, buffer []int32) int32 {
	return runtime.WorldChunksLoaded(world, buffer)
}

func WorldChunkHold(world int32, tag string, chunks []int32) int32 {
	return runtime.WorldChunkHold(world, tag, chunks)
}

func WorldChunkRelease(world int32, tag string, chunks []int32) int32 {
	return runtime.WorldChunkRelease(world, tag, chunks)
}

func WorldChunkHeld(world int32, tag string, buffer []int32) int32 {
	return runtime.WorldChunkHeld(world, tag, buffer)
}

func WorldChunksResend(world int32, chunks []int32) int32 {
	return runtime.WorldChunksResend(world, chunks)
}

func WorldStoredGet(world int32, chunk_x int32, chunk_z int32, buffer []byte) int32 {
	return runtime.WorldStoredGet(world, chunk_x, chunk_z, buffer)
}

func WorldStoredPut(world int32, chunk_x int32, chunk_z int32, nbt []byte) int32 {
	return runtime.WorldStoredPut(world, chunk_x, chunk_z, nbt)
}

func WorldStoredList(world int32, region_x int32, region_z int32, buffer []int32) int32 {
	return runtime.WorldStoredList(world, region_x, region_z, buffer)
}

func WorldLightAt(world int32, x int32, y int32, z int32, layer int32) int32 {
	return runtime.WorldLightAt(world, x, y, z, layer)
}

func WorldLightRead(world int32, chunk_x int32, section_y int32, chunk_z int32, layer int32, flags int32, buffer []byte) int32 {
	return runtime.WorldLightRead(world, chunk_x, section_y, chunk_z, layer, flags, buffer)
}

func WorldLightWrite(world int32, chunk_x int32, section_y int32, chunk_z int32, layer int32, nibbles []byte) int32 {
	return runtime.WorldLightWrite(world, chunk_x, section_y, chunk_z, layer, nibbles)
}

func WorldLightCheck(world int32, chunks []int32, done runtime.Handler) int32 {
	return runtime.WorldLightCheck(world, chunks, done)
}

func WorldBiomeGet(world int32, x int32, y int32, z int32, buffer []byte) int32 {
	return runtime.WorldBiomeGet(world, x, y, z, buffer)
}

func WorldBiomeSet(world int32, x int32, y int32, z int32, biome string) int32 {
	return runtime.WorldBiomeSet(world, x, y, z, biome)
}

func WorldBiomesRead(world int32, chunk_x int32, chunk_z int32, buffer []int32) int32 {
	return runtime.WorldBiomesRead(world, chunk_x, chunk_z, buffer)
}

func WorldBiomesWrite(world int32, chunk_x int32, chunk_z int32, ids []int32) int32 {
	return runtime.WorldBiomesWrite(world, chunk_x, chunk_z, ids)
}

func WorldSound(world int32, sound string, category string, x float64, y float64, z float64, volume float32, pitch float32, seed int64) int32 {
	return runtime.WorldSound(world, sound, category, x, y, z, volume, pitch, seed)
}

func WorldParticle(world int32, spec []byte) int32 { return runtime.WorldParticle(world, spec) }

func WorldLevelEvent(world int32, kind int32, x int32, y int32, z int32, data int32, flags int32) int32 {
	return runtime.WorldLevelEvent(world, kind, x, y, z, data, flags)
}

func WorldBlockEvent(world int32, x int32, y int32, z int32, first int32, second int32) int32 {
	return runtime.WorldBlockEvent(world, x, y, z, first, second)
}

func WorldExplode(world int32, x float64, y float64, z float64, power float32, flags int32, source int32) int32 {
	return runtime.WorldExplode(world, x, y, z, power, flags, source)
}

func WorldLightning(world int32, x float64, y float64, z float64) int32 {
	return runtime.WorldLightning(world, x, y, z)
}

func WorldGameEvent(world int32, eventId string, x float64, y float64, z float64, source string) int32 {
	return runtime.WorldGameEvent(world, eventId, x, y, z, source)
}

func WorldPlayerEffect(player int32, op string, args []byte, buffer []byte) int32 {
	return runtime.WorldPlayerEffect(player, op, args, buffer)
}

func WorldBorderGet(world int32, buffer []byte) int32 { return runtime.WorldBorderGet(world, buffer) }

func WorldBorderSet(world int32, spec []byte) int32 { return runtime.WorldBorderSet(world, spec) }

func WorldCall(world int32, op string, args []byte, buffer []byte) int32 {
	return runtime.WorldCall(world, op, args, buffer)
}

func StructureList(buffer []byte) int32 { return runtime.StructureList(buffer) }

func StructureGet(id string, buffer []byte) int32 { return runtime.StructureGet(id, buffer) }

func StructurePut(id string, nbt []byte, flags int32) int32 {
	return runtime.StructurePut(id, nbt, flags)
}

func StructureRemove(id string, flags int32) int32 { return runtime.StructureRemove(id, flags) }

func StructureFile(id string, buffer []byte) int32 { return runtime.StructureFile(id, buffer) }

func StructureCapture(world int32, x int32, y int32, z int32, size_x int32, size_y int32, size_z int32, flags int32, buffer []byte) int32 {
	return runtime.StructureCapture(world, x, y, z, size_x, size_y, size_z, flags, buffer)
}

func StructureScan(nbt []byte, spec []byte, buffer []byte) int32 {
	return runtime.StructureScan(nbt, spec, buffer)
}

func StructurePlace(nbt []byte, spec []byte) int32 { return runtime.StructurePlace(nbt, spec) }

func DatafixCall(op string, args []byte, buffer []byte) int32 {
	return runtime.DatafixCall(op, args, buffer)
}

func NoiseQuery(spec []byte, buffer []byte) int32 { return runtime.NoiseQuery(spec, buffer) }

func NoiseCreate(spec []byte) int32 { return runtime.NoiseCreate(spec) }

func NoiseSample(noise int32, x float64, y float64, z float64, buffer *float64) int32 {
	return runtime.NoiseSample(noise, x, y, z, buffer)
}

func NoiseInfo(noise int32, buffer []byte) int32 { return runtime.NoiseInfo(noise, buffer) }

func NoiseRelease(noise int32) int32 { return runtime.NoiseRelease(noise) }

func BlockStateId(text string) int32 { return runtime.BlockStateId(text) }

func BlockStateText(state int32, buffer []byte) int32 { return runtime.BlockStateText(state, buffer) }

func WorldsData(query string, args []byte, buffer []byte) int32 {
	return runtime.WorldsData(query, args, buffer)
}

func WorldValueResult(ctx int32, buffer []byte) int32 { return runtime.WorldValueResult(ctx, buffer) }

func WorldRequestCancel(request int32) int32 { return runtime.WorldRequestCancel(request) }
