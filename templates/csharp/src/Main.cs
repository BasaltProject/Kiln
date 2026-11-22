using Basalt.Server.Core;
using Basalt.Server.Items;
using Basalt.Server.Values;
using SdkCommands = global::Basalt.Server.Commands.Commands;
using SdkConfig = global::Basalt.Server.Config.Config;
using SdkData = global::Basalt.Server.Data.Data;
using SdkEvents = global::Basalt.Server.Events.Events;
using SdkLogger = global::Basalt.Server.Log.Logger;
using SdkMenus = global::Basalt.Server.Menus.Menus;
using SdkPlayers = global::Basalt.Server.Players.Players;
using SdkTasks = global::Basalt.Server.Tasks.Tasks;
using SdkText = global::Basalt.Server.Text.Text;

namespace Basalt.Server.Example;

public static class Plugin
{
    static int shop;

    static void BuySparkles(int ctx)
    {
        int player = SdkMenus.MenuClickPlayer(ctx);
        long price = SdkConfig.ConfigI64("shop.sparkles-price", 100);
        long balance = SdkData.DataGetI64(player, "coins", 0);
        if (balance < price)
        {
            SdkPlayers.Send(player, "@shop-cant-afford", new Args().Add("price", price.ToString()).Add("amount", balance.ToString()));
            return;
        }
        SdkData.DataSetI64(player, "coins", balance - price);
        SdkPlayers.Particle(player, "minecraft:happy_villager", 20, 0.5);
        SdkPlayers.Send(player, "@shop-bought", new Args().Add("item", "Sparkles").Add("price", price.ToString()));
    }

    static void OnJoin(int ctx)
    {
        int player = SdkEvents.EventPlayer(ctx);
        if (player == 0)
        {
            return;
        }
        long balance = SdkData.DataGetI64(player, "coins", 0) + SdkConfig.ConfigI64("welcome-bonus", 50);
        SdkData.DataSetI64(player, "coins", balance);
        SdkPlayers.Send(player, "@welcome", new Args().Add("name", SdkPlayers.PlayerName(player) ?? "").Add("amount", balance.ToString()));
    }

    static void Hello(int ctx)
    {
        long times = SdkCommands.CommandArgI64(ctx, "times") ?? 1;
        Args args = new Args().Add("name", SdkCommands.CommandSourceName(ctx) ?? "");
        for (long i = 0; i < times; i++)
        {
            SdkCommands.CommandReply(ctx, "@hello", args);
        }
    }

    static void OpenShop(int ctx)
    {
        if (SdkCommands.CommandIsConsole(ctx))
        {
            SdkCommands.CommandReply(ctx, "<red>The shop needs a player.");
            return;
        }
        SdkMenus.MenuOpen(shop, SdkPlayers.PlayerByName(SdkCommands.CommandSourceName(ctx) ?? ""));
    }

    public static void Init()
    {
        SdkLogger.LogInfo("starting up");
        SdkEvents.On("player.join", Priority.Normal, OnJoin);
        SdkCommands.Command("@ID@ hello [times: int 1..5]", "", Hello);
        SdkCommands.Command("@ID@ shop", "", OpenShop);

        shop = SdkMenus.MenuCreate(SdkText.Translate("shop-title") ?? "", 3);
        string lore = SdkText.Translate("shop-item-sparkles-lore", new Args().Add("price", "100")) ?? "";
        SdkMenus.MenuSet(shop, 13, "minecraft:glowstone_dust", SdkText.Translate("shop-item-sparkles") ?? "", lore, BuySparkles);

        int period = (int)SdkConfig.ConfigI64("announce-period", 1200);
        SdkTasks.Every(period, period, ctx =>
        {
            foreach (int player in SdkPlayers.PlayersOnline())
            {
                SdkPlayers.Send(player, "@announce");
            }
        });
    }
}
