package commands

import "basaltmc.org/sdk/proxy/internal/runtime"

func Command(spec, permission string, handler runtime.Handler) int32 {
	return runtime.Command(spec, permission, handler)
}

func CommandReply(ctx runtime.Ctx, text string, args *runtime.Args) int32 {
	return runtime.CommandReply(ctx, text, args)
}

func CommandArgStr(ctx runtime.Ctx, name string) (string, bool) {
	return runtime.CommandArgStr(ctx, name)
}

func CommandArgI64(ctx runtime.Ctx, name string) (int64, bool) {
	return runtime.CommandArgI64(ctx, name)
}

func CommandSourceName(ctx runtime.Ctx) (string, bool) { return runtime.CommandSourceName(ctx) }

func CommandIsConsole(ctx runtime.Ctx) bool { return runtime.CommandIsConsole(ctx) }
