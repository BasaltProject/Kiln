@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.text

import org.basaltmc.server.core.*
import org.basaltmc.server.core.Args
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*

fun translate(key: String, args: Args? = null): String? = fetch { out, cap ->
    val k = str(key)
    val a = args(args)
    rawTranslate(k.ptr, k.len, a.ptr, a.len, out, cap)
}
