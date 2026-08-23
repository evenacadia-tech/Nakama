---
type: Operations Guide
title: Session automation
description: Explains Claude Code lifecycle hooks, local primers, write gates, automatic Git publication, and stop reminders.
tags: [collaboration, hooks, claude-code]
sources:
  - id: openwiki-source-1fe463fcf07912e5cdbb5a91
    resource: repo://.claude/settings.json
  - id: openwiki-source-aad178a39283868e8f309c49
    resource: repo://tools/hooks/agent-reminder-nakama.sh
  - id: openwiki-source-25ad80f34f17b718324d9c47
    resource: repo://tools/hooks/auto-push.sh
  - id: openwiki-source-f9904b78d113650e7c06ada4
    resource: repo://tools/hooks/commit-stop.sh
  - id: openwiki-source-2cd8494ca40e3254ac097611
    resource: repo://tools/hooks/design-primer.sh
  - id: openwiki-source-fd1a6e57ae515b99b5738b11
    resource: repo://tools/hooks/fremdmodell-riegel.sh
  - id: openwiki-source-6e8b720fc9a1d660df2ad418
    resource: repo://tools/hooks/git-automatik-probe.sh
  - id: openwiki-source-06ae03db6b94d5384fee9f4a
    resource: repo://tools/hooks/git-riegel.sh
  - id: openwiki-source-122f2d678161005919212438
    resource: repo://tools/hooks/hub-primer.sh
  - id: openwiki-source-36d1ae790c978d9eec851f1d
    resource: repo://tools/hooks/hub-stop.sh
  - id: openwiki-source-16d98a5e3231fc234899d002
    resource: repo://tools/hooks/kreativ-schleuse.sh
  - id: openwiki-source-c04e6e0477b585d394faf344
    resource: repo://tools/hooks/nakama-primer.sh
  - id: openwiki-source-83c898d08de72014297c15fa
    resource: repo://tools/hooks/prototyp-schleuse.sh
  - id: openwiki-source-8b1a99cf0635e70cf082311f
    resource: repo://tools/hooks/schleusen-probe.sh
  - id: openwiki-source-e49e9df85da5cafd2dc48e0f
    resource: repo://tools/hooks/session-start-marker.sh
  - id: openwiki-source-0bb50aef357b01738b1e12b7
    resource: repo://tools/hub/test_stop_hook.sh
generated: {by: "claude-code", at: "2026-08-23T10:03:23.427Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-23T10:03:23.427Z
---

# Session automation

`.claude/settings.json` declares repository automation for Claude Code
`SessionStart`, `PreToolUse`, `PostToolUse`, and `Stop` events. It currently
declares no `SessionEnd` hook.
These settings do not establish that Codex, Nimbalyst, or another host runs the
same hooks. Treat the scripts as Claude Code lifecycle contracts.

## Lifecycle map

```mermaid
flowchart LR
    Start[SessionStart] --> Marker[Create session marker]
    Start --> Primers[Inject product, depth, Hub, design summaries]
    Pre[PreToolUse] --> GitGate[Git safety gate]
    Pre --> ModelGate[External-model read-only gate]
    Pre --> Gates[Prototype and legacy design gates]
    Post[PostToolUse] --> Advice[Realtime and schema reminders]
    Post --> Push[Push commits ahead of upstream]
    Stop[Stop] --> HubNag[One-time local Hub reminder]
    Stop --> CommitNag[One-time uncommitted-work reminder]
```

On startup, resume, or clear, `session-start-marker.sh` records epoch, current
short HEAD, working directory, and a retired field under
`~/.claude/sessions/<session_id>.start`. It prunes markers older than seven
days. Missing session identity or an unavailable marker directory degrades
silently.

The configured primers summarize repository state:

- `nakama-primer.sh` extracts the marked truth block from root `CLAUDE.md`,
  then adds recent commits and at most ten dirty paths;
- `hub-primer.sh` reads only local `docs/hub/hub.json` plus Git drift and never
  contacts the deployed briefing endpoint; its completion and next-step status
  vocabulary matches the generated local plan view;
- `design-primer.sh` reports local design-document, latest-acceptance, Figma
  snapshot, and design-contract state;
- `depth-primer.sh` injects the repository's deeper-review guidance.

## Write gates and post actions

PreToolUse first applies `git-riegel.sh` and `fremdmodell-riegel.sh` to Bash
and PowerShell commands. The Git gate blocks broad staging without an explicit
pathspec, amend, force-push and forcing refspecs, remote deletion, destructive
reset or cleanup, mass discard, and forced branch deletion. It recognizes Git
behind an absolute path and options such as `-C`; quoted prose is not treated
as a command. The emergency bypass works only when
`NAKAMA_GIT_RIEGEL_AUS=1` is actually set at a command boundary, not merely
mentioned in a comment or message.

The external-model gate keeps Antigravity/Gemini invocations in an audit role.
Ordinary `agy` calls and plan mode remain available, while
`--dangerously-skip-permissions` and `--mode accept-edits` are blocked because
they authorize unattended writes. This is a command-surface restriction, not
a claim that an external model's findings are correct; findings still require
repository evidence.

The other PreToolUse group applies the legacy `eq-copilot/design/`
creative-release gate and the `design/prototyp/` design-contract gate. Their
shared write-target parser recognizes direct Write/Edit paths and common Bash
destinations. The prototype gate exits 2 when a prototype write lacks an
accepted `*designvertrag*.md`; reads and writes elsewhere pass. The creative
gate requires a `.claude/kreativ-freigabe.md` marker younger than 24 hours.

PostToolUse is advisory. It adds context after changes to plugin realtime
sources or versioned schemas but does not block the completed edit. After each
Bash or PowerShell command, `auto-push.sh` also checks whether the current
branch is ahead of its upstream. It skips detached HEAD, merge, and rebase
states; otherwise it pushes without an interactive prompt. A failed push is
remembered for that HEAD so later commands do not repeat the same failure.

## Stop behavior

`hub-stop.sh` does not pull or push the Hub. If commits occurred after session
start but `docs/hub/hub.json` was not touched, it emits one blocking reminder.
A per-session nag marker prevents a loop, and recursive Stop calls are silent.

`commit-stop.sh` independently looks for uncommitted paths whose modification
time is later than the session-start marker. It presents that candidate list
once, but does not stage or choose ownership: the agent must commit only its
own paths explicitly and leave parallel-session work untouched. Together,
commit Stop and auto-push close the local-commit and remote-publication halves
without introducing a configured SessionEnd action.

## Focused checks

The repository supplies Bash probes for the blocking and lifecycle workflows:

```bash
bash tools/hub/test_stop_hook.sh
bash tools/hooks/schleusen-probe.sh
bash tools/hooks/git-automatik-probe.sh
```

The first covers block, no-commit, Hub-touched, recursive, and repeated Stop
conditions. The second exercises allowed and denied Write, Edit, and Bash
destinations, including source-versus-destination and heredoc edge cases. The
third checks blocked and allowed Git commands, both directions of the external-
model gate, and auto-push's local gate.

New lifecycle behavior belongs in `.claude/settings.json` with a narrowly
scoped script and both pass and block tests. Keep real Hub synchronization in
[briefing sync](briefing-sync.md); hooks currently summarize and remind only.
