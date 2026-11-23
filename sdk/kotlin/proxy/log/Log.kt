@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.proxy.log

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.core.Level
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.internal.rawLog
import org.basaltmc.proxy.internal.str
import org.basaltmc.proxy.values.*

fun log(level: Int, msg: String) = withScopedMemoryAllocator { a -> val m = a.str(msg); rawLog(level, m.ptr, m.len) }
fun logDebug(msg: String) = log(Level.DEBUG, msg)
fun logInfo(msg: String) = log(Level.INFO, msg)
fun logWarn(msg: String) = log(Level.WARN, msg)
fun logError(msg: String) = log(Level.ERROR, msg)
