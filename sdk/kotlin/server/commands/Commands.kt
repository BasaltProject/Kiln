@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.commands
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.Args
import org.basaltmc.server.core.Ctx
import org.basaltmc.server.core.Handler
import org.basaltmc.server.internal.*

fun command(spec: String, permission: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val s = a.str(spec)
    val p = a.str(permission)
    rawCommandRegister(s.ptr, s.len, p.ptr, p.len, tokenFor(handler))
}
fun commandReply(ctx: Ctx, text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawCommandReply(ctx, t.ptr, t.len, p.ptr, p.len)
}
fun commandArgStr(ctx: Ctx, name: String): String? = fetch { out, cap -> val n = str(name); rawCommandArgStr(ctx, n.ptr, n.len, out, cap) }
fun commandArgI64(ctx: Ctx, name: String): Long? = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val out = a.allocate(8)
    if (rawCommandArgI64(ctx, n.ptr, n.len, out.address.toInt()) == 0) out.loadLong() else null
}
fun commandSourceName(ctx: Ctx): String? = fetch { out, cap -> rawCommandSourceName(ctx, out, cap) }
fun commandIsConsole(ctx: Ctx): Boolean = rawCommandIsConsole(ctx) != 0
