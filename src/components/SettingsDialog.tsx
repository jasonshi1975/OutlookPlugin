import React, { useState, useEffect } from 'react';
import { UserConfig, AI_SERVICES, AIServiceType } from '../config/AIConfig';

interface SettingsDialogProps {
  config: UserConfig;
  onSave: (config: UserConfig) => void;
  onCancel: () => void;
}

function SettingsDialog({ config, onSave, onCancel }: SettingsDialogProps) {
  const [formData, setFormData] = useState<UserConfig>(config);

  const handleServiceChange = (service: AIServiceType) => {
    const serviceConfig = AI_SERVICES[service];
    setFormData({
      ...formData,
      aiService: service,
      apiUrl: serviceConfig.url,
      model: serviceConfig.models[0],
    });
  };

  return (
    <div className="dialog-overlay">
      <div className="dialog settings-dialog">
        <div className="dialog-header">
          <h2>Settings</h2>
          <button className="close-btn" onClick={onCancel}>×</button>
        </div>

        <div className="dialog-body">
          <div className="form-section">
            <label>AI Service</label>
            <select
              value={formData.aiService}
              onChange={(e) => handleServiceChange(e.target.value as AIServiceType)}
            >
              {Object.entries(AI_SERVICES).map(([key, svc]) => (
                <option key={key} value={key}>{svc.name}</option>
              ))}
            </select>
          </div>

          <div className="form-section">
            <label>API URL</label>
            <input
              type="text"
              value={formData.apiUrl}
              onChange={(e) => setFormData({ ...formData, apiUrl: e.target.value })}
              placeholder="API endpoint URL"
            />
          </div>

          <div className="form-section">
            <label>Model</label>
            <select
              value={formData.model}
              onChange={(e) => setFormData({ ...formData, model: e.target.value })}
            >
              {AI_SERVICES[formData.aiService].models.map((model) => (
                <option key={model} value={model}>{model}</option>
              ))}
            </select>
          </div>

          <div className="form-section">
            <label>API Key</label>
            <input
              type="password"
              value={formData.apiKey}
              onChange={(e) => setFormData({ ...formData, apiKey: e.target.value })}
              placeholder="Enter your API key"
            />
          </div>

          <div className="form-section">
            <label>Target Language (Translation)</label>
            <select
              value={formData.targetLanguage}
              onChange={(e) => setFormData({ ...formData, targetLanguage: e.target.value })}
            >
              <option value="Chinese">Chinese</option>
              <option value="English">English</option>
              <option value="Japanese">Japanese</option>
              <option value="German">German</option>
              <option value="French">French</option>
            </select>
          </div>

          <div className="form-section">
            <label>Reply Tone</label>
            <select
              value={formData.replyTone}
              onChange={(e) => setFormData({ ...formData, replyTone: e.target.value })}
            >
              <option value="Formal">Formal Business</option>
              <option value="Friendly">Friendly</option>
              <option value="Concise">Concise</option>
            </select>
          </div>
        </div>

        <div className="dialog-actions">
          <button className="btn-cancel" onClick={onCancel}>Cancel</button>
          <button className="btn-save" onClick={() => onSave(formData)}>Save</button>
        </div>
      </div>
    </div>
  );
}

export default SettingsDialog;