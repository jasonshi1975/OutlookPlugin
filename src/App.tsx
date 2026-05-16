import React, { useState, useEffect } from 'react';
import ConfigStorage from './config/ConfigStorage';
import AIService from './api/AIService';
import { UserConfig, AI_SERVICES, AIServiceType } from './config/AIConfig';
import SettingsDialog from './components/SettingsDialog';
import SummaryDialog from './components/SummaryDialog';
import ReplyDialog from './components/ReplyDialog';
import PolishDialog from './components/PolishDialog';
import './App.css';

type DialogType = 'none' | 'settings' | 'summary' | 'reply' | 'polish';

function App() {
  const [config, setConfig] = useState<UserConfig | null>(null);
  const [dialog, setDialog] = useState<DialogType>('none');
  const [loading, setLoading] = useState(true);
  const [emailContent, setEmailContent] = useState('');

  useEffect(() => {
    async function init() {
      const savedConfig = await ConfigStorage.initialize();
      setConfig(savedConfig);
      setLoading(false);

      // Get current email content
      if (Office.context.mailbox.item) {
        Office.context.mailbox.item.body.getAsync(
          Office.CoercionType.Text,
          (result) => {
            if (result.status === Office.AsyncResultStatus.Succeeded) {
              setEmailContent(result.value);
            }
          }
        );
      }
    }
    init();
  }, []);

  const handleSaveConfig = async (newConfig: UserConfig) => {
    await ConfigStorage.save(newConfig);
    setConfig(newConfig);
    setDialog('none');
  };

  if (loading) {
    return <div className="loading">Loading...</div>;
  }

  if (!config?.apiKey) {
    return <SettingsDialog config={config || DEFAULT_CONFIG} onSave={handleSaveConfig} onCancel={() => setDialog('none')} />;
  }

  const aiService = new AIService(config);

  return (
    <div className="app">
      {dialog === 'none' && (
        <div className="toolbar">
          <button onClick={() => setDialog('settings')}>Settings</button>
          <button onClick={() => setDialog('summary')}>Summary</button>
          <button onClick={() => setDialog('reply')}>Reply</button>
          <button onClick={() => setDialog('polish')}>Polish</button>
        </div>
      )}

      {dialog === 'settings' && (
        <SettingsDialog config={config} onSave={handleSaveConfig} onCancel={() => setDialog('none')} />
      )}

      {dialog === 'summary' && (
        <SummaryDialog
          emailContent={emailContent}
          aiService={aiService}
          onClose={() => setDialog('none')}
        />
      )}

      {dialog === 'reply' && (
        <ReplyDialog
          emailContent={emailContent}
          aiService={aiService}
          onClose={() => setDialog('none')}
        />
      )}

      {dialog === 'polish' && (
        <PolishDialog
          emailContent={emailContent}
          aiService={aiService}
          onClose={() => setDialog('none')}
        />
      )}
    </div>
  );
}

export default App;