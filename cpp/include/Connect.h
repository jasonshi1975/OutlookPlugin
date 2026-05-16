#pragma once

#include <atlbase.h>
#include <atlcom.h>
#include <string>

// COM Add-in Connect class - implements IDTExtensibility2
class ATL_NO_VTABLE CConnect :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CConnect, &CLSID_Connect>,
    public IDispatchImpl<IConnect, &IID_IConnect, &LIBID_AIAssistantLib, 1, 0>,
    public IDTExtensibility2
{
public:
    CConnect();
    ~CConnect();

    DECLARE_REGISTRY_RESOURCEID(IDR_CONNECT)
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CConnect)
        COM_INTERFACE_ENTRY(IConnect)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IDTExtensibility2)
    END_COM_MAP()

    // IDTExtensibility2 Methods
    STDMETHOD(OnConnection)(IDispatch* pApplication, ext_ConnectMode ConnectMode,
                            IDispatch* pAddInInst, SAFEARRAY** custom);

    STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY** custom);

    STDMETHOD(OnStartupComplete)(SAFEARRAY** custom);

    STDMETHOD(OnBeginShutdown)(SAFEARRAY** custom);

    STDMETHOD(OnAddInsUpdate)(SAFEARRAY** custom);

    // Button click handlers (called from Ribbon)
    STDMETHOD(OnSettingsClick)(IDispatch* pDisp);
    STDMETHOD(OnTranslateClick)(IDispatch* pDisp);
    STDMETHOD(OnSummaryClick)(IDispatch* pDisp);
    STDMETHOD(OnReplyClick)(IDispatch* pDisp);
    STDMETHOD(OnPolishClick)(IDispatch* pDisp);

private:
    CComPtr<IDispatch> m_pApplication;  // Outlook Application
    CComPtr<IDispatch> m_pAddInInstance;

    // Helper objects
    OutlookHelper* m_pOutlookHelper;
    IAIHttpClient* m_pAIHttpClient;

    // Config storage
    AIServiceConfig m_config;

    // Load/Save config
    void LoadConfig();
    void SaveConfig();

    // Show dialog (90% screen)
    void ShowDialog(const std::wstring& dialogType);
};