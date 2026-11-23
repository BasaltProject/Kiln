@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.players

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.core.Args
import org.basaltmc.server.core.Player
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

fun playerByName(name: String): Player = withScopedMemoryAllocator { a -> val n = a.str(name); rawPlayerByName(n.ptr, n.len) }
fun playersOnline(): List<Player> {
    var cap = 64
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val out = a.allocate(cap * 4)
            val n = rawPlayersOnline(out.address.toInt(), cap)
            if (n < 0) null else if (n > cap) n else List(n) { (out + it * 4).loadInt() }
        }
        if (r == null) return emptyList()
        if (r is Int) cap = r else {
            @Suppress("UNCHECKED_CAST")
            return r as List<Player>
        }
    }
}
fun playerName(player: Player): String? = fetch { out, cap -> rawPlayerName(player, out, cap) }
fun send(player: Player, text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawPlayerSend(player, t.ptr, t.len, p.ptr, p.len)
}
fun actionBar(player: Player, text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawPlayerActionBar(player, t.ptr, t.len, p.ptr, p.len)
}
