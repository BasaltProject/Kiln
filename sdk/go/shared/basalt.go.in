// Package basalt is the Basalt plugin SDK for Go (TinyGo): the plugin ABI over strings. Call Init from a package
// level init function with the function that registers everything.
package runtime

import "unsafe"

type Player = int32
type Ctx = int32
type Handler = func(ctx Ctx)

const (
	Debug = 0
	Info  = 1
	Warn  = 2
	Error = 3

	Lowest  = 0
	Low     = 1
	Normal  = 2
	High    = 3
	Highest = 4
)

//go:wasmimport basalt log
func rawLog(level int32, msg *byte, n int32)

//go:wasmimport basalt config_i64
func rawConfigI64(key *byte, klen int32, def int64) int64

//go:wasmimport basalt config_f64
func rawConfigF64(key *byte, klen int32, def float64) float64

//go:wasmimport basalt config_bool
func rawConfigBool(key *byte, klen int32, def int32) int32

//go:wasmimport basalt config_str
func rawConfigStr(key *byte, klen int32, out *byte, cap int32) int32

//go:wasmimport basalt translate
func rawTranslate(key *byte, klen int32, args *byte, alen int32, out *byte, cap int32) int32

//go:wasmimport basalt player_by_name
func rawPlayerByName(name *byte, n int32) int32

//go:wasmimport basalt players_online
func rawPlayersOnline(out *int32, cap int32) int32

//go:wasmimport basalt player_name
func rawPlayerName(player int32, out *byte, cap int32) int32

//go:wasmimport basalt player_send
func rawPlayerSend(player int32, text *byte, tlen int32, args *byte, alen int32) int32

//go:wasmimport basalt player_action_bar
func rawPlayerActionBar(player int32, text *byte, tlen int32, args *byte, alen int32) int32

//go:wasmimport basalt player_particle
func rawPlayerParticle(player int32, id *byte, idlen int32, count int32, spread float64) int32

//go:wasmimport basalt player_sound
func rawPlayerSound(player int32, id *byte, idlen int32, volume float32, pitch float32) int32

//go:wasmimport basalt data_get_i64
func rawDataGetI64(player int32, key *byte, klen int32, def int64) int64

//go:wasmimport basalt data_set_i64
func rawDataSetI64(player int32, key *byte, klen int32, value int64) int32

//go:wasmimport basalt data_get_bool
func rawDataGetBool(player int32, key *byte, klen int32, def int32) int32

//go:wasmimport basalt data_set_bool
func rawDataSetBool(player int32, key *byte, klen int32, value int32) int32

//go:wasmimport basalt data_get_str
func rawDataGetStr(player int32, key *byte, klen int32, out *byte, cap int32) int32

//go:wasmimport basalt data_set_str
func rawDataSetStr(player int32, key *byte, klen int32, value *byte, vlen int32) int32

//go:wasmimport basalt data_remove
func rawDataRemove(player int32, key *byte, klen int32) int32

//go:wasmimport basalt store_top
func rawStoreTop(key *byte, klen int32, limit int32, out *byte, cap int32) int32

//go:wasmimport basalt event_on
func rawEventOn(name *byte, nlen int32, priority int32, token int32) int32

//go:wasmimport basalt event_player
func rawEventPlayer(ctx int32) int32

//go:wasmimport basalt event_field_str
func rawEventFieldStr(ctx int32, field *byte, flen int32, out *byte, cap int32) int32

//go:wasmimport basalt event_field_i64
func rawEventFieldI64(ctx int32, field *byte, flen int32, out *int64) int32

//go:wasmimport basalt event_cancel
func rawEventCancel(ctx int32, cancelled int32) int32

//go:wasmimport basalt command_register
func rawCommandRegister(spec *byte, slen int32, permission *byte, plen int32, token int32) int32

//go:wasmimport basalt command_source_name
func rawCommandSourceName(ctx int32, out *byte, cap int32) int32

//go:wasmimport basalt command_is_console
func rawCommandIsConsole(ctx int32) int32

//go:wasmimport basalt command_reply
func rawCommandReply(ctx int32, text *byte, tlen int32, args *byte, alen int32) int32

//go:wasmimport basalt command_arg_str
func rawCommandArgStr(ctx int32, name *byte, nlen int32, out *byte, cap int32) int32

//go:wasmimport basalt command_arg_i64
func rawCommandArgI64(ctx int32, name *byte, nlen int32, out *int64) int32

//go:wasmimport basalt task_after
func rawTaskAfter(delay int32, token int32) int32

//go:wasmimport basalt task_every
func rawTaskEvery(delay int32, period int32, token int32) int32

//go:wasmimport basalt task_cancel
func rawTaskCancel(task int32) int32

//go:wasmimport basalt placeholder_register
func rawPlaceholderRegister(name *byte, nlen int32, token int32) int32

//go:wasmimport basalt placeholder_player
func rawPlaceholderPlayer(ctx int32) int32

//go:wasmimport basalt placeholder_result
func rawPlaceholderResult(ctx int32, text *byte, n int32) int32

