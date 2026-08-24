# Nakama Gen Web-Simulator

Der Simulator ist die ausführbare Referenz für Zustände, Gesten und Motion der
760×430-Oberfläche. Er verwendet deterministische Demo-Daten und ist weder eine
DSP-Referenz noch eine später in das Plug-in eingebettete Webseite.

## Öffnen

- Direkt: `design/prototyp/index.html` doppelklicken.
- Für reproduzierbare Browser-Prüfungen im Ordner `design/` einen statischen
  Server starten und `/prototyp/` öffnen. So bleiben auch die gemeinsam
  versionierten Figma-Assets unter `design/assets/` erreichbar.

Das Prüfpanel liegt bewusst außerhalb der Plug-in-Fläche. Es schaltet Szenario,
Skalierung, Verbindung, Transport, Messung, Vergleich, Automation und den intern
erzeugten Demo-Loop.

## Reproduzierbare Zustände

Query-Parameter:

- `fixture=overview-golden|eq-golden|sixteen-sources|disconnected|recording-gate|stale-automation`
- `scale=1|1.25|1.5|2`
- `capture=1` blendet das Prüfpanel aus, friert den automatischen Takt ein und
  stellt über `window.__nakama.step(ms)` einen kontrollierten Testtakt bereit.

Beispiele:

- `/prototyp/?fixture=overview-golden&capture=1`
- `/prototyp/?fixture=eq-golden&scale=2&capture=1`

## Vertrag und Prüfungen

- `contract/` enthält Asset-, Zustands- und Motion-Vertrag.
- `src/demo-adapter.js` ist der deterministische Web-Adapter.
- `tests/state.test.mjs` prüft Lebenszyklus, Lease, Schutzbereiche, Automation,
  Drag, Quellenreihenfolge und Global-Werte.
- `tools/verify_baselines.py` prüft Goldens, Hashes und alle Qualitätsstufen.

Die native JUCE-/VST3-Oberfläche beginnt erst nach benannter Web-Abnahme. Bis
dahin bleibt `JUCE_WEB_BROWSER=0` unverändert und es gibt keine Browser-Abhängigkeit
im Plug-in.
