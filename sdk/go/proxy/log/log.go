package log

import "basaltmc.org/sdk/proxy/internal/runtime"

func Log(level int32, msg string) { runtime.Log(level, msg) }

func LogDebug(msg string) { runtime.LogDebug(msg) }

func LogInfo(msg string) { runtime.LogInfo(msg) }

func LogWarn(msg string) { runtime.LogWarn(msg) }

func LogError(msg string) { runtime.LogError(msg) }
