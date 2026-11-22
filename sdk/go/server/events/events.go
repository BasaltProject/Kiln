package events

import "basaltmc.org/sdk/server/internal/runtime"

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

const EventFlagPost = runtime.EventFlagPost

const EventFlagCancelled = runtime.EventFlagCancelled

func EventOnEx(name string, priority, flags int32, filters []byte, handler runtime.Handler) int32 {
	return runtime.EventOnEx(name, priority, flags, filters, handler)
}

func EventName(ctx runtime.Ctx) (string, bool) { return runtime.EventName(ctx) }

func EventGet(ctx runtime.Ctx, field string, initial int) (runtime.Value, error) {
	return runtime.EventGet(ctx, field, initial)
}

func EventSet(ctx runtime.Ctx, field string, value []byte) int32 {
	return runtime.EventSet(ctx, field, value)
}

func EventFields(ctx runtime.Ctx, initial int) (runtime.Value, error) {
	return runtime.EventFields(ctx, initial)
}

func EventPayload(ctx runtime.Ctx, initial int) (runtime.Value, error) {
	return runtime.EventPayload(ctx, initial)
}

func EventDescribe(name string, initial int) (runtime.Value, error) {
	return runtime.EventDescribe(name, initial)
}

func EventNames(initial int) (runtime.Value, error) { return runtime.EventNames(initial) }

func EventCancelled(ctx runtime.Ctx) int32 { return runtime.EventCancelled(ctx) }

func EventCancelWith(ctx runtime.Ctx, text string, args *runtime.Args) int32 {
	return runtime.EventCancelWith(ctx, text, args)
}

func EventCancelMessage(ctx runtime.Ctx) (string, bool) { return runtime.EventCancelMessage(ctx) }

func EventIsPost(ctx runtime.Ctx) int32 { return runtime.EventIsPost(ctx) }

func EventEmit(name string, value []byte) int32 { return runtime.EventEmit(name, value) }

func PlayerByUuid(uuid string) runtime.Player { return runtime.PlayerByUuid(uuid) }

func PlayerUuid(player runtime.Player) (string, bool) { return runtime.PlayerUuid(player) }

func PlayerGet(player runtime.Player, name string, initial int) (runtime.Value, error) {
	return runtime.PlayerGet(player, name, initial)
}

func PlayerSet(player runtime.Player, name string, value []byte) int32 {
	return runtime.PlayerSet(player, name, value)
}

func PlayerCall(player runtime.Player, op string, args []byte, initial int) (runtime.Value, error) {
	return runtime.PlayerCall(player, op, args, initial)
}

func PlayerDescribe(initial int) (runtime.Value, error) { return runtime.PlayerDescribe(initial) }

func PlayerHealth(player runtime.Player) (float64, bool) { return runtime.PlayerHealth(player) }

func PlayerSetHealth(player runtime.Player, health float64) int32 {
	return runtime.PlayerSetHealth(player, health)
}

func PlayerGamemode(player runtime.Player) (string, bool) { return runtime.PlayerGamemode(player) }

func PlayerSetGamemode(player runtime.Player, mode string) int32 {
	return runtime.PlayerSetGamemode(player, mode)
}

func PlayerTeleport(player runtime.Player, world string, x, y, z float64, yaw, pitch float32) int32 {
	return runtime.PlayerTeleport(player, world, x, y, z, yaw, pitch)
}

func PlayerTitle(player runtime.Player, title, subtitle string, fadeIn, stay, fadeOut int32, args *runtime.Args) int32 {
	return runtime.PlayerTitle(player, title, subtitle, fadeIn, stay, fadeOut, args)
}

func PlayerKick(player runtime.Player, text string, args *runtime.Args) int32 {
	return runtime.PlayerKick(player, text, args)
}

func PlayerEffect(player runtime.Player, effect string, ticks, amplifier int32) int32 {
	return runtime.PlayerEffect(player, effect, ticks, amplifier)
}

func PlayerOpenInventory(player runtime.Player, kind string, spec []byte) int32 {
	return runtime.PlayerOpenInventory(player, kind, spec)
}

func PlayerCloseWindow(player runtime.Player) int32 { return runtime.PlayerCloseWindow(player) }

func FormOpen(player runtime.Player, form []byte, handler runtime.Handler) int32 {
	return runtime.FormOpen(player, form, handler)
}

func FormClose(player runtime.Player) int32 { return runtime.FormClose(player) }

func DialogShow(player runtime.Player, dialog string, handler runtime.Handler) int32 {
	return runtime.DialogShow(player, dialog, handler)
}

func DialogClear(player runtime.Player) int32 { return runtime.DialogClear(player) }

func CompletionValue(ctx runtime.Ctx, initial int) (runtime.Value, error) {
	return runtime.CompletionValue(ctx, initial)
}
