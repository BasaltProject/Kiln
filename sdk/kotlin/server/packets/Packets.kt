@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.packets

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

// Plugin channels and raw packets (abi/areas/packets.h). Calls return the import result: a count or handle, or a
// negative ABI code. The out-calls return an OutResult: `error` is 0 or the negative code, `bytes` the answer, and
// `value()` opens it as a document.



private const val PACKETS_INITIAL = 1024

fun channelListen(name: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawChannelListen(n.ptr, n.len, tokenFor(handler))
}

fun channelSend(player: Player, name: String, body: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawChannelSend(player, n.ptr, n.len, b.ptr, b.len)
}

/** `priority` is "first", "early", "normal", "late", "last" or null for the default. */
fun packetListen(pattern: String, priority: String?, observe: Boolean, handler: Handler): Int {
    var document = ByteArray(0)
    if (priority != null || observe) {
        val writer = ValueWriter().beginMap()
        if (priority != null) writer.key("priority").string(priority)
        if (observe) writer.key("observe").bool(true)
        document = writer.end().finish() ?: return -3
    }
    return withScopedMemoryAllocator { a ->
        val p = a.str(pattern)
        val d = a.bytes(document)
        rawPacketListen(p.ptr, p.len, d.ptr, d.len, tokenFor(handler))
    }
}

fun packetUnlisten(listener: Int): Int = rawPacketUnlisten(listener)

fun packetSend(player: Player, name: String, body: ByteArray, intercept: Boolean): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawPacketSend(player, n.ptr, n.len, b.ptr, b.len, if (intercept) 1 else 0)
}

fun packetReceive(player: Player, name: String, body: ByteArray, intercept: Boolean): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawPacketReceive(player, n.ptr, n.len, b.ptr, b.len, if (intercept) 1 else 0)
}

fun packetLookup(name: String): OutResult = callOut(PACKETS_INITIAL) { out, cap ->
    val n = str(name)
    rawPacketLookup(n.ptr, n.len, out, cap)
}

fun packetNames(prefix: String): OutResult = callOut(PACKETS_INITIAL) { out, cap ->
    val p = str(prefix)
    rawPacketNames(p.ptr, p.len, out, cap)
}

fun packetPlayerActions(): OutResult = callOut(PACKETS_INITIAL) { out, cap -> rawPacketPlayerActions(out, cap) }

/** `args` is an encoded list value. */
fun packetPlayerReceive(player: Player, action: String, args: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(action)
    val g = a.bytes(args)
    rawPacketPlayerReceive(player, n.ptr, n.len, g.ptr, g.len)
}

fun packetCtxInfo(ctx: Ctx): OutResult = callOut(PACKETS_INITIAL) { out, cap -> rawPacketCtxInfo(ctx, out, cap) }

fun packetCtxBody(ctx: Ctx): OutResult = callOut(PACKETS_INITIAL) { out, cap -> rawPacketCtxBody(ctx, out, cap) }

fun packetCtxPlayer(ctx: Ctx): Player = rawPacketCtxPlayer(ctx)

fun packetCtxCancel(ctx: Ctx): Int = rawPacketCtxCancel(ctx)

fun packetCtxSetBody(ctx: Ctx, body: ByteArray): Int = withScopedMemoryAllocator { a ->
    val b = a.bytes(body)
    rawPacketCtxSetBody(ctx, b.ptr, b.len)
}

fun packetCtxSend(ctx: Ctx, name: String, body: ByteArray, intercept: Boolean): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawPacketCtxSend(ctx, n.ptr, n.len, b.ptr, b.len, if (intercept) 1 else 0)
}

fun packetCtxReceive(ctx: Ctx, name: String, body: ByteArray, intercept: Boolean): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawPacketCtxReceive(ctx, n.ptr, n.len, b.ptr, b.len, if (intercept) 1 else 0)
}
