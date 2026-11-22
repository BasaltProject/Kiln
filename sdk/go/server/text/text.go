package text

import "basaltmc.org/sdk/server/internal/runtime"

func Translate(key string, args *runtime.Args) (string, bool) { return runtime.Translate(key, args) }
