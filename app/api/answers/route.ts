import { NextResponse } from 'next/server';
import { getDatabase } from '@/db';

const authors = new Set(['Phil', 'Claude', 'Codex']);

function shortText(value: unknown, limit: number) {
  return typeof value === 'string' ? value.trim().slice(0, limit) : '';
}

export async function POST(request: Request) {
  try {
    const body = await request.json() as Record<string, unknown>;
    const questionId = shortText(body.questionId, 80);
    const choice = shortText(body.choice, 160);
    const note = shortText(body.note, 2000);
    const author = shortText(body.author, 40);

    if (!/^[A-Za-z0-9.-]{1,80}$/.test(questionId) || !choice || !authors.has(author)) {
      return NextResponse.json(
        { error: 'Die Antwort ist unvollständig. Bitte wähle eine Option.' },
        { status: 400 },
      );
    }

    const updatedAt = Date.now();
    const database = await getDatabase();
    await database
      .prepare(`
        INSERT INTO answers (question_id, choice, note, author, status, updated_at)
        VALUES (?, ?, ?, ?, 'neu', ?)
        ON CONFLICT(question_id) DO UPDATE SET
          choice = excluded.choice,
          note = excluded.note,
          author = excluded.author,
          status = 'neu',
          updated_at = excluded.updated_at
      `)
      .bind(questionId, choice, note, author, updatedAt)
      .run();

    return NextResponse.json({
      questionId,
      choice,
      note,
      author,
      status: 'neu',
      updatedAt,
    });
  } catch {
    return NextResponse.json(
      { error: 'Die Antwort konnte nicht gespeichert werden. Bitte versuche es noch einmal.' },
      { status: 500 },
    );
  }
}
