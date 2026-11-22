@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

// Server area: permissions, dynamic commands, broadcast, plugin files and storage, services and scheduler completions.
package org.basaltmc.server.server
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.withScopedMemoryAllocator

private const val SERVER_BUFFER = 1024

private fun MemoryAllocator.doc(data: ByteArray?): Bytes = if (data == null) Bytes(0, 0) else bytes(data)

private fun text(answer: OutResult): String? = if (answer.error == 0) answer.bytes.decodeToString() else null

const val BROADCAST_PLAYERS = 1
const val BROADCAST_CONSOLE = 2
const val PROVIDE_CHECK = 1
const val PROVIDE_META = 2

fun permissionCheck(subject: String, node: String): Int = withScopedMemoryAllocator { a ->
    val s = a.str(subject)
    val n = a.str(node)
    rawPermissionCheck(s.ptr, s.len, n.ptr, n.len)
}

fun permissionHas(player: Player, node: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(node)
    rawPermissionHas(player, n.ptr, n.len)
}

private fun permissionSet(player: Player, node: String, mode: Int): Int = withScopedMemoryAllocator { a ->
    val n = a.str(node)
    rawPermissionSet(player, n.ptr, n.len, mode)
}

fun permissionGrant(player: Player, node: String): Int = permissionSet(player, node, 1)

fun permissionDeny(player: Player, node: String): Int = permissionSet(player, node, 0)

fun permissionDrop(player: Player, node: String): Int = permissionSet(player, node, -1)

/** The meta text of a subject, or null when there is none. */
fun permissionMeta(subject: String, key: String): String? = text(callOut(128) { out, cap ->
    val s = str(subject)
    val k = str(key)
    rawPermissionMeta(s.ptr, s.len, k.ptr, k.len, out, cap)
})

fun permissionProvide(priority: Int, kinds: Int, handler: Handler): Int =
    rawPermissionProvide(priority, kinds, tokenFor(handler))

fun permissionAnswer(ctx: Ctx, answer: Int): Int = rawPermissionAnswer(ctx, answer)

fun permissionAnswerMeta(ctx: Ctx, text: String): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    rawPermissionAnswerMeta(ctx, t.ptr, t.len)
}

/** A text field (`node` or `kind`) of the question a provider callback is asked, or null. */
fun permissionQuery(ctx: Ctx, field: String): String? = valueResult(ctx).value()?.find(field)?.asString()

fun command(spec: String, permission: String, handler: Handler, complete: Handler? = null): Int =
    withScopedMemoryAllocator { a ->
        val s = a.str(spec)
        val p = a.str(permission)
        rawCommandRegisterEx(s.ptr, s.len, p.ptr, p.len, tokenFor(handler), if (complete == null) -1 else tokenFor(complete))
    }

fun commandAlias(name: String, alias: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val l = a.str(alias)
    rawCommandAlias(n.ptr, n.len, l.ptr, l.len)
}

fun commandUnregister(name: String): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawCommandUnregister(n.ptr, n.len)
}

/** Runs a command line as the console (player 0) or as `player`. */
fun commandRun(line: String, player: Player = 0): Int = withScopedMemoryAllocator { a ->
    val l = a.str(line)
    rawCommandRun(l.ptr, l.len, player)
}

fun completeInput(ctx: Ctx): String? = text(callOut(256) { out, cap -> rawCommandCompleteInput(ctx, out, cap) })

fun completeAdd(ctx: Ctx, candidate: String): Int = withScopedMemoryAllocator { a ->
    val c = a.str(candidate)
    rawCommandCompleteAdd(ctx, c.ptr, c.len)
}

fun commandArgF64(ctx: Ctx, name: String): Double? = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val out = a.allocate(8)
    if (rawCommandArgF64(ctx, n.ptr, n.len, out.address.toInt()) == 0) Double.fromBits(out.loadLong()) else null
}

fun commandArgBool(ctx: Ctx, name: String): Boolean? = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val out = a.allocate(4)
    if (rawCommandArgBool(ctx, n.ptr, n.len, out.address.toInt()) == 0) out.loadInt() != 0 else null
}

/** `text` may be a "@key" lang key; the flags are BROADCAST_PLAYERS and BROADCAST_CONSOLE. */
fun broadcast(text: String, args: Args?, flags: Int): Int = withScopedMemoryAllocator { a ->
    val t = a.str(text)
    val p = a.args(args)
    rawServerBroadcast(t.ptr, t.len, p.ptr, p.len, flags)
}

