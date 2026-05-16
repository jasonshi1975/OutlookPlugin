#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <functional>

// AI Service Configuration
struct AIServiceConfig {
    std::wstring name;
    std::wstring url;
    std::wstring apiKey;
    std::wstring model;
};

// AI Service Types
enum class AIServiceType {
    OpenAI,
    DeepSeek,
    Claude,
    Kimi,
    MiniMax,
    Qianwen
};

// Default AI Service URLs
const wchar_t* DEFAULT_URLS[] = {
    L"https://api.openai.com/v1",
    L"https://api.deepseek.com/v1",
    L"https://api.anthropic.com/v1",
    L"https://api.moonshot.cn/v1",
    L"https://api.minimax.chat/v1",
    L"https://dashscope.aliyuncs.com/compatible-mode/v1"
};

// AI HTTP Client Interface
class IAIHttpClient {
public:
    virtual ~IAIHttpClient() = default;

    virtual std::wstring Translate(const std::wstring& content, const std::wstring& targetLanguage) = 0;
    virtual std::wstring Summarize(const std::wstring& content, const std::wstring& style) = 0;
    virtual std::wstring GenerateReply(const std::wstring& content, const std::wstring& tone) = 0;
    virtual std::wstring Polish(const std::wstring& content, const std::wstring& style) = 0;

    virtual void SetConfig(const AIServiceConfig& config) = 0;
};

// Factory function
IAIHttpClient* CreateAIHttpClient();