package admin

import "basaltmc.org/sdk/proxy/internal/runtime"

func ServersRegister(name, host string, port int32, like string) (int32, string) {
	return runtime.ProxyServersRegister(name, host, port, like)
}

func ServersUnregister(name string) int32 { return runtime.ProxyServersUnregister(name) }

func ServersPing(name string, handler runtime.Handler) int32 {
	return runtime.ProxyServersPing(name, handler)
}

func Wait(ticks int32, handler runtime.Handler) int32 { return runtime.ProxyWait(ticks, handler) }

func CompletionValue(ctx runtime.Ctx) (runtime.Value, error) {
	return runtime.ProxyCompletionValue(ctx)
}

func ConsoleRun(line string) int32 { return runtime.ProxyConsoleRun(line) }

func PlayerChat(player runtime.Player, text string) int32 {
	return runtime.ProxyPlayerChat(player, text)
}

func PlayerSendJson(player runtime.Player, json string) int32 {
	return runtime.ProxyPlayerSendJson(player, json)
}

func PlayerKickJson(player runtime.Player, json string) int32 {
	return runtime.ProxyPlayerKickJson(player, json)
}

func PermissionCheck(who, node, server string) int32 {
	return runtime.ProxyPermissionCheck(who, node, server)
}

func PermissionMeta(who, key string) (string, bool) { return runtime.ProxyPermissionMeta(who, key) }

func PermissionProvide(check, meta runtime.Handler, priority int32) int32 {
	return runtime.ProxyPermissionProvide(check, meta, priority)
}

func PermissionUnprovide(provider int32) int32 { return runtime.ProxyPermissionUnprovide(provider) }

func PermissionRequest(ctx runtime.Ctx) (runtime.Value, error) {
	return runtime.ProxyPermissionRequest(ctx)
}

func PermissionAnswer(ctx runtime.Ctx, answer int32) int32 {
	return runtime.ProxyPermissionAnswer(ctx, answer)
}

func PermissionAnswerText(ctx runtime.Ctx, text string) int32 {
	return runtime.ProxyPermissionAnswerText(ctx, text)
}

func PermissionChanged() int32 { return runtime.ProxyPermissionChanged() }

func PermissionOverride(name, node string, value int32) int32 {
	return runtime.ProxyPermissionOverride(name, node, value)
}

func PermissionList(name string) (runtime.Value, error) { return runtime.ProxyPermissionList(name) }

func GroupGet(name string) (string, bool) { return runtime.ProxyGroupGet(name) }

func GroupSet(name, group string) int32 { return runtime.ProxyGroupSet(name, group) }

func DataValue(holder, key string) (runtime.Value, error) { return runtime.ProxyDataValue(holder, key) }

func DataSetValue(holder, key string, doc []byte) int32 {
	return runtime.ProxyDataSetValue(holder, key, doc)
}

func DataSetInt(holder, key string, number int64) int32 {
	return runtime.ProxyDataSetInt(holder, key, number)
}

func DataSetFloat(holder, key string, number float64) int32 {
	return runtime.ProxyDataSetFloat(holder, key, number)
}

func DataSetBool(holder, key string, flag bool) int32 {
	return runtime.ProxyDataSetBool(holder, key, flag)
}

func DataSetStr(holder, key, text string) int32 { return runtime.ProxyDataSetStr(holder, key, text) }

func DataRemove(holder, key string) int32 { return runtime.ProxyDataRemove(holder, key) }

func StoreTop(key string, limit int32) (runtime.Value, error) {
	return runtime.ProxyStoreTop(key, limit)
}
