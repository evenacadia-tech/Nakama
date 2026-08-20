# Fixture-Manifest — Aux-/PDC-Spike (SONDE-004a)

Entwurf §66.3 verlangt es so: die Mediendateien duerfen ausserhalb des
Repositories liegen, **Manifest, Hash und erwartetes Resultat gehoeren hinein**.

Die WAVs sind nicht eingecheckt (regenerierbar in Sekunden):

```powershell
py -3.13 tools/eq-copilot/erzeuge_aux_spike_fixtures.py
```

Der Erzeuger ist deterministisch — zwei Laeufe liefern bytegleiche Dateien und
damit dieselben Hashes. Weicht ein Hash ab, ist entweder der Erzeuger geaendert
worden oder die Datei ist nicht die, fuer die man sie haelt.

## Dateien

| Datei | Rate | Laenge | Impuls ab Sample | Impulslaenge | Spitze | SHA-256 |
|---|---:|---:|---:|---:|---:|---|
| `impuls-44100.wav` | 44100 | 88200 (2,0 s) | 22050 (0,5 s) | 3 Samples | 0,9 | `6EEC38186549BD9D19922E018B76EB17B79BF6C8300AE05AA49E0D064647FDCC` |
| `impuls-48000.wav` | 48000 | 96000 (2,0 s) | 24000 (0,5 s) | 3 Samples | 0,9 | `D4E4D119D73D29AE2DF7125BA3FC685A7FC9A00A0DF9B742BA7CEDF689AFB5A1` |

Beide: Stereo, 16 Bit PCM, identischer Inhalt auf beiden Kanaelen, davor und
danach exakte Stille.

**Die Rate muss zur Projektrate passen.** Sonst resampelt FL, der Impuls
verschmiert ueber mehrere Samples und die Flanke wird weich. Diese Maschine
faehrt FL Studio ASIO auf **44100 Hz** — im Zweifel `impuls-44100.wav`.

## Erwartetes Resultat

Gemessen wird mit `EqCop-Aux-Spike.vst3` (Wegwerf-Target, Bau siehe
`eq-copilot/docs/FL-TERMIN-A-AUX-PDC.md`). Dieselbe Datei laeuft gleichzeitig
auf den Main-Weg und ueber den Sidechain auf einen Aux-Bus.

| Fall | was der Spike zeigen muss | Bedeutung |
|---|---|---|
| Aux-Bus wird von FL gar nicht angeboten | `INAKTIV — der Host hat diesen Bus nicht zugeschaltet` | Aux-Capability **unsupported**; Kernfunktion 17 und exakte Attribution entfallen |
| Aux aktiv, aber ohne Signal | `Signal: nein` | Routing im Wrapper zeigt auf die falschen Pins |
| Aux aktiv, Versatz **0** | `Versatz: 0 Samples` | FL kompensiert den Aux-Weg vollstaendig — **supported** |
| Aux aktiv, Versatz konstant ungleich 0 | `Versatz: +N Samples` | fester Offset; brauchbar, wenn N ueber Recall stabil bleibt |
| Versatz aendert sich zwischen zwei Messungen | zwei verschiedene N | nicht recallstabil ⇒ Fallback nach Entwurf §51 |
| `Versatz: — (Dauersignal …)` | Protokoll verletzt | im Projekt laeuft noch Musik; erst alles stumm schalten |

**Kein Ergebnis ist ein schlechtes Ergebnis.** Auch `unsupported` ist ein
Messwert — der Plan haengt genau deshalb an dieser Messung, damit spaeter keine
Funktion gebaut wird, die FL nicht tragen kann. Verboten ist nur `unknown,
spaeter pruefen` (Exit-Gate P0).
