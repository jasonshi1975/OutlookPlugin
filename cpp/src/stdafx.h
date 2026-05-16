#pragma once

#include <windows.h>
#include <ole2.h>
#include <objbase.h>
#include <string>

// Outlook Object Model DISPID values
#define DISPID_MAIL_SUBJECT      0x000F
#define DISPID_MAIL_BODY         0x0010
#define DISPID_MAIL_HTMLBODY     0x0011
#define DISPID_MAIL_DISPLAY      0x0FA0
#define DISPID_MAIL_REPLY        0x0FA1
#define DISPID_EXPLORER_SELECTION 0x0002
#define DISPID_SELECTION_ITEM    0x0001
#define DISPID_SELECTION_COUNT   0x0003

// IDTExtensibility2 interface GUID
static const GUID IID_IDTExtensibility2 =
{ 0xB65AD801, 0xAB47, 0x400F, { 0xB6, 0x88, 0x00, 0x19, 0x2A, 0x4E, 0x23, 0x84 } };

// ext_ConnectMode enum
enum ext_ConnectMode {
    ext_cm_AfterStartup = 0,
    ext_cm_Startup = 1,
    ext_cm_External = 2,
    ext_cm_CommandLine = 3
};

// ext_DisconnectMode enum
enum ext_DisconnectMode {
    ext_dm_HostShutdown = 0,
    ext_dm_UserClosed = 1
};

// AI Service Config
struct AIServiceConfig {
    std::wstring service;
    std::wstring url;
    std::wstring model;
    std::wstring apiKey;
};

// Default URLs
const wchar_t* AI_URLS[] = {
    L"https://api.openai.com/v1",
    L"https://api.deepseek.com/v1",
    L"https://api.anthropic.com/v1",
    L"https://api.moonshot.cn/v1",
    L"https://api.minimax.chat/v1",
    L"https://dashscope.aliyuncs.com/compatible-mode/v1"
};