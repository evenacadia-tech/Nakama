# 2026-08-25 — Festes Seitenverhältnis bleibt, freies Layout vertagt

## Der Entscheid (bindend, User-Wort 25.08.2026)

> „ja aber flexible skalierung ist wesentlich schwerer zu designen. lassen
> wir es erstmal dabei."

## Wozu die Frage gestellt wurde

Beim Review des Codex-Skills `audio-plugin-ux`
(`docs/review-2026-08-25-audio-plugin-ux.md`) fiel auf, dass die
Pro-Q-Disclosure-Mechanik bei Nakama enger aufgeht als beim Vorbild. Grund:

- Pro-Q gewinnt beim Vergrößern **Informationsfläche** — mehr
  Spektrumauflösung, mehr gleichzeitig lesbare Bänder, das Panel frisst
  anteilig weniger Canvas.
- Nakama gewinnt beim Vergrößern nur **Zoom** — festes Verhältnis, dieselben
  Elemente größer.

Daraufhin die Rückfrage des Users, wer den 760×430-„Zwang" auferlegt habe.
Antwort: wir selbst — Zahl aus einer Messung (`design/werkzeug/formfaktor.html`),
abgenommen am 20.08. mit „so passt es 3 größen"
(`2026-08-20-groessen-alle-drei.md`), am 24.08. auf beide Gen-Seiten
verschärft. Kein FL-Studio-Limit, keine VST3-Grenze.

## Was damit festliegt

1. **Festes Seitenverhältnis bleibt.** Freies Layout (Verhältnis frei, Inhalt
   reflowt) ist **vertagt**, nicht verworfen. Die Formulierung „erstmal" ist
   Teil des Entscheids.
2. **Die Entwurfsbedingung lautet:** 8 Bänder × 13 Parameter + 5 globale
   Werte auf einem nicht umformbaren Feld. Jeder Layout-Vorschlag für Gen
   Seite 2 muss darunter bestehen.
3. **Damit ist der offene Teil von Regel 3** in
   `design/docs/ui-spielregeln-eq-zentrale.md` („Freie Skalierung ist
   technisch da […], aber als Produktentscheid offen") **vorerst beantwortet**:
   sie wird nicht genutzt. NAK-65 bleibt im Übrigen offen.

## Befund, nicht Entscheid — gehört ins Repo, nicht dem User zur Last

Der laufende Editor arbeitet auf `kEinheitB = 750`, `kEinheitH = 520`
(`eq-copilot/plugin/src/EqCopilotAssetKit.h:22-23`, Verhältnis 1,44), während
die abgenommene Designgröße 760×430 das Verhältnis 1,77 hat
(`eq-copilot/plugin/src/PluginEditor.cpp:184-190`). Das ist die
Material-Kit-Front, laut `CLAUDE.md` ein nie abgenommenes Provisorium.
Abgenommenes Designmaß und laufender Editor sind derzeit zwei verschiedene
Formate. Das ist zu klären, bevor Gen Seite 2 gebaut wird — sonst entsteht
ein Entwurf gegen ein Maß, das der Editor nicht rendert.

## Bezug

- `docs/review-2026-08-25-audio-plugin-ux.md` (Befund B6)
- `design/abnahmen/2026-08-20-groessen-alle-drei.md` (Ursprung der Maße)
- `design/docs/ui-spielregeln-eq-zentrale.md` Regel 3
