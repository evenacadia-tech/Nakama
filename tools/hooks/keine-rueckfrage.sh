#!/usr/bin/env bash
# PermissionRequest: jede Berechtigungsrueckfrage wird ohne Dialog abgelehnt.
#
# Warum es ihn gibt (User 11.09.2026 und 19.09.2026): eine Rueckfrage um halb
# zwei nachts friert den Dirigentenlauf bis zum Vormittag ein. `dontAsk` fragt
# zwar nie, lehnt aber auch ab, was der Dirigent koennen muss (Schreiben unter
# `.claude/`, Doku code.claude.com/docs/en/permission-modes, "Protected paths":
# dontAsk = Denied, auto = Routed to the classifier). Der Dirigent laeuft
# deshalb im Auto-Modus; dessen Rueckfall auf einen Dialog (3 Blocks in Folge
# oder 20 je Session, nicht einstellbar) faengt dieser Hook ab: abgelehnt statt
# gewartet, die Session arbeitet weiter.
#
# Der Hook lockert nichts: er kennt nur "deny". Welche Werkzeuge er NICHT
# sieht, bestimmt allein der matcher in `.claude/settings.json` (die Datei
# aendert nur der User). Der matcher MUSS AskUserQuestion und ExitPlanMode
# ausnehmen: die Userfrage laeuft durch denselben Berechtigungsweg und wird
# sonst mit abgelehnt - auch mit allow-Regel und im Auto-Modus (gemessen
# 19.09.2026); `/fragen` und jede Produktfrage des Dirigenten waeren tot.
#
# Gegenprobe: eine Session im Manual-Modus mit diesem Hook meldet bei einem
# Write "Denied by PermissionRequest hook" und endet, statt zu warten
# (gemessen 19.09.2026, CLI 2.1.278, -p und --bg).

cat >/dev/null
printf '%s\n' '{"hookSpecificOutput":{"hookEventName":"PermissionRequest","decision":{"behavior":"deny","message":"NAKAMA-RIEGEL: keine Rueckfragen in diesem Projekt. Aktion abgelehnt; einen anderen zulaessigen Weg waehlen oder den Schritt einem Worker geben."}}}'
