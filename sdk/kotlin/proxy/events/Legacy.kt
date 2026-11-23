@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class, kotlin.wasm.ExperimentalWasmInterop::class)

package org.basaltmc.proxy.events

import kotlin.wasm.unsafe.*
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.internal.*

fun eventSetStr(ctx: Ctx, field: String, value: String): Int = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    val v = a.str(value)
    rawProxyEventSetStr(ctx, f.ptr, f.len, v.ptr, v.len)
}

fun eventSetI64(ctx: Ctx, field: String, value: Long): Int = withScopedMemoryAllocator { a ->
    val f = a.str(field)
    rawProxyEventSetI64(ctx, f.ptr, f.len, value)
}
