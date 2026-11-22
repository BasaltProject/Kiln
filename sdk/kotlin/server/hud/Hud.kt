@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

// HUD area (abi/areas/hud.h): scoreboards, boss bars, holograms, NPCs, tab and chat surfaces. Specs are encoded values;
// an out array receives an encoded value (UTF-8 text for team and display lookups).
package org.basaltmc.server.hud
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.withScopedMemoryAllocator

fun scoreboardMain(): Int = withScopedMemoryAllocator { a ->
    val status = rawHudScoreboardMain()
    status
}

fun scoreboardCreate(): Int = withScopedMemoryAllocator { a ->
    val status = rawHudScoreboardCreate()
    status
}

fun scoreboardRelease(board: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudScoreboardRelease(board)
    status
}

fun scoreboardList(board: Int, what: String, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val whatB = a.str(what)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudScoreboardList(board, whatB.ptr, whatB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun objectiveAdd(board: Int, name: String, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val specB = a.bytes(spec)
    val status = rawHudObjectiveAdd(board, nameB.ptr, nameB.len, specB.ptr, specB.len)
    status
}

fun objectiveRemove(board: Int, name: String): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val status = rawHudObjectiveRemove(board, nameB.ptr, nameB.len)
    status
}

fun displaySet(board: Int, slot: String, objective: String): Int = withScopedMemoryAllocator { a ->
    val slotB = a.str(slot)
    val objectiveB = a.str(objective)
    val status = rawHudDisplaySet(board, slotB.ptr, slotB.len, objectiveB.ptr, objectiveB.len)
    status
}

fun displayGet(board: Int, slot: String, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val slotB = a.str(slot)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudDisplayGet(board, slotB.ptr, slotB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun scoreGet(board: Int, entry: String, objective: String, out: IntArray): Int = withScopedMemoryAllocator { a ->
    val entryB = a.str(entry)
    val objectiveB = a.str(objective)
    val outB = a.allocate(4)
    val status = rawHudScoreGet(board, entryB.ptr, entryB.len, objectiveB.ptr, objectiveB.len, outB.address.toInt())
    if (status == 0) out[0] = outB.loadInt()
    status
}

fun scoreSet(board: Int, entry: String, objective: String, value: Int): Int = withScopedMemoryAllocator { a ->
    val entryB = a.str(entry)
    val objectiveB = a.str(objective)
    val status = rawHudScoreSet(board, entryB.ptr, entryB.len, objectiveB.ptr, objectiveB.len, value)
    status
}

fun scoreReset(board: Int, entry: String, objective: String): Int = withScopedMemoryAllocator { a ->
    val entryB = a.str(entry)
    val objectiveB = a.str(objective)
    val status = rawHudScoreReset(board, entryB.ptr, entryB.len, objectiveB.ptr, objectiveB.len)
    status
}

fun teamAdd(board: Int, name: String): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val status = rawHudTeamAdd(board, nameB.ptr, nameB.len)
    status
}

fun teamSet(board: Int, name: String, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val specB = a.bytes(spec)
    val status = rawHudTeamSet(board, nameB.ptr, nameB.len, specB.ptr, specB.len)
    status
}

fun teamRemove(board: Int, name: String): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val status = rawHudTeamRemove(board, nameB.ptr, nameB.len)
    status
}

fun teamJoin(board: Int, team: String, entry: String): Int = withScopedMemoryAllocator { a ->
    val teamB = a.str(team)
    val entryB = a.str(entry)
    val status = rawHudTeamJoin(board, teamB.ptr, teamB.len, entryB.ptr, entryB.len)
    status
}

fun teamLeave(board: Int, team: String, entry: String): Int = withScopedMemoryAllocator { a ->
    val teamB = a.str(team)
    val entryB = a.str(entry)
    val status = rawHudTeamLeave(board, teamB.ptr, teamB.len, entryB.ptr, entryB.len)
    status
}

fun teamOf(board: Int, entry: String, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val entryB = a.str(entry)
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudTeamOf(board, entryB.ptr, entryB.len, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun scoreboardShow(player: Int, board: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudScoreboardShow(player, board)
    status
}

fun scoreboardShown(player: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudScoreboardShown(player)
    status
}

fun bossbarCreate(spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudBossbarCreate(specB.ptr, specB.len)
    status
}

fun bossbarAttach(id: String): Int = withScopedMemoryAllocator { a ->
    val idB = a.str(id)
    val status = rawHudBossbarAttach(idB.ptr, idB.len)
    status
}

fun bossbarUpdate(bar: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudBossbarUpdate(bar, specB.ptr, specB.len)
    status
}

fun bossbarRemove(bar: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudBossbarRemove(bar)
    status
}

fun bossbarPlayerAdd(bar: Int, player: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudBossbarPlayerAdd(bar, player)
    status
}

fun bossbarPlayerRemove(bar: Int, player: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudBossbarPlayerRemove(bar, player)
    status
}

fun bossbarPlayers(bar: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudBossbarPlayers(bar, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun bossbarGet(bar: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudBossbarGet(bar, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun bossbarKept(out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudBossbarKept(outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun hologramCreate(world: String, text: String, x: Double, y: Double, z: Double): Int = withScopedMemoryAllocator { a ->
    val worldB = a.str(world)
    val textB = a.str(text)
    val status = rawHudHologramCreate(worldB.ptr, worldB.len, textB.ptr, textB.len, x, y, z)
    status
}

fun hologramSet(hologram: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudHologramSet(hologram, specB.ptr, specB.len)
    status
}

fun hologramGet(hologram: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudHologramGet(hologram, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun hologramRemove(hologram: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudHologramRemove(hologram)
    status
}

fun npcCreate(spec: ByteArray, handler: Handler?): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudNpcCreate(specB.ptr, specB.len, if (handler == null) 0 else tokenFor(handler))
    status
}

fun npcSet(npc: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudNpcSet(npc, specB.ptr, specB.len)
    status
}

fun npcGet(npc: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudNpcGet(npc, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun npcItem(npc: Int, slot: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudNpcItem(npc, slot, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun npcAttribute(npc: Int, name: String, out: DoubleArray): Int = withScopedMemoryAllocator { a ->
    val nameB = a.str(name)
    val outB = a.allocate(8)
    val status = rawHudNpcAttribute(npc, nameB.ptr, nameB.len, outB.address.toInt())
    if (status == 0) out[0] = Double.fromBits(outB.loadLong())
    status
}

fun npcAnimate(npc: Int, action: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudNpcAnimate(npc, action)
    status
}

fun npcRemove(npc: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudNpcRemove(npc)
    status
}

fun playerSet(player: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudPlayerSet(player, specB.ptr, specB.len)
    status
}

fun playerGet(player: Int, out: ByteArray): Int = withScopedMemoryAllocator { a ->
    val outB = a.allocate(maxOf(out.size, 1))
    val status = rawHudPlayerGet(player, outB.address.toInt(), out.size)
    if (status in 0..out.size) { for (i in 0 until status) out[i] = (outB + i).loadByte() }
    status
}

fun tabEntry(viewer: Int, entity_id: Int, listed: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudTabEntry(viewer, entity_id, listed)
    status
}

fun surfaceClaim(surface: String, priority: Int, spec: ByteArray, handler: Handler?): Int = withScopedMemoryAllocator { a ->
    val surfaceB = a.str(surface)
    val specB = a.bytes(spec)
    val status = rawHudSurfaceClaim(surfaceB.ptr, surfaceB.len, priority, specB.ptr, specB.len, if (handler == null) 0 else tokenFor(handler))
    status
}

fun surfaceUpdate(claim: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudSurfaceUpdate(claim, specB.ptr, specB.len)
    status
}

fun surfaceRelease(claim: Int): Int = withScopedMemoryAllocator { a ->
    val status = rawHudSurfaceRelease(claim)
    status
}

fun surfaceAnswer(ctx: Int, spec: ByteArray): Int = withScopedMemoryAllocator { a ->
    val specB = a.bytes(spec)
    val status = rawHudSurfaceAnswer(ctx, specB.ptr, specB.len)
    status
}