//go:wasmimport basalt menu_create
func rawMenuCreate(title *byte, tlen int32, rows int32) int32

//go:wasmimport basalt menu_set
func rawMenuSet(menu int32, slot int32, item *byte, ilen int32, name *byte, nlen int32, lore *byte, llen int32, token int32) int32

//go:wasmimport basalt menu_open
func rawMenuOpen(menu int32, player int32) int32

//go:wasmimport basalt menu_click_player
func rawMenuClickPlayer(ctx int32) int32

var setup func()
var handlers []Handler

// Init records the function that registers everything; call it from a package level init function.
func Init(register func()) { setup = register }

//go:wasmexport basalt_abi_version
func abiVersion() int32 { return 1 }

//go:wasmexport basalt_init
func initPlugin() {
	if setup != nil {
		setup()
	}
}

//go:wasmexport basalt_callback
func callback(token, ctx int32) {
	if token > 0 && int(token) <= len(handlers) {
		handlers[token-1](ctx)
	}
}

func tokenFor(handler Handler) int32 {
	if handler == nil {
		return 0
	}
	handlers = append(handlers, handler)
	return int32(len(handlers))
}

func ptr(s string) (*byte, int32) { return unsafe.StringData(s), int32(len(s)) }

func fetch(call func(out *byte, cap int32) int32) (string, bool) { return fetchWith(256, call) }

func fetchWith(size int, call func(out *byte, cap int32) int32) (string, bool) {
	buffer := make([]byte, size)
	for {
		needed := call(&buffer[0], int32(len(buffer)))
		if needed < 0 {
			return "", false
		}
		if int(needed) <= len(buffer) {
			return string(buffer[:needed]), true
		}
		buffer = make([]byte, needed)
	}
}

// Args are named arguments for text: add pairs, then pass the list to Send, Translate or CommandReply.
type Args struct {
	body  []byte
	count uint32
}

func NewArgs() *Args { return &Args{} }

func (a *Args) Add(key, value string) *Args {
	a.count++
	for _, part := range [2]string{key, value} {
		n := uint32(len(part))
		a.body = append(a.body, byte(n), byte(n>>8), byte(n>>16), byte(n>>24))
		a.body = append(a.body, part...)
	}
	return a
}

func (a *Args) packed() (*byte, int32) {
	if a == nil || a.count == 0 {
		return nil, 0
	}
	out := []byte{byte(a.count), byte(a.count >> 8), byte(a.count >> 16), byte(a.count >> 24)}
	out = append(out, a.body...)
	return &out[0], int32(len(out))
}

func Log(level int32, msg string) { p, n := ptr(msg); rawLog(level, p, n) }
func LogDebug(msg string)         { Log(Debug, msg) }
func LogInfo(msg string)          { Log(Info, msg) }
func LogWarn(msg string)          { Log(Warn, msg) }
func LogError(msg string)         { Log(Error, msg) }

func ConfigI64(key string, def int64) int64 { k, kl := ptr(key); return rawConfigI64(k, kl, def) }
func ConfigF64(key string, def float64) float64 {
	k, kl := ptr(key)
	return rawConfigF64(k, kl, def)
}
func ConfigBool(key string, def bool) bool {
	k, kl := ptr(key)
	return rawConfigBool(k, kl, b2i(def)) != 0
}
func ConfigStr(key string) (string, bool) {
	k, kl := ptr(key)
	return fetch(func(out *byte, cap int32) int32 { return rawConfigStr(k, kl, out, cap) })
}
func Translate(key string, args *Args) (string, bool) {
	k, kl := ptr(key)
	a, al := args.packed()
	return fetch(func(out *byte, cap int32) int32 { return rawTranslate(k, kl, a, al, out, cap) })
}

func b2i(v bool) int32 {
	if v {
		return 1
	}
	return 0
}

func PlayerByName(name string) Player { p, n := ptr(name); return rawPlayerByName(p, n) }
func PlayersOnline() []Player {
	out := make([]int32, 64)
	for {
		count := rawPlayersOnline(&out[0], int32(len(out)))
		if count < 0 {
			return nil
		}
		if int(count) <= len(out) {
			return out[:count]
		}
		out = make([]int32, count)
	}
}
func PlayerName(player Player) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawPlayerName(player, out, cap) })
}
func Send(player Player, text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawPlayerSend(player, t, tl, a, al)
}
func ActionBar(player Player, text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawPlayerActionBar(player, t, tl, a, al)
}
func Particle(player Player, id string, count int32, spread float64) int32 {
	i, il := ptr(id)
	return rawPlayerParticle(player, i, il, count, spread)
}
func Sound(player Player, id string, volume, pitch float32) int32 {
	i, il := ptr(id)
	return rawPlayerSound(player, i, il, volume, pitch)
}

