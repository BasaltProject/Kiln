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

fun placeholder(name: String, handler: Handler): Int =
    withScopedMemoryAllocator { a -> val n = a.str(name); rawPlaceholderRegister(n.ptr, n.len, tokenFor(handler)) }
fun placeholderPlayer(ctx: Ctx): Player = rawPlaceholderPlayer(ctx)
fun placeholderResult(ctx: Ctx, text: String): Int =
    withScopedMemoryAllocator { a -> val t = a.str(text); rawPlaceholderResult(ctx, t.ptr, t.len) }