fun placeholderFill(text: String, player: Player = 0): String? = text(callOut(256) { out, cap ->
    val t = str(text)
    rawPlaceholderFill(t.ptr, t.len, player, out, cap)
})

fun serverInfo(): OutResult = callOut(SERVER_BUFFER) { out, cap -> rawServerInfo(out, cap) }

fun serverCall(op: String, args: ValueWriter? = null): OutResult = callOut(SERVER_BUFFER) { out, cap ->
    val o = str(op)
    val d = doc(args?.finish())
    rawServerCall(o.ptr, o.len, d.ptr, d.len, out, cap)
}

fun profileLookup(name: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawProfileLookup(n.ptr, n.len, tokenFor(handler))
}

/** The value a completion callback (a wait, a lookup, a service call) carries. */
fun valueResult(ctx: Ctx): OutResult = callOut(SERVER_BUFFER) { out, cap -> rawValueResult(ctx, out, cap) }

/** Reads up to `cap` bytes of a plugin file. */
fun fileRead(path: String, cap: Int): OutResult = callOut(cap) { out, c ->
    val p = str(path)
    rawFilesRead(p.ptr, p.len, out, c)
}

fun fileWrite(path: String, data: String): Int = withScopedMemoryAllocator { a ->
    val p = a.str(path)
    val d = a.str(data)
    rawFilesWrite(p.ptr, p.len, d.ptr, d.len)
}

fun fileExists(path: String): Boolean = withScopedMemoryAllocator { a ->
    val p = a.str(path)
    rawFilesExists(p.ptr, p.len) == 1
}

fun fileDelete(path: String, recursive: Boolean = false): Int = withScopedMemoryAllocator { a ->
    val p = a.str(path)
    rawFilesDelete(p.ptr, p.len, if (recursive) 1 else 0)
}

private fun pathValue(path: String, call: (Int, Int, Int, Int) -> Int): OutResult =
    callOut(SERVER_BUFFER) { out, cap ->
        val p = str(path)
        call(p.ptr, p.len, out, cap)
    }

fun fileList(prefix: String): OutResult = pathValue(prefix, ::rawFilesList)

fun fileStat(path: String): OutResult = pathValue(path, ::rawFilesStat)

fun fileWalk(path: String): OutResult = pathValue(path, ::rawFilesWalk)

private fun storeSet(key: String, doc: ByteArray?): Int = withScopedMemoryAllocator { a ->
    val k = a.str(key)
    val d = a.doc(doc)
    rawStoreSet(k.ptr, k.len, d.ptr, d.len)
}

fun storeSet(key: String, value: Long): Int = storeSet(key, ValueWriter().int(value).finish())

fun storeSet(key: String, value: String): Int = storeSet(key, ValueWriter().string(value).finish())

fun storeGet(key: String): OutResult = pathValue(key, ::rawStoreGet)

fun dataWrite(name: String, doc: ValueWriter): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val d = a.doc(doc.finish())
    rawDataWrite(n.ptr, n.len, d.ptr, d.len)
}

fun dataRead(name: String): OutResult = pathValue(name, ::rawDataRead)

fun configValue(key: String): OutResult = pathValue(key, ::rawConfigValue)

fun serviceRegister(name: String, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawServiceRegister(n.ptr, n.len, tokenFor(handler))
}

fun serviceReply(ctx: Ctx, doc: ValueWriter): Int = withScopedMemoryAllocator { a ->
    val d = a.doc(doc.finish())
    rawServiceReply(ctx, d.ptr, d.len)
}

fun serviceReplyNull(ctx: Ctx): Int = rawServiceReply(ctx, 0, 0)

fun serviceHas(name: String): Boolean = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    rawServiceHas(n.ptr, n.len) == 1
}

fun serviceCall(name: String, args: ValueWriter, timeout: Int, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val n = a.str(name)
    val d = a.doc(args.finish())
    rawServiceCall(n.ptr, n.len, d.ptr, d.len, timeout, tokenFor(handler))
}

fun taskWait(ticks: Int, handler: Handler): Int = rawTaskWait(ticks, tokenFor(handler))

fun taskAt(world: String, x: Double, z: Double, handler: Handler): Int = withScopedMemoryAllocator { a ->
    val w = a.str(world)
    rawTaskAt(w.ptr, w.len, x, z, tokenFor(handler))
}
