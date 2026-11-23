@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.players

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.core.Player
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

fun particle(player: Player, id: String, count: Int, spread: Double): Int =
    withScopedMemoryAllocator { a -> val i = a.str(id); rawPlayerParticle(player, i.ptr, i.len, count, spread) }
fun sound(player: Player, id: String, volume: Float, pitch: Float): Int =
    withScopedMemoryAllocator { a -> val i = a.str(id); rawPlayerSound(player, i.ptr, i.len, volume, pitch) }
