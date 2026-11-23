@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.players

import kotlin.wasm.unsafe.*
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*

fun playerUuid(player: Player): String? = fetch { out, cap -> rawProxyPlayerUuid(player, out, cap) }

fun playerServer(player: Player): String? = fetch { out, cap -> rawProxyPlayerServer(player, out, cap) }

fun playerPing(player: Player): Int = rawProxyPlayerPing(player)

fun playerSettingStr(player: Player, field: String): String? = fetch { out, cap ->
    val f = str(field)
    rawProxyPlayerSettingStr(player, f.ptr, f.len, out, cap)
}

fun playerSettingI64(player: Player, field: String): Long? = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    val out = a.allocate(8)
    if (rawProxyPlayerSettingI64(player, f.ptr, f.len, out.address.toInt()) >= 0) out.loadLong() else null
}

fun playerHasPermission(player: Player, node: String): Boolean = withScopedMemoryAllocator { a ->
    val n = a.str(node)
    rawProxyPlayerHasPermission(player, n.ptr, n.len) == 1
}
/** Times are in ticks; a negative one means the client default. */

fun playerTitle(player: Player, title: String, subtitle: String, fadeIn: Int = -1, stay: Int = -1, fadeOut: Int = -1): Int =
    withScopedMemoryAllocator { a ->
        val t = a.str(title)
        val s = a.str(subtitle)
        rawProxyPlayerTitle(player, t.ptr, t.len, s.ptr, s.len, fadeIn, stay, fadeOut)
    }

fun playerConnect(player: Player, server: String): Int = withScopedMemoryAllocator { a ->
    val s = a.str(server)
    rawProxyPlayerConnect(player, s.ptr, s.len)
}

fun playerKick(player: Player, reason: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val r = a.str(reason)
    val p = a.args(args)
    rawProxyPlayerKick(player, r.ptr, r.len, p.ptr, p.len)
}
