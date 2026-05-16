using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Extensibility;
using Microsoft.Office.Interop.Outlook;
using Application = Microsoft.Office.Interop.Outlook.Application;

namespace AIAssistant;

// COM GUIDs
[Guid("a1b2c3d4-e5f6-7890-abcd-ef1234567890")]
[ClassInterface(ClassInterfaceType.AutoDual)]
[ComVisible(true)]
public class AIAddin : IDTExtensibility2, IRibbonExtensibility
{
    private Application? outlookApp;
    private AIHttpClient? httpClient;
    private AIConfig config;
    private string logPath = Path.Combine(Environment.GetEnvironmentVariable("TEMP") ?? "C:\\Temp", "AIAssistantCS_log.txt");

    public AIAddin()
    {
        config = AIConfig.Load();
        httpClient = new AIHttpClient(config);
        Log("=== AIAddin constructor ===");
    }

    #region Logging
    private void Log(string msg)
    {
        try
        {
            using var sw = new StreamWriter(logPath, true);
            sw.WriteLine($"[{DateTime.Now:HH:mm:ss}] {msg}");
        }
        catch { }
    }
    #endregion

    #region IDTExtensibility2
    public void OnConnection(object Application, ext_ConnectMode ConnectMode, object AddInInst, ref Array custom)
    {
        Log($"=== OnConnection === ConnectMode: {ConnectMode}");
        outlookApp = Application as Application;
        Log("  Outlook Application acquired");
    }

    public void OnDisconnection(ext_DisconnectMode RemoveMode, ref Array custom)
    {
        Log("=== OnDisconnection ===");
        config.Save();
    }

    public void OnStartupComplete(ref Array custom)
    {
        Log("=== OnStartupComplete === Outlook ready");
    }

    public void OnBeginShutdown(ref Array custom)
    {
        Log("=== OnBeginShutdown ===");
    }

    public void OnAddInsUpdate(ref Array custom)
    {
        Log("=== OnAddInsUpdate ===");
    }
    #endregion

    #region IRibbonExtensibility
    public string GetCustomUI(string RibbonID)
    {
        Log($"=== GetCustomUI === RibbonID: {RibbonID}");

        if (RibbonID == "Microsoft.Outlook.Explorer" || RibbonID == "Microsoft.Outlook.Mail.Compose")
        {
            string xml = @"
<customUI xmlns='http://schemas.microsoft.com/office/2006/01/customui'>
  <ribbon>
    <tabs>
      <tab id='AIAssistantTab' label='AI Assistant'>
        <group id='AIGroup' label='AI Tools'>
          <button id='btnTranslate' label='Translate' onAction='OnTranslate' size='large'/>
          <button id='btnSummary' label='Summary' onAction='OnSummary' size='large'/>
          <button id='btnReply' label='Reply' onAction='OnReply' size='large'/>
          <button id='btnPolish' label='Polish' onAction='OnPolish' size='large'/>
          <button id='btnSettings' label='Settings' onAction='OnSettings' size='large'/>
        </group>
      </tab>
    </tabs>
  </ribbon>
</customUI>";
            Log("  Returning Ribbon XML");
            return xml;
        }

        Log("  No Ribbon for this context");
        return "";
    }
    #endregion

    #region Ribbon Callbacks
    public void OnTranslate(IRibbonControl control)
    {
        Log("=== OnTranslate ===");
        try
        {
            var mail = GetCurrentMailItem();
            if (mail == null)
            {
                MessageBox.Show("Please select an email first.", "AI Assistant");
                return;
            }

            string content = mail.Body ?? mail.HTMLBody ?? "";
            string translated = httpClient?.Translate(content) ?? "Error: No connection";

            mail.HTMLBody = $@"
<div style='background:#f0f7ff;border:2px solid #0078d4;padding:12px;'>
<b>AI Translation:</b><br>{translated}
</div><hr>{mail.HTMLBody}";

            Log("  Translation applied");
        }
        catch (System.Exception ex)
        {
            Log($"  ERROR: {ex.Message}");
            MessageBox.Show($"Error: {ex.Message}", "AI Assistant");
        }
    }

    public void OnSummary(IRibbonControl control)
    {
        Log("=== OnSummary ===");
        try
        {
            var mail = GetCurrentMailItem();
            if (mail == null) return;

            string content = mail.Body ?? "";
            string summary = httpClient?.Summarize(content) ?? "Error";
            MessageBox.Show(summary, "Summary", MessageBoxButtons.OK);
        }
        catch (System.Exception ex)
        {
            Log($"  ERROR: {ex.Message}");
        }
    }

    public void OnReply(IRibbonControl control)
    {
        Log("=== OnReply ===");
        try
        {
            var mail = GetCurrentMailItem();
            if (mail == null) return;

            string content = mail.Body ?? "";
            string reply = httpClient?.GenerateReply(content) ?? "Error";

            var replyMail = mail.Reply();
            replyMail.Body = reply;
            replyMail.Display();
        }
        catch (System.Exception ex)
        {
            Log($"  ERROR: {ex.Message}");
        }
    }

    public void OnPolish(IRibbonControl control)
    {
        Log("=== OnPolish ===");
        try
        {
            var mail = GetCurrentMailItem();
            if (mail == null) return;

            string content = mail.Body ?? "";
            string polished = httpClient?.Polish(content) ?? "Error";
            MessageBox.Show(polished, "Polished", MessageBoxButtons.OK);
        }
        catch (System.Exception ex)
        {
            Log($"  ERROR: {ex.Message}");
        }
    }

    public void OnSettings(IRibbonControl control)
    {
        Log("=== OnSettings ===");
        using var form = new SettingsForm(config);
        if (form.ShowDialog() == DialogResult.OK)
        {
            config = form.Config;
            config.Save();
            httpClient = new AIHttpClient(config);
            Log("  Settings updated");
        }
    }
    #endregion

    #region Helper
    private MailItem? GetCurrentMailItem()
    {
        if (outlookApp == null) return null;

        try
        {
            var explorer = outlookApp.ActiveExplorer();
            if (explorer == null) return null;

            var selection = explorer.Selection;
            if (selection.Count == 0) return null;

            return selection[1] as MailItem;
        }
        catch (System.Exception ex)
        {
            Log($"GetCurrentMailItem ERROR: {ex.Message}");
            return null;
        }
    }
    #endregion
}