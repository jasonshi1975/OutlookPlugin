#include "stdafx.h"
#include <winhttp.h>
#include <sstream>

// Simple JSON extraction without external library
std::wstring ExtractContent(const std::string& response) {
    // Find "content" field in response
    std::string key = "\"content\"";
    size_t pos = response.find(key);
    if (pos == std::string::npos) return L"";

    pos = response.find("\"", pos + key.length() + 1);
    if (pos == std::string::npos) return L"";

    size_t endPos = response.find("\"", pos + 1);
    if (endPos == std::string::npos) return L"";

    std::string content = response.substr(pos + 1, endPos - pos - 1);

    // Convert to wide string
    int len = MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, NULL, 0);
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, &result[0], len);

    return result;
}

std::string WideToUtf8(const std::wstring& wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

std::string EscapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            default: result += c;
        }
    }
    return result;
}

class AIHttpClient {
public:
    AIHttpClient() : hSession(NULL), hConnect(NULL) {}

    ~AIHttpClient() {
        if (hConnect) WinHttpCloseHandle(hConnect);
        if (hSession) WinHttpCloseHandle(hSession);
    }

    void SetConfig(const AIServiceConfig& cfg) {
        config = cfg;
        InitSession();
    }

    std::wstring Translate(const std::wstring& content, const std::wstring& targetLang) {
        std::wstring prompt = L"You are a translator. Translate to " + targetLang + L":";
        return CallAPI(prompt, content);
    }

    std::wstring Summarize(const std::wstring& content) {
        std::wstring prompt = L"You are an email summarizer. Summarize key points:";
        return CallAPI(prompt, content);
    }

    std::wstring GenerateReply(const std::wstring& content) {
        std::wstring prompt = L"You are an email reply generator. Generate a professional reply:";
        return CallAPI(prompt, content);
    }

    std::wstring Polish(const std::wstring& content) {
        std::wstring prompt = L"You are an email editor. Polish and improve:";
        return CallAPI(prompt, content);
    }

private:
    AIServiceConfig config;
    HINTERNET hSession;
    HINTERNET hConnect;

    void InitSession() {
        hSession = WinHttpOpen(L"AI Assistant Add-in/1.0",
                                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                WINHTTP_NO_PROXY_NAME,
                                WINHTTP_NO_PROXY_BYPASS, 0);

        // Extract host from URL
        std::wstring host = GetHost(config.url);
        hConnect = WinHttpConnect(hSession, host.c_str(),
                                   INTERNET_DEFAULT_HTTPS_PORT, 0);
    }

    std::wstring GetHost(const std::wstring& url) {
        size_t start = url.find(L"://");
        if (start == std::wstring::npos) return url;
        start += 3;
        size_t end = url.find(L"/", start);
        if (end == std::wstring::npos) return url.substr(start);
        return url.substr(start, end - start);
    }

    std::wstring CallAPI(const std::wstring& systemPrompt, const std::wstring& userContent) {
        if (!hConnect) return L"Error: Not connected";

        // Build JSON request
        std::string body = BuildRequest(systemPrompt, userContent);

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
                                                 L"/chat/completions",
                                                 NULL, WINHTTP_NO_REFERER,
                                                 WINHTTP_DEFAULT_ACCEPT_TYPES,
                                                 WINHTTP_FLAG_SECURE);

        // Set headers
        std::wstring headers = L"Content-Type: application/json\r\nAuthorization: Bearer "
                                + config.apiKey + L"\r\n";
        WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

        // Send
        BOOL result = WinHttpSendRequest(hRequest,
                                          WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                          (LPVOID)body.c_str(), body.length(),
                                          body.length(), 0);

        if (!result) {
            WinHttpCloseHandle(hRequest);
            return L"Error: Send failed";
        }

        // Receive
        WinHttpReceiveResponse(hRequest, NULL);

        // Read data
        std::string response;
        DWORD dwSize = 0;
        DWORD dwDownloaded = 0;
        LPSTR pszOutBuffer;

        do {
            dwSize = 0;
            WinHttpQueryDataAvailable(hRequest, &dwSize);
            if (dwSize == 0) break;

            pszOutBuffer = new char[dwSize + 1];
            ZeroMemory(pszOutBuffer, dwSize + 1);

            if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                response += pszOutBuffer;
            }
            delete[] pszOutBuffer;
        } while (dwSize > 0);

        WinHttpCloseHandle(hRequest);

        return ExtractContent(response);
    }

    std::string BuildRequest(const std::wstring& systemPrompt, const std::wstring& userContent) {
        std::string modelUtf8 = WideToUtf8(config.model);
        std::string systemUtf8 = WideToUtf8(systemPrompt);
        std::string userUtf8 = WideToUtf8(userContent);

        std::ostringstream oss;
        oss << "{\"model\":\"" << modelUtf8 << "\","
            << "\"messages\":["
            << "{\"role\":\"system\",\"content\":\"" << EscapeJson(systemUtf8) << "\"},"
            << "{\"role\":\"user\",\"content\":\"" << EscapeJson(userUtf8) << "\"}"
            << "],"
            << "\"temperature\":0.7,"
            << "\"max_tokens\":2000}";

        return oss.str();
    }
};