package runtime

// Proxy events area (abi/areas/proxy_events.h): event fields as values, chat, command and kick decisions, plugin
// identity, config arrays and typed command arguments. The *Value calls return the raw encoded value for the values
// reader; a negative status is the ABI error code.

const (
	peTagFalse  = 1
	peTagTrue   = 2
	peTagInt    = 3
	peTagString = 5
)

func peFetchValue(call func(out *byte, cap int32) int32) ([]byte, int32) {
	buffer := make([]byte, 256)
	for {
		needed := call(&buffer[0], int32(len(buffer)))
		if needed < 0 {
			return nil, needed
		}
		if int(needed) <= len(buffer) {
			return buffer[:needed], 0
		}
		buffer = make([]byte, needed)
	}
}

func peReadLeb(bytes []byte, at *int) (uint64, bool) {
	var result uint64
	for shift := uint(0); shift < 70 && *at < len(bytes); shift += 7 {
		b := bytes[*at]
		*at++
		result |= uint64(b&0x7f) << shift
		if b&0x80 == 0 {
			return result, true
		}
	}
	return 0, false
}

func peAppendLeb(out []byte, value uint64) []byte {
	for {
		b := byte(value & 0x7f)
		value >>= 7
		if value != 0 {
			out = append(out, b|0x80)
			continue
		}
		return append(out, b)
	}
}

// ProxyWatch registers an observer of the finished event; a decider is registered with On.
func ProxyWatch(event string, handler Handler) int32 {
	e, el := ptr(event)
	return rawProxyEventWatch(e, el, tokenFor(handler))
}

// ProxyEventGetValue returns the encoded value of a field; an empty name gives a map of every readable field.
func ProxyEventGetValue(ctx Ctx, field string) ([]byte, int32) {
	f, fl := ptr(field)
	return peFetchValue(func(out *byte, cap int32) int32 { return rawProxyEventGet(ctx, f, fl, out, cap) })
}

func ProxyEventGetStr(ctx Ctx, field string) (string, bool) {
	bytes, status := ProxyEventGetValue(ctx, field)
	at := 1
	if status < 0 || len(bytes) == 0 || bytes[0] != peTagString {
		return "", false
	}
	length, ok := peReadLeb(bytes, &at)
	if !ok || at+int(length) > len(bytes) {
		return "", false
	}
	return string(bytes[at : at+int(length)]), true
}

func ProxyEventGetI64(ctx Ctx, field string) (int64, bool) {
	bytes, status := ProxyEventGetValue(ctx, field)
	at := 1
	if status < 0 || len(bytes) == 0 || bytes[0] != peTagInt {
		return 0, false
	}
	zigzag, ok := peReadLeb(bytes, &at)
	return int64(zigzag>>1) ^ -int64(zigzag&1), ok
}

func ProxyEventGetBool(ctx Ctx, field string) (bool, bool) {
	bytes, status := ProxyEventGetValue(ctx, field)
	if status < 0 || len(bytes) != 1 || (bytes[0] != peTagTrue && bytes[0] != peTagFalse) {
		return false, false
	}
	return bytes[0] == peTagTrue, true
}

// ProxyEventFields returns a list of maps (name, type, readable, writable), one per field of the event.
func ProxyEventFields(ctx Ctx) ([]byte, int32) {
	return peFetchValue(func(out *byte, cap int32) int32 { return rawProxyEventFields(ctx, out, cap) })
}

func ProxyEventSetValue(ctx Ctx, field string, value []byte) int32 {
	f, fl := ptr(field)
	if len(value) == 0 {
		return -3
	}
	return rawProxyEventSet(ctx, f, fl, &value[0], int32(len(value)))
}

func ProxyEventSetText(ctx Ctx, field, text string) int32 {
	value := peAppendLeb([]byte{peTagString}, uint64(len(text)))
	return ProxyEventSetValue(ctx, field, append(value, text...))
}

func ProxyEventSetInt(ctx Ctx, field string, number int64) int32 {
	return ProxyEventSetValue(ctx, field, peAppendLeb([]byte{peTagInt}, uint64(number<<1)^uint64(number>>63)))
}

func ProxyEventSetFlag(ctx Ctx, field string, flag bool) int32 {
	tag := byte(peTagFalse)
	if flag {
		tag = peTagTrue
	}
	return ProxyEventSetValue(ctx, field, []byte{tag})
}

// ProxyEventDeny cancels a decided event and shows reason (MiniMessage, or a '@' lang key) to the player.
func ProxyEventDeny(ctx Ctx, reason string) int32 {
	if reason != "" {
		if shown := ProxyEventSetText(ctx, "reason", reason); shown < 0 {
			return shown
		}
	}
	return ProxyEventSetFlag(ctx, "cancelled", true)
}

// ProxyPluginInfo reads "id", "name" or "version".
func ProxyPluginInfo(field string) (string, bool) {
	f, fl := ptr(field)
	return fetch(func(out *byte, cap int32) int32 { return rawProxyPluginInfo(f, fl, out, cap) })
}

func ProxyPluginManifest() ([]byte, int32) {
	return peFetchValue(func(out *byte, cap int32) int32 { return rawProxyPluginManifest(out, cap) })
}

// ProxyConfigValue reads the config.toml value at a dotted key (arrays are lists, tables are maps); "" is the file.
func ProxyConfigValue(key string) ([]byte, int32) {
	k, kl := ptr(key)
	return peFetchValue(func(out *byte, cap int32) int32 { return rawProxyConfigValue(k, kl, out, cap) })
}

func ProxyCommandArgF64(ctx Ctx, name string) (float64, bool) {
	n, nl := ptr(name)
	var value float64
	ok := rawProxyCommandArgF64(ctx, n, nl, &value) == 0
	return value, ok
}

func ProxyCommandArgBool(ctx Ctx, name string) (bool, bool) {
	n, nl := ptr(name)
	var value int32
	ok := rawProxyCommandArgBool(ctx, n, nl, &value) == 0
	return value != 0, ok
}

func ProxyCommandArgs(ctx Ctx) ([]byte, int32) {
	return peFetchValue(func(out *byte, cap int32) int32 { return rawProxyCommandArgs(ctx, out, cap) })
}
