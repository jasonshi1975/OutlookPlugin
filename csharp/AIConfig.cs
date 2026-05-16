using System;
using System.IO;
using System.Text.Json;
using Microsoft.Win32;

namespace AIAssistant;

public class AIConfig
{
    public string Service { get; set; } = "OpenAI";
    public string Url { get; set; } = "https://api.openai.com/v1";
    public string Model { get; set; } = "gpt-4o";
    public string ApiKey { get; set; } = "";

    private const string RegKey = "Software\\AIAssistant\\Outlook";

    public static AIConfig Load()
    {
        var config = new AIConfig();
        try
        {
            using var key = Registry.CurrentUser.OpenSubKey(RegKey);
            if (key != null)
            {
                config.Service = key.GetValue("Service")?.ToString() ?? "OpenAI";
                config.Url = key.GetValue("URL")?.ToString() ?? "https://api.openai.com/v1";
                config.Model = key.GetValue("Model")?.ToString() ?? "gpt-4o";
                config.ApiKey = key.GetValue("ApiKey")?.ToString() ?? "";
            }
        }
        catch { }
        return config;
    }

    public void Save()
    {
        try
        {
            using var key = Registry.CurrentUser.CreateSubKey(RegKey);
            key.SetValue("Service", Service);
            key.SetValue("URL", Url);
            key.SetValue("Model", Model);
            key.SetValue("ApiKey", ApiKey);
        }
        catch { }
    }
}