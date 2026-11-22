package players

import "basaltmc.org/sdk/server/internal/runtime"

func PlayerByName(name string) runtime.Player { return runtime.PlayerByName(name) }

func PlayersOnline() []runtime.Player { return runtime.PlayersOnline() }

func PlayerName(player runtime.Player) (string, bool) { return runtime.PlayerName(player) }

func Send(player runtime.Player, text string, args *runtime.Args) int32 {
	return runtime.Send(player, text, args)
}

func ActionBar(player runtime.Player, text string, args *runtime.Args) int32 {
	return runtime.ActionBar(player, text, args)
}

func Particle(player runtime.Player, id string, count int32, spread float64) int32 {
	return runtime.Particle(player, id, count, spread)
}

func Sound(player runtime.Player, id string, volume, pitch float32) int32 {
	return runtime.Sound(player, id, volume, pitch)
}
