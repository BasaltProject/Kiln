package config

import "basaltmc.org/sdk/server/internal/runtime"

func ConfigI64(key string, def int64) int64 { return runtime.ConfigI64(key, def) }

func ConfigF64(key string, def float64) float64 { return runtime.ConfigF64(key, def) }

func ConfigBool(key string, def bool) bool { return runtime.ConfigBool(key, def) }

func ConfigStr(key string) (string, bool) { return runtime.ConfigStr(key) }
