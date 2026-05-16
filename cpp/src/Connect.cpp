#include "stdafx.h"
#include "Connect.h"
#include "AIHttpClient.h"
#include "OutlookHelper.h"
#include <fstream>
#include <shlobj.h>  // For SHGetFolderPath (config storage)

// Registry key for config storage
const wchar_t* REG_KEY = L"Software\\AIAssistant\\Outlook";
const wchar_t* REG_SERVICE = L"Service";
const wchar_t* REG_URL = L"URL";
const wchar_t* REG_MODEL = L"Model";
const wchar_t* REG_KEY_API = L"ApiKey";  // Note: API key stored encrypted

CConnect::CConnect()
    : m_pApplication(nullptr)
    , m_pAddInInstance(nullptr)
    , m_pOutlookHelper(nullptr)
    , m_pAIHttpClient(nullptr)
{
    m_pOutlookHelper = new OutlookHelper();
    m_pAIHttpClient = CreateAIHttpClient();
    LoadConfig();
}

CConnect::~CConnect() {
    if (m_pOutlookHelper) delete m_pOutlookHelper;
    if (m_pAIHttpClient) delete m_pAIHttpClient;
}

// IDTExtensibility2::OnConnection
STDMETHODIMP CConnect::OnConnection(IDispatch* pApplication, ext_ConnectMode ConnectMode,
                                     IDispatch* pAddInInst, SAFEARRAY** custom) {
    m_pApplication = pApplication;
    m_pAddInInstance = pAddInInst;

    // Initialize Outlook helper
    if (m_pOutlookHelper) {
        m_pOutlookHelper->Initialize(pApplication);
    }

    // Initialize AI client with config
    if (m_pAIHttpClient) {
        m_pAIHttpClient->SetConfig(m_config);
    }

    return S_OK;
}

// IDTExtensibility2::OnDisconnection
STDMETHODIMP CConnect::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY** custom) {
    m_pApplication.Release();
    m_pAddInInstance.Release();
    return S_OK;
}

// IDTExtensibility2::OnStartupComplete
STDMETHODIMP CConnect::OnStartupComplete(SAFEARRAY** custom) {
    // Add-in is now fully loaded
    return S_OK;
}

// IDTExtensibility2::OnBeginShutdown
STDMETHODIMP CConnect::OnBeginShutdown(SAFEARRAY** custom) {
    SaveConfig();
    return S_OK;
}

// IDTExtensibility2::OnAddInsUpdate
STDMETHODIMP CConnect::OnAddInsUpdate(SAFEARRAY** custom) {
    return S_OK;
}

// Settings button click
STDMETHODIMP CConnect::OnSettingsClick(IDispatch* pDisp) {
    // Show settings dialog (90% screen)
    ShowDialog(L"settings");
    return S_OK;
}

// Translate button click
STDMETHODIMP CConnect::OnTranslateClick(IDispatch* pDisp) {
    if (!m_pOutlookHelper || !m_pAIHttpClient) return E_FAIL;

    // Get current mail item
    CComPtr<IDispatch> pMailItem = m_pOutlookHelper->GetCurrentMailItem();
    if (!pMailItem) {
        // Show error message
        MessageBox(nullptr, L"Please select an email first.", L"AI Assistant", MB_OK);
        return S_OK;
    }

    // Get email content
    std::wstring content = m_pOutlookHelper->GetMailBodyText(pMailItem);
    if (content.empty()) {
        MessageBox(nullptr, L"Could not read email content.", L"AI Assistant", MB_OK);
        return S_OK;
    }

    // Call AI to translate
    std::wstring targetLanguage = L"Chinese";  // Default, can be configured
    std::wstring translated = m_pAIHttpClient->Translate(content, targetLanguage);

    // Show inline translation overlay
    m_pOutlookHelper->ShowInlineTranslation(pMailItem, translated);

    return S_OK;
}

// Summary button click
STDMETHODIMP CConnect::OnSummaryClick(IDispatch* pDisp) {
    if (!m_pOutlookHelper || !m_pAIHttpClient) return E_FAIL;

    CComPtr<IDispatch> pMailItem = m_pOutlookHelper->GetCurrentMailItem();
    if (!pMailItem) {
        MessageBox(nullptr, L"Please select an email first.", L"AI Assistant", MB_OK);
        return S_OK;
    }

    std::wstring content = m_pOutlookHelper->GetMailBodyText(pMailItem);
    std::wstring summary = m_pAIHttpClient->Summarize(content, L"bullet points");

    // Show summary in large dialog
    ShowDialog(L"summary");
    // TODO: Pass summary content to dialog

    return S_OK;
}

// Reply button click
STDMETHODIMP CConnect::OnReplyClick(IDispatch* pDisp) {
    if (!m_pOutlookHelper || !m_pAIHttpClient) return E_FAIL;

    CComPtr<IDispatch> pMailItem = m_pOutlookHelper->GetCurrentMailItem();
    if (!pMailItem) {
        MessageBox(nullptr, L"Please select an email first.", L"AI Assistant", MB_OK);
        return S_OK;
    }

    std::wstring content = m_pOutlookHelper->GetMailBodyText(pMailItem);
    std::wstring reply = m_pAIHttpClient->GenerateReply(content, L"Formal");

    // Show reply dialog (90% screen)
    ShowDialog(L"reply");
    // TODO: Pass reply content to dialog

    return S_OK;
}

// Polish button click
STDMETHODIMP CConnect::OnPolishClick(IDispatch* pDisp) {
    if (!m_pOutlookHelper || !m_pAIHttpClient) return E_FAIL;

    // For polish, we need to check if we're in compose mode
    // Get current item (could be draft being edited)
    CComPtr<IDispatch> pMailItem = m_pOutlookHelper->GetCurrentMailItem();
    if (!pMailItem) {
        MessageBox(nullptr, L"Please select or create an email.", L"AI Assistant", MB_OK);
        return S_OK;
    }

    std::wstring content = m_pOutlookHelper->GetMailBodyText(pMailItem);
    std::wstring polished = m_pAIHttpClient->Polish(content, L"formal");

    // Show polish dialog
    ShowDialog(L"polish");
    // TODO: Pass polished content to dialog

    return S_OK;
}

void CConnect::LoadConfig() {
    // Load from registry
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey);

    if (result == ERROR_SUCCESS) {
        wchar_t buffer[1024];
        DWORD dwSize = sizeof(buffer);

        // Read service type
        if (RegQueryValueEx(hKey, REG_SERVICE, nullptr, nullptr, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS) {
            m_config.name = buffer;
        }

        // Read URL
        dwSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, REG_URL, nullptr, nullptr, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS) {
            m_config.url = buffer;
        }

        // Read model
        dwSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, REG_MODEL, nullptr, nullptr, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS) {
            m_config.model = buffer;
        }

        // Read API key (TODO: decrypt)
        dwSize = sizeof(buffer);
        if (RegQueryValueEx(hKey, REG_KEY_API, nullptr, nullptr, (LPBYTE)buffer, &dwSize) == ERROR_SUCCESS) {
            m_config.apiKey = buffer;  // In production, decrypt this!
        }

        RegCloseKey(hKey);
    } else {
        // Default config
        m_config.name = L"OpenAI";
        m_config.url = DEFAULT_URLS[0];  // OpenAI
        m_config.model = L"gpt-4o";
        m_config.apiKey = L"";
    }
}

void CConnect::SaveConfig() {
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, nullptr,
                                  REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);

    if (result == ERROR_SUCCESS) {
        RegSetValueEx(hKey, REG_SERVICE, 0, REG_SZ, (LPBYTE)m_config.name.c_str(),
                      (m_config.name.length() + 1) * sizeof(wchar_t));
        RegSetValueEx(hKey, REG_URL, 0, REG_SZ, (LPBYTE)m_config.url.c_str(),
                      (m_config.url.length() + 1) * sizeof(wchar_t));
        RegSetValueEx(hKey, REG_MODEL, 0, REG_SZ, (LPBYTE)m_config.model.c_str(),
                      (m_config.model.length() + 1) * sizeof(wchar_t));

        // Save API key (TODO: encrypt)
        RegSetValueEx(hKey, REG_KEY_API, 0, REG_SZ, (LPBYTE)m_config.apiKey.c_str(),
                      (m_config.apiKey.length() + 1) * sizeof(wchar_t));

        RegCloseKey(hKey);
    }
}

void CConnect::ShowDialog(const std::wstring& dialogType) {
    // Create large dialog (90% screen)
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int dialogWidth = screenWidth * 90 / 100;
    int dialogHeight = screenHeight * 90 / 100;

    // TODO: Create actual dialog window with Win32 API
    // For now, show a simple message box as placeholder
    MessageBox(nullptr,
               (L"Dialog: " + dialogType + L"\n\nFeature coming in next iteration.").c_str(),
               L"AI Assistant",
               MB_OK);
}