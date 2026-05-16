import { UserConfig, DEFAULT_CONFIG } from './AIConfig';

const STORAGE_KEY = 'ai-assistant-config';
const ENCRYPTION_SALT = 'outlook-ai-assistant-v1';

class ConfigStorage {
  private static deriveKey(password: string): Promise<CryptoKey> {
    const encoder = new TextEncoder();
    const keyMaterial = crypto.subtle.importKey(
      'raw',
      encoder.encode(password),
      'PBKDF2',
      false,
      ['deriveBits', 'deriveKey']
    );
    return keyMaterial.then((material) =>
      crypto.subtle.deriveKey(
        {
          name: 'PBKDF2',
          salt: encoder.encode(ENCRYPTION_SALT),
          iterations: 100000,
          hash: 'SHA-256',
        },
        material,
        { name: 'AES-GCM', length: 256 },
        false,
        ['encrypt', 'decrypt']
      )
    );
  }

  static async encrypt(data: string): Promise<string> {
    const key = await this.deriveKey('local-storage-key');
    const encoder = new TextEncoder();
    const iv = crypto.getRandomValues(new Uint8Array(12));
    const encrypted = await crypto.subtle.encrypt(
      { name: 'AES-GCM', iv },
      key,
      encoder.encode(data)
    );
    const combined = new Uint8Array(iv.length + encrypted.byteLength);
    combined.set(iv);
    combined.set(new Uint8Array(encrypted), iv.length);
    return btoa(String.fromCharCode(...combined));
  }

  static async decrypt(data: string): Promise<string> {
    const key = await this.deriveKey('local-storage-key');
    const combined = Uint8Array.from(atob(data), (c) => c.charCodeAt(0));
    const iv = combined.slice(0, 12);
    const encrypted = combined.slice(12);
    const decrypted = await crypto.subtle.decrypt(
      { name: 'AES-GCM', iv },
      key,
      encrypted
    );
    return new TextDecoder().decode(decrypted);
  }

  static async save(config: UserConfig): Promise<void> {
    const data = JSON.stringify(config);
    const encrypted = await this.encrypt(data);
    localStorage.setItem(STORAGE_KEY, encrypted);
  }

  static async load(): Promise<UserConfig | null> {
    const encrypted = localStorage.getItem(STORAGE_KEY);
    if (!encrypted) return null;
    try {
      const decrypted = await this.decrypt(encrypted);
      return JSON.parse(decrypted);
    } catch {
      return null;
    }
  }

  static clear(): void {
    localStorage.removeItem(STORAGE_KEY);
  }

  static async initialize(): Promise<UserConfig> {
    const saved = await this.load();
    if (saved && saved.apiKey) return saved;
    return DEFAULT_CONFIG;
  }
}

export default ConfigStorage;