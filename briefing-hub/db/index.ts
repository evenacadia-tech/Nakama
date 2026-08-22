import { env } from 'cloudflare:workers';

let initialization: Promise<void> | undefined;

async function ensureSchema(database: D1Database) {
  await database.batch([
    database.prepare(`
      CREATE TABLE IF NOT EXISTS answers (
        question_id TEXT PRIMARY KEY,
        choice TEXT NOT NULL,
        note TEXT NOT NULL DEFAULT '',
        author TEXT NOT NULL DEFAULT 'Phil',
        status TEXT NOT NULL DEFAULT 'neu',
        updated_at INTEGER NOT NULL
      )
    `),
    database.prepare(`
      CREATE TABLE IF NOT EXISTS briefing_items (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        kind TEXT NOT NULL,
        title TEXT NOT NULL,
        summary TEXT NOT NULL,
        reason TEXT NOT NULL DEFAULT '',
        urgency TEXT NOT NULL DEFAULT 'wenn du dazu kommst',
        author TEXT NOT NULL,
        status TEXT NOT NULL DEFAULT 'offen',
        created_at INTEGER NOT NULL,
        updated_at INTEGER NOT NULL
      )
    `),
    database.prepare(`
      CREATE INDEX IF NOT EXISTS idx_briefing_items_created_at
      ON briefing_items(created_at)
    `),
    database.prepare(`
      CREATE TABLE IF NOT EXISTS hub_state (
        key TEXT PRIMARY KEY,
        payload_json TEXT NOT NULL,
        updated_by TEXT NOT NULL,
        updated_at INTEGER NOT NULL
      )
    `),
    database.prepare('PRAGMA optimize'),
  ]);
}

export async function getDatabase() {
  if (!env.DB) {
    throw new Error('Der gemeinsame Speicher ist nicht verfügbar.');
  }

  initialization ??= ensureSchema(env.DB);
  await initialization;
  return env.DB;
}
