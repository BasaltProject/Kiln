package runtime

// Items area: an item stack builder, an Inventory over the inventory handles and thin wrappers for the other imports.

const itemBuffer = 512

// ItemStack builds the `{id, count[, name]}` document; an empty name leaves the field out.
func ItemStack(id string, count int32, name string) ([]byte, error) {
	w := NewValueWriterLimit(itemBuffer)
	w.BeginMap().Key("id").String(id).Key("count").Int(int64(count))
	if name != "" {
		w.Key("name").String(name)
	}
	return w.End().Finish()
}

var nullDocument = []byte{byte(tagNull)}

// Inventory is a handle on a player, block, ender chest, window or entity inventory; call Release when done.
type Inventory struct{ handle int32 }

func InventoryOfPlayer(player Player) Inventory { return Inventory{rawInventoryOfPlayer(player)} }
func InventoryOfWindow(player Player) Inventory { return Inventory{rawInventoryOfWindow(player)} }
func InventoryOfEnder(player Player) Inventory  { return Inventory{rawInventoryOfEnder(player)} }
func InventoryOfEntity(entity int32) Inventory  { return Inventory{rawInventoryOfEntity(entity)} }

func InventoryOfBlock(world string, x, y, z int32) Inventory {
	w, wl := ptr(world)
	return Inventory{rawInventoryOfBlock(w, wl, x, y, z)}
}

// Handle is the handle, or the negative error that made it.
func (i Inventory) Handle() int32 { return i.handle }
func (i Inventory) Valid() bool   { return i.handle > 0 }
func (i Inventory) Size() int32   { return rawInventorySize(i.handle) }

func (i Inventory) HeldSlot() int32 { return rawInventoryHeldSlot(i.handle) }
func (i Inventory) SetHeldSlot(slot int32) int32 {
	return rawInventorySetHeldSlot(i.handle, slot)
}

func (i Inventory) set(slot int32, doc []byte) int32 {
	p, n := bytesPtr(doc)
	return rawInventorySet(i.handle, slot, p, n)
}

func (i Inventory) Put(slot int32, id string, count int32) int32 {
	doc, err := ItemStack(id, count, "")
	if err != nil {
		return -3
	}
	return i.set(slot, doc)
}

func (i Inventory) ClearSlot(slot int32) int32 { return i.set(slot, nullDocument) }

// Give returns how many items did not fit, or a negative error.
func (i Inventory) Give(id string, count int32, name string) int32 {
	doc, err := ItemStack(id, count, name)
	if err != nil {
		return -3
	}
	p, n := bytesPtr(doc)
	return rawInventoryAdd(i.handle, p, n)
}

// Take returns how many items were removed.
func (i Inventory) Take(id string, count int32) int32 {
	p, n := ptr(id)
	return rawInventoryRemove(i.handle, p, n, count)
}

func (i Inventory) Count(id string) int32 {
	p, n := ptr(id)
	return rawInventoryCount(i.handle, p, n)
}

// SlotCount is the count of the stack in the slot, 0 for an empty slot, negative for an error.
func (i Inventory) SlotCount(slot int32) int32 {
	buf := make([]byte, itemBuffer)
	n := rawInventoryGet(i.handle, slot, &buf[0], itemBuffer)
	if n < 0 {
		return n
	}
	if n > itemBuffer {
		return -3
	}
	root, ok := OpenValue(buf[:n])
	if !ok {
		return -3
	}
	if root.IsNull() {
		return 0
	}
	field, _ := root.Find("count")
	count, _ := field.AsInt()
	return int32(count)
}

func (i Inventory) Clear() int32 { return rawInventoryClear(i.handle) }

// Get writes the slot document to out; the result is the length it needs.
func (i Inventory) Get(slot int32, out []byte) int32 {
	p, n := bytesPtr(out)
	return rawInventoryGet(i.handle, slot, p, n)
}

func (i Inventory) Contents(out []byte) int32 {
	p, n := bytesPtr(out)
	return rawInventoryContents(i.handle, p, n)
}

func (i *Inventory) Release() {
	if i.handle > 0 {
		rawInventoryRelease(i.handle)
	}
	i.handle = 0
}

// MenuSlot puts an item in a container menu slot; onClick runs when a player clicks it.
func MenuSlot(menu, slot int32, id string, count int32, name string, onClick Handler) int32 {
	doc, err := ItemStack(id, count, name)
	if err != nil {
		return -3
	}
	token := tokenFor(onClick)
	p, n := bytesPtr(doc)
	return rawContainerMenuSet(menu, slot, p, n, token)
}

func ItemCooldown(player Player, id string) int32 {
	p, n := ptr(id)
	return rawItemCooldown(player, p, n)
}

func ItemSetCooldown(player Player, id string, ticks int32) int32 {
	p, n := ptr(id)
	return rawItemSetCooldown(player, p, n, ticks)
}

func RecipeKnown(id string) bool {
	p, n := ptr(id)
	return rawRecipeHas(p, n) > 0
}

