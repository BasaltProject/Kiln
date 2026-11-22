@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)
package org.basaltmc.proxy.network
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import kotlin.wasm.unsafe.*

fun serversList(): List<String> =
    (fetch { out, cap -> rawProxyServersList(out, cap) } ?: "").split('\n').filter { it.isNotEmpty() }

fun serverAddress(name: String): String? = fetch { out, cap ->
    val n = str(name)
    rawProxyServerAddress(n.ptr, n.len, out, cap)
}

fun playersOn(server: String): List<Player> {
    var cap = 64
    while (true) {
        val r: Any? = withScopedMemoryAllocator { a ->
            val s = a.str(server)
            val out = a.allocate(cap * 4)
            val n = rawProxyPlayersOn(s.ptr, s.len, out.address.toInt(), cap)
            if (n < 0) null else if (n > cap) n else List(n) { (out + it * 4).loadInt() }
        }
        if (r == null) return emptyList()
        if (r is Int) cap = r else {
            @Suppress("UNCHECKED_CAST")
            return r as List<Player>
        }
    }
}
