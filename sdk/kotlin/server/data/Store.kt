@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.data
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.internal.*

class TopEntry(val name: String, val value: Long)

/** The `limit` highest values of a numeric data key, best first. */
fun storeTop(key: String, limit: Int): List<TopEntry> {
    var cap = 1024
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val k = a.str(key)
            val out = a.allocate(cap)
            val n = rawStoreTop(k.ptr, k.len, limit, out.address.toInt(), cap)
            if (n < 0) emptyList<TopEntry>() else if (n > cap) n else {
                val rows = ArrayList<TopEntry>()
                var at = 0
                while (at + 4 <= n) {
                    val len = (out + at).loadInt()
                    at += 4
                    if (at + len + 8 > n) break
                    val name = read(out.address.toInt() + at, len)
                    at += len
                    rows.add(TopEntry(name, (out + at).loadLong()))
                    at += 8
                }
                rows
            }
        }
        if (r is Int) cap = r else {
            @Suppress("UNCHECKED_CAST")
            return r as List<TopEntry>
        }
    }
}
