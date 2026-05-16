using System;
using System.Runtime.InteropServices;
using Microsoft.Win32;
using System.Reflection;

namespace AIAssistant;

public static class ComRegistration
{
    private const string Clsid = "{a1b2c3d4-e5f6-7890-abcd-ef1234567890}";
    private const string ProgId = "AIAssistant";

    [ComRegisterFunction]
    public static void Register(Type t)
    {
        Log("=== Register === COM Registration");

        // Get DLL path
        string dllPath = Assembly.GetExecutingAssembly().Location;
        Log($"  DLL Path: {dllPath}");

        // Register CLSID
        using (var key = Registry.ClassesRoot.CreateSubKey($"CLSID\\{Clsid}"))
        {
            key.SetValue("", "AI Assistant Add-in");
        }

        using (var key = Registry.ClassesRoot.CreateSubKey($"CLSID\\{Clsid}\\InprocServer32"))
        {
            key.SetValue("", dllPath);
            key.SetValue("ThreadingModel", "Both");
        }

        // Register ProgID
        using (var key = Registry.ClassesRoot.CreateSubKey(ProgId))
        {
            key.SetValue("", "AI Assistant Add-in");
        }

        using (var key = Registry.ClassesRoot.CreateSubKey($"{ProgId}\\CLSID"))
        {
            key.SetValue("", Clsid);
        }

        // Register Outlook Add-in
        using (var key = Registry.CurrentUser.CreateSubKey("Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant"))
        {
            key.SetValue("LoadBehavior", 3);
            key.SetValue("FriendlyName", "AI Assistant");
            key.SetValue("Description", "AI-powered email assistant");
        }

        using (var key = Registry.CurrentUser.CreateSubKey("Software\\Microsoft\\Office\\16.0\\Outlook\\Addins\\AIAssistant"))
        {
            key.SetValue("LoadBehavior", 3);
            key.SetValue("FriendlyName", "AI Assistant");
            key.SetValue("Description", "AI-powered email assistant");
        }

        Log("  Registration complete");
    }

    [ComUnregisterFunction]
    public static void Unregister(Type t)
    {
        Log("=== Unregister === COM Unregistration");

        try
        {
            Registry.ClassesRoot.DeleteSubKeyTree($"CLSID\\{Clsid}", false);
            Registry.ClassesRoot.DeleteSubKeyTree(ProgId, false);
            Registry.CurrentUser.DeleteSubKeyTree("Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant", false);
            Registry.CurrentUser.DeleteSubKeyTree("Software\\Microsoft\\Office\\16.0\\Outlook\\Addins\\AIAssistant", false);
        }
        catch { }

        Log("  Unregistration complete");
    }

    private static void Log(string msg)
    {
        try
        {
            var path = System.IO.Path.Combine(
                Environment.GetEnvironmentVariable("TEMP") ?? "C:\\Temp",
                "AIAssistantCS_reg.log");
            using var sw = new System.IO.StreamWriter(path, true);
            sw.WriteLine($"[{DateTime.Now:HH:mm:ss}] {msg}");
        }
        catch { }
    }
}