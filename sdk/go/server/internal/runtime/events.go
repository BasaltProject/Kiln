package runtime

// Events area: slices and strings over the raw imports, documents as Value. Statuses are the negative BASALT_ERR_*
// codes as int32.

const (
	EventFlagPost      int32 = 1
	EventFlagCancelled int32 = 2
)

func bytesPtr(b []byte) (*byte, int32) {
	if len(b) == 0 {
		return nil, 0
	}
	return &b[0], int32(len(b))
}

// EventOnEx listens to name; filters is a finished map document or nil.
func EventOnEx(name string, priority, flags int32, filters []byte, handler Handler) int32 {
	n, nl := ptr(name)
	f, fl := bytesPtr(filters)
	return rawEventOnEx(n, nl, priority, flags, f, fl, tokenFor(handler))
}

func EventName(ctx Ctx) (string, bool) {
	return fetchWith(64, func(out *byte, cap int32) int32 { return rawEventName(ctx, out, cap) })
}

func EventGet(ctx Ctx, field string, initial int) (Value, error) {
	f, fl := ptr(field)
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawEventGet(ctx, f, fl, out, cap) })
}

func EventSet(ctx Ctx, field string, value []byte) int32 {
	f, fl := ptr(field)
	v, vl := bytesPtr(value)
	return rawEventSet(ctx, f, fl, v, vl)
}

func EventFields(ctx Ctx, initial int) (Value, error) {
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawEventFields(ctx, out, cap) })
}

func EventPayload(ctx Ctx, initial int) (Value, error) {
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawEventPayload(ctx, out, cap) })
}

func EventDescribe(name string, initial int) (Value, error) {
	n, nl := ptr(name)
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawEventDescribe(n, nl, out, cap) })
}

func EventNames(initial int) (Value, error) {
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawEventNames(out, cap) })
}

func EventCancelled(ctx Ctx) int32 { return rawEventCancelled(ctx) }

func EventCancelWith(ctx Ctx, text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawEventCancelWith(ctx, t, tl, a, al)
}

func EventCancelMessage(ctx Ctx) (string, bool) {
	return fetchWith(256, func(out *byte, cap int32) int32 { return rawEventCancelMessage(ctx, out, cap) })
}

func EventIsPost(ctx Ctx) int32 { return rawEventIsPost(ctx) }

func EventEmit(name string, value []byte) int32 {
	n, nl := ptr(name)
	v, vl := bytesPtr(value)
	return rawEventEmit(n, nl, v, vl)
}

func PlayerByUuid(uuid string) Player {
	u, ul := ptr(uuid)
	return rawPlayerByUuid(u, ul)
}

func PlayerUuid(player Player) (string, bool) {
	return fetchWith(64, func(out *byte, cap int32) int32 { return rawPlayerUuid(player, out, cap) })
}

func PlayerGet(player Player, name string, initial int) (Value, error) {
	n, nl := ptr(name)
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawPlayerGet(player, n, nl, out, cap) })
}

func PlayerSet(player Player, name string, value []byte) int32 {
	n, nl := ptr(name)
	v, vl := bytesPtr(value)
	return rawPlayerSet(player, n, nl, v, vl)
}

// PlayerCall runs a named operation; args is a finished list document or nil.
func PlayerCall(player Player, op string, args []byte, initial int) (Value, error) {
	o, ol := ptr(op)
	a, al := bytesPtr(args)
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawPlayerCall(player, o, ol, a, al, out, cap) })
}

func PlayerDescribe(initial int) (Value, error) {
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawPlayerDescribe(out, cap) })
}

func PlayerHealth(player Player) (float64, bool) {
	var health float64
	return health, rawPlayerHealth(player, &health) >= 0
}

func PlayerSetHealth(player Player, health float64) int32 { return rawPlayerSetHealth(player, health) }

func PlayerGamemode(player Player) (string, bool) {
	return fetchWith(16, func(out *byte, cap int32) int32 { return rawPlayerGamemode(player, out, cap) })
}

func PlayerSetGamemode(player Player, mode string) int32 {
	m, ml := ptr(mode)
	return rawPlayerSetGamemode(player, m, ml)
}

func PlayerTeleport(player Player, world string, x, y, z float64, yaw, pitch float32) int32 {
	w, wl := ptr(world)
	return rawPlayerTeleport(player, w, wl, x, y, z, yaw, pitch)
}

func PlayerTitle(player Player, title, subtitle string, fadeIn, stay, fadeOut int32, args *Args) int32 {
	t, tl := ptr(title)
	s, sl := ptr(subtitle)
	a, al := args.packed()
	return rawPlayerTitle(player, t, tl, s, sl, fadeIn, stay, fadeOut, a, al)
}

func PlayerKick(player Player, text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawPlayerKick(player, t, tl, a, al)
}

func PlayerEffect(player Player, effect string, ticks, amplifier int32) int32 {
	e, el := ptr(effect)
	return rawPlayerEffect(player, e, el, ticks, amplifier)
}

func PlayerOpenInventory(player Player, kind string, spec []byte) int32 {
	k, kl := ptr(kind)
	s, sl := bytesPtr(spec)
	return rawPlayerOpenInventory(player, k, kl, s, sl)
}

func PlayerCloseWindow(player Player) int32 { return rawPlayerCloseWindow(player) }

// FormOpen shows a form (a finished document); the handler runs once with the answer, read with CompletionValue.
func FormOpen(player Player, form []byte, handler Handler) int32 {
	f, fl := bytesPtr(form)
	return rawFormOpen(player, f, fl, tokenFor(handler))
}

func FormClose(player Player) int32 { return rawFormClose(player) }

func DialogShow(player Player, dialog string, handler Handler) int32 {
	d, dl := ptr(dialog)
	return rawDialogShow(player, d, dl, tokenFor(handler))
}

func DialogClear(player Player) int32 { return rawDialogClear(player) }

func CompletionValue(ctx Ctx, initial int) (Value, error) {
	return CallValue(initial, func(out *byte, cap int32) int32 { return rawValueResult(ctx, out, cap) })
}
