# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Outlook COM Add-in (C++ ATL) providing AI-powered email assistance:

- **AI翻译**: 翻译邮件内容
- **AI汇总**: 汇总邮件要点
- **AI自动回复**: 根据邮件内容生成回复
- **AI润色**: 润色新建/回复邮件

## Technology Stack

- C++ ATL COM Add-in
- Outlook Object Model (MSOUTL.OLB)
- WinHTTP for AI API calls
- Visual Studio 2022

## Build Commands

```bash
# Build using Visual Studio
# Open cpp/AIAssistant.sln in Visual Studio 2022
# Build → Build Solution (Ctrl+Shift+B)

# Or use MSBuild from command line:
cd cpp
msbuild AIAssistant.vcxproj /p:Configuration=Release /p:Platform=Win32
```

## Deployment

```bash
# Register the DLL (requires admin privileges)
regsvr32 AIAssistant.dll

# Unregister
regsvr32 /u AIAssistant.dll
```

## Architecture

```
cpp/
├── include/
│   ├── AIHttpClient.h     # AI API interface
│   ├── OutlookHelper.h    # Outlook OM wrapper
│   ├── Connect.h          # IDTExtensibility2 implementation
│   └── resource.h         # Resource IDs
├── src/
│   ├── AIHttpClient.cpp   # WinHTTP AI API calls
│   ├── OutlookHelper.cpp  # MailItem operations
│   ├── Connect.cpp        # Add-in lifecycle
│   ├── dllmain.cpp        # DLL entry + registration
│   └── stdafx.h           # Precompiled header
├── res/
│   ├── Ribbon.xml         # UI definition (5 buttons)
│   ├── AIAssistant.rc     # Resource file
│   └── Connect.rgs        # Registry script
├── AIAssistant.vcxproj    # VS project file
└── AIAssistant.sln        # VS solution file
```

## Supported AI Services

| Service | URL | Models |
|---------|-----|--------|
| OpenAI | api.openai.com/v1 | gpt-4o, gpt-4-turbo |
| DeepSeek | api.deepseek.com/v1 | deepseek-chat |
| Claude | api.anthropic.com/v1 | claude-3-5-sonnet |
| Kimi | api.moonshot.cn/v1 | moonshot-v1-8k |
| MiniMax | api.minimax.chat/v1 | abab6.5-chat |
| Qianwen | dashscope.aliyuncs.com/v1 | qwen-max |

## Key Patterns

### Outlook MailItem Access

```cpp
CComPtr<IDispatch> pMailItem = m_pOutlookHelper->GetCurrentMailItem();
std::wstring content = m_pOutlookHelper->GetMailBodyText(pMailItem);
```

### AI API Call

```cpp
std::wstring translated = m_pAIHttpClient->Translate(content, L"Chinese");
```

### Inline Translation Overlay

```cpp
m_pOutlookHelper->ShowInlineTranslation(pMailItem, translatedContent);
```

## Config Storage

- Registry: `HKCU\Software\AIAssistant\Outlook`
- API Key: Stored encrypted (production should use DPAPI)

## Development Notes

- Requires Visual Studio 2022 with v143 toolset
- Requires Windows SDK 10.0
- Link against winhttp.lib for HTTP calls
- Import MSOUTL.OLB for Outlook type definitions