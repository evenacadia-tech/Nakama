# 2026-08-28 — Suna-Stilllegung vorgezogen: der FL-Schritt misst zwei Ziele

## Entscheid (bindend, User-Wort 28.08.2026)

Anlass: Der Dirigent hatte für den offenen FL-Schritt von S9 (`SONDE-007b`,
§55 Klausel 1 „Alle drei Ziele scannen und laden in FL") die Installation
und den Scan von drei Bundles beschrieben — Gen, Suna, Probeeq.

> „moment. suna gibt es nichtmehr . nurnoch Nakama Gen mit 2 Oberflächen
> und Nakama Probeeq"

Auf den Vorschlag, die im Plan für S28b (`SONDE-015b`) vorgesehene
Stilllegung des Suna-Ziels als eigenes kleines Ticket direkt nach S8
vorzuziehen, damit der FL-Schritt das Produkt misst, das es gibt:

> „ja stell das einfach richtig"

## Folge

- Neues Ticket **S9b `SONDE-007c`** in Phase P1, vor dem FL-Schritt von S9:
  `NakamaSuna_VST3` wird stillgelegt, das Prüfbein A15 verabschiedet sich
  deklariert aus dem Kanon, das Installer-Manifest liefert zwei Bundles plus
  Broker, der Suna-Eintrag in `plugin-identities-v1.json` wird als
  stillgelegt markiert. Die eingefrorenen Kennungen von Gen und Probeeq
  bleiben unberührt (NAK-30 gilt weiter); der Rückweg bleibt vollständig.
- S28b (`SONDE-015b`) verliert den Suna-Teil; es bleibt der Gen-Master-EQ.
- §55 Klausel 1 lautet danach: **beide** Ziele scannen und laden in FL. Der
  User-Termin (Installation per Admin-Klick, Scan und Laden in FL) findet
  erst nach S9b statt und misst Nakama Gen und Nakama Probeeq.
- Produktwahrheit unverändert seit 23.08.2026 (Register, NAK-65): zwei Apps,
  Gen mit zwei Oberflächen, Probeeq als Messsonde und EQ-Ausführer.

Eingearbeitet in `docs/plan/plan.json` (S9, S9b, S28b),
`docs/bauaufteilung-sonden.md` §3 und `docs/FL-Nakama-Sonden-Design-Entwurf.md`
(§55-Nachtrag, Tickettabelle) am 28.08.2026.
