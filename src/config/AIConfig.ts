// AI Service Provider Configuration
export type AIServiceType = 'openai' | 'deepseek' | 'claude' | 'kimi' | 'minimax' | 'qianwen';

export interface AIServiceConfig {
  name: string;
  url: string;
  models: string[];
}

export const AI_SERVICES: Record<AIServiceType, AIServiceConfig> = {
  openai: {
    name: 'OpenAI',
    url: 'https://api.openai.com/v1',
    models: ['gpt-4o', 'gpt-4o-mini', 'gpt-4-turbo', 'gpt-4', 'gpt-3.5-turbo'],
  },
  deepseek: {
    name: 'DeepSeek',
    url: 'https://api.deepseek.com/v1',
    models: ['deepseek-chat', 'deepseek-coder', 'deepseek-reasoner'],
  },
  claude: {
    name: 'Claude',
    url: 'https://api.anthropic.com/v1',
    models: ['claude-3-5-sonnet-20241022', 'claude-3-opus-20240229', 'claude-3-haiku-20240307'],
  },
  kimi: {
    name: 'Kimi',
    url: 'https://api.moonshot.cn/v1',
    models: ['moonshot-v1-8k', 'moonshot-v1-32k', 'moonshot-v1-128k'],
  },
  minimax: {
    name: 'MiniMax',
    url: 'https://api.minimax.chat/v1',
    models: ['abab6.5-chat', 'abab5.5-chat', 'abab5.5s-chat'],
  },
  qianwen: {
    name: 'Qianwen',
    url: 'https://dashscope.aliyuncs.com/compatible-mode/v1',
    models: ['qwen-max', 'qwen-plus', 'qwen-turbo', 'qwen-long'],
  },
};

// User Configuration
export interface UserConfig {
  aiService: AIServiceType;
  apiUrl: string;
  model: string;
  apiKey: string;
  targetLanguage: string;
  replyTone: string;
}

export const DEFAULT_CONFIG: UserConfig = {
  aiService: 'openai',
  apiUrl: AI_SERVICES.openai.url,
  model: 'gpt-4o',
  apiKey: '',
  targetLanguage: 'Chinese',
  replyTone: 'Formal',
};