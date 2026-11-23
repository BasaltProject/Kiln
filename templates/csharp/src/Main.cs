using Basalt.Server.Commands;
using Basalt.Server.Config;
using Basalt.Server.Core;
using Basalt.Server.Data;
using Basalt.Server.Events;
using Basalt.Server.Items;
using Basalt.Server.Log;
using Basalt.Server.Menus;
using Basalt.Server.Players;
using Basalt.Server.Tasks;
using Basalt.Server.Text;
using Basalt.Server.Values;

namespace Basalt.Server.Example;

public static class Plugin
{
    static int shop;

    static void BuySparkles(int ctx)
    {
        int player = InventoryMenu.MenuClickPlayer(ctx);
        long price = Settings.ConfigI64("shop.sparkles-price", 100);
        long balance = PlayerStore.DataGetI64(player, "coins", 0);
        if (balance < price)
        {
            Directory.Send(player, "@shop-cant-afford", new Args().Add("price", price.ToString()).Add("amount", balance.ToString()));
            return;
        }
        PlayerStore.DataSetI64(player, "coins", balance - price);
        Directory.Particle(player, "minecraft:happy_villager", 20, 0.5);
        Directory.Send(player, "@shop-bought", new Args().Add("item", "Sparkles").Add("price", price.ToString()));
    }

    static void OnJoin(int ctx)
    {
        int player = EventBus.EventPlayer(ctx);
        if (player == 0)
        {
            return;
        }
        long balance = PlayerStore.DataGetI64(player, "coins", 0) + Settings.ConfigI64("welcome-bonus", 50);
        PlayerStore.DataSetI64(player, "coins", balance);
        Directory.Send(player, "@welcome", new Args().Add("name", Directory.PlayerName(player) ?? "").Add("amount", balance.ToString()));
    }

    static void Hello(int ctx)
    {
        long times = Arguments.I64(ctx, "times") ?? 1;
        Args args = new Args().Add("name", Sender.Name(ctx) ?? "");
        for (long i = 0; i < times; i++)
        {
            Sender.Reply(ctx, "@hello", args);
        }
    }

    static void OpenShop(int ctx)
    {
        if (Sender.IsConsole(ctx))
        {
            Sender.Reply(ctx, "<red>The shop needs a player.");
            return;
        }
        InventoryMenu.MenuOpen(shop, Directory.PlayerByName(Sender.Name(ctx) ?? ""));
    }

    public static void Init()
    {
        Logger.LogInfo("starting up");
        EventBus.On("player.join", Priority.Normal, OnJoin);
        Registry.Register("@ID@ hello [times: int 1..5]", "", Hello);
        Registry.Register("@ID@ shop", "", OpenShop);

        shop = InventoryMenu.MenuCreate(Translations.Translate("shop-title") ?? "", 3);
        string lore = Translations.Translate("shop-item-sparkles-lore", new Args().Add("price", "100")) ?? "";
        InventoryMenu.MenuSet(shop, 13, "minecraft:glowstone_dust", Translations.Translate("shop-item-sparkles") ?? "", lore, BuySparkles);

        int period = (int)Settings.ConfigI64("announce-period", 1200);
        Scheduler.Every(period, period, ctx =>
        {
            foreach (int player in Directory.PlayersOnline())
            {
                Directory.Send(player, "@announce");
            }
        });
    }
}
