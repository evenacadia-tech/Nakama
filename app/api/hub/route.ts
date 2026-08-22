import { NextResponse } from 'next/server';
import seedHub from '@/data/hub.json';
import { getDatabase } from '@/db';

type AnswerRow = {
  question_id: string;
  choice: string;
  note: string;
  author: string;
  status: string;
  updated_at: number;
};

type ItemRow = {
  id: number;
  kind: string;
  title: string;
  summary: string;
  reason: string;
  urgency: string;
  author: string;
  status: string;
  created_at: number;
  updated_at: number;
};

type StateRow = {
  payload_json: string;
  updated_by: string;
  updated_at: number;
};

export async function GET() {
  try {
    const database = await getDatabase();
    const [stateResult, answerResult, itemResult] = await database.batch([
      database.prepare(`
        SELECT payload_json, updated_by, updated_at
        FROM hub_state
        WHERE key = 'current'
        LIMIT 1
      `),
      database.prepare(`
        SELECT question_id, choice, note, author, status, updated_at
        FROM answers
        ORDER BY updated_at DESC
      `),
      database.prepare(`
        SELECT id, kind, title, summary, reason, urgency, author, status, created_at, updated_at
        FROM briefing_items
        ORDER BY created_at DESC
      `),
    ]);

    const stateRow = (stateResult.results as StateRow[])[0];
    let currentHub: unknown = seedHub;
    if (stateRow?.payload_json) {
      try {
        currentHub = JSON.parse(stateRow.payload_json);
      } catch {
        currentHub = seedHub;
      }
    }

    const answers = Object.fromEntries(
      (answerResult.results as AnswerRow[]).map((row) => [
        row.question_id,
        {
          choice: row.choice,
          note: row.note,
          author: row.author,
          status: row.status,
          updatedAt: row.updated_at,
        },
      ]),
    );

    const items = (itemResult.results as ItemRow[]).map((row) => ({
      id: row.id,
      kind: row.kind,
      title: row.title,
      summary: row.summary,
      reason: row.reason,
      urgency: row.urgency,
      author: row.author,
      status: row.status,
      createdAt: row.created_at,
      updatedAt: row.updated_at,
    }));

    return NextResponse.json(
      {
        schemaVersion: 1,
        hub: currentHub,
        answers,
        items,
        stateUpdatedBy: stateRow?.updated_by ?? 'Projektquelle',
        stateUpdatedAt: stateRow?.updated_at ?? null,
        agentHint: 'Antworten über POST /api/answers, neue Punkte über POST /api/items, Projektstand über POST /api/state.',
      },
      { headers: { 'Cache-Control': 'no-store' } },
    );
  } catch {
    return NextResponse.json(
      {
        schemaVersion: 1,
        hub: seedHub,
        answers: {},
        items: [],
        warning: 'Der gemeinsame Speicher ist gerade nicht erreichbar. Der letzte Projektstand bleibt lesbar.',
      },
      { status: 200, headers: { 'Cache-Control': 'no-store' } },
    );
  }
}
