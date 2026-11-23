package org.basaltmc.proxy.tasks

import org.basaltmc.proxy.core.*
import org.basaltmc.proxy.core.Handler
import org.basaltmc.proxy.internal.*
import org.basaltmc.proxy.values.*

fun after(delay: Int, handler: Handler): Int = rawTaskAfter(delay, tokenFor(handler))
fun every(delay: Int, period: Int, handler: Handler): Int = rawTaskEvery(delay, period, tokenFor(handler))
fun taskCancel(task: Int): Int = rawTaskCancel(task)
