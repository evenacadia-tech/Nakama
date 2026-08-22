import { NextResponse } from 'next/server';
import { getDatabase } from '@/db';

const authors = new Set(['Phil', 'Claude', 'Codex']);
const kinds = new Set(['Entscheidung', 'Update', 'Blocker']);
const urgencies = new Set(['jetzt', 'wenn du dazu kommst', 'wissen', 'später']);

function shortText(value: unknown, limit: number) {
  return typeof value === 'string' ? value.trim().slice(0, limit) : '';
}

export async function POST(request: Request) {
  try {
    const body = await request.json() as Record<string, unknown>;
    const kind = shortText(body.kind, 40);
    const title = shortText(body.title, 160);
    const summary = shortText(body.summary, 2000);
    const reason = shortText(body.reason, 1200);
    const urgency = shortText(body.urgency, 40);
    const author = shortText(body.author, 40);

    if (
      !kinds.has(kind) ||
      title.length < 4 ||
      summary.length < 4 ||
      !urgencies.has(urgency) ||
      !authors.has(author)
    ) {
      return NextResponse.json(
        { error: 'Titel, Kurzbeschreibung, Art und Autor werden benötigt.' },
        { status: 400 },
      );
    }

    const now = Date.now();
    const database = await getDatabase();
    const result = await database
      .prepare(`
        INSERT INTO briefing_items (
          kind, title, summary, reason, urgency, author, status, created_at, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, 'offen', ?, ?)
      `)
      .bind(kind, title, summary, reason, urgency, author, now, now)
      .run();

    return NextResponse.json({
      id: Number(result.meta.last_row_id),
      kind,
      title,
      summary,
      reason,
      urgency,
      author,
      status: 'offen',
      createdAt: now,
      updatedAt: now,
    });
  } catch {
    return NextResponse.json(
      { error: 'Der neue Punkt konnte nicht gespeichert werden. Bitte versuche es noch einmal.' },
      { status: 500 },
    );
  }
}
