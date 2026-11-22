package items

import "basaltmc.org/sdk/server/internal/runtime"

func ItemStack(id string, count int32, name string) ([]byte, error) {
	return runtime.ItemStack(id, count, name)
}

type Inventory = runtime.Inventory

func InventoryOfPlayer(player runtime.Player) runtime.Inventory {
	return runtime.InventoryOfPlayer(player)
}

func InventoryOfWindow(player runtime.Player) runtime.Inventory {
	return runtime.InventoryOfWindow(player)
}

func InventoryOfEnder(player runtime.Player) runtime.Inventory {
	return runtime.InventoryOfEnder(player)
}

func InventoryOfEntity(entity int32) runtime.Inventory { return runtime.InventoryOfEntity(entity) }

func InventoryOfBlock(world string, x, y, z int32) runtime.Inventory {
	return runtime.InventoryOfBlock(world, x, y, z)
}

func MenuSlot(menu, slot int32, id string, count int32, name string, onClick runtime.Handler) int32 {
	return runtime.MenuSlot(menu, slot, id, count, name, onClick)
}

func ItemCooldown(player runtime.Player, id string) int32 { return runtime.ItemCooldown(player, id) }

func ItemSetCooldown(player runtime.Player, id string, ticks int32) int32 {
	return runtime.ItemSetCooldown(player, id, ticks)
}

func RecipeKnown(id string) bool { return runtime.RecipeKnown(id) }

func RegistryGet(kind, key string, out []byte) int32 { return runtime.RegistryGet(kind, key, out) }

func EnchantmentLevelValue(spec []byte, level int32) float64 {
	return runtime.EnchantmentLevelValue(spec, level)
}

func RecipeAdd(id, json string) int32 { return runtime.RecipeAdd(id, json) }

func RecipeRemove(id string) int32 { return runtime.RecipeRemove(id) }

func RecipeIds(out []byte) int32 { return runtime.RecipeIds(out) }

func RecipeGet(id string, out []byte) int32 { return runtime.RecipeGet(id, out) }

func RecipeStation(kind string, input []byte, out []byte) int32 {
	return runtime.RecipeStation(kind, input, out)
}

func RecipeBookGet(player runtime.Player, out []byte) int32 {
	return runtime.RecipeBookGet(player, out)
}

func RecipeBookSet(player runtime.Player, ids []byte, learn bool) int32 {
	return runtime.RecipeBookSet(player, ids, learn)
}

func RecipeBookHas(player runtime.Player, id string) int32 { return runtime.RecipeBookHas(player, id) }

func RecipeBookResend(player runtime.Player) int32 { return runtime.RecipeBookResend(player) }

func MerchantOpen(player runtime.Player, spec []byte) int32 {
	return runtime.MerchantOpen(player, spec)
}

func WindowOpen(player runtime.Player, spec []byte) int32 { return runtime.WindowOpen(player, spec) }

func WindowInfo(player runtime.Player, out []byte) int32 { return runtime.WindowInfo(player, out) }

func WindowClose(player runtime.Player) int32 { return runtime.WindowClose(player) }

func WindowUpdate(player runtime.Player, spec []byte) int32 {
	return runtime.WindowUpdate(player, spec)
}

func ContainerMenuCreate(spec []byte) int32 { return runtime.ContainerMenuCreate(spec) }

func ContainerMenuOpen(menu int32, player runtime.Player) int32 {
	return runtime.ContainerMenuOpen(menu, player)
}

func ContainerMenuClose(menu int32, player runtime.Player) int32 {
	return runtime.ContainerMenuClose(menu, player)
}

func ContainerMenuClickInfo(ctx runtime.Ctx, out []byte) int32 {
	return runtime.ContainerMenuClickInfo(ctx, out)
}

func ContainerMenuClickResult(ctx runtime.Ctx, item []byte) int32 {
	return runtime.ContainerMenuClickResult(ctx, item)
}

func ContainerMenuClickCancel(ctx runtime.Ctx, cancelled bool) int32 {
	return runtime.ContainerMenuClickCancel(ctx, cancelled)
}

func LootTables(out []byte) int32 { return runtime.LootTables(out) }

func LootRoll(table string, spec []byte, out []byte) int32 { return runtime.LootRoll(table, spec, out) }

func LootRollJson(json string, spec []byte, out []byte) int32 {
	return runtime.LootRollJson(json, spec, out)
}

func LootNumber(json string, asFloat bool, seed int64, out []byte) int32 {
	return runtime.LootNumber(json, asFloat, seed, out)
}

func LootApply(spec []byte, out []byte) int32 { return runtime.LootApply(spec, out) }

func LootEnchant(spec []byte, out []byte) int32 { return runtime.LootEnchant(spec, out) }

func TradesRoll(profession string, level int32, spec []byte, out []byte) int32 {
	return runtime.TradesRoll(profession, level, spec, out)
}
