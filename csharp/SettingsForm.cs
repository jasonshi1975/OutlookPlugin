using System;
using System.Windows.Forms;

namespace AIAssistant;

public class SettingsForm : Form
{
    private ComboBox cmbService;
    private TextBox txtUrl;
    private TextBox txtModel;
    private TextBox txtApiKey;
    private Button btnOk;
    private Button btnCancel;

    public AIConfig Config { get; private set; }

    public SettingsForm(AIConfig initialConfig)
    {
        Config = initialConfig;
        InitializeComponent();
        LoadConfig();
    }

    private void InitializeComponent()
    {
        this.Text = "AI Assistant Settings";
        this.Width = 400;
        this.Height = 250;
        this.FormBorderStyle = FormBorderStyle.FixedDialog;
        this.StartPosition = FormStartPosition.CenterScreen;

        // Service dropdown
        cmbService = new ComboBox { Left = 120, Top = 20, Width = 250 };
        cmbService.Items.AddRange(new object[] { "OpenAI", "DeepSeek", "Claude", "Kimi", "MiniMax", "Qianwen" });
        cmbService.DropDownStyle = ComboBoxStyle.DropDownList;
        cmbService.SelectedIndexChanged += CmbService_SelectedIndexChanged;

        var lblService = new Label { Text = "Service:", Left = 20, Top = 20, Width = 100 };

        // URL
        txtUrl = new TextBox { Left = 120, Top = 50, Width = 250 };
        var lblUrl = new Label { Text = "API URL:", Left = 20, Top = 50, Width = 100 };

        // Model
        txtModel = new TextBox { Left = 120, Top = 80, Width = 250 };
        var lblModel = new Label { Text = "Model:", Left = 20, Top = 80, Width = 100 };

        // API Key
        txtApiKey = new TextBox { Left = 120, Top = 110, Width = 250 };
        txtApiKey.UseSystemPasswordChar = true;
        var lblKey = new Label { Text = "API Key:", Left = 20, Top = 110, Width = 100 };

        // Buttons
        btnOk = new Button { Text = "OK", Left = 120, Top = 150, Width = 80 };
        btnOk.Click += BtnOk_Click;
        btnOk.DialogResult = DialogResult.OK;

        btnCancel = new Button { Text = "Cancel", Left = 220, Top = 150, Width = 80 };
        btnCancel.Click += (s, e) => this.Close();
        btnCancel.DialogResult = DialogResult.Cancel;

        this.Controls.AddRange(new Control[] {
            lblService, cmbService,
            lblUrl, txtUrl,
            lblModel, txtModel,
            lblKey, txtApiKey,
            btnOk, btnCancel
        });

        this.AcceptButton = btnOk;
        this.CancelButton = btnCancel;
    }

    private void LoadConfig()
    {
        cmbService.SelectedItem = Config.Service;
        txtUrl.Text = Config.Url;
        txtModel.Text = Config.Model;
        txtApiKey.Text = Config.ApiKey;
    }

    private void CmbService_SelectedIndexChanged(object sender, EventArgs e)
    {
        var service = cmbService.SelectedItem?.ToString() ?? "OpenAI";
        txtUrl.Text = GetDefaultUrl(service);
        txtModel.Text = GetDefaultModel(service);
    }

    private string GetDefaultUrl(string service)
    {
        return service switch
        {
            "OpenAI" => "https://api.openai.com/v1",
            "DeepSeek" => "https://api.deepseek.com/v1",
            "Claude" => "https://api.anthropic.com/v1",
            "Kimi" => "https://api.moonshot.cn/v1",
            "MiniMax" => "https://api.minimax.chat/v1",
            "Qianwen" => "https://dashscope.aliyuncs.com/compatible-mode/v1",
            _ => "https://api.openai.com/v1"
        };
    }

    private string GetDefaultModel(string service)
    {
        return service switch
        {
            "OpenAI" => "gpt-4o",
            "DeepSeek" => "deepseek-chat",
            "Claude" => "claude-3-5-sonnet-20241022",
            "Kimi" => "moonshot-v1-8k",
            "MiniMax" => "abab6.5-chat",
            "Qianwen" => "qwen-max",
            _ => "gpt-4o"
        };
    }

    private void BtnOk_Click(object sender, EventArgs e)
    {
        Config.Service = cmbService.SelectedItem?.ToString() ?? "OpenAI";
        Config.Url = txtUrl.Text;
        Config.Model = txtModel.Text;
        Config.ApiKey = txtApiKey.Text;
        this.Close();
    }
}