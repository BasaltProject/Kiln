@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.network

import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

// Proxy network area (abi/areas/proxy_network.h): messaging channels, raw packets and antibot control. The calls that
// answer a value return its raw encoded bytes for the values reader, or null on a negative code.



private inline fun netFetch(call: MemoryAllocator.(Int, Int) -> Int): ByteArray? {
    var cap = 256
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val out = a.allocate(cap)
            val n = a.call(out.address.toInt(), cap)
            if (n < 0) null else if (n > cap) n else ByteArray(n) { (out + it).loadByte() }
        }
        if (r == null) return null
        if (r is Int) cap = r else return r as ByteArray
    }
}

fun channelRegister(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyNetworkChannelRegister(n.ptr, n.len)
}

fun channelUnregister(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyNetworkChannelUnregister(n.ptr, n.len)
}

/** Observes the messages of the plugin's channels; an empty filter takes all of them. */
fun messageOn(filter: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val f = a.str(filter)
    rawProxyNetworkMessageOn(f.ptr, f.len, tokenFor(handler))
}

/** The encoded map of the message (player_name, player_uuid, channel, data, source, server). */
fun messageGet(ctx: Ctx): ByteArray? = netFetch { out, cap -> rawProxyNetworkMessageGet(ctx, out, cap) }

/** Sends [data] on a channel; [towardClient] false goes to the player's backend, true to the client. */
fun messageSend(player: Player, channel: String, data: ByteArray, towardClient: Boolean): Int =
    withScopedMemoryAllocator { a ->
        val c = a.str(channel)
        val d = a.bytes(data)
        rawProxyNetworkMessageSend(player, c.ptr, c.len, d.ptr, d.len, if (towardClient) 1 else 0)
    }

fun packetResolve(name: String): ByteArray? = netFetch { out, cap ->
    val n = str(name)
    rawProxyPacketResolve(n.ptr, n.len, out, cap)
}

fun packetNames(): ByteArray? = netFetch { out, cap -> rawProxyPacketNames(out, cap) }

/** A decider listener; [opts] is an encoded value map (priority, fail) or empty. Returns the listener. */
fun packetOn(name: String, opts: ByteArray, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val o = a.bytes(opts)
    rawProxyPacketOn(n.ptr, n.len, o.ptr, o.len, tokenFor(handler))
}

/** An observer listener: it sees the final packet and cannot cancel or change it. */
fun packetWatch(name: String, opts: ByteArray, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val o = a.bytes(opts)
    rawProxyPacketWatch(n.ptr, n.len, o.ptr, o.len, tokenFor(handler))
}

fun packetRemove(listener: Int): Int = rawProxyPacketRemove(listener)

fun packetGet(ctx: Ctx): ByteArray? = netFetch { out, cap -> rawProxyPacketGet(ctx, out, cap) }

fun packetBody(ctx: Ctx): ByteArray? = netFetch { out, cap -> rawProxyPacketBody(ctx, out, cap) }

fun packetSetBody(ctx: Ctx, body: ByteArray): Int = withScopedMemoryAllocator { a ->
    val b = a.bytes(body)
    rawProxyPacketSetBody(ctx, b.ptr, b.len)
}

fun packetCancel(ctx: Ctx): Int = rawProxyPacketCancel(ctx)

fun packetInject(ctx: Ctx, name: String, body: ByteArray, intercept: Boolean): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawProxyPacketInject(ctx, n.ptr, n.len, b.ptr, b.len, if (intercept) 1 else 0)
}

fun packetSend(player: Player, name: String, body: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawProxyPacketSend(player, n.ptr, n.len, b.ptr, b.len)
}

fun packetReceive(player: Player, name: String, body: ByteArray): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val b = a.bytes(body)
    rawProxyPacketReceive(player, n.ptr, n.len, b.ptr, b.len)
}

fun antibotVerify(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyAntibotVerify(n.ptr, n.len)
}

fun antibotUnverify(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyAntibotUnverify(n.ptr, n.len)
}

fun antibotIsVerified(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyAntibotIsVerified(n.ptr, n.len)
}

fun antibotBan(address: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(address)
    rawProxyAntibotBan(n.ptr, n.len)
}

fun antibotUnban(address: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(address)
    rawProxyAntibotUnban(n.ptr, n.len)
}

fun antibotUnderAttack(): Int = rawProxyAntibotUnderAttack()

fun antibotOn(handler: Handler): Int = rawProxyAntibotOn(tokenFor(handler))

fun antibotEventGet(ctx: Ctx): ByteArray? = netFetch { out, cap -> rawProxyAntibotEventGet(ctx, out, cap) }