func RegistryGet(kind, key string, out []byte) int32 {
	k, kl := ptr(kind)
	e, el := ptr(key)
	o, oc := bytesPtr(out)
	return rawRegistryGet(k, kl, e, el, o, oc)
}

func EnchantmentLevelValue(spec []byte, level int32) float64 {
	p, n := bytesPtr(spec)
	return rawEnchantmentLevelValue(p, n, level)
}

func RecipeAdd(id, json string) int32 {
	i, il := ptr(id)
	j, jl := ptr(json)
	return rawRecipeAdd(i, il, j, jl)
}

func RecipeRemove(id string) int32 { i, il := ptr(id); return rawRecipeRemove(i, il) }

func RecipeIds(out []byte) int32 { p, n := bytesPtr(out); return rawRecipeIds(p, n) }

func RecipeGet(id string, out []byte) int32 {
	i, il := ptr(id)
	p, n := bytesPtr(out)
	return rawRecipeGet(i, il, p, n)
}

func RecipeStation(kind string, input []byte, out []byte) int32 {
	k, kl := ptr(kind)
	a, al := bytesPtr(input)
	p, n := bytesPtr(out)
	return rawRecipeStation(k, kl, a, al, p, n)
}

func RecipeBookGet(player Player, out []byte) int32 {
	p, n := bytesPtr(out)
	return rawRecipeBookGet(player, p, n)
}

func RecipeBookSet(player Player, ids []byte, learn bool) int32 {
	p, n := bytesPtr(ids)
	return rawRecipeBookSet(player, p, n, b2i(learn))
}

func RecipeBookHas(player Player, id string) int32 {
	p, n := ptr(id)
	return rawRecipeBookHas(player, p, n)
}

func RecipeBookResend(player Player) int32 { return rawRecipeBookResend(player) }

func MerchantOpen(player Player, spec []byte) int32 {
	p, n := bytesPtr(spec)
	return rawMerchantOpen(player, p, n)
}

func WindowOpen(player Player, spec []byte) int32 {
	p, n := bytesPtr(spec)
	return rawWindowOpen(player, p, n)
}

func WindowInfo(player Player, out []byte) int32 {
	p, n := bytesPtr(out)
	return rawWindowInfo(player, p, n)
}

func WindowClose(player Player) int32 { return rawWindowClose(player) }

func WindowUpdate(player Player, spec []byte) int32 {
	p, n := bytesPtr(spec)
	return rawWindowUpdate(player, p, n)
}

func ContainerMenuCreate(spec []byte) int32 {
	p, n := bytesPtr(spec)
	return rawContainerMenuCreate(p, n)
}

func ContainerMenuOpen(menu int32, player Player) int32 { return rawContainerMenuOpen(menu, player) }

func ContainerMenuClose(menu int32, player Player) int32 {
	return rawContainerMenuClose(menu, player)
}

func ContainerMenuClickInfo(ctx Ctx, out []byte) int32 {
	p, n := bytesPtr(out)
	return rawContainerMenuClickInfo(ctx, p, n)
}

func ContainerMenuClickResult(ctx Ctx, item []byte) int32 {
	p, n := bytesPtr(item)
	return rawContainerMenuClickResult(ctx, p, n)
}

func ContainerMenuClickCancel(ctx Ctx, cancelled bool) int32 {
	return rawContainerMenuClickCancel(ctx, b2i(cancelled))
}

func LootTables(out []byte) int32 { p, n := bytesPtr(out); return rawLootTables(p, n) }

func LootRoll(table string, spec []byte, out []byte) int32 {
	t, tl := ptr(table)
	s, sl := bytesPtr(spec)
	p, n := bytesPtr(out)
	return rawLootRoll(t, tl, s, sl, p, n)
}

func LootRollJson(json string, spec []byte, out []byte) int32 {
	j, jl := ptr(json)
	s, sl := bytesPtr(spec)
	p, n := bytesPtr(out)
	return rawLootRollJson(j, jl, s, sl, p, n)
}

func LootNumber(json string, asFloat bool, seed int64, out []byte) int32 {
	j, jl := ptr(json)
	p, n := bytesPtr(out)
	return rawLootNumber(j, jl, b2i(asFloat), seed, p, n)
}

func LootApply(spec []byte, out []byte) int32 {
	s, sl := bytesPtr(spec)
	p, n := bytesPtr(out)
	return rawLootApply(s, sl, p, n)
}

func LootEnchant(spec []byte, out []byte) int32 {
	s, sl := bytesPtr(spec)
	p, n := bytesPtr(out)
	return rawLootEnchant(s, sl, p, n)
}

func TradesRoll(profession string, level int32, spec []byte, out []byte) int32 {
	f, fl := ptr(profession)
	s, sl := bytesPtr(spec)
	p, n := bytesPtr(out)
	return rawTradesRoll(f, fl, level, s, sl, p, n)
}
