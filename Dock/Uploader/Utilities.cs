using System;

namespace MonoUtils
{
    internal class Utilities
    {
        internal static bool IsRunningOnMono()
        {
            return Type.GetType("Mono.Runtime") != null;
        }
    }
}