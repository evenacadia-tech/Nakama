CREATE TABLE `answers` (
	`question_id` text PRIMARY KEY NOT NULL,
	`choice` text NOT NULL,
	`note` text DEFAULT '' NOT NULL,
	`author` text DEFAULT 'Phil' NOT NULL,
	`status` text DEFAULT 'neu' NOT NULL,
	`updated_at` integer NOT NULL
);
--> statement-breakpoint
CREATE TABLE `briefing_items` (
	`id` integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	`kind` text NOT NULL,
	`title` text NOT NULL,
	`summary` text NOT NULL,
	`reason` text DEFAULT '' NOT NULL,
	`urgency` text DEFAULT 'wenn du dazu kommst' NOT NULL,
	`author` text NOT NULL,
	`status` text DEFAULT 'offen' NOT NULL,
	`created_at` integer NOT NULL,
	`updated_at` integer NOT NULL
);
--> statement-breakpoint
CREATE INDEX `idx_briefing_items_created_at` ON `briefing_items` (`created_at`);