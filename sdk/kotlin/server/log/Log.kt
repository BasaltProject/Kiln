@file:OptIn(kotlin.wasm.unsafe.UnsafeWasmMemoryApi::class)

package org.basaltmc.server.log

import kotlin.wasm.unsafe.withScopedMemoryAllocator
import org.basaltmc.server.core.*
import org.basaltmc.server.core.Level
import org.basaltmc.server.internal.*
import org.basaltmc.server.internal.rawLog
import org.basaltmc.server.internal.str
import org.basaltmc.server.values.*

fun log(level: Int, msg: String) = withScopedMemoryAllocator { a -> val m = a.str(msg); rawLog(level, m.ptr, m.len) }
fun logDebug(msg: String) = log(Level.DEBUG, msg)
fun logInfo(msg: String) = log(Level.INFO, msg)
fun logWarn(msg: String) = log(Level.WARN, msg)
fun logError(msg: String) = log(Level.ERROR, msg)
