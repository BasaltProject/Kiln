using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Basalt.Server.Core;
using Basalt.Server.Internal;
using Basalt.Server.Values;
using static Basalt.Server.Internal.Native;
using static Basalt.Server.Core.Level;
using static Basalt.Server.Core.Priority;
namespace Basalt.Server.Data;

    public readonly struct TopEntry
    {
        public readonly string Name;
        public readonly long Value;

        public TopEntry(string name, long value)
        {
            Name = name;
            Value = value;
        }
    }
