package server

import "basaltmc.org/sdk/server/internal/runtime"

const BroadcastPlayers = runtime.BroadcastPlayers

const BroadcastConsole = runtime.BroadcastConsole

const ProvideCheck = runtime.ProvideCheck

const ProvideMeta = runtime.ProvideMeta

func ServerResult(ctx runtime.Ctx) (runtime.Value, int32) { return runtime.ServerResult(ctx) }

func PermissionCheck(subject, node string) int32 { return runtime.PermissionCheck(subject, node) }

func PermissionHas(player runtime.Player, node string) int32 {
	return runtime.PermissionHas(player, node)
}

func PermissionGrant(player runtime.Player, node string) int32 {
	return runtime.PermissionGrant(player, node)
}

func PermissionDeny(player runtime.Player, node string) int32 {
	return runtime.PermissionDeny(player, node)
}

func PermissionDrop(player runtime.Player, node string) int32 {
	return runtime.PermissionDrop(player, node)
}

func PermissionMeta(subject, key string) (string, bool) { return runtime.PermissionMeta(subject, key) }

func PermissionProvide(priority, kinds int32, handler runtime.Handler) int32 {
	return runtime.PermissionProvide(priority, kinds, handler)
}

func PermissionAnswer(ctx runtime.Ctx, answer int32) int32 {
	return runtime.PermissionAnswer(ctx, answer)
}

func PermissionAnswerMeta(ctx runtime.Ctx, text string) int32 {
	return runtime.PermissionAnswerMeta(ctx, text)
}

func PermissionQuery(ctx runtime.Ctx, field string) (string, bool) {
	return runtime.PermissionQuery(ctx, field)
}

func CommandEx(spec, permission string, handler, complete runtime.Handler) int32 {
	return runtime.CommandEx(spec, permission, handler, complete)
}

func CommandAlias(name, alias string) int32 { return runtime.CommandAlias(name, alias) }

func CommandUnregister(name string) int32 { return runtime.CommandUnregister(name) }

func CommandRun(line string, player runtime.Player) int32 { return runtime.CommandRun(line, player) }

func CompleteInput(ctx runtime.Ctx) (string, bool) { return runtime.CompleteInput(ctx) }

func CompleteAdd(ctx runtime.Ctx, candidate string) int32 { return runtime.CompleteAdd(ctx, candidate) }

func CommandArgF64(ctx runtime.Ctx, name string) (float64, bool) {
	return runtime.CommandArgF64(ctx, name)
}

func CommandArgBool(ctx runtime.Ctx, name string) (bool, bool) {
	return runtime.CommandArgBool(ctx, name)
}

func Broadcast(text string, args *runtime.Args, flags int32) int32 {
	return runtime.Broadcast(text, args, flags)
}

func PlaceholderFill(text string, player runtime.Player) (string, bool) {
	return runtime.PlaceholderFill(text, player)
}

func ServerInfo() (runtime.Value, int32) { return runtime.ServerInfo() }

func ServerCall(op string, args []byte) (runtime.Value, int32) { return runtime.ServerCall(op, args) }

func ProfileLookup(name string, handler runtime.Handler) int32 {
	return runtime.ProfileLookup(name, handler)
}

func FileRead(path string, buf []byte) int32 { return runtime.FileRead(path, buf) }

func FileWrite(path, data string) int32 { return runtime.FileWrite(path, data) }

func FileExists(path string) bool { return runtime.FileExists(path) }

func FileDelete(path string, recursive bool) int32 { return runtime.FileDelete(path, recursive) }

func FileList(prefix string) (runtime.Value, int32) { return runtime.FileList(prefix) }

func FileStat(path string) (runtime.Value, int32) { return runtime.FileStat(path) }

func FileWalk(path string) (runtime.Value, int32) { return runtime.FileWalk(path) }

func StoreSetInt(key string, value int64) int32 { return runtime.StoreSetInt(key, value) }

func StoreSetString(key, value string) int32 { return runtime.StoreSetString(key, value) }

func StoreGet(key string) (runtime.Value, int32) { return runtime.StoreGet(key) }

func DataWrite(name string, doc []byte) int32 { return runtime.DataWrite(name, doc) }

func DataRead(name string) (runtime.Value, int32) { return runtime.DataRead(name) }

func ConfigValue(key string) (runtime.Value, int32) { return runtime.ConfigValue(key) }

func ServiceRegister(name string, handler runtime.Handler) int32 {
	return runtime.ServiceRegister(name, handler)
}

func ServiceReply(ctx runtime.Ctx, doc []byte) int32 { return runtime.ServiceReply(ctx, doc) }

func ServiceHas(name string) bool { return runtime.ServiceHas(name) }

func ServiceCall(name string, args []byte, timeout int32, handler runtime.Handler) int32 {
	return runtime.ServiceCall(name, args, timeout, handler)
}

func Wait(ticks int32, handler runtime.Handler) int32 { return runtime.Wait(ticks, handler) }

func At(world string, x, z float64, handler runtime.Handler) int32 {
	return runtime.At(world, x, z, handler)
}
