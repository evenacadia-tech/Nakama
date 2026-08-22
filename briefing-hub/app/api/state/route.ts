import { NextResponse } from 'next/server';
import { getDatabase } from '@/db';

const authors = new Set(['Phil', 'Claude', 'Codex']);

function validHub(value: unknown): value is Record<string, unknown> {
  if (!value || typeof value !== 'object' || Array.isArray(value)) return false;
  const hub = value as Record<string, unknown>;
  return (
    typeof hub.stand === 'string' &&
    Array.isArray(hub.bei_dir) &&
    Array.isArray(hub.plan) &&
    Boolean(hub.design && typeof hub.design === 'object')
  );
}

export async function POST(request: Request) {
  try {
    const body = await request.json() as { author?: unknown; hub?: unknown };
    const author = typeof body.author === 'string' ? body.author.trim() : '';

    if (!authors.has(author) || !validHub(body.hub)) {
      return NextResponse.json(
        { error: 'Autor oder Projektstand ist ungültig.' },
        { status: 400 },
      );
    }

    const payloadJson = JSON.stringify(body.hub);
    if (payloadJson.length > 500_000) {
      return NextResponse.json(
        { error: 'Der Projektstand ist zu groß.' },
        { status: 413 },
      );
    }

    const updatedAt = Date.now();
    const database = await getDatabase();
    await database
      .prepare(`
        INSERT INTO hub_state (key, payload_json, updated_by, updated_at)
        VALUES ('current', ?, ?, ?)
        ON CONFLICT(key) DO UPDATE SET
          payload_json = excluded.payload_json,
          updated_by = excluded.updated_by,
          updated_at = excluded.updated_at
      `)
      .bind(payloadJson, author, updatedAt)
      .run();

    return NextResponse.json({ ok: true, updatedBy: author, updatedAt });
  } catch {
    return NextResponse.json(
      { error: 'Der Projektstand konnte nicht gespeichert werden.' },
      { status: 500 },
    );
  }
}
