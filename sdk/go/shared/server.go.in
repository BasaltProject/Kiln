package runtime

// Server area: permissions, dynamic commands, broadcast, plugin files and storage, services and the scheduler
// completions. Calls that answer a value return it with the import status (0 on success, negative on error).

const serverBuffer = 1024

const (
	BroadcastPlayers = 1
	BroadcastConsole = 2
	ProvideCheck     = 1
	ProvideMeta      = 2
)

func valueStatus(v Value, err error) (Value, int32) {
	if err == nil {
		return v, 0
	}
	if code, ok := err.(CallError); ok {
		return Value{}, int32(code)
	}
	return Value{}, -3
}

func callValue(call func(out *byte, cap int32) int32) (Value, int32) {
	return valueStatus(CallValue(serverBuffer, call))
}

// ServerResult is the value a completion callback (a wait, a lookup, a service call or a provider query) carries.
func ServerResult(ctx Ctx) (Value, int32) {
	return callValue(func(out *byte, cap int32) int32 { return rawValueResult(ctx, out, cap) })
}

func PermissionCheck(subject, node string) int32 {
	s, sl := ptr(subject)
	n, nl := ptr(node)
	return rawPermissionCheck(s, sl, n, nl)
}

func PermissionHas(player Player, node string) int32 {
	n, nl := ptr(node)
	return rawPermissionHas(player, n, nl)
}

func permissionSet(player Player, node string, mode int32) int32 {
	n, nl := ptr(node)
	return rawPermissionSet(player, n, nl, mode)
}

func PermissionGrant(player Player, node string) int32 { return permissionSet(player, node, 1) }
func PermissionDeny(player Player, node string) int32  { return permissionSet(player, node, 0) }
func PermissionDrop(player Player, node string) int32  { return permissionSet(player, node, -1) }

func PermissionMeta(subject, key string) (string, bool) {
	s, sl := ptr(subject)
	k, kl := ptr(key)
	return fetchWith(128, func(out *byte, cap int32) int32 { return rawPermissionMeta(s, sl, k, kl, out, cap) })
}

func PermissionProvide(priority, kinds int32, handler Handler) int32 {
	return rawPermissionProvide(priority, kinds, tokenFor(handler))
}

func PermissionAnswer(ctx Ctx, answer int32) int32 { return rawPermissionAnswer(ctx, answer) }

func PermissionAnswerMeta(ctx Ctx, text string) int32 {
	t, tl := ptr(text)
	return rawPermissionAnswerMeta(ctx, t, tl)
}

// PermissionQuery is the node, or meta key, or kind a provider callback is asked about.
func PermissionQuery(ctx Ctx, field string) (string, bool) {
	root, status := ServerResult(ctx)
	if status < 0 {
		return "", false
	}
	item, ok := root.Find(field)
	if !ok {
		return "", false
	}
	return item.AsString()
}

// CommandEx registers a command that can be aliased and unregistered later; complete may be nil.
func CommandEx(spec, permission string, handler, complete Handler) int32 {
	s, sl := ptr(spec)
	p, pl := ptr(permission)
	token := tokenFor(handler)
	completeToken := int32(-1)
	if complete != nil {
		completeToken = tokenFor(complete)
	}
	return rawCommandRegisterEx(s, sl, p, pl, token, completeToken)
}

func CommandAlias(name, alias string) int32 {
	n, nl := ptr(name)
	a, al := ptr(alias)
	return rawCommandAlias(n, nl, a, al)
}

func CommandUnregister(name string) int32 {
	n, nl := ptr(name)
	return rawCommandUnregister(n, nl)
}

// CommandRun runs a command line as the console (player 0) or as player.
func CommandRun(line string, player Player) int32 {
	l, ll := ptr(line)
	return rawCommandRun(l, ll, player)
}

func CompleteInput(ctx Ctx) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawCommandCompleteInput(ctx, out, cap) })
}

func CompleteAdd(ctx Ctx, candidate string) int32 {
	c, cl := ptr(candidate)
	return rawCommandCompleteAdd(ctx, c, cl)
}

func CommandArgF64(ctx Ctx, name string) (float64, bool) {
	n, nl := ptr(name)
	var value float64
	ok := rawCommandArgF64(ctx, n, nl, &value) == 0
	return value, ok
}

func CommandArgBool(ctx Ctx, name string) (bool, bool) {
	n, nl := ptr(name)
	var value int32
	ok := rawCommandArgBool(ctx, n, nl, &value) == 0
	return value != 0, ok
}

// Broadcast text may be a "@key" lang key; args may be nil.
func Broadcast(text string, args *Args, flags int32) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawServerBroadcast(t, tl, a, al, flags)
}

