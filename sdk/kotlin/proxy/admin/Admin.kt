@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.admin

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

// Proxy admin area (abi/areas/proxy_admin.h): backends, console and chat, permission providers, groups, component JSON,
// the plugin store, data by holder and waiting. Calls that give a value answer a Value, or null on a negative code.



private const val INVALID = -3

/** The status of servers_register (0 when it worked) and the reason the proxy gave otherwise. */
class Registration(val status: Int, val reason: String)

fun serversRegister(name: String, host: String, port: Int, like: String): Registration =
    withScopedMemoryAllocator { a ->
        val n = a.str(name)
        val h = a.str(host)
        val l = a.str(like)
        val err = a.allocate(256)
        val status = rawProxyServersRegister(n.ptr, n.len, h.ptr, h.len, port, l.ptr, l.len, err.address.toInt(), 256)
        var end = 0
        while (status < 0 && end < 256 && (err + end).loadByte().toInt() != 0) end++
        Registration(status, ByteArray(end) { (err + it).loadByte() }.decodeToString())
    }

fun serversUnregister(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyServersUnregister(n.ptr, n.len)
}

/** Pings a backend; the handler reads the answer with [completionValue]. Returns the request handle. */
fun serversPing(name: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProxyServersPing(n.ptr, n.len, tokenFor(handler))
}

/** Runs the handler after [ticks] ticks of 50 ms. Returns the request handle. */
fun wait(ticks: Int, handler: Handler): Int = rawProxyWait(ticks, tokenFor(handler))

fun requestCancel(request: Int): Int = rawRequestCancel(request)

fun completionValue(ctx: Ctx): Value? = callOut { out, cap -> rawValueResult(ctx, out, cap) }.value()

/** 1 when the line named a command, 0 when not, -4 without the capability and the operator's grant. */
fun consoleRun(line: String): Int = withScopedMemoryAllocator { a ->
    val l = a.str(line)
    rawProxyConsoleRun(l.ptr, l.len)
}

fun playerChat(player: Player, text: String): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    rawProxyPlayerChat(player, t.ptr, t.len)
}

fun playerSendJson(player: Player, json: String): Int = withScopedMemoryAllocator { a ->
    val j = a.str(json)
    rawProxyPlayerSendJson(player, j.ptr, j.len)
}

fun playerKickJson(player: Player, json: String): Int = withScopedMemoryAllocator { a ->
    val j = a.str(json)
    rawProxyPlayerKickJson(player, j.ptr, j.len)
}

/** 1 granted, 0 denied, -1 undecided. [who] "" is the console; [server] may be empty. */
fun permissionCheck(who: String, node: String, server: String = ""): Int {
    val context = if (server.isEmpty()) ByteArray(0) else
        ValueWriter().beginMap().key("server").string(server).end().finish() ?: return INVALID
    return withScopedMemoryAllocator { a ->
        val w = a.str(who)
        val n = a.str(node)
        val c = a.bytes(context)
        rawProxyPermissionCheck(w.ptr, w.len, n.ptr, n.len, c.ptr, c.len)
    }
}

fun permissionMeta(who: String, key: String): String? = fetch { out, cap ->
    val w = str(who)
    val k = str(key)
    rawProxyPermissionMeta(w.ptr, w.len, k.ptr, k.len, k.ptr, 0, out, cap)
}

/** Registers provider callbacks (null for none); returns the provider handle. */
fun permissionProvide(check: Handler?, meta: Handler?, priority: Int = 0): Int =
    rawProxyPermissionProvide(if (check == null) 0 else tokenFor(check), if (meta == null) 0 else tokenFor(meta), priority)

fun permissionUnprovide(provider: Int): Int = rawProxyPermissionUnprovide(provider)

/** The question of a provider callback: a map with kind, name, uuid, console, text and context. */
fun permissionRequest(ctx: Ctx): Value? = callOut { out, cap -> rawProxyPermissionRequest(ctx, out, cap) }.value()

/** 1 grants, 0 denies, negative leaves it to the next provider. */
fun permissionAnswer(ctx: Ctx, answer: Int): Int = rawProxyPermissionAnswer(ctx, answer)

fun permissionAnswerText(ctx: Ctx, text: String): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    rawProxyPermissionAnswerText(ctx, t.ptr, t.len)
}

fun permissionChanged(): Int = rawProxyPermissionChanged()

/** 1 grants, 0 denies, -1 removes. */
fun permissionOverride(name: String, node: String, value: Int): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val o = a.str(node)
    rawProxyPermissionOverride(n.ptr, n.len, o.ptr, o.len, value)
}

fun permissionList(name: String): Value? = callOut { out, cap ->
    val n = str(name)
    rawProxyPermissionList(n.ptr, n.len, out, cap)
}.value()

fun groupGet(name: String): String? = fetch { out, cap ->
    val n = str(name)
    rawProxyGroupGet(n.ptr, n.len, out, cap)
}

fun groupSet(name: String, group: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val g = a.str(group)
    rawProxyGroupSet(n.ptr, n.len, g.ptr, g.len)
}

/** Reads a store key (holder null) or a player data key of [holder]; null when unset. */
fun dataValue(holder: String?, key: String): Value? = callOut { out, cap ->
    val k = str(key)
    if (holder == null) rawProxyStoreGet(k.ptr, k.len, out, cap)
    else {
        val h = str(holder)
        rawProxyPlayerDataGet(h.ptr, h.len, k.ptr, k.len, out, cap)
    }
}.value()

fun dataSetValue(holder: String?, key: String, doc: ByteArray?): Int {
    if (doc == null || doc.isEmpty()) return INVALID
    return withScopedMemoryAllocator { a ->
        val k = a.str(key)
        val d = a.bytes(doc)
        if (holder == null) rawProxyStoreSet(k.ptr, k.len, d.ptr, d.len)
        else {
            val h = a.str(holder)
            rawProxyPlayerDataSet(h.ptr, h.len, k.ptr, k.len, d.ptr, d.len)
        }
    }
}

fun dataSetLong(holder: String?, key: String, number: Long): Int =
    dataSetValue(holder, key, ValueWriter().int(number).finish())

fun dataSetDouble(holder: String?, key: String, number: Double): Int =
    dataSetValue(holder, key, ValueWriter().float(number).finish())

fun dataSetBool(holder: String?, key: String, flag: Boolean): Int =
    dataSetValue(holder, key, ValueWriter().bool(flag).finish())

fun dataSetStr(holder: String?, key: String, text: String): Int =
    dataSetValue(holder, key, ValueWriter().string(text).finish())

fun dataRemove(holder: String?, key: String): Int = dataSetValue(holder, key, ValueWriter().nul().finish())

/** The best [limit] numeric values of a store key: a list of maps with uuid, name and value. */
fun storeTop(key: String, limit: Int): Value? = callOut { out, cap ->
    val k = str(key)
    rawProxyStoreTop(k.ptr, k.len, limit, out, cap)
}.value()
