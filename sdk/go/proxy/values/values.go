package values

import "basaltmc.org/sdk/proxy/internal/runtime"

const ValueMaxBytes = runtime.ValueMaxBytes

const ValueMaxDepth = runtime.ValueMaxDepth

const ValueMaxNodes = runtime.ValueMaxNodes

const ValueMaxKeyBytes = runtime.ValueMaxKeyBytes

type ValueKind = runtime.ValueKind

const KindAbsent = runtime.KindAbsent

const KindNull = runtime.KindNull

const KindBool = runtime.KindBool

const KindInt = runtime.KindInt

const KindFloat = runtime.KindFloat

const KindString = runtime.KindString

const KindBytes = runtime.KindBytes

const KindList = runtime.KindList

const KindMap = runtime.KindMap

type ValueWriter = runtime.ValueWriter

func NewValueWriter() *runtime.ValueWriter { return runtime.NewValueWriter() }

func NewValueWriterLimit(limit int) *runtime.ValueWriter { return runtime.NewValueWriterLimit(limit) }

type Value = runtime.Value

func OpenValue(doc []byte) (runtime.Value, bool) { return runtime.OpenValue(doc) }

type ValueIter = runtime.ValueIter

type CallError = runtime.CallError

func CallOut(initial int, call func(out *byte, cap int32) int32) ([]byte, error) {
	return runtime.CallOut(initial, call)
}

func CallValue(initial int, call func(out *byte, cap int32) int32) (runtime.Value, error) {
	return runtime.CallValue(initial, call)
}
