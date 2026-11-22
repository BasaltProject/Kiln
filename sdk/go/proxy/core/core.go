package core

import "basaltmc.org/sdk/proxy/internal/runtime"

type Ctx = runtime.Ctx

type Handler = runtime.Handler

const Debug = runtime.Debug

const Info = runtime.Info

const Warn = runtime.Warn

const Error = runtime.Error

const Lowest = runtime.Lowest

const Low = runtime.Low

const Normal = runtime.Normal

const High = runtime.High

const Highest = runtime.Highest

func Init(register func()) { runtime.Init(register) }

type Args = runtime.Args

func NewArgs() *runtime.Args { return runtime.NewArgs() }

type Player = runtime.Player
