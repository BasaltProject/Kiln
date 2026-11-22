@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.menus
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.Ctx
import org.basaltmc.server.core.Handler
import org.basaltmc.server.core.Player
import org.basaltmc.server.internal.*

fun menuCreate(title: String, rows: Int): Int = withScopedMemoryAllocator { a -> val t = a.str(title); rawMenuCreate(t.ptr, t.len, rows) }
fun menuSet(menu: Int, slot: Int, item: String, name: String, lore: String, onClick: Handler): Int = withScopedMemoryAllocator { a ->
    val i = a.str(item)
    val n = a.str(name)
    val l = a.str(lore)
    rawMenuSet(menu, slot, i.ptr, i.len, n.ptr, n.len, l.ptr, l.len, tokenFor(onClick))
}
fun menuOpen(menu: Int, player: Player): Int = rawMenuOpen(menu, player)

fun menuClickPlayer(ctx: Ctx): Player = rawMenuClickPlayer(ctx)
