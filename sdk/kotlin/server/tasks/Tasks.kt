package org.basaltmc.server.tasks
import org.basaltmc.server.core.*
import org.basaltmc.server.internal.*
import org.basaltmc.server.values.*


import org.basaltmc.server.core.Handler
import org.basaltmc.server.internal.*

fun after(delay: Int, handler: Handler): Int = rawTaskAfter(delay, tokenFor(handler))
fun every(delay: Int, period: Int, handler: Handler): Int = rawTaskEvery(delay, period, tokenFor(handler))
fun taskCancel(task: Int): Int = rawTaskCancel(task)
