package runtime

import (
	"errors"
	"math"
)

// The value format of basalt_values.h: a builder over a growable buffer and a reader over a validated document.

const (
	ValueMaxBytes    = 1 << 20
	ValueMaxDepth    = 16
	ValueMaxNodes    = 65536
	ValueMaxKeyBytes = 256
)

const (
	tagNull   = 0
	tagFalse  = 1
	tagTrue   = 2
	tagInt    = 3
	tagFloat  = 4
	tagString = 5
	tagBytes  = 6
	tagList   = 7
	tagMap    = 8
)

type ValueKind int

const (
	KindAbsent ValueKind = iota - 1
	KindNull
	KindBool
	KindInt
	KindFloat
	KindString
	KindBytes
	KindList
	KindMap
)

var (
	ErrValueInvalid  = errors.New("basalt: invalid value")
	ErrValueOverflow = errors.New("basalt: value over the size limit")
)

type valueFrame struct {
	start      int
	count      int
	isMap      bool
	pendingKey bool
}

// ValueWriter builds one document. A failure is sticky: later calls do nothing and Finish reports it.
type ValueWriter struct {
	buf    []byte
	limit  int
	nodes  int
	frames []valueFrame
	err    error
}

func NewValueWriter() *ValueWriter { return &ValueWriter{limit: ValueMaxBytes} }

// NewValueWriterLimit fails with ErrValueOverflow once the document would pass limit bytes.
func NewValueWriterLimit(limit int) *ValueWriter {
	if limit > ValueMaxBytes {
		limit = ValueMaxBytes
	}
	return &ValueWriter{limit: limit}
}

func (w *ValueWriter) Err() error { return w.err }

func (w *ValueWriter) fail(err error) {
	if w.err == nil {
		w.err = err
	}
}

func (w *ValueWriter) room(extra int) bool {
	if w.err != nil {
		return false
	}
	if extra > w.limit-len(w.buf) {
		w.fail(ErrValueOverflow)
		return false
	}
	return true
}

func (w *ValueWriter) put(b byte) {
	if w.room(1) {
		w.buf = append(w.buf, b)
	}
}

func (w *ValueWriter) leb(v uint64) {
	for v >= 0x80 {
		w.put(byte(v&0x7F) | 0x80)
		v >>= 7
	}
	w.put(byte(v))
}

func (w *ValueWriter) begin() bool {
	if w.err != nil {
		return false
	}
	if n := len(w.frames); n == 0 {
		if w.nodes > 0 {
			w.fail(ErrValueInvalid)
			return false
		}
	} else if top := &w.frames[n-1]; top.isMap {
		if !top.pendingKey {
			w.fail(ErrValueInvalid)
			return false
		}
		top.pendingKey = false
	} else {
		top.count++
	}
	w.nodes++
	if len(w.frames) > ValueMaxDepth || w.nodes > ValueMaxNodes {
		w.fail(ErrValueInvalid)
		return false
	}
	return true
}

func (w *ValueWriter) Null() *ValueWriter {
	if w.begin() {
		w.put(tagNull)
	}
	return w
}

func (w *ValueWriter) Bool(v bool) *ValueWriter {
	if w.begin() {
		if v {
			w.put(tagTrue)
		} else {
			w.put(tagFalse)
		}
	}
	return w
}

func (w *ValueWriter) Int(v int64) *ValueWriter {
	if w.begin() {
		w.put(tagInt)
		w.leb(uint64(v)<<1 ^ uint64(v>>63))
	}
	return w
}

func (w *ValueWriter) Float(v float64) *ValueWriter {
	if w.begin() {
		w.put(tagFloat)
		bits := math.Float64bits(v)
		for shift := 0; shift < 64; shift += 8 {
			w.put(byte(bits >> shift))
		}
	}
	return w
}

func (w *ValueWriter) blob(tag byte, n int, add func()) {
	if w.begin() {
		w.put(tag)
		w.leb(uint64(n))
		if w.room(n) {
			add()
		}
	}
}

