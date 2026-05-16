import React, { useState } from 'react';
import AIService from '../api/AIService';

interface PolishDialogProps {
  emailContent: string;
  aiService: AIService;
  onClose: () => void;
}

function PolishDialog({ emailContent, aiService, onClose }: PolishDialogProps) {
  const [style, setStyle] = useState('formal');
  const [language, setLanguage] = useState('English');
  const [result, setResult] = useState('');
  const [loading, setLoading] = useState(false);

  const handlePolish = async () => {
    setLoading(true);
    try {
      const polished = await aiService.polish(emailContent, style, language);
      setResult(polished);
    } catch (err) {
      setResult(`Error: ${err.message}`);
    }
    setLoading(false);
  };

  return (
    <div className="dialog-overlay">
      <div className="dialog large-dialog">
        <div className="dialog-header">
          <h2>Polish Email</h2>
          <button className="close-btn" onClick={onClose}>×</button>
        </div>

        <div className="dialog-body two-column">
          <div className="left-panel">
            <h3>Current Draft</h3>
            <div className="content-box draft">{emailContent}</div>
          </div>

          <div className="right-panel">
            <div className="options">
              <label>Polish Style</label>
              <select value={style} onChange={(e) => setStyle(e.target.value)}>
                <option value="formal">Formal Business</option>
                <option value="friendly">Friendly Professional</option>
                <option value="concise">Concise Clear</option>
              </select>

              <label>Language</label>
              <select value={language} onChange={(e) => setLanguage(e.target.value)}>
                <option value="English">English</option>
                <option value="Chinese">Chinese</option>
              </select>
            </div>

            <div className="result-box polished">
              {loading ? <div className="spinner">Polishing...</div> : result}
            </div>
          </div>
        </div>

        <div className="dialog-actions">
          <button className="btn-cancel" onClick={onClose}>Cancel</button>
          <button className="btn-regenerate" onClick={handlePolish} disabled={loading}>
            Polish Again
          </button>
          <button className="btn-apply" onClick={() => navigator.clipboard.writeText(result)}>
            Copy Result
          </button>
        </div>
      </div>
    </div>
  );
}

export default PolishDialog;