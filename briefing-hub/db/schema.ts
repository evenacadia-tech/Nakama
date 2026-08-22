import { index, integer, sqliteTable, text } from 'drizzle-orm/sqlite-core';

export const answers = sqliteTable('answers', {
  questionId: text('question_id').primaryKey(),
  choice: text('choice').notNull(),
  note: text('note').notNull().default(''),
  author: text('author').notNull().default('Phil'),
  status: text('status').notNull().default('neu'),
  updatedAt: integer('updated_at').notNull(),
});

export const briefingItems = sqliteTable(
  'briefing_items',
  {
    id: integer('id').primaryKey({ autoIncrement: true }),
    kind: text('kind').notNull(),
    title: text('title').notNull(),
    summary: text('summary').notNull(),
    reason: text('reason').notNull().default(''),
    urgency: text('urgency').notNull().default('wenn du dazu kommst'),
    author: text('author').notNull(),
    status: text('status').notNull().default('offen'),
    createdAt: integer('created_at').notNull(),
    updatedAt: integer('updated_at').notNull(),
  },
  (table) => [index('idx_briefing_items_created_at').on(table.createdAt)],
);

export const hubState = sqliteTable('hub_state', {
  key: text('key').primaryKey(),
  payloadJson: text('payload_json').notNull(),
  updatedBy: text('updated_by').notNull(),
  updatedAt: integer('updated_at').notNull(),
});
