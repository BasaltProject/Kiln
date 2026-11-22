@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.proxy.text
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*


import org.basaltmc.proxy.core.Args
import org.basaltmc.proxy.internal.*

fun translate(key: String, args: Args? = null): String? = fetch { out, cap ->
    val k = str(key)
    val a = args(args)
    rawTranslate(k.ptr, k.len, a.ptr, a.len, out, cap)
}