func (w *ValueWriter) String(s string) *ValueWriter {
	w.blob(tagString, len(s), func() { w.buf = append(w.buf, s...) })
	return w
}

func (w *ValueWriter) Bytes(b []byte) *ValueWriter {
	w.blob(tagBytes, len(b), func() { w.buf = append(w.buf, b...) })
	return w
}

func (w *ValueWriter) open(isMap bool, tag byte) {
	if w.begin() {
		w.put(tag)
		w.frames = append(w.frames, valueFrame{start: len(w.buf), isMap: isMap})
	}
}

func (w *ValueWriter) BeginList() *ValueWriter { w.open(false, tagList); return w }

func (w *ValueWriter) BeginMap() *ValueWriter { w.open(true, tagMap); return w }

// End closes the innermost list or map, inserting its count in front of the items.
func (w *ValueWriter) End() *ValueWriter {
	if w.err != nil {
		return w
	}
	n := len(w.frames)
	if n == 0 || (w.frames[n-1].isMap && w.frames[n-1].pendingKey) {
		w.fail(ErrValueInvalid)
		return w
	}
	top := w.frames[n-1]
	w.frames = w.frames[:n-1]
	var count []byte
	for v := uint64(top.count); ; v >>= 7 {
		if v < 0x80 {
			count = append(count, byte(v))
			break
		}
		count = append(count, byte(v&0x7F)|0x80)
	}
	if w.room(len(count)) {
		w.buf = append(w.buf, count...)
		copy(w.buf[top.start+len(count):], w.buf[top.start:len(w.buf)-len(count)])
		copy(w.buf[top.start:], count)
	}
	return w
}

// Key writes the next key of the open map; the value follows.
func (w *ValueWriter) Key(name string) *ValueWriter {
	if w.err != nil {
		return w
	}
	n := len(w.frames)
	if n == 0 || !w.frames[n-1].isMap || w.frames[n-1].pendingKey || len(name) > ValueMaxKeyBytes {
		w.fail(ErrValueInvalid)
		return w
	}
	w.leb(uint64(len(name)))
	if w.room(len(name)) {
		w.buf = append(w.buf, name...)
	}
	w.frames[n-1].count++
	w.frames[n-1].pendingKey = true
	return w
}

// Finish returns the document, valid until the writer is used again.
func (w *ValueWriter) Finish() ([]byte, error) {
	if w.err != nil {
		return nil, w.err
	}
	if len(w.frames) != 0 || w.nodes == 0 {
		return nil, ErrValueInvalid
	}
	return w.buf, nil
}

func readLeb(b []byte, pos *int) (uint64, bool) {
	var result uint64
	for shift := uint(0); shift < 70; shift += 7 {
		if *pos >= len(b) {
			return 0, false
		}
		c := b[*pos]
		*pos++
		if shift == 63 && c > 1 {
			return 0, false
		}
		result |= uint64(c&0x7F) << shift
		if c&0x80 == 0 {
			return result, true
		}
	}
	return 0, false
}

func skipValue(b []byte, start int) int {
	if start < 0 || start >= len(b) {
		return -1
	}
	tag := b[start]
	pos := start + 1
	switch tag {
	case tagInt:
		if _, ok := readLeb(b, &pos); !ok {
			return -1
		}
		return pos
	case tagFloat:
		if len(b)-pos < 8 {
			return -1
		}
		return pos + 8
	case tagString, tagBytes:
		n, ok := readLeb(b, &pos)
		if !ok || n > uint64(len(b)-pos) {
			return -1
		}
		return pos + int(n)
	case tagList, tagMap:
		n, ok := readLeb(b, &pos)
		if !ok {
			return -1
		}
		for ; n > 0 && pos >= 0; n-- {
			if tag == tagMap {
				k, ok := readLeb(b, &pos)
				if !ok || k > uint64(len(b)-pos) {
					return -1
				}
				pos += int(k)
			}
			pos = skipValue(b, pos)
		}
		return pos
	}
	return pos
}