func DataGetI64(player Player, key string, def int64) int64 {
	k, kl := ptr(key)
	return rawDataGetI64(player, k, kl, def)
}
func DataSetI64(player Player, key string, value int64) int32 {
	k, kl := ptr(key)
	return rawDataSetI64(player, k, kl, value)
}
func DataGetBool(player Player, key string, def bool) bool {
	k, kl := ptr(key)
	return rawDataGetBool(player, k, kl, b2i(def)) != 0
}
func DataSetBool(player Player, key string, value bool) int32 {
	k, kl := ptr(key)
	return rawDataSetBool(player, k, kl, b2i(value))
}
func DataGetStr(player Player, key string) (string, bool) {
	k, kl := ptr(key)
	return fetch(func(out *byte, cap int32) int32 { return rawDataGetStr(player, k, kl, out, cap) })
}
func DataSetStr(player Player, key, value string) int32 {
	k, kl := ptr(key)
	v, vl := ptr(value)
	return rawDataSetStr(player, k, kl, v, vl)
}
func DataRemove(player Player, key string) int32 {
	k, kl := ptr(key)
	return rawDataRemove(player, k, kl)
}

// TopEntry is one row of StoreTop.
type TopEntry struct {
	Name  string
	Value int64
}

// StoreTop returns the limit highest values of a numeric data key, best first.
func StoreTop(key string, limit int32) []TopEntry {
	k, kl := ptr(key)
	data, ok := fetchWith(1024, func(out *byte, cap int32) int32 { return rawStoreTop(k, kl, limit, out, cap) })
	if !ok {
		return nil
	}
	var rows []TopEntry
	for at := 0; at+4 <= len(data); {
		n := int(data[at]) | int(data[at+1])<<8 | int(data[at+2])<<16 | int(data[at+3])<<24
		at += 4
		if at+n+8 > len(data) {
			break
		}
		name := data[at : at+n]
		at += n
		var value int64
		for i := 7; i >= 0; i-- {
			value = value<<8 | int64(data[at+i])
		}
		at += 8
		rows = append(rows, TopEntry{name, value})
	}
	return rows
}

func On(event string, priority int32, handler Handler) int32 {
	e, el := ptr(event)
	return rawEventOn(e, el, priority, tokenFor(handler))
}
func EventPlayer(ctx Ctx) Player { return rawEventPlayer(ctx) }
func EventFieldStr(ctx Ctx, field string) (string, bool) {
	f, fl := ptr(field)
	return fetch(func(out *byte, cap int32) int32 { return rawEventFieldStr(ctx, f, fl, out, cap) })
}
func EventFieldI64(ctx Ctx, field string) (int64, bool) {
	f, fl := ptr(field)
	var value int64
	ok := rawEventFieldI64(ctx, f, fl, &value) == 0
	return value, ok
}
func EventCancel(ctx Ctx, cancelled bool) int32 { return rawEventCancel(ctx, b2i(cancelled)) }

func Command(spec, permission string, handler Handler) int32 {
	s, sl := ptr(spec)
	p, pl := ptr(permission)
	return rawCommandRegister(s, sl, p, pl, tokenFor(handler))
}
func CommandReply(ctx Ctx, text string, args *Args) int32 {
	t, tl := ptr(text)
	a, al := args.packed()
	return rawCommandReply(ctx, t, tl, a, al)
}
func CommandArgStr(ctx Ctx, name string) (string, bool) {
	n, nl := ptr(name)
	return fetch(func(out *byte, cap int32) int32 { return rawCommandArgStr(ctx, n, nl, out, cap) })
}
func CommandArgI64(ctx Ctx, name string) (int64, bool) {
	n, nl := ptr(name)
	var value int64
	ok := rawCommandArgI64(ctx, n, nl, &value) == 0
	return value, ok
}
func CommandSourceName(ctx Ctx) (string, bool) {
	return fetch(func(out *byte, cap int32) int32 { return rawCommandSourceName(ctx, out, cap) })
}
func CommandIsConsole(ctx Ctx) bool { return rawCommandIsConsole(ctx) != 0 }

func After(delay int32, handler Handler) int32 { return rawTaskAfter(delay, tokenFor(handler)) }
func Every(delay, period int32, handler Handler) int32 {
	return rawTaskEvery(delay, period, tokenFor(handler))
}
func TaskCancel(task int32) int32 { return rawTaskCancel(task) }

func Placeholder(name string, handler Handler) int32 {
	n, nl := ptr(name)
	return rawPlaceholderRegister(n, nl, tokenFor(handler))
}
func PlaceholderPlayer(ctx Ctx) Player { return rawPlaceholderPlayer(ctx) }
func PlaceholderResult(ctx Ctx, text string) int32 {
	t, tl := ptr(text)
	return rawPlaceholderResult(ctx, t, tl)
}

func MenuCreate(title string, rows int32) int32 {
	t, tl := ptr(title)
	return rawMenuCreate(t, tl, rows)
}
func MenuSet(menu, slot int32, item, name, lore string, onClick Handler) int32 {
	i, il := ptr(item)
	n, nl := ptr(name)
	l, ll := ptr(lore)
	return rawMenuSet(menu, slot, i, il, n, nl, l, ll, tokenFor(onClick))
}
func MenuOpen(menu int32, player Player) int32 { return rawMenuOpen(menu, player) }
func MenuClickPlayer(ctx Ctx) Player           { return rawMenuClickPlayer(ctx) }
