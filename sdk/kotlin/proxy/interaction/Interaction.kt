@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.interaction

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

// Proxy interaction area (abi/areas/proxy_interaction.h): dialogs, resource packs, cookies, transfer and extra player
// operations. Observed events and completions are read with valueResult and the Value reader.



/** A dialog button: the label and the command the client runs as the player when it is clicked. */
class DialogButton(val label: String, val command: String)

/** Observes player.resource_pack: the value holds player, player_name, id and status. */
fun watchResourcePacks(handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str("player.resource_pack")
    rawProxyInteractionWatch(n.ptr, n.len, tokenFor(handler))
}

/** Observes player.settings: the value holds player, player_name and settings. */
fun watchSettings(handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str("player.settings")
    rawProxyInteractionWatch(n.ptr, n.len, tokenFor(handler))
}

/** The value of an observed event or a completion, or null. */
fun valueResult(ctx: Ctx): Value? = callOut(256) { out, cap -> rawValueResult(ctx, out, cap) }.value()

/** kind is "notice", "confirmation" or "multi_action"; body may be null. */
fun dialogShow(player: Player, kind: String, title: String, body: String?, buttons: List<DialogButton>, closable: Boolean = true): Int {
    val w = ValueWriter()
    w.beginMap().key("kind").string(kind).key("title").string(title)
    if (body != null) w.key("body").string(body)
    w.key("buttons").beginList()
    for (button in buttons) w.beginMap().key("label").string(button.label).key("command").string(button.command).end()
    w.end().key("can_close_with_escape").bool(closable).end()
    val doc = w.finish() ?: return -3
    return withScopedMemoryAllocator { a ->
        val d = a.bytes(doc)
        rawProxyDialogShow(player, d.ptr, d.len)
    }
}

fun dialogClear(player: Player): Int = rawProxyDialogClear(player)

/** Pushes a pack and returns its id text, or null when it failed; hash, prompt and id may be null. */
fun resourcePackPush(player: Player, url: String, hash: String? = null, required: Boolean = false, prompt: String? = null, id: String? = null): String? {
    val w = ValueWriter()
    w.beginMap().key("url").string(url)
    if (hash != null) w.key("hash").string(hash)
    w.key("required").bool(required)
    if (prompt != null) w.key("prompt").string(prompt)
    if (id != null) w.key("id").string(id)
    w.end()
    val doc = w.finish() ?: return null
    return fetch { out, cap ->
        val d = bytes(doc)
        rawProxyResourcePackPush(player, d.ptr, d.len, out, cap)
    }
}

fun resourcePackRemove(player: Player, id: String): Int = withScopedMemoryAllocator { a ->
    val i = a.str(id)
    rawProxyResourcePackRemove(player, i.ptr, i.len)
}

fun resourcePackClear(player: Player): Int = rawProxyResourcePackClear(player)

fun cookieSet(player: Player, key: String, data: ByteArray): Int = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    val d = a.bytes(data)
    rawProxyCookieSet(player, k.ptr, k.len, d.ptr, d.len)
}

/** Returns the request handle; the handler reads the cookie with [cookieResult]. */
fun cookieRequest(player: Player, key: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    rawProxyCookieRequest(player, k.ptr, k.len, tokenFor(handler))
}

/** The cookie of a completion, or null when the client had none. */
fun cookieResult(ctx: Ctx): ByteArray? = valueResult(ctx)?.asBytes()

fun requestCancel(request: Int): Int = rawRequestCancel(request)

fun playerTransfer(player: Player, host: String, port: Int = 25565): Int = withScopedMemoryAllocator { a ->
    val h = a.str(host)
    rawProxyPlayerTransfer(player, h.ptr, h.len, port)
}

fun playersByUuid(uuid: String): Player = withScopedMemoryAllocator { a ->
    val u = a.str(uuid)
    rawProxyPlayersByUuid(u.ptr, u.len)
}

/** Players on the proxy, or on [server] when it is not empty. */
fun playersCount(server: String = ""): Int = withScopedMemoryAllocator { a ->
    val s = a.str(server)
    rawProxyPlayersCount(s.ptr, s.len)
}

fun playersBroadcast(text: String, args: Args? = null): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawProxyPlayersBroadcast(t.ptr, t.len, p.ptr, p.len)
}

fun playerAddress(player: Player): String? = fetch { out, cap -> rawProxyPlayerAddress(player, out, cap) }

/** The client port, a negative code when unknown. */
fun playerPort(player: Player): Int = rawProxyPlayerPort(player)

/** The client settings map, or null until the client sent it. */
fun playerSettings(player: Player): Value? = callOut(256) { out, cap -> rawProxyPlayerSettings(player, out, cap) }.value()

fun playerDataGetF64(player: Player, key: String, fallback: Double): Double = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    rawProxyPlayerDataGetF64(player, k.ptr, k.len, fallback)
}

fun playerDataSetF64(player: Player, key: String, value: Double): Int = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    rawProxyPlayerDataSetF64(player, k.ptr, k.len, value)
}