func walkValue(b []byte, start, depth int, nodes *int) int {
	if depth > ValueMaxDepth {
		return -1
	}
	*nodes++
	if *nodes > ValueMaxNodes || start >= len(b) {
		return -1
	}
	tag := b[start]
	pos := start + 1
	switch tag {
	case tagNull, tagFalse, tagTrue:
		return pos
	case tagInt:
		if _, ok := readLeb(b, &pos); !ok {
			return -1
		}
		return pos
	case tagFloat:
		if len(b)-pos < 8 {
			return -1
		}
		return pos + 8
	case tagString, tagBytes:
		n, ok := readLeb(b, &pos)
		if !ok || n > uint64(len(b)-pos) {
			return -1
		}
		return pos + int(n)
	case tagList:
		n, ok := readLeb(b, &pos)
		if !ok || n > uint64(len(b)-pos) {
			return -1
		}
		for i := uint64(0); i < n; i++ {
			if pos = walkValue(b, pos, depth+1, nodes); pos < 0 {
				return -1
			}
		}
		return pos
	case tagMap:
		n, ok := readLeb(b, &pos)
		if !ok || n > uint64(len(b)-pos) {
			return -1
		}
		first := pos
		for i := uint64(0); i < n; i++ {
			klen, ok := readLeb(b, &pos)
			if !ok || klen > ValueMaxKeyBytes || klen > uint64(len(b)-pos) {
				return -1
			}
			name := b[pos : pos+int(klen)]
			scan := first
			for j := uint64(0); j < i; j++ {
				olen, _ := readLeb(b, &scan)
				if string(b[scan:scan+int(olen)]) == string(name) {
					return -1
				}
				if scan = skipValue(b, scan+int(olen)); scan < 0 {
					return -1
				}
			}
			if pos = walkValue(b, pos+len(name), depth+1, nodes); pos < 0 {
				return -1
			}
		}
		return pos
	}
	return -1
}

// Value is a position in a validated document; the zero Value is absent.
type Value struct {
	b   []byte
	off int
}

// OpenValue validates the whole document (tags, limits, duplicate keys, trailing bytes).
func OpenValue(doc []byte) (Value, bool) {
	if len(doc) == 0 || len(doc) > ValueMaxBytes {
		return Value{}, false
	}
	nodes := 0
	if walkValue(doc, 0, 0, &nodes) != len(doc) {
		return Value{}, false
	}
	return Value{b: doc}, true
}

func (v Value) Present() bool { return v.b != nil }

func (v Value) Kind() ValueKind {
	if v.b == nil || v.off >= len(v.b) {
		return KindAbsent
	}
	switch v.b[v.off] {
	case tagFalse, tagTrue:
		return KindBool
	case tagInt:
		return KindInt
	case tagFloat:
		return KindFloat
	case tagString:
		return KindString
	case tagBytes:
		return KindBytes
	case tagList:
		return KindList
	case tagMap:
		return KindMap
	}
	return KindNull
}

func (v Value) IsNull() bool { return v.Kind() == KindNull }

func (v Value) AsBool() (bool, bool) {
	if v.Kind() != KindBool {
		return false, false
	}
	return v.b[v.off] == tagTrue, true
}

func (v Value) AsInt() (int64, bool) {
	if v.Kind() != KindInt {
		return 0, false
	}
	pos := v.off + 1
	raw, ok := readLeb(v.b, &pos)
	if !ok {
		return 0, false
	}
	return int64(raw>>1) ^ -int64(raw&1), true
}

func (v Value) AsFloat() (float64, bool) {
	if v.Kind() != KindFloat || len(v.b)-v.off < 9 {
		return 0, false
	}
	var bits uint64
	for i := 0; i < 8; i++ {
		bits |= uint64(v.b[v.off+1+i]) << (8 * uint(i))
	}
	return math.Float64frombits(bits), true
}

func (v Value) payload(kind ValueKind) ([]byte, bool) {
	if v.Kind() != kind {
		return nil, false
	}
	pos := v.off + 1
	n, ok := readLeb(v.b, &pos)
	if !ok || n > uint64(len(v.b)-pos) {
		return nil, false
	}
	return v.b[pos : pos+int(n)], true
}

