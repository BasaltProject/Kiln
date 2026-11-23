@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.proxy.config

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

fun configI64(key: String, def: Long): Long = withScopedMemoryAllocator { a -> val k = a.str(key); rawConfigI64(k.ptr, k.len, def) }
fun configF64(key: String, def: Double): Double = withScopedMemoryAllocator { a -> val k = a.str(key); rawConfigF64(k.ptr, k.len, def) }
fun configBool(key: String, def: Boolean): Boolean =
    withScopedMemoryAllocator { a -> val k = a.str(key); rawConfigBool(k.ptr, k.len, if (def) 1 else 0) != 0 }
fun configStr(key: String): String? = fetch { out, cap -> val k = str(key); rawConfigStr(k.ptr, k.len, out, cap) }
