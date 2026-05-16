import React, { useState } from 'react';
import AIService from '../api/AIService';

interface SummaryDialogProps {
  emailContent: string;
  aiService: AIService;
  onClose: () => void;
}

function SummaryDialog({ emailContent, aiService, onClose }: SummaryDialogProps) {
  const [style, setStyle] = useState('bullet-points');
  const [result, setResult] = useState('');
  const [loading, setLoading] = useState(false);

  const handleGenerate = async () => {
    setLoading(true);
    try {
      const summary = await aiService.summarize(emailContent, style);
      setResult(summary);
    } catch (err) {
      setResult(`Error: ${err.message}`);
    }
    setLoading(false);
  };

  return (
    <div className="dialog-overlay">
      <div className="dialog large-dialog">
        <div className="dialog-header">
          <h2>Email Summary</h2>
          <button className="close-btn" onClick={onClose}>×</button>
        </div>

        <div className="dialog-body two-column">
          <div className="left-panel">
            <h3>Original Email</h3>
            <div className="content-box">{emailContent}</div>
          </div>

          <div className="right-panel">
            <div className="options">
              <label>Summary Style</label>
              <select value={style} onChange={(e) => setStyle(e.target.value)}>
                <option value="bullet-points">Bullet Points</option>
                <option value="paragraph">Paragraph</option>
                <option value="key-info">Key Information</option>
              </select>
            </div>

            <div className="result-box">
              {loading ? <div className="spinner">Processing...</div> : result}
            </div>
          </div>
        </div>

        <div className="dialog-actions">
          <button className="btn-cancel" onClick={onClose}>Close</button>
          <button className="btn-regenerate" onClick={handleGenerate} disabled={loading}>
            Regenerate
          </button>
          <button className="btn-apply" onClick={() => navigator.clipboard.writeText(result)}>
            Copy Summary
          </button>
        </div>
      </div>
    </div>
  );
}

export default SummaryDialog;