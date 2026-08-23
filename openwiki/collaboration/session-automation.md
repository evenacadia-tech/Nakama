---
type: Operations Guide
title: Session automation
description: Explains Claude Code lifecycle hooks, local primers, write gates, automatic Git publication, and stop reminders.
tags: [collaboration, hooks, claude-code]
sources:
  - resource: repo://tools/hooks/plan-primer.sh
  - resource: repo://tools/hooks/planstand.sh
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
generated: {by: "claude-code", at: "2026-08-23T16:26:37.107Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-23T16:26:37.107Z
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
    Start --> Primers[Inject product, depth, plan-status, design summaries]
    Pre[PreToolUse] --> GitGate[Git safety gate]
    Pre --> ModelGate[External-model read-only gate]
    Pre --> Gates[Prototype and legacy design gates]
    Post[PostToolUse] --> Advice[Realtime and schema reminders]
    Post --> Plan[Refresh plan status when sources moved]
    Post --> Push[Push commits ahead of upstream]
    Stop[Stop] --> CommitNag[One-time uncommitted-work reminder]
```

On startup, resume, or clear, `session-start-marker.sh` records epoch, current
short HEAD, working directory, and a retired field under
`~/.claude/sessions/<session_id>.start`. It prunes markers older than seven
days. Missing session identity or an unavailable marker directory degrades
silently.

The configured primers summarize repository state:

- `nakama-primer.sh` extracts the marked truth block from root `CLAUDE.md`,
  then adds recent commits and at most ten dirty paths;
- `plan-primer.sh` first invokes the plan-status refresh, then reads the
  generated sheet and the open-question file. It carries no synchronization
  duty at all, because the status it reports is computed rather than kept —
  see [Plan status and open questions](plan-status.md);
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
sources or versioned schemas but does not block the completed edit. Two further
scripts then act on measured state rather than on the command that just ran.
`planstand.sh` recomputes the plan sheet when its stamp no longer matches the
last commit touching a status source, and commits that one file by pathspec;
`auto-push.sh` checks whether the current branch is ahead of its upstream. Both
skip detached HEAD, merge, and rebase states. A failed push is remembered for
that HEAD so later commands do not repeat the same failure, and the ordering
means a sheet commit created in one round is published by the push check in the
same round.

## Stop behavior

`commit-stop.sh` looks for uncommitted paths whose modification
time is later than the session-start marker. It presents that candidate list
once, but does not stage or choose ownership: the agent must commit only its
own paths explicitly and leave parallel-session work untouched. It is now the
only consumer of the session-start marker, and together with auto-push it
closes the local-commit and remote-publication halves without a configured
SessionEnd action.

## Focused checks

The repository supplies Bash probes for the write gates and the Git surface:

```bash
bash tools/hooks/schleusen-probe.sh
bash tools/hooks/git-automatik-probe.sh
```

The first exercises allowed and denied Write, Edit, and Bash destinations,
including source-versus-destination and heredoc edge cases. The second checks
blocked and allowed Git commands, both directions of the external-model gate,
and auto-push's local gate. A third probe covering the former briefing Stop
reminder was removed with that reminder.

New lifecycle behavior belongs in `.claude/settings.json` with a narrowly
scoped script and both pass and block tests. Hooks summarize, remind, and — for
the plan sheet and the push — act on measured repository state; the computation
they invoke lives in [Plan status and open questions](plan-status.md).
