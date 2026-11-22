package events

import "basaltmc.org/sdk/proxy/internal/runtime"

func On(event string, priority int32, handler runtime.Handler) int32 {
	return runtime.On(event, priority, handler)
}

func EventPlayer(ctx runtime.Ctx) runtime.Player { return runtime.EventPlayer(ctx) }

func EventFieldStr(ctx runtime.Ctx, field string) (string, bool) {
	return runtime.EventFieldStr(ctx, field)
}

func EventFieldI64(ctx runtime.Ctx, field string) (int64, bool) {
	return runtime.EventFieldI64(ctx, field)
}

func EventCancel(ctx runtime.Ctx, cancelled bool) int32 { return runtime.EventCancel(ctx, cancelled) }

func EventSetStr(ctx runtime.Ctx, field, value string) int32 {
	return runtime.ProxyEventSetStr(ctx, field, value)
}

func EventSetI64(ctx runtime.Ctx, field string, value int64) int32 {
	return runtime.ProxyEventSetI64(ctx, field, value)
}

func Watch(event string, handler runtime.Handler) int32 { return runtime.ProxyWatch(event, handler) }

func EventGetValue(ctx runtime.Ctx, field string) ([]byte, int32) {
	return runtime.ProxyEventGetValue(ctx, field)
}

func EventGetStr(ctx runtime.Ctx, field string) (string, bool) {
	return runtime.ProxyEventGetStr(ctx, field)
}

func EventGetI64(ctx runtime.Ctx, field string) (int64, bool) {
	return runtime.ProxyEventGetI64(ctx, field)
}

func EventGetBool(ctx runtime.Ctx, field string) (bool, bool) {
	return runtime.ProxyEventGetBool(ctx, field)
}

func EventFields(ctx runtime.Ctx) ([]byte, int32) { return runtime.ProxyEventFields(ctx) }

func EventSetValue(ctx runtime.Ctx, field string, value []byte) int32 {
	return runtime.ProxyEventSetValue(ctx, field, value)
}

func EventSetText(ctx runtime.Ctx, field, text string) int32 {
	return runtime.ProxyEventSetText(ctx, field, text)
}

func EventSetInt(ctx runtime.Ctx, field string, number int64) int32 {
	return runtime.ProxyEventSetInt(ctx, field, number)
}

func EventSetFlag(ctx runtime.Ctx, field string, flag bool) int32 {
	return runtime.ProxyEventSetFlag(ctx, field, flag)
}

func EventDeny(ctx runtime.Ctx, reason string) int32 { return runtime.ProxyEventDeny(ctx, reason) }

func PluginInfo(field string) (string, bool) { return runtime.ProxyPluginInfo(field) }

func PluginManifest() ([]byte, int32) { return runtime.ProxyPluginManifest() }

func ConfigValue(key string) ([]byte, int32) { return runtime.ProxyConfigValue(key) }

func CommandArgF64(ctx runtime.Ctx, name string) (float64, bool) {
	return runtime.ProxyCommandArgF64(ctx, name)
}

func CommandArgBool(ctx runtime.Ctx, name string) (bool, bool) {
	return runtime.ProxyCommandArgBool(ctx, name)
}

func CommandArgs(ctx runtime.Ctx) ([]byte, int32) { return runtime.ProxyCommandArgs(ctx) }
