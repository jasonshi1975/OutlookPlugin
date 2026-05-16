using System;
using System.Runtime.InteropServices;

namespace AIAssistant;

// IRibbonExtensibility - official Office GUID
[Guid("000C0396-0000-0000-C000-000000000046")]
[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
[ComVisible(true)]
public interface IRibbonExtensibility
{
    string GetCustomUI(string RibbonID);
}

// IRibbonControl - passed to callback methods
[Guid("000C0397-0000-0000-C000-000000000046")]
[InterfaceType(ComInterfaceType.InterfaceIsDual)]
[ComVisible(true)]
public interface IRibbonControl
{
    string Id { get; }
    object Context { get; }
    string Tag { get; }
}