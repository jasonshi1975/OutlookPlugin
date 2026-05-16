#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <ole2.h>
#include <objbase.h>
#include <winhttp.h>
#include <shlobj.h>
#include <string>
#include <sstream>
#include <ctime>

// ==================== LOG SYSTEM ====================
static void LogToFile(const wchar_t* msg) {
    wchar_t logPath[MAX_PATH];
    GetEnvironmentVariableW(L"TEMP", logPath, MAX_PATH);
    wcscat(logPath, L"\\AIAssistant_log.txt");

    HANDLE hFile = CreateFileW(logPath,
                                FILE_APPEND_DATA, FILE_SHARE_READ,
                                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        char timestamp[64];
        time_t now = time(NULL);
        struct tm* t = localtime(&now);
        sprintf(timestamp, "[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);

        DWORD written;
        WriteFile(hFile, timestamp, strlen(timestamp), &written, NULL);

        int len = WideCharToMultiByte(CP_UTF8, 0, msg, -1, NULL, 0, NULL, NULL);
        char* buf = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, msg, -1, buf, len, NULL, NULL);
        WriteFile(hFile, buf, len-1, &written, NULL);
        WriteFile(hFile, "\r\n", 2, &written, NULL);
        delete[] buf;
        CloseHandle(hFile);
    }
}

static void LogGuid(const wchar_t* prefix, REFGUID guid) {
    wchar_t buf[128];
    swprintf(buf, 128, L"%s {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
             prefix, guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
             guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7], guid.Data4[8]);
    LogToFile(buf);
}

// ==================== IDTExtensibility2 Interface Definition ====================
MIDL_INTERFACE("B65AD801-ABAF-11D0-BB8B-00A0C90F2744")
IDTExtensibility2 : public IDispatch {
public:
    virtual HRESULT STDMETHODCALLTYPE OnConnection(
        IDispatch* Application,
        long ConnectMode,
        IDispatch* AddInInst,
        SAFEARRAY** custom) = 0;

    virtual HRESULT STDMETHODCALLTYPE OnDisconnection(
        long RemoveMode,
        SAFEARRAY** custom) = 0;

    virtual HRESULT STDMETHODCALLTYPE OnStartupComplete(
        SAFEARRAY** custom) = 0;

    virtual HRESULT STDMETHODCALLTYPE OnBeginShutdown(
        SAFEARRAY** custom) = 0;

    virtual HRESULT STDMETHODCALLTYPE OnAddInsUpdate(
        SAFEARRAY** custom) = 0;
};

// ==================== IRibbonExtensibility Interface Definition ====================
MIDL_INTERFACE("000C0396-0000-0000-C000-000000000046")
IRibbonExtensibility : public IDispatch {
public:
    virtual HRESULT STDMETHODCALLTYPE GetCustomUI(
        BSTR RibbonID,
        BSTR* RibbonXml) = 0;
};

// ==================== Ribbon Callback Interface ====================
MIDL_INTERFACE("000C0397-0000-0000-C000-000000000046")
IRibbonControl : public IDispatch {
public:
    virtual HRESULT STDMETHODCALLTYPE get_Id(BSTR* Id) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Context(IDispatch** Context) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Tag(BSTR* Tag) = 0;
};

// ==================== AI Config ====================
struct AIServiceConfig {
    std::wstring service;
    std::wstring url;
    std::wstring model;
    std::wstring apiKey;
};

const wchar_t* AI_URLS[] = {
    L"https://api.openai.com/v1",
    L"https://api.deepseek.com/v1",
    L"https://api.anthropic.com/v1",
    L"https://api.moonshot.cn/v1",
    L"https://api.minimax.chat/v1",
    L"https://dashscope.aliyuncs.com/compatible-mode/v1"
};

const wchar_t* REG_KEY = L"Software\\AIAssistant\\Outlook";

// ==================== Helper Functions ====================
std::string WideToUtf8(const std::wstring& wstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

std::string EscapeJson(const std::string& s) {
    std::string r;
    for (char c : s) {
        switch (c) {
            case '"': r += "\\\""; break;
            case '\\': r += "\\\\"; break;
            case '\n': r += "\\n"; break;
            case '\r': r += "\\r"; break;
            default: r += c;
        }
    }
    return r;
}

std::wstring ExtractContent(const std::string& resp) {
    std::string key = "\"content\"";
    size_t pos = resp.find(key);
    if (pos == std::string::npos) return L"Error: No content";
    pos = resp.find("\"", pos + key.length() + 1);
    if (pos == std::string::npos) return L"Error: Parse failed";
    size_t end = resp.find("\"", pos + 1);
    std::string content = resp.substr(pos + 1, end - pos - 1);
    int len = MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, NULL, 0);
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, content.c_str(), -1, &result[0], len);
    return result;
}

// ==================== AI HTTP Client ====================
class AIHttpClient {
    AIServiceConfig cfg;
    HINTERNET hSess, hConn;

public:
    AIHttpClient() : hSess(NULL), hConn(NULL) {}
    ~AIHttpClient() {
        if (hConn) WinHttpCloseHandle(hConn);
        if (hSess) WinHttpCloseHandle(hSess);
    }

    void SetConfig(const AIServiceConfig& c) {
        if (hSess == NULL) {
            cfg = c;
            Init();
        }
    }

    std::wstring Translate(const std::wstring& content) {
        return Call(L"Translate this email to Chinese:", content);
    }
    std::wstring Summarize(const std::wstring& content) {
        return Call(L"Summarize key points:", content);
    }
    std::wstring GenerateReply(const std::wstring& content) {
        return Call(L"Generate a professional reply:", content);
    }
    std::wstring Polish(const std::wstring& content) {
        return Call(L"Polish and improve:", content);
    }

private:
    void Init() {
        hSess = WinHttpOpen(L"AI Assistant", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                            WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        std::wstring host = GetHost(cfg.url);
        hConn = WinHttpConnect(hSess, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    }

    std::wstring GetHost(const std::wstring& url) {
        size_t s = url.find(L"://");
        if (s == std::wstring::npos) return url;
        s += 3;
        size_t e = url.find(L"/", s);
        return (e == std::wstring::npos) ? url.substr(s) : url.substr(s, e - s);
    }

    std::wstring Call(const std::wstring& prompt, const std::wstring& content) {
        if (!hConn) return L"Error: No connection";

        std::string body = "{\"model\":\"" + WideToUtf8(cfg.model) + "\","
            "\"messages\":[{\"role\":\"system\",\"content\":\"" + EscapeJson(WideToUtf8(prompt)) + "\"},"
            "{\"role\":\"user\",\"content\":\"" + EscapeJson(WideToUtf8(content)) + "\"}],"
            "\"temperature\":0.7,\"max_tokens\":2000}";

        HINTERNET hReq = WinHttpOpenRequest(hConn, L"POST", L"/chat/completions",
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

        std::wstring hdrs = L"Content-Type: application/json\r\nAuthorization: Bearer " + cfg.apiKey + L"\r\n";
        WinHttpAddRequestHeaders(hReq, hdrs.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);

        WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                           (LPVOID)body.c_str(), body.length(), body.length(), 0);
        WinHttpReceiveResponse(hReq, NULL);

        std::string resp;
        DWORD sz;
        do {
            sz = 0;
            WinHttpQueryDataAvailable(hReq, &sz);
            if (!sz) break;
            char* buf = new char[sz + 1];
            ZeroMemory(buf, sz + 1);
            DWORD downloaded;
            if (WinHttpReadData(hReq, buf, sz, &downloaded)) resp += buf;
            delete[] buf;
        } while (sz);

        WinHttpCloseHandle(hReq);
        return ExtractContent(resp);
    }
};

// ==================== Outlook Helper ====================
class OutlookHelper {
    IDispatch* pApp;
public:
    OutlookHelper() : pApp(NULL) {}
    void Init(IDispatch* app) {
        if (pApp == NULL && app != NULL) {
            pApp = app;
            pApp->AddRef();
        }
    }
    ~OutlookHelper() { if (pApp) pApp->Release(); }

    IDispatch* GetMailItem() {
        if (!pApp) return NULL;
        DISPID did;
        OLECHAR* name = L"ActiveExplorer";
        pApp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &did);
        VARIANT res;
        DISPPARAMS dp = {NULL, NULL, 0, 0};
        pApp->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, &res, NULL, NULL);
        IDispatch* pExp = res.pdispVal;
        if (!pExp) return NULL;

        name = L"Selection";
        pExp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &did);
        pExp->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &res, NULL, NULL);
        IDispatch* pSel = res.pdispVal;
        pExp->Release();
        if (!pSel) return NULL;

        VARIANT idx; idx.vt = VT_I4; idx.lVal = 1;
        dp.cArgs = 1; dp.rgvarg = &idx;
        name = L"Item";
        pSel->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &did);
        pSel->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, &res, NULL, NULL);
        pSel->Release();
        return res.pdispVal;
    }

    std::wstring GetBody(IDispatch* mail) {
        if (!mail) return L"";
        DISPID did;
        OLECHAR* name = L"Body";
        mail->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &did);
        VARIANT res;
        DISPPARAMS dp = {NULL, NULL, 0, 0};
        mail->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, &res, NULL, NULL);
        if (res.vt == VT_BSTR) {
            std::wstring s = res.bstrVal;
            VariantClear(&res);
            return s;
        }
        return L"";
    }

    void SetBody(IDispatch* mail, const std::wstring& body) {
        if (!mail) return;
        DISPID did;
        OLECHAR* name = L"HTMLBody";
        mail->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &did);
        VARIANT v; v.vt = VT_BSTR; v.bstrVal = SysAllocString(body.c_str());
        DISPPARAMS dp = {&v, NULL, 1, 0};
        mail->Invoke(did, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
        VariantClear(&v);
    }

    void ShowTranslation(IDispatch* mail, const std::wstring& trans) {
        std::wstring orig = GetBody(mail);
        SetBody(mail, L"<div style='background:#f0f7ff;border:2px solid #0078d4;padding:12px;'>"
                  L"<b>AI Translation:</b><br>" + trans + L"</div><hr>" + orig);
    }
};

// ==================== COM Add-in Implementation ====================
static const CLSID CLSID_AIAddin = {0xa1b2c3d4,0xe5f6,0x7890,{0xab,0xcd,0xef,0x12,0x34,0x56,0x78,0x90}};

// GUID for IDTExtensibility2 (official - CORRECTED from log)
static const GUID IID_IDTExtensibility2 =
{0xB65AD801,0xABAF,0x11D0,{0xBB,0x8B,0x00,0xA0,0xC9,0x0F,0x27,0x44}};

// GUID for IRibbonExtensibility
static const GUID IID_IRibbonExtensibility =
{0x000C0396,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

class AIAddin : public IDTExtensibility2, public IRibbonExtensibility {
    ULONG ref;
    IDispatch* pApp;
    OutlookHelper* pOH;
    AIHttpClient* pAI;
    AIServiceConfig cfg;

public:
    AIAddin() : ref(1), pApp(NULL), pOH(new OutlookHelper()), pAI(new AIHttpClient()) { LoadCfg(); }
    ~AIAddin() { delete pOH; delete pAI; if (pApp) pApp->Release(); }

    // IUnknown - CRITICAL: return correct vtable offset for each interface
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        LogGuid(L"QueryInterface called for IID:", riid);

        if (riid == IID_IUnknown) {
            *ppv = static_cast<IDTExtensibility2*>(this);  // Primary vtable
            AddRef();
            LogToFile(L"  -> Returned IID_IUnknown (SUCCESS)");
            return S_OK;
        }
        if (riid == IID_IDispatch) {
            *ppv = static_cast<IDTExtensibility2*>(this);  // Same as IUnknown for primary
            AddRef();
            LogToFile(L"  -> Returned IID_IDispatch (SUCCESS)");
            return S_OK;
        }
        if (riid == IID_IDTExtensibility2) {
            *ppv = static_cast<IDTExtensibility2*>(this);
            AddRef();
            LogToFile(L"  -> Returned IID_IDTExtensibility2 (SUCCESS)");
            return S_OK;
        }
        if (riid == IID_IRibbonExtensibility) {
            // CRITICAL: must return IRibbonExtensibility subobject pointer
            // not 'this' which points to IDTExtensibility2 subobject
            *ppv = static_cast<IRibbonExtensibility*>(this);
            AddRef();
            LogToFile(L"  -> Returned IID_IRibbonExtensibility (SUCCESS)");
            return S_OK;
        }
        LogToFile(L"  -> E_NOINTERFACE (interface not supported)");
        *ppv = NULL; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return ++ref; }
    STDMETHODIMP_(ULONG) Release() { if (--ref == 0) delete this; return ref; }

    // IDispatch stubs (GetTypeInfoCount/GetTypeInfo not used)
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) { *pctinfo = 0; return S_OK; }
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) { *ppTInfo = NULL; return E_NOTIMPL; }

    // IRibbonExtensibility - returns Ribbon XML
    STDMETHODIMP GetCustomUI(BSTR RibbonID, BSTR* RibbonXml) {
        LogToFile(L"=== GetCustomUI === Outlook requesting Ribbon XML");
        if (RibbonID) {
            wchar_t buf[256];
            swprintf(buf, 256, L"  RibbonID: %s", RibbonID);
            LogToFile(buf);
        }

        const wchar_t* xml =
            L"<customUI xmlns=\"http://schemas.microsoft.com/office/2006/01/customui\">"
            L"<ribbon>"
            L"<tabs>"
            L"<tab id=\"AIAssistantTab\" label=\"AI Assistant\">"
            L"<group id=\"AIGroup\" label=\"AI Tools\">"
            L"<button id=\"btnTranslate\" label=\"Translate\" onAction=\"OnTranslate\"/>"
            L"<button id=\"btnSummary\" label=\"Summary\" onAction=\"OnSummary\"/>"
            L"<button id=\"btnReply\" label=\"Reply\" onAction=\"OnReply\"/>"
            L"<button id=\"btnPolish\" label=\"Polish\" onAction=\"OnPolish\"/>"
            L"</group>"
            L"</tab>"
            L"</tabs>"
            L"</ribbon>"
            L"</customUI>";
        *RibbonXml = SysAllocString(xml);
        LogToFile(L"  Ribbon XML allocated, returning SUCCESS");
        return S_OK;
    }

    // IDTExtensibility2
    STDMETHODIMP OnConnection(IDispatch* App, long cm, IDispatch* addin, SAFEARRAY** custom) {
        LogToFile(L"=== OnConnection === Add-in loading into Outlook");
        wchar_t buf[64];
        swprintf(buf, 64, L"  ConnectMode: %d", cm);
        LogToFile(buf);

        // Validate ConnectMode (0-3 are valid per ext_ConnectMode enum)
        if (cm < 0 || cm > 3) {
            LogToFile(L"  WARNING: Invalid ConnectMode, ignoring this call");
            return S_OK;  // Return success but don't initialize
        }

        // Defensive: only initialize once
        if (pApp == NULL && App != NULL) {
            LogToFile(L"  Initializing for first time...");
            pApp = App;
            pApp->AddRef();
            pOH->Init(App);
            pAI->SetConfig(cfg);
            LogToFile(L"  First-time initialization completed");
        } else {
            LogToFile(L"  Already initialized, skipping re-init");
        }

        LogToFile(L"  OnConnection completed SUCCESS");
        return S_OK;
    }
    STDMETHODIMP OnDisconnection(long dm, SAFEARRAY** custom) {
        LogToFile(L"=== OnDisconnection ===");
        SaveCfg();
        return S_OK;
    }
    STDMETHODIMP OnStartupComplete(SAFEARRAY** custom) {
        LogToFile(L"=== OnStartupComplete === Outlook startup finished");
        return S_OK;
    }
    STDMETHODIMP OnBeginShutdown(SAFEARRAY** custom) {
        LogToFile(L"=== OnBeginShutdown ===");
        return S_OK;
    }
    STDMETHODIMP OnAddInsUpdate(SAFEARRAY** custom) {
        LogToFile(L"=== OnAddInsUpdate ===");
        return S_OK;
    }

    // IDispatch::Invoke - handles Ribbon callbacks
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                        DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr) {
        wchar_t buf[64];
        swprintf(buf, 64, L"=== Invoke === DISPID: %d", dispIdMember);
        LogToFile(buf);

        // Ribbon button callbacks
        switch (dispIdMember) {
            case 100: LogToFile(L"  -> OnTranslate"); OnTranslate(); return S_OK;
            case 101: LogToFile(L"  -> OnSummary"); OnSummary(); return S_OK;
            case 102: LogToFile(L"  -> OnReply"); OnReply(); return S_OK;
            case 103: LogToFile(L"  -> OnPolish"); OnPolish(); return S_OK;
        }
        return E_NOTIMPL;
    }

    // IDispatch::GetIDsOfNames - maps button names to DISPID
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId) {
        LogToFile(L"=== GetIDsOfNames === Mapping method names to DISPIDs");
        if (cNames == 0) return E_INVALIDARG;
        for (UINT i = 0; i < cNames; i++) {
            wchar_t buf[256];
            swprintf(buf, 256, L"  Requesting name: %s", rgszNames[i]);
            LogToFile(buf);

            if (wcscmp(rgszNames[i], L"OnTranslate") == 0) { rgDispId[i] = 100; LogToFile(L"    -> mapped to DISPID 100"); }
            else if (wcscmp(rgszNames[i], L"OnSummary") == 0) { rgDispId[i] = 101; LogToFile(L"    -> mapped to DISPID 101"); }
            else if (wcscmp(rgszNames[i], L"OnReply") == 0) { rgDispId[i] = 102; LogToFile(L"    -> mapped to DISPID 102"); }
            else if (wcscmp(rgszNames[i], L"OnPolish") == 0) rgDispId[i] = 103;
            else rgDispId[i] = DISPID_UNKNOWN;
        }
        return S_OK;
    }

    // Button handlers (called from Invoke)
    void OnTranslate() {
        IDispatch* mail = pOH->GetMailItem();
        if (!mail) { MessageBoxW(NULL, L"Select an email first", L"AI Assistant", MB_OK); return; }
        std::wstring content = pOH->GetBody(mail);
        std::wstring trans = pAI->Translate(content);
        pOH->ShowTranslation(mail, trans);
        mail->Release();
    }
    void OnSummary() {
        IDispatch* mail = pOH->GetMailItem();
        if (!mail) return;
        std::wstring s = pAI->Summarize(pOH->GetBody(mail));
        MessageBoxW(NULL, s.c_str(), L"Summary", MB_OK);
        mail->Release();
    }
    void OnReply() {
        IDispatch* mail = pOH->GetMailItem();
        if (!mail) return;
        std::wstring r = pAI->GenerateReply(pOH->GetBody(mail));
        MessageBoxW(NULL, r.c_str(), L"Suggested Reply", MB_OK);
        mail->Release();
    }
    void OnPolish() {
        IDispatch* mail = pOH->GetMailItem();
        if (!mail) return;
        std::wstring p = pAI->Polish(pOH->GetBody(mail));
        MessageBoxW(NULL, p.c_str(), L"Polished", MB_OK);
        mail->Release();
    }

private:
    void LoadCfg() {
        HKEY hk;
        wchar_t buf[1024]; DWORD sz;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hk) == ERROR_SUCCESS) {
            sz = sizeof(buf);
            if (RegQueryValueExW(hk, L"URL", 0, NULL, (LPBYTE)buf, &sz) == ERROR_SUCCESS) cfg.url = buf;
            sz = sizeof(buf);
            if (RegQueryValueExW(hk, L"Model", 0, NULL, (LPBYTE)buf, &sz) == ERROR_SUCCESS) cfg.model = buf;
            sz = sizeof(buf);
            if (RegQueryValueExW(hk, L"ApiKey", 0, NULL, (LPBYTE)buf, &sz) == ERROR_SUCCESS) cfg.apiKey = buf;
            RegCloseKey(hk);
        } else {
            cfg.url = AI_URLS[0]; cfg.model = L"gpt-4o"; cfg.apiKey = L"";
        }
    }
    void SaveCfg() {
        HKEY hk;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL) == ERROR_SUCCESS) {
            RegSetValueExW(hk, L"URL", 0, REG_SZ, (LPBYTE)cfg.url.c_str(), (cfg.url.length()+1)*2);
            RegSetValueExW(hk, L"Model", 0, REG_SZ, (LPBYTE)cfg.model.c_str(), (cfg.model.length()+1)*2);
            RegSetValueExW(hk, L"ApiKey", 0, REG_SZ, (LPBYTE)cfg.apiKey.c_str(), (cfg.apiKey.length()+1)*2);
            RegCloseKey(hk);
        }
    }
};

// ==================== Class Factory ====================
class AIFactory : public IClassFactory {
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) { *ppv = this; return S_OK; }
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** ppv) {
        LogToFile(L"=== ClassFactory::CreateInstance === Creating AIAddin object");
        if (outer) {
            LogToFile(L"  FAILED: CLASS_E_NOAGGREGATION (outer != NULL)");
            return CLASS_E_NOAGGREGATION;
        }
        LogToFile(L"  Creating new AIAddin instance...");
        AIAddin* p = new AIAddin();
        HRESULT hr = p->QueryInterface(riid, ppv);
        wchar_t buf[64];
        swprintf(buf, 64, L"  QueryInterface result: 0x%08X", hr);
        LogToFile(buf);
        return hr;
    }
    STDMETHODIMP LockServer(BOOL lock) { return S_OK; }
};

// ==================== DLL Exports ====================
HINSTANCE gInst;

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv) {
    LogToFile(L"=== DllGetClassObject === Outlook requesting Class Factory");
    LogGuid(L"  Requested CLSID:", clsid);
    LogGuid(L"  Requested IID:", riid);

    if (clsid == CLSID_AIAddin) {
        LogToFile(L"  CLSID matches! Creating AIFactory...");
        AIFactory* f = new AIFactory();
        HRESULT hr = f->QueryInterface(riid, ppv);
        wchar_t buf[64];
        swprintf(buf, 64, L"  AIFactory::QueryInterface result: 0x%08X", hr);
        LogToFile(buf);
        return hr;
    }
    LogToFile(L"  CLSID mismatch -> CLASS_E_CLASSNOTAVAILABLE");
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow() { return S_OK; }

STDAPI DllRegisterServer() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(gInst, path, MAX_PATH);

    HKEY hk;
    std::wstring key = L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}";
    RegCreateKeyExW(HKEY_CLASSES_ROOT, key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
    RegSetValueExW(hk, NULL, 0, REG_SZ, (LPBYTE)L"AI Assistant", 24);
    RegCloseKey(hk);

    RegCreateKeyExW(HKEY_CLASSES_ROOT, (key+L"\\InprocServer32").c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
    RegSetValueExW(hk, NULL, 0, REG_SZ, (LPBYTE)path, (wcslen(path)+1)*2);
    RegSetValueExW(hk, L"ThreadingModel", 0, REG_SZ, (LPBYTE)L"Apartment", 18);
    RegCloseKey(hk);

    RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
    DWORD lb = 3;
    RegSetValueExW(hk, L"LoadBehavior", 0, REG_DWORD, (LPBYTE)&lb, 4);
    RegCloseKey(hk);

    return S_OK;
}

STDAPI DllUnregisterServer() {
    RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Office\\Outlook\\Addins\\AIAssistant");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}\\InprocServer32");
    RegDeleteKeyW(HKEY_CLASSES_ROOT, L"CLSID\\{a1b2c3d4-e5f6-7890-abcd-ef1234567890}");
    return S_OK;
}

BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        gInst = (HINSTANCE)h;
        LogToFile(L"=== DLL_PROCESS_ATTACH === DLL loaded by Outlook");
        MessageBoxW(NULL, L"AIAssistant DLL loaded!", L"AI Assistant Debug", MB_OK);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        LogToFile(L"=== DLL_PROCESS_DETACH === DLL unloaded");
    }
    return TRUE;
}