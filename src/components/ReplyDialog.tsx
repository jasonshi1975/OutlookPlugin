import React, { useState } from 'react';
import AIService from '../api/AIService';

interface ReplyDialogProps {
  emailContent: string;
  aiService: AIService;
  onClose: () => void;
}

function ReplyDialog({ emailContent, aiService, onClose }: ReplyDialogProps) {
  const [tone, setTone] = useState('Formal');
  const [replyType, setReplyType] = useState('reply-all');
  const [extraInstructions, setExtraInstructions] = useState('');
  const [result, setResult] = useState('');
  const [loading, setLoading] = useState(false);

  const handleGenerate = async () => {
    setLoading(true);
    try {
      const reply = await aiService.generateReply(emailContent, tone, extraInstructions);
      setResult(reply);
    } catch (err) {
      setResult(`Error: ${err.message}`);
    }
    setLoading(false);
  };

  const handleApply = () => {
    if (replyType === 'reply-all') {
      Office.context.mailbox.item.displayReplyAllFormAsync(result);
    } else {
      Office.context.mailbox.item.displayReplyFormAsync(result);
    }
    onClose();
  };

  return (
    <div className="dialog-overlay">
      <div className="dialog large-dialog">
        <div className="dialog-header">
          <h2>Generate Reply</h2>
          <button className="close-btn" onClick={onClose}>×</button>
        </div>

        <div className="dialog-body two-column">
          <div className="left-panel">
            <h3>Original Email</h3>
            <div className="content-box">{emailContent}</div>
          </div>

          <div className="right-panel">
            <div className="options">
              <label>Reply Tone</label>
              <select value={tone} onChange={(e) => setTone(e.target.value)}>
                <option value="Formal">Formal Business</option>
                <option value="Friendly">Friendly</option>
                <option value="Concise">Concise</option>
              </select>

              <label>Reply Type</label>
              <select value={replyType} onChange={(e) => setReplyType(e.target.value)}>
                <option value="reply-all">Reply All</option>
                <option value="reply">Reply to Sender Only</option>
              </select>

              <label>Extra Instructions</label>
              <input
                type="text"
                value={extraInstructions}
                onChange={(e) => setExtraInstructions(e.target.value)}
                placeholder="e.g., Emphasize my design suggestions..."
              />
            </div>

            <div className="result-box">
              {loading ? <div className="spinner">Processing...</div> : result}
            </div>
          </div>
        </div>

        <div className="dialog-actions">
          <button className="btn-cancel" onClick={onClose}>Cancel</button>
          <button className="btn-regenerate" onClick={handleGenerate} disabled={loading}>
            Regenerate
          </button>
          <button className="btn-apply" onClick={handleApply} disabled={!result}>
            Apply Reply
          </button>
        </div>
      </div>
    </div>
  );
}

export default ReplyDialog;