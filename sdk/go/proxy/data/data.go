package data

import "basaltmc.org/sdk/proxy/internal/runtime"

func DataGetI64(player runtime.Player, key string, def int64) int64 {
	return runtime.DataGetI64(player, key, def)
}

func DataSetI64(player runtime.Player, key string, value int64) int32 {
	return runtime.DataSetI64(player, key, value)
}

func DataGetBool(player runtime.Player, key string, def bool) bool {
	return runtime.DataGetBool(player, key, def)
}

func DataSetBool(player runtime.Player, key string, value bool) int32 {
	return runtime.DataSetBool(player, key, value)
}

func DataGetStr(player runtime.Player, key string) (string, bool) {
	return runtime.DataGetStr(player, key)
}

func DataSetStr(player runtime.Player, key, value string) int32 {
	return runtime.DataSetStr(player, key, value)
}

func DataRemove(player runtime.Player, key string) int32 { return runtime.DataRemove(player, key) }