func PlaceholderFill(text string, player Player) (string, bool) {
	t, tl := ptr(text)
	return fetch(func(out *byte, cap int32) int32 { return rawPlaceholderFill(t, tl, player, out, cap) })
}

func ServerInfo() (Value, int32) {
	return callValue(func(out *byte, cap int32) int32 { return rawServerInfo(out, cap) })
}

// ServerCall runs an admin operation; args is a finished document, or nil for null.
func ServerCall(op string, args []byte) (Value, int32) {
	o, ol := ptr(op)
	a, al := bytesPtr(args)
	return callValue(func(out *byte, cap int32) int32 { return rawServerCall(o, ol, a, al, out, cap) })
}

func ProfileLookup(name string, handler Handler) int32 {
	n, nl := ptr(name)
	return rawProfileLookup(n, nl, tokenFor(handler))
}

func FileRead(path string, buf []byte) int32 {
	p, pl := ptr(path)
	out, _ := bytesPtr(buf)
	return rawFilesRead(p, pl, out, int32(len(buf)))
}

func FileWrite(path, data string) int32 {
	p, pl := ptr(path)
	d, dl := ptr(data)
	return rawFilesWrite(p, pl, d, dl)
}

func FileExists(path string) bool {
	p, pl := ptr(path)
	return rawFilesExists(p, pl) == 1
}

func FileDelete(path string, recursive bool) int32 {
	p, pl := ptr(path)
	return rawFilesDelete(p, pl, b2i(recursive))
}

func FileList(prefix string) (Value, int32) {
	p, pl := ptr(prefix)
	return callValue(func(out *byte, cap int32) int32 { return rawFilesList(p, pl, out, cap) })
}

func FileStat(path string) (Value, int32) {
	p, pl := ptr(path)
	return callValue(func(out *byte, cap int32) int32 { return rawFilesStat(p, pl, out, cap) })
}

func FileWalk(path string) (Value, int32) {
	p, pl := ptr(path)
	return callValue(func(out *byte, cap int32) int32 { return rawFilesWalk(p, pl, out, cap) })
}

func storeSetDoc(key string, doc []byte, err error) int32 {
	if err != nil {
		return -3
	}
	k, kl := ptr(key)
	d, dl := bytesPtr(doc)
	return rawStoreSet(k, kl, d, dl)
}

func StoreSetInt(key string, value int64) int32 {
	doc, err := NewValueWriterLimit(16).Int(value).Finish()
	return storeSetDoc(key, doc, err)
}

func StoreSetString(key, value string) int32 {
	doc, err := NewValueWriterLimit(256).String(value).Finish()
	return storeSetDoc(key, doc, err)
}

func StoreGet(key string) (Value, int32) {
	k, kl := ptr(key)
	return callValue(func(out *byte, cap int32) int32 { return rawStoreGet(k, kl, out, cap) })
}

func DataWrite(name string, doc []byte) int32 {
	n, nl := ptr(name)
	d, dl := bytesPtr(doc)
	return rawDataWrite(n, nl, d, dl)
}

func DataRead(name string) (Value, int32) {
	n, nl := ptr(name)
	return callValue(func(out *byte, cap int32) int32 { return rawDataRead(n, nl, out, cap) })
}

// ConfigValue answers the whole config table for an empty key.
func ConfigValue(key string) (Value, int32) {
	k, kl := ptr(key)
	return callValue(func(out *byte, cap int32) int32 { return rawConfigValue(k, kl, out, cap) })
}

func ServiceRegister(name string, handler Handler) int32 {
	n, nl := ptr(name)
	return rawServiceRegister(n, nl, tokenFor(handler))
}

// ServiceReply answers the call a service handler runs for; doc is a finished document, or nil for null.
func ServiceReply(ctx Ctx, doc []byte) int32 {
	d, dl := bytesPtr(doc)
	return rawServiceReply(ctx, d, dl)
}

func ServiceHas(name string) bool {
	n, nl := ptr(name)
	return rawServiceHas(n, nl) == 1
}

func ServiceCall(name string, args []byte, timeout int32, handler Handler) int32 {
	n, nl := ptr(name)
	a, al := bytesPtr(args)
	return rawServiceCall(n, nl, a, al, timeout, tokenFor(handler))
}

func Wait(ticks int32, handler Handler) int32 { return rawTaskWait(ticks, tokenFor(handler)) }

func At(world string, x, z float64, handler Handler) int32 {
	w, wl := ptr(world)
	return rawTaskAt(w, wl, x, z, tokenFor(handler))
}