// AsString copies the payload of a string value.
func (v Value) AsString() (string, bool) {
	p, ok := v.payload(KindString)
	return string(p), ok
}

// AsBytes returns the payload of a bytes value, borrowed from the document.
func (v Value) AsBytes() ([]byte, bool) { return v.payload(KindBytes) }

// Count is the number of items of a list or entries of a map, -1 for any other value.
func (v Value) Count() int {
	if k := v.Kind(); k != KindList && k != KindMap {
		return -1
	}
	pos := v.off + 1
	n, ok := readLeb(v.b, &pos)
	if !ok {
		return -1
	}
	return int(n)
}

// ValueIter walks the items of a list or the entries of a map.
type ValueIter struct {
	b     []byte
	pos   int
	left  uint64
	isMap bool
}

func (v Value) Iter() *ValueIter {
	k := v.Kind()
	pos := v.off + 1
	var left uint64
	if k == KindList || k == KindMap {
		left, _ = readLeb(v.b, &pos)
	}
	return &ValueIter{b: v.b, pos: pos, left: left, isMap: k == KindMap}
}

// Next returns the next item and, for a map, its key (empty for a list).
func (it *ValueIter) Next() (key string, item Value, ok bool) {
	if it.left == 0 || it.pos >= len(it.b) {
		return "", Value{}, false
	}
	pos := it.pos
	if it.isMap {
		klen, good := readLeb(it.b, &pos)
		if !good || klen > uint64(len(it.b)-pos) {
			it.left = 0
			return "", Value{}, false
		}
		key = string(it.b[pos : pos+int(klen)])
		pos += int(klen)
	}
	item = Value{b: it.b, off: pos}
	if it.pos = skipValue(it.b, pos); it.pos < 0 {
		it.pos = len(it.b)
	}
	it.left--
	return key, item, true
}

func (v Value) At(index int) (Value, bool) {
	if v.Kind() != KindList || index < 0 {
		return Value{}, false
	}
	it := v.Iter()
	for i := 0; ; i++ {
		_, item, ok := it.Next()
		if !ok {
			return Value{}, false
		}
		if i == index {
			return item, true
		}
	}
}

func (v Value) Find(key string) (Value, bool) {
	if v.Kind() != KindMap {
		return Value{}, false
	}
	it := v.Iter()
	for {
		name, item, ok := it.Next()
		if !ok {
			return Value{}, false
		}
		if name == key {
			return item, true
		}
	}
}

// CallError wraps a negative result of an import: -1 absent, -2 stale, -3 invalid, -4 denied.
type CallError int32

func (e CallError) Error() string {
	switch e {
	case -1:
		return "basalt: absent"
	case -2:
		return "basalt: stale handle"
	case -3:
		return "basalt: invalid argument"
	case -4:
		return "basalt: denied"
	}
	return "basalt: call failed"
}

var ErrValueTooLarge = errors.New("basalt: answer too large")

// CallOut calls an import that fills (out, cap) and returns the length it needs: with initial bytes, then once more
// with exactly the needed size when the answer did not fit.
func CallOut(initial int, call func(out *byte, cap int32) int32) ([]byte, error) {
	if initial < 1 {
		initial = 1
	}
	buf := make([]byte, initial)
	n := call(&buf[0], int32(len(buf)))
	if n < 0 {
		return nil, CallError(n)
	}
	if int(n) > len(buf) {
		if n > ValueMaxBytes {
			return nil, ErrValueTooLarge
		}
		buf = make([]byte, n)
		if n = call(&buf[0], int32(len(buf))); n < 0 {
			return nil, CallError(n)
		}
		if int(n) > len(buf) {
			return nil, ErrValueTooLarge
		}
	}
	return buf[:n], nil
}

// CallValue is CallOut followed by validating the answer as a document.
func CallValue(initial int, call func(out *byte, cap int32) int32) (Value, error) {
	doc, err := CallOut(initial, call)
	if err != nil {
		return Value{}, err
	}
	v, ok := OpenValue(doc)
	if !ok {
		return Value{}, ErrValueInvalid
	}
	return v, nil
}
