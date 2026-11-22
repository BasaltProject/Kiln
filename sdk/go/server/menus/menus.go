package menus

import "basaltmc.org/sdk/server/internal/runtime"

func Placeholder(name string, handler runtime.Handler) int32 {
	return runtime.Placeholder(name, handler)
}

func PlaceholderPlayer(ctx runtime.Ctx) runtime.Player { return runtime.PlaceholderPlayer(ctx) }

func PlaceholderResult(ctx runtime.Ctx, text string) int32 {
	return runtime.PlaceholderResult(ctx, text)
}

func MenuCreate(title string, rows int32) int32 { return runtime.MenuCreate(title, rows) }

func MenuSet(menu, slot int32, item, name, lore string, onClick runtime.Handler) int32 {
	return runtime.MenuSet(menu, slot, item, name, lore, onClick)
}

func MenuOpen(menu int32, player runtime.Player) int32 { return runtime.MenuOpen(menu, player) }

func MenuClickPlayer(ctx runtime.Ctx) runtime.Player { return runtime.MenuClickPlayer(ctx) }
