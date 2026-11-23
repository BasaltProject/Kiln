using Basalt.Proxy.Core;
using Basalt.Proxy.Internal;

namespace Basalt.Proxy.Events;

public static partial class EventBus {
    public static int EventSetStr(int ctx, string field, string value) => ProxyNative.EventSetStr(ctx, field, value);
    public static int EventSetI64(int ctx, string field, long value) => ProxyNative.EventSetI64(ctx, field, value);
}
