#include "stdafx.h"
#include "AIHttpClient.cpp"

// Outlook Helper - direct COM calls without ATL
class OutlookHelper {
public:
    OutlookHelper() : pApp(NULL) {}
    ~OutlookHelper() { if (pApp) pApp->Release(); }

    void Initialize(IDispatch* application) {
        pApp = application;
        if (pApp) pApp->AddRef();
    }

    IDispatch* GetCurrentMailItem() {
        if (!pApp) return NULL;

        // Get ActiveExplorer
        DISPID dispid;
        OLECHAR* name = L"ActiveExplorer";
        pApp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);

        VARIANT result;
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        pApp->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                      DISPATCH_METHOD, &params, &result, NULL, NULL);

        IDispatch* pExplorer = result.pdispVal;
        if (!pExplorer) return NULL;

        // Get Selection
        name = L"Selection";
        pExplorer->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
        pExplorer->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                           DISPATCH_PROPERTYGET, &params, &result, NULL, NULL);

        IDispatch* pSelection = result.pdispVal;
        pExplorer->Release();

        if (!pSelection) return NULL;

        // Get first item
        VARIANT index;
        index.vt = VT_I4;
        index.lVal = 1;
        params.cArgs = 1;
        params.rgvarg = &index;

        name = L"Item";
        pSelection->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
        pSelection->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                           DISPATCH_METHOD, &params, &result, NULL, NULL);

        pSelection->Release();
        return result.pdispVal;
    }

    std::wstring GetMailBody(IDispatch* pMail) {
        if (!pMail) return L"";

        DISPID dispid;
        OLECHAR* name = L"Body";
        pMail->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);

        VARIANT result;
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        pMail->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                       DISPATCH_PROPERTYGET, &params, &result, NULL, NULL);

        if (result.vt == VT_BSTR) {
            std::wstring body = result.bstrVal;
            VariantClear(&result);
            return body;
        }
        return L"";
    }

    void SetMailBody(IDispatch* pMail, const std::wstring& body) {
        if (!pMail) return;

        DISPID dispid;
        OLECHAR* name = L"HTMLBody";
        pMail->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);

        VARIANT value;
        value.vt = VT_BSTR;
        value.bstrVal = SysAllocString(body.c_str());

        DISPPARAMS params = { &value, NULL, 1, 0 };
        pMail->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                       DISPATCH_PROPERTYPUT, &params, NULL, NULL, NULL);

        VariantClear(&value);
    }

    void DisplayReply(IDispatch* pMail, const std::wstring& replyContent) {
        if (!pMail) return;

        // Call Reply method
        DISPID dispid;
        OLECHAR* name = L"Reply";
        pMail->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);

        VARIANT result;
        DISPPARAMS params = { NULL, NULL, 0, 0 };
        pMail->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                       DISPATCH_METHOD, &params, &result, NULL, NULL);

        IDispatch* pReply = result.pdispVal;
        if (pReply) {
            SetMailBody(pReply, replyContent);

            // Display
            name = L"Display";
            pReply->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
            pReply->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT,
                            DISPATCH_METHOD, &params, NULL, NULL, NULL);
            pReply->Release();
        }
    }

    void ShowInlineTranslation(IDispatch* pMail, const std::wstring& translated) {
        // Insert overlay div into HTML body
        std::wstring originalBody = GetMailBody(pMail);
        std::wstring overlay =
            L"<div style='background:#f0f7ff;border:2px solid #0078d4;padding:16px;'>"
            L"<b>AI Translation:</b><br>" + translated + L"</div><hr>";
        SetMailBody(pMail, overlay + originalBody);
    }

private:
    IDispatch* pApp;
};