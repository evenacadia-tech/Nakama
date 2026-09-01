# 2026-09-01 — Dynamischer EQ: das Band schwingt sichtbar mit

## Wortlaut des Users

> „habe eine lücke im plan gefunden. bei einschalten des dynamischen eq muss
> das band dynmaisch mitschwingen wie man es von anderen eq kennt"

Gemeldet in der Dirigenten-Sitzung am 01.09.2026, unmittelbar nach dem
Abschluss von SONDE-012.

## Bindende Konsequenz

1. **Produkt:** Ist an einem Band die Dynamik zugeschaltet, zeigt das
   Kurvenbild die tatsächliche laufende dynamische Anhebung oder Absenkung
   dieses Bands sichtbar mit — das Band „atmet" mit dem Signal, wie es der
   Marktstandard vergleichbarer EQs vormacht (z. B. FabFilter Pro-Q). Ein
   statisches Bild bei aktiver Dynamik wäre eine unehrliche Anzeige.
2. **Ort:** Die Anzeige gehört auf Gens EQ-Fläche (Fläche 2), dort, wo die
   Bänder bedient werden — Lieferumfang S31b (`SONDE-020`). Sie gilt für die
   ferngesteuerten Sonden-EQs und gleichermaßen für Gens eigenen Master-EQ
   (S28b, dort lokal ohne IPC).
3. **Datenweg:** Die Dynamik rechnet in der Sonde (S26–28, `SONDE-015`). Der
   laufende Wert je Band (momentane dynamische Verstärkungsänderung) muss
   deshalb mit Anzeigekadenz von der Sonde zu Gen reisen — ein
   Telemetriefeld im Featureframe. Nach der Regel aus
   `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` (der Vertragsanteil,
   der später bricht, ist der NAME) wird der Feldname noch vor dem
   R0-Einfrieren durch G2 reserviert; die Nutzlast entsteht mit dem EQ-Kern
   in S26–28. Eigentümer der Reservierung: S26–28.
4. **Abgrenzung:** Dieser Entscheid legt die tatsächliche laufende Bewegung im
   Kurvenbild fest. Aktivierung, Ausschalten, Rückweg, Schließen/Fokus und der
   ruhige Aktivmarker des Mini-Panels wurden getrennt im abgeschlossenen
   [`2026-09-01-technische-ui-architektur-fuenferblock-01.md`](2026-09-01-technische-ui-architektur-fuenferblock-01.md)
   entschieden; beide Entscheide ergänzen einander und öffnen einander nicht
   erneut. Anzeigeeinstellungen (U17.3) dürfen die Live-Wahrheit darstellen,
   aber nie verfälschen.
