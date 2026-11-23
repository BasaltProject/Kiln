@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.data

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.core.Player
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

fun dataGetI64(player: Player, key: String, def: Long): Long =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawDataGetI64(player, k.ptr, k.len, def) }
fun dataSetI64(player: Player, key: String, value: Long): Int =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawDataSetI64(player, k.ptr, k.len, value) }
fun dataGetBool(player: Player, key: String, def: Boolean): Boolean =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawDataGetBool(player, k.ptr, k.len, if (def) 1 else 0) != 0 }
fun dataSetBool(player: Player, key: String, value: Boolean): Int =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawDataSetBool(player, k.ptr, k.len, if (value) 1 else 0) }
fun dataGetStr(player: Player, key: String): String? = fetch { out, cap -> val k = str(key); rawDataGetStr(player, k.ptr, k.len, out, cap) }
fun dataSetStr(player: Player, key: String, value: String): Int = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    val v = a.str(value)
    rawDataSetStr(player, k.ptr, k.len, v.ptr, v.len)
}
fun dataRemove(player: Player, key: String): Int =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawDataRemove(player, k.ptr, k.len) }
