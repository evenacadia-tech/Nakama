CREATE TABLE `hub_state` (
	`key` text PRIMARY KEY NOT NULL,
	`payload_json` text NOT NULL,
	`updated_by` text NOT NULL,
	`updated_at` integer NOT NULL
);
