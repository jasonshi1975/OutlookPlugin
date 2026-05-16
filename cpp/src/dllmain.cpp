#include "stdafx.h"
#include "AIHttpClient.cpp"
#include "OutlookHelper.cpp"
#include <shlobj.h>

// CLSID for our add-in
static const CLSID CLSID_AIAssistant =
{ 0xa1b2c3d4, 0xe5f6, 0x7890, { 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90 } };

// Registry config key
const wchar_t* REG_KEY = L"Software\\AIAssistant\\Outlook";

class AIAssistantAddin : public IDTExtensibility2 {
public:
    AIAssistantAddin() : refCount(1), pApplication(NULL), pOutlookHelper(NULL), pAIClient(NULL) {
        pOutlookHelper = new OutlookHelper();
        pAIClient = new AIHttpClient();
        LoadConfig();
    }

    ~AIAssistantAddin() {
        if (pOutlookHelper) delete pOutlookHelper;
        if (pAIClient) delete pAIClient;
        if (pApplication) pApplication->Release();
    }

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IDTExtensibility2) {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() {
        return ++refCount;
    }

    STDMETHOD_(ULONG, Release)() {
        ULONG count = --refCount;
        if (count == 0) delete this;
        return count;
    }

    // IDTExtensibility2
    STDMETHOD(OnConnection)(IDispatch* Application, ext_ConnectMode ConnectMode,
                             IDispatch* AddInInst, SAFEARRAY** custom) {
        pApplication = Application;
        if (pApplication) pApplication->AddRef();

        if (pOutlookHelper) pOutlookHelper->Initialize(Application);
        if (pAIClient) pAIClient->SetConfig(config);

        return S_OK;
    }

    STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY** custom) {
        SaveConfig();
        return S_OK;
    }

    STDMETHOD(OnStartupComplete)(SAFEARRAY** custom) { return S_OK; }
    STDMETHOD(OnBeginShutdown)(SAFEARRAY** custom) { return S_OK; }
    STDMETHOD(OnAddInsUpdate)(SAFEARRAY** custom) { return S_OK; }

    // Button handlers (called from Ribbon via IDispatch)
    STDMETHOD(OnTranslateClick)() {
        if (!pOutlookHelper || !pAIClient) return E_FAIL;

        IDispatch* pMail = pOutlookHelper->GetCurrentMailItem();
        if (!pMail) {
            MessageBox(NULL, L"Please select an email first.", L"AI Assistant", MB_OK);
            return S_OK;
        }

        std::wstring content = pOutlookHelper->GetMailBody(pMail);
        std::wstring translated = pAIClient->Translate(content, L"Chinese");
        pOutlookHelper->ShowInlineTranslation(pMail, translated);
        pMail->Release();

        return S_OK;
    }

    STDMETHOD(OnSummaryClick)() {
        IDispatch* pMail = pOutlookHelper->GetCurrentMailItem();
        if (!pMail) return E_FAIL;

        std::wstring content = pOutlookHelper->GetMailBody(pMail);
        std::wstring summary = pAIClient->Summarize(content);
        MessageBox(NULL, summary.c_str(), L"Email Summary", MB_OK);
        pMail->Release();

        return S_OK;
    }

    STDMETHOD(OnReplyClick)() {
        IDispatch* pMail = pOutlookHelper->GetCurrentMailItem();
        if (!pMail) return E_FAIL;

        std::wstring content = pOutlookHelper->GetMailBody(pMail);
        std::wstring reply = pAIClient->GenerateReply(content);
        pOutlookHelper->DisplayReply(pMail, reply);
        pMail->Release();

        return S_OK;
    }

    STDMETHOD(OnPolishClick)() {
        IDispatch* pMail = pOutlookHelper->GetCurrentMailItem();
        if (!pMail) return E_FAIL;

        std::wstring content = pOutlookHelper->GetMailBody(pMail);
        std::wstring polished = pAIClient->Polish(content);
        MessageBox(NULL, polished.c_str(), L"Polished Email", MB_OK);
        pMail->Release();

        return S_OK;
    }

private:
    ULONG refCount;
    IDispatch* pApplication;
    OutlookHelper* pOutlookHelper;
    AIHttpClient* pAIClient;
    AIServiceConfig config;

    void LoadConfig() {
        HKEY hKey;
        if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            wchar_t buf[1024];
            DWORD size = sizeof(buf);

            if (RegQueryValueEx(hKey, L"Service", NULL, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
                config.service = buf;

            size = sizeof(buf);
            if (RegQueryValueEx(hKey, L"URL", NULL, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
                config.url = buf;

            size = sizeof(buf);
            if (RegQueryValueEx(hKey, L"Model", NULL, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
                config.model = buf;

            size = sizeof(buf);
            if (RegQueryValueEx(hKey, L"ApiKey", NULL, NULL, (LPBYTE)buf, &size) == ERROR_SUCCESS)
                config.apiKey = buf;

            RegCloseKey(hKey);
        } else {
            // Defaults
            config.service = L"OpenAI";
            config.url = AI_URLS[0];
            config.model = L"gpt-4o";
            config.apiKey = L"";
        }
    }

    void SaveConfig() {
        HKEY hKey;
        if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, NULL,
                           REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
            RegSetValueEx(hKey, L"Service", 0, REG_SZ, (LPBYTE)config.service.c_str(),
                          (config.service.length()+1)*sizeof(wchar_t));
            RegSetValueEx(hKey, L"URL", 0, REG_SZ, (LPBYTE)config.url.c_str(),
                          (config.url.length()+1)*sizeof(wchar_t));
            RegSetValueEx(hKey, L"Model", 0, REG_SZ, (LPBYTE)config.model.c_str(),
                          (config.model.length()+1)*sizeof(wchar_t));
            RegSetValueEx(hKey, L"ApiKey", 0, REG_SZ, (LPBYTE)config.apiKey.c_str(),
                          (config.apiKey.length()+1)*sizeof(wchar_t));
            RegCloseKey(hKey);
        }
    }
};

// Class Factory
class AIAssistantClassFactory : public IClassFactory {
public:
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }

    STDMETHOD(CreateInstance)(IUnknown* pOuter, REFIID riid, void** ppv) {
        if (pOuter) return CLASS_E_NOAGGREGATION;
        AIAssistantAddin* pAddin = new AIAssistantAddin();
        return pAddin->QueryInterface(riid, ppv);
    }

    STDMETHOD(LockServer)(BOOL fLock) { return S_OK; }
};

// DLL exports
HINSTANCE g_hInst = NULL;

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    if (rclsid == CLSID_AIAssistant) {
        AIAssistantClassFactory* pFactory = new AIAssistantClassFactory();
        return pFactory->QueryInterface(riid, ppv);
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() { return S_OK; }

STDAPI DllRegisterServer() {
    wchar_t modulePath[MAX_PATH];
    GetModuleFileName(g_hInst, modulePath, MAX_PATH);

    // Register CLSID
    HKEY hKey;
    std::wstring keyPath = L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}";
    RegCreateKeyEx(HKEY_CLASSES_ROOT, keyPath.c_str(), 0, NULL,
                   REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)L"AI Assistant",
                  (wcslen(L"AI Assistant")+1)*sizeof(wchar_t));
    RegCloseKey(hKey);

    // Register InprocServer32
    RegCreateKeyEx(HKEY_CLASSES_ROOT, (keyPath+L"\\InprocServer32").c_str(),
                   0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)modulePath,
                  (wcslen(modulePath)+1)*sizeof(wchar_t));
    RegSetValueEx(hKey, L"ThreadingModel", 0, REG_SZ, (LPBYTE)L"Apartment",
                  (wcslen(L"Apartment")+1)*sizeof(wchar_t));
    RegCloseKey(hKey);

    // Register Outlook Add-in
    RegCreateKeyEx(HKEY_CURRENT_USER,
                   L"Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant",
                   0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    DWORD loadBehavior = 3;
    RegSetValueEx(hKey, L"LoadBehavior", 0, REG_DWORD, (LPBYTE)&loadBehavior, sizeof(DWORD));
    RegCloseKey(hKey);

    return S_OK;
}

STDAPI DllUnregisterServer() {
    RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant");
    RegDeleteKey(HKEY_CLASSES_ROOT, L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}\\InprocServer32");
    RegDeleteKey(HKEY_CLASSES_ROOT, L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}");
    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) g_hInst = (HINSTANCE)hModule;
    return TRUE;
}