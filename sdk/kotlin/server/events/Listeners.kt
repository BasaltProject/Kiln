@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.events

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.core.Ctx
import org.basaltmc.server.core.Handler
import org.basaltmc.server.core.Player
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

fun on(event: String, priority: Int, handler: Handler): Int =
    withScopedMemoryAllocator { a -> val e = a.str(event); rawEventOn(e.ptr, e.len, priority, tokenFor(handler)) }
fun eventPlayer(ctx: Ctx): Player = rawEventPlayer(ctx)
fun eventFieldStr(ctx: Ctx, field: String): String? = fetch { out, cap -> val f = str(field); rawEventFieldStr(ctx, f.ptr, f.len, out, cap) }
fun eventFieldI64(ctx: Ctx, field: String): Long? = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    val out = a.allocate(8)
    if (rawEventFieldI64(ctx, f.ptr, f.len, out.address.toInt()) == 0) out.loadLong() else null
}
fun eventCancel(ctx: Ctx, cancelled: Boolean): Int = rawEventCancel(ctx, if (cancelled) 1 else 0)
