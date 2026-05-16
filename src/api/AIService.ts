import type { AIServiceType } from '../config/AIConfig';
import { AI_SERVICES, UserConfig } from '../config/AIConfig';

interface ChatMessage {
  role: 'system' | 'user' | 'assistant';
  content: string;
}

interface ChatResponse {
  choices: Array<{
    message: {
      content: string;
    };
  }>;
}

class AIService {
  private config: UserConfig;

  constructor(config: UserConfig) {
    this.config = config;
  }

  updateConfig(config: UserConfig): void {
    this.config = config;
  }

  private getHeaders(): Record<string, string> {
    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
    };

    if (this.config.aiService === 'claude') {
      headers['x-api-key'] = this.config.apiKey;
      headers['anthropic-version'] = '2023-06-01';
    } else {
      headers['Authorization'] = `Bearer ${this.config.apiKey}`;
    }

    return headers;
  }

  private async callAPI(messages: ChatMessage[]): Promise<string> {
    const url = `${this.config.apiUrl}/chat/completions`;

    const body = {
      model: this.config.model,
      messages,
      temperature: 0.7,
      max_tokens: 2000,
    };

    const response = await fetch(url, {
      method: 'POST',
      headers: this.getHeaders(),
      body: JSON.stringify(body),
    });

    if (!response.ok) {
      const error = await response.text();
      throw new Error(`API Error: ${response.status} - ${error}`);
    }

    const data: ChatResponse = await response.json();
    return data.choices[0]?.message?.content || '';
  }

  async translate(content: string, targetLanguage: string): Promise<string> {
    const messages: ChatMessage[] = [
      {
        role: 'system',
        content: `You are a professional translator. Translate the following email content to ${targetLanguage}. Maintain the original formatting and tone. Do not add any explanations.`,
      },
      {
        role: 'user',
        content: content,
      },
    ];
    return this.callAPI(messages);
  }

  async summarize(content: string, style: string): Promise<string> {
    const messages: ChatMessage[] = [
      {
        role: 'system',
        content: `You are an email summarizer. Summarize the following email in ${style} style. Focus on key points, action items, and important information. Use clear bullet points.`,
      },
      {
        role: 'user',
        content: content,
      },
    ];
    return this.callAPI(messages);
  }

  async generateReply(content: string, tone: string, extraInstructions?: string): Promise<string> {
    const systemPrompt = `You are an email reply generator. Generate a professional reply to the following email with ${tone} tone. ${extraInstructions ? `Additional instructions: ${extraInstructions}` : ''}`;

    const messages: ChatMessage[] = [
      {
        role: 'system',
        content: systemPrompt,
      },
      {
        role: 'user',
        content: `Original email:\n${content}`,
      },
    ];
    return this.callAPI(messages);
  }

  async polish(content: string, style: string, language: string): Promise<string> {
    const messages: ChatMessage[] = [
      {
        role: 'system',
        content: `You are an email editor. Polish and improve the following email draft in ${language} with ${style} style. Fix grammar, improve clarity, and make it more professional. Do not change the core message.`,
      },
      {
        role: 'user',
        content: content,
      },
    ];
    return this.callAPI(messages);
  }
}

export default AIService;
export type { AIServiceType };
export { AI_SERVICES };