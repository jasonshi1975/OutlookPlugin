#pragma once

#include <atlbase.h>
#include <string>
#import "C:\Program Files\Common Files\Microsoft Shared\OFFICE16\MSOUTL.OLB" raw_interfaces_only, named_guids

// Outlook Helper - wraps Outlook Object Model
class OutlookHelper {
public:
    OutlookHelper();
    ~OutlookHelper();

    // Initialize with Outlook Application
    void Initialize(IDispatch* pApplication);

    // Get current mail item
    CComPtr<IDispatch> GetCurrentMailItem();

    // Get mail body (text)
    std::wstring GetMailBodyText(IDispatch* pMailItem);

    // Get mail subject
    std::wstring GetMailSubject(IDispatch* pMailItem);

    // Set mail body (for inline translation)
    void SetMailBodyHTML(IDispatch* pMailItem, const std::wstring& htmlBody);

    // Display reply form
    void DisplayReplyForm(IDispatch* pMailItem, const std::wstring& replyContent);
    void DisplayReplyAllForm(IDispatch* pMailItem, const std::wstring& replyContent);

    // Show inline translation overlay (insert HTML)
    void ShowInlineTranslation(IDispatch* pMailItem, const std::wstring& translatedContent);
    void RemoveInlineTranslation(IDispatch* pMailItem);

private:
    CComPtr<IDispatch> m_pApplication;
    CComPtr<IDispatch> m_pExplorer;
    CComPtr<IDispatch> m_pCurrentItem;

    // Helper to call Outlook method
    HRESULT InvokeMethod(IDispatch* pDisp, DISPID dispid, VARIANT* pResult,
                         VARIANT* pParams = nullptr, UINT cParams = 0);

    // Helper to get property
    HRESULT GetProperty(IDispatch* pDisp, DISPID dispid, VARIANT* pResult);

    // Helper to set property
    HRESULT SetProperty(IDispatch* pDisp, DISPID dispid, const VARIANT& value);
};