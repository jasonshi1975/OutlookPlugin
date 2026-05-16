using System;
using System.Net.Http;
using System.Text;
using Newtonsoft.Json.Linq;

namespace AIAssistant;

public class AIHttpClient
{
    private readonly AIConfig config;
    private readonly HttpClient client;

    public AIHttpClient(AIConfig config)
    {
        this.config = config;
        client = new HttpClient();
        client.BaseAddress = new Uri(config.Url);
    }

    public string Translate(string content)
    {
        return CallAI("Translate this email to Chinese:", content);
    }

    public string Summarize(string content)
    {
        return CallAI("Summarize the key points of this email:", content);
    }

    public string GenerateReply(string content)
    {
        return CallAI("Generate a professional reply to this email:", content);
    }

    public string Polish(string content)
    {
        return CallAI("Polish and improve this email:", content);
    }

    private string CallAI(string prompt, string content)
    {
        try
        {
            var body = new
            {
                model = config.Model,
                messages = new[]
                {
                    new { role = "system", content = prompt },
                    new { role = "user", content = content }
                },
                temperature = 0.7,
                max_tokens = 2000
            };

            var json = Newtonsoft.Json.JsonConvert.SerializeObject(body);
            var request = new HttpRequestMessage(HttpMethod.Post, "/chat/completions");
            request.Content = new StringContent(json, Encoding.UTF8, "application/json");
            request.Headers.Add("Authorization", $"Bearer {config.ApiKey}");

            var response = client.SendAsync(request).Result;
            var responseJson = response.Content.ReadAsStringAsync().Result;

            var obj = JObject.Parse(responseJson);
            var resultContent = obj["choices"]?[0]?["message"]?["content"]?.ToString();
            return resultContent ?? "Error: No response";
        }
        catch (Exception ex)
        {
            return $"Error: {ex.Message}";
        }
    }
}