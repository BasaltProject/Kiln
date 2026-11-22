package tasks

import "basaltmc.org/sdk/server/internal/runtime"

func After(delay int32, handler runtime.Handler) int32 { return runtime.After(delay, handler) }

func Every(delay, period int32, handler runtime.Handler) int32 {
	return runtime.Every(delay, period, handler)
}

func TaskCancel(task int32) int32 { return runtime.TaskCancel(task) }
