@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.internal

import kotlin.wasm.unsafe.MemoryAllocator
import kotlin.wasm.unsafe.Pointer
import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.Args
import org.basaltmc.server.core.Handler

// Linear memory and callback plumbing shared by every area package.


internal val handlers = ArrayList<Handler>()

internal fun tokenFor(handler: Handler): Int {
    handlers.add(handler)
    return handlers.size
}

internal class Bytes(val ptr: Int, val len: Int)

internal fun MemoryAllocator.bytes(data: ByteArray): Bytes {
    if (data.isEmpty()) return Bytes(0, 0)
    val p = allocate(data.size)
    for (i in data.indices) (p + i).storeByte(data[i])
    return Bytes(p.address.toInt(), data.size)
}

internal fun MemoryAllocator.str(text: String) = bytes(text.encodeToByteArray())

internal fun MemoryAllocator.args(args: Args?) = if (args == null || args.count == 0) Bytes(0, 0) else bytes(args.packed())

internal fun read(at: Int, n: Int): String {
    val p = Pointer(at.toUInt())
    return ByteArray(n) { (p + it).loadByte() }.decodeToString()
}

internal inline fun fetch(call: MemoryAllocator.(Int, Int) -> Int): String? {
    var cap = 256
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val out = a.allocate(cap).address.toInt()
            val n = a.call(out, cap)
            if (n < 0) null else if (n > cap) n else read(out, n)
        }
        if (r == null) return null
        if (r is Int) cap = r else return r as String
    }
}
