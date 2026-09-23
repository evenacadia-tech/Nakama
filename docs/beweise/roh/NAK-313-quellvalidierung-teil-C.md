# NAK-313 — Quellvalidierung Teil C (lesender Opus-Agent, HEAD 43549a0e)

**Ticket:** NAK-313 · Planschritt S25l „Tiefenaudit 3 abarbeiten, Teil 5: State und
Verträge" · Etappe 0 „Quellvalidierung", **Teil C**
**HEAD:** `43549a0eab4d810cbe9e148e7c8962bc47190042` (Zweig `master`)
**Datum:** 23.09.2026 (Start 19:11:59, letzte Messung vor dem Schreiben 19:39:03, beides
`date` in der Session) · **Modell:** Opus 5.5 (lesend)
**IDs (2):** T3-03-10 (W31, `binary_telemetry`) und T3-14-04 (W33, Exportkollision).
Dazu die Regelfragen 1 und 2 und ein Kartenentwurf für W33.
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen `:387`
und `:591`; Skeptikerzeile Phase 10 `:521` zu T3-03-10 — zu T3-14-04 gibt es keine, weil
Phase 14 nach Phase 10/11 lief und ausdrücklich „nicht rückwirkend durch Phase 10/11
gelaufen" ist, `:584`; Bündel B26 `:96` und `:141`; Kartenentwurf PF11-04 `:232-237`;
Phase-14-Abschnitt `:578-603`; Arbeitsübersicht W31 `:834`, W33 `:836`) und die
Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-03-vertrag.md`
(`:29`, `:73`, `:125`, `:152`, `:178`, `:202`), `phase-14-blindspots.md` (`:22-24`, `:33`,
`:75-83`, `:99-103`), `phase-15-paket1-verspaetete-ergebnisse.md` (`:411`: T3-14-04
„kein Bezug"), `phase-15-paket3-teilfehler.md` (`:84`, nur Lesescope),
`phase-15-zentral.md` (`:62`, nur Lesescope), `phase-10-skeptiker.md` (`:62`, `:131`),
`phase-11-synthese.md` (`:86`, `:131`, `:222-227`), `phase-16-abdeckung.md` (führt keine der
beiden IDs; W31 und W33 stehen in der Arbeitsübersicht als „Altbasis").
**Gate:** Schritt S25l, `docs/plan/plan.json:369` (Feld `text`); Registerzeilen
`docs/offene-punkte.md:317` (NAK-313), `:295` (NAK-286), `:294` (NAK-285).
**Muster:** `docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` (Trennung Technik und Produkt,
Kartenprüfung mit Wortlautvorschlag).

**Gate-Text wörtlich** (`docs/plan/plan.json:369`, Feld `text`, der hier einschlägige Teil):

> „ANGELEGT 18.09.2026 (Register NAK-313; Arbeitsübersicht Phase 16 W10, W11, W30, W31,
> W33). INHALT: 16 Befunde an State und Verträgen — … T3-03-10 (binary_telemetry),
> T3-14-04 (Exportkollision in derselben Sekunde). GRENZE: Schemas sind Verträge —
> versionieren, Altstände laden, unbekannte Felder erhalten, Save und Load gemeinsam,
> Discriminator/Zieladresse/Revision/Capability nicht additiv; Fixtures bytegleich mit
> .gitattributes; Cross-Language-Gleichheit C++/Rust; keine stillen Rundungen.
> Produktentscheide als Karten nur, wo der Vertrag schweigt (Duplikatregel, Archiv- oder
> Überschreibregel beim Export). BEWEIS: docs/beweise/NAK-313.md mit Matrix, identischen
> Positiv-/Negativvektoren durch echte C++- und Rust-Reader (A5, A8, A11, A12, B2, B3c,
> B10, B13, A4), vollem Kanon GRÜN und Laufzeit-Arm."

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon,
kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion, kein Export in eine
Nutzerablage. Git nur lesend (`rev-parse`, `status`, `diff --stat`, `show`, `blame`, `log`,
`grep`). Jede Zeilennummer unten ist am HEAD `43549a0e` selbst nachgelesen, keine aus dem
Audit übernommen. Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand; Legacy-Bezeichner
(`EQ-Copilot`, `EqCop*`, `Eqcp`) sind kein Befund; der Designteil ist geparkt — unten steht
keine Oberfläche, nur Dateiverhalten und ehrliche Zustandsmeldung.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* (19:11:59) und
*unmittelbar vor dem Schreiben dieser Datei* (19:35:14 und erneut 19:39:03) identisch: die
zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` sowie die drei
Auftragsdateien `docs/beweise/roh/NAK-313-quellvalidierung-teil-A-auftrag.txt`,
`…-teil-B-auftrag.txt` und `…-teil-C-auftrag.txt`, sonst nichts. HEAD alle drei Male
`43549a0e`. Der HEAD ist **nicht** gewandert; ein Basisabgleich war nicht nötig. Diese
Datei ist die einzige Schreibaktion des Agenten; `-teil-A.md` und `-teil-B.md` gehören den
Parallelagenten und wurden nicht angefasst.

**Basis der Auditmessungen gegen HEAD.** Phase 03 maß an `8f7c6f5f`, Phase 14 an
`e008811e`. `git diff --stat 8f7c6f5f HEAD` über `controlclient/Intern.h`,
`controlclient/Vertrag.cpp`, `TelemetryClient.cpp`, `WireEnvelope.h`,
`broker/src/coordinator/senke.rs`, `liveness.rs`, `broker/src/transport/v3.rs`,
`host-capabilities-fl-v1.json`, `pruefe_host_capabilities.py`,
`reservierte-nachrichten-v1.json` und den Entwurf: **leer** — alle Stellen von T3-03-10
sind seit dem Auditstand wörtlich unverändert. Einzige Ausnahme ist
`eq-copilot/plugin/sonde/SondeProcessor.cpp` (NAK-311/312, +561 Zeilen): der P2-Aufruf, den
der Audit als `:669` führt, steht am Auditstand wörtlich als
`if (telemetryV3.veroeffentlichen (frame, v3Hello().adresse))` (`git show 8f7c6f5f`,
Zeile 669) und am HEAD unverändert in `:905`. `git diff --stat e008811e HEAD` über
`prozessor/Analyse.cpp` und `tests/BriefkastenTestMain.cpp`: **leer**; `PluginEditor.cpp`
ist verändert (NAK-312), der Festhalten-Handgriff steht aber weiter in `:132-146`.
`SourcesModel.cpp` ist verändert, die Capability-Prüfung steht jetzt in `:178-198`.
`eq-ipc-v3.schema.json` änderte seit `e008811e` nur einen `$comment` an `auto_gain_db`.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-03-10 (W31) | **PRÄZISIERT**: die Mechanik trägt vollständig; „keine Zusage" und „unbestimmt" treffen nicht — Entwurf §53.6 und §33.1 samt Vertrag sprechen und widersprechen sich; die Beweisbedingung des Berichts ist seit SONDE-010 (T2 PASS 30.08.2026) erfüllt, das Bit wurde nie nachgezogen; Capabilities reisen im Heartbeat, nicht im Hello | DEFEKT gegen §53.6 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3981-3982`, Datenstand), dazu ein nicht registrierter Widerspruch §53.6:3994 ↔ §33.1/Vertrag | low (Audit: medium) | `Intern.h:401` gegen `host-capabilities-fl-v1.json:33` (neuer Vergleich in A13 oder B3c/B10) und die Relation „P2 gesendet ⇒ `supported` gemeldet" in A22/B10 | **nein** — Technik (Regelfrage 1) |
| T3-14-04 (W33) | **BESTÄTIGT**, drei Ergänzungen: schädlich sind zwei Instanzen mit gleichem bereinigtem Namen und die Uhrrückstellung, nicht der Doppelklick; die Erfolgsmeldung der ersten Instanz wird dabei unwahr; ein Teil-Schreibfehler meldet Erfolg und zerstört die Vorgängerdatei | LÜCKE mit Produktentscheid (Reaktion auf die Kollision); der Meldungsanteil ist DEFEKT gegen `CLAUDE.md:44-45` und unabhängig von der Karte | low (Audit: medium) | Bein B30 neben `BriefkastenTestMain.cpp:367-403`; Mutation am Schreibaufruf `Analyse.cpp:1429` | **ja** — Kartenentwurf unten, Empfehlung Weg 1 |

---

## 1. T3-03-10 — `binary_telemetry` meldet `unsupported`, P2 läuft binär (W31)

*Befundzeile: `BEFUNDE.md:387` · Skeptikerzeile Phase 10: `:521` · Bündel B26: `:96`, `:141`
· Kartenentwurf PF11-04: `:232-237` · Arbeitsübersicht W31: `:834` · Rohbelege:
`roh/phase-03-vertrag.md:29`, `:125`, `:152`, `:178`, `:202`; `roh/phase-10-skeptiker.md:62`,
`:131`; `roh/phase-11-synthese.md:86`, `:222-227`*

### a) Behauptung des Audits

Das Produkt meldet `binary_telemetry` als `unsupported`, während dieselbe Sonde
P2-FlatBuffers sendet und der Broker sie annimmt; die operative Bedeutung des
Capability-Werts an der Transportkante ist unbestimmt, eine Zusage gibt es nicht
(`BEFUNDE.md:387`, bestätigt als LÜCKE in `:521`).

### b) Quellkette am HEAD

**Station 1 — der Schreiber ist eine Konstante.**
`eq-copilot/plugin/core/ipc/controlclient/Intern.h:388-403`:

```
inline std::string capabilitiesJson()
{
    // Maschinenlesbare Wahrheit aus host-capabilities-fl-v1.json: zwei in FL
    // gemessene Faehigkeiten, acht feste Fallbacks. Diese Funktion wird nur
    // auf dem Clientthread gerufen.
    return "{\"host_context_presence\":\"supported\","
           ...
           "\"binary_telemetry\":\"unsupported\","
           "\"remote_control\":\"unsupported\"}";
}
```

Kein Zustand, kein Build- oder Laufzeitbezug: jede v3-Instanz (Gen und Probeeq) meldet
denselben Satz. Eingeführt am 31.08.2026 in `901598fc` (SONDE-011 Phase B, Nacharbeit
Runde 1) als Abschrift des Berichts — einen Tag **nach** SONDE-010 T2 PASS (Station 9).

**Station 2 — der Weg auf den Draht: Heartbeat, nicht Hello.** Der Bootstrap-Hello trägt
keine Capabilities: `eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:142-155`
(`helloAlsJson` schreibt `type`, `connection_kind`, `protocol`, `plugin_version`,
`plugin_kind`, `adresse`, optional `host`, `audio`), und das Schema kennt dort kein Feld:
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:850`
`"required": ["type", "connection_kind", "protocol", "plugin_version", "plugin_kind", "adresse", "audio"]`
bei `additionalProperties: false`. Die Capabilities reisen **im Heartbeat**:
`Vertrag.cpp:164` `+ ",\"capabilities\":" + capabilitiesJson()`, gesendet über P0 aus
`controlclient/Verbindung.cpp:259-260` und `:272`; Schema `:925`
`"required": ["type", "adresse", "sequence", "state_revision", "capabilities", "zaehler"]`,
`:932` `"capabilities": { "$ref": "#/$defs/capabilities" }`. Der Auftrag spricht von
„capabilities im Hello"; am HEAD trifft das nicht zu. Folge für jede Broker-Auswertung:
das Bit ist erst mit dem ersten Heartbeat bekannt (1 Hz, §33.2), die Telemetrie koppelt
aber schon nach dem `welcome`.

**Station 3 — der P2-Sender fragt nichts.** `eq-copilot/plugin/sonde/SondeProcessor.cpp:905-910`
im Worker:

```
                if (telemetryV3.veroeffentlichen (frame, v3Hello().adresse))
                {
                    letzterProducerFrame = frame;
                    letzterProducerFrameVorhanden = true;
                    producerPublikationen.fetch_add (1);
                }
```

Unbedingt, je gebautem Rahmen (10-Hz-Kadenz, §33.2). Der Audiothread legt nur den
Analyseblock in die Queue (`:668` `analyseQueue.veroeffentliche (&abgriff, 1, kanaele,
samples, stempel);`) und berührt den TelemetryClient nicht. Den Produzenten gibt es seit
`1de37897` (01.09.2026, SONDE-012 Phase B Etappe B3a).

**Station 4 — das Format ist fest FlatBuffers; einen JSON-Weg gibt es nicht.**
`eq-copilot/plugin/core/ipc/TelemetryClient.cpp:368-374`:

```
bool TelemetryClient::veroeffentlichen (const analyse::FeatureFrame& frame,
                                        const Adresse& quelle)
{
    std::vector<std::uint8_t> puffer;
    return featureFrameAlsFlatbuffer (frame, quelle, puffer)
        && veroeffentlichen (puffer.data(), puffer.size());
}
```

`:780-781` `if (! envelopeSchreiben (Familie::p2, kFeatureBatchSchemaMinor, frame.data(),
n, ausgang)`; die Kodierung folgt allein aus der Familie,
`eq-copilot/plugin/core/ipc/WireEnvelope.h:114-117`:

```
constexpr Kodierung erwarteteKodierung (Familie f) noexcept
{
    return f == Familie::p2 ? Kodierung::flatBuffers : Kodierung::json;
}
```

Im Client existiert keine Formatwahl und kein JSON-Telemetriepfad.

**Station 5 — der Vertrag verbietet JSON auf P2 ausdrücklich.**
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json:284-286`:

```
      "name": "telemetry_frame",
      "eigentuemer": "SONDE-005b",
      "grund": "P2-Featureframes sind FlatBuffers (`FeatureBatch` mit CRC32C), kein JSON — Entwurf §33.1. Der Name bleibt reserviert, damit ihn keine JSON-Familie belegt."
```

Das Schema lehnt reservierte Familien ab (`eq-ipc-v3.schema.json:5`), das Negativfixture
`eq-copilot/fixtures/v3/ungueltig/reservierter-typ-telemetry.json` hält das fest. Der
Rust-Envelope wertet JSON auf P2 als Verstoß: `broker/src/transport/v3.rs:83-88`
(`Familie::P2 => Kodierung::FlatBuffers`) und `:280-284`
(`if e != f.erwartete_kodierung() { verstoesse.push(Verstoss::EncodingPasstNichtZurFamilie); }`).
Eine „reduzierte JSON-Kadenz" für Livetelemetrie ist damit **in keiner der beiden
Implementierungen ausdrückbar**.

**Station 6 — der Broker nimmt P2 ohne Bitprüfung an.**
`broker/src/coordinator/senke.rs:125-164`: `fn p2` verifiziert das FeatureBatch
(`:126-132`), verlangt genau einen Frame (`:133-136`), den aktuellen Telemetrie-Client
(`:143-145`), die Routingfreigabe (`:149-156`) und die eigene Adresse (`:157-164`). Das Bit
kommt nicht vor. Die Routingfreigabe `broker/src/coordinator/flush.rs:289-298` verlangt
`routing_bereit`, keinen Trennwunsch, keinen Guard und die Aliasfreigabe — keinen Heartbeat.
Nach der Annahme speichert `:208-220` den Frame für die Messsicht, `:225-256` pusht ihn an
die Abonnenten, und `:257-260` ruft `evidence_dispatch_fuer_link`, dessen Ergebnis
verworfen wird; die Funktion zählt nur `evidence_angenommen`/`evidence_gesperrt`
(`broker/src/coordinator/intervention.rs:632-647`, `:669-673`).

**Station 7 — die Gegenrichtung ebenso.** Der Broker pusht P2 an Gen ohne Blick auf Gens
gemeldetes Bit: `broker/src/coordinator/subscription.rs:375-413`
(`push.messframe_schreiben (link_id, &instance_id, &payload)` in `:411`). Gen meldet über
denselben Schreiber ebenfalls `unsupported` und empfängt trotzdem binär.

**Station 8 — wer das Bit liest: niemand operativ.**
- Broker: `broker/src/coordinator/liveness.rs:409` holt `capabilities` aus dem Heartbeat,
  `:471-479` legt es in den Deskriptor, `descriptor_aus_heartbeat` kopiert es in `:719` und
  `:748`; die Deskriptorprüfung verlangt nur ein Objekt (`:703-705`), und der Kommentar sagt
  es ehrlich (`:648-652`): „GRENZE: der INHALT von `capabilities` (die zehn Faehigkeiten aus
  Entwurf Paragraph 53.6) wird hier nicht geprueft. Der Heartbeat-Weg prueft ihn ebenfalls
  nicht". `git grep binary_telemetry -- broker/src` trifft nur Testcode:
  `broker/src/coordinator/mod.rs:537` (Heartbeat im Testmodul),
  `broker/src/transport/server_v3/tests_abonnement.rs:664` und `tests_hilfe.rs:248`.
- Gen: `eq-copilot/plugin/src/SourcesModel.cpp:178-198` prüft zehn Namen und je
  `supported|unsupported` — die Form, nie den Wert; `git grep` findet im Produktcode keinen
  weiteren Leser.
- Bein A13: `tools/eq-copilot/pruefe_host_capabilities.py` liest nur den Bericht, nicht den
  C++-Schreiber (Station 10).

**Station 9 — die Evidenz hängt nicht am Bit.** `SondeProcessor.cpp:912-917`:

```
                // SONDE-013 M-05: der Evidenzsnapshot geht ueber P1 auf dem
                // Controllink, nicht ueber P2. §33.2 trennt beide Kadenzen
                // ausdruecklich — der Liveframe ist 10 Hz und binaer, die
                // Evidenz 1 bis 4 Hz und JSON.
                if (frame.evidenzFrisch)
                    evidenzSnapshotSenden (frame);
```

Der Broker empfängt Evidenz als P1-JSON (`senke.rs:106-108`); P2-Frames landen nur in der
Messsicht (`broker/src/coordinator/sicht.rs:132-190`) und im Push. Kein Evidenz- oder
Hypothesenweg liest `messframes` (`git grep` über `broker/src`: nur `link.rs`,
`liveness.rs`, `senke.rs`, `sicht.rs`, `subscription.rs`, `zustand.rs`).

**Station 10 — der Bericht und sein Riegel.** `eq-copilot/identity/host-capabilities-fl-v1.json:33`
`"binary_telemetry": "unsupported",`, Beleg `:97-102`:

```
    "binary_telemetry": {
      "beweis_nach_53_6": "v3-CRC/Fuzz/Lasttest",
      "termin": "keiner (kein Hostbit)",
      "rohfeld": "Beweis entsteht mit SONDE-010 (Transport-Fuzz, Backpressure); bis dahin unsupported",
      "fallback_nach_53_6": "reduzierte JSON-Kadenz, kein P0-Verlust"
    },
```

Dieselbe Datei legt als Regel fest (`:16`): „Ein Bit aendert sich nur mit einer neuen
Messung (Datei in docs/beweise/termin-*/), nie mit einer Versionsnummer" — für ein Bit
ohne Hostmessung ist das nie erfüllbar; ebenso `:15` „was kein Geraet gemessen hat, ist
unsupported mit seinem festen Fallback". A13 zementiert das:
`tools/eq-copilot/pruefe_host_capabilities.py:237-239` verlangt für jedes `supported`
einen Termin `A`, `B` oder `A + B`, und `:258-265` schreibt
`"binary_telemetry": "unsupported"` als erwarteten Wert fest. In
`docs/beweise/termin-a/` und `termin-b/` liegt kein Nachweis des Bits (`grep` ohne Treffer)
— er soll dort auch nicht liegen.

**Station 11 — der Beweisstand hat den Bericht überholt.** Die Bedingung aus `:100` ist
erfüllt: `docs/beweise/SONDE-010.md:3-5` trägt `T1 PASS 2026-08-29` und `T2 PASS
2026-08-30`, Gate `:11` „CRC/Fuzz/Backpressure/Reconnect ohne P0-Starvation". Die Beine
dazu stehen in `tools/beweise.ps1:627-637` („S14-15/SONDE-010: der v3-Nachrichtenweg —
Drei neue A-Beine plus B10"): A21 (`:636`, „3000 gekippte P2-Payloadbits fallen einzeln an
der CRC", 20 000 Zufallspuffer) und A22 (`:637`, „32 echte C++-Sondenpaare … fluten P2 …
und WAEHRENDDESSEN geht kein einziger P0-Frame verloren"). Das ist nach Wortlaut
„v3-CRC/Fuzz/Lasttest" samt „kein P0-Verlust". `docs/beweise/SONDE-004.md:43` hatte
festgehalten: „`binary_telemetry`, `remote_control` **unsupported** bis zu ihren Beweisen
(SONDE-010 bzw. 016/017)". Nachgezogen wurde nie; weder `SONDE-010.md` noch ein späteres
Manifest erwähnt das Bit außerhalb der A13-Ausgaben.

**Erreichbarkeit: Stufe 1.** Jeder Heartbeat jeder v3-Instanz trägt den Wert; jede Probeeq
mit v3-Verbindung sendet P2. Eine Nutzerwirkung entsteht heute nicht, weil kein Leser den
Wert auswertet.

### c) Zusage wörtlich

Die Quellen schweigen **nicht**. Es sprechen:

- Entwurf §53.6, `docs/FL-Nakama-Sonden-Design-Entwurf.md:3981-3982`: „Capabilities sind
  maschinenlesbare Tatsachen aus Build **und** laufendem Host. Ein Capabilitybit wird nur
  nach seinem Golden gesetzt."
- dieselbe Tabelle, `:3994`: „| `binary_telemetry` | v3-CRC/Fuzz/Lasttest | reduzierte
  JSON-Kadenz, kein P0-Verlust |" (Spaltenköpfe `:3984`: „Capability | Beweis | Verhalten
  ohne Beweis").
- `:3997-3998`: „Main leitet aus den Einzelbits sichtbare Produktfähigkeiten ab. Ein neuer
  Minor-Build darf ein Bit nicht anhand seiner Versionsnummer vermuten."
- Entwurf §33.1, `:2012-2013`: „kanonisches UTF-8-JSON für Handshake, Session, Zustand,
  Evidenz und Transaktionen; ein versioniertes FlatBuffers-`FeatureBatch` mit CRC32C für die
  häufigen P2-Featureframes." und `:2035` „P0/P1 verwenden JSON; P2 verweist auf ein
  `FeatureBatch`."
- `:2040-2042`: „Discriminator, Zieladresse, Revision, Capability und sicherheitsrelevante
  Felder sind **nicht** additiv und erzwingen bei unbekannter Bedeutung Ablehnung."
  Gleichlautend `CLAUDE.md:180-181` und `eq-ipc-v3.schema.json:99` („NICHT additiv …
  ein spaeterer Nachtrag KEINE Minor-Erweiterung, sondern ein Major-Schritt").
  `eq-copilot/schemas/v3/README.md:390-391`: „Was S4 liefert, sind **Werte**; die Namen
  stehen jetzt."
- `CLAUDE.md:61-63`: „Über beiden stehen aktueller Vertrag und Code. Widersprüche werden als
  Zeile in `docs/offene-punkte.md` geführt, nie kaschiert."

**Was daraus folgt.** Unter dem geltenden Vertrag (Station 5) kann das Produkt die Zeile
`:3994` nicht erfüllen: ihr Rückfall hat keine Drahtform. Entweder gilt der SONDE-010-Beweis
als Golden — dann ist das gemeldete `unsupported` keine „maschinenlesbare Tatsache"
(`:3981`) —, oder er gilt nicht — dann verletzt der binäre P2-Versand das „Verhalten ohne
Beweis" (`:3994`). In beiden Lesarten bricht ein Satz aus §53.6. Welche Lesart gilt, hat
keine Quelle entschieden. Der Widerspruch §53.6:3994 ↔ §33.1/Vertrag steht nicht im
Register: `grep binary_telemetry docs/offene-punkte.md` trifft nur NAK-313 selbst (`:317`).

`CLAUDE.md:191-193` („Begriffe nicht vermischen") nennt kein Paar, das das Bit betrifft; der
Befund ist aber dieselbe Art Vermischung (historischer Beweisstand in einer Transportaussage).
Eine Userentscheidung zum Bit gibt es nicht (`docs/plan/fragen.json`, `design/abnahmen/`:
kein Treffer).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A13 `pruefe_host_capabilities.py`, Behauptung `tools/beweise.ps1:584` („jedes supported hat einen Termin, jedes unsupported seinen festen Fallback") | Bericht gegen Schemaform und Termin-Rohdaten; `erwartete_bits` fest (`:258-265`) | liest den C++-Schreiber nicht; schließt ein `supported` ohne Hosttermin strukturell aus (`:237-239`) — das Bein würde die Korrektur selbst rot werfen |
| A3 `EqCopMarkierungTest` (`tools/beweise.ps1:504-509`), `MarkierungTestMain.cpp:1092-1140` gegen `eq-copilot/fixtures/v3/heartbeat-wire-v1.json:47`, `:52`, `:57` | Heartbeat-Bytes des C++-Schreibers gegen eine **handgeschriebene** Byteinstanz — sie enthält `binary_telemetry\":\"unsupported\"`; Rust-Leser derselben Datei `broker/tests/sonde013_verdrahtung.rs:4611` | pinnt den Wert an ein Handfixture, nicht an den Bericht und nicht an das gesendete Format; eine Korrektur des Werts muss diese bytegleiche Datei mitziehen |
| A5 `pruefe_v3_vertrag.py` (`:544`), Fixtures `capability-unknown.json`, `capabilities-unvollstaendig.json` u. a. | Form des Capabilityobjekts | keine Aussage über den Wert oder das Transportformat |
| A21/A22 (`:636`, `:637`) | der §53.6-Beweis selbst: CRC, Fuzz, Last mit P0-Erhalt | niemand leitet daraus den Bitwert ab |
| B10 `EqCopIpcTest` (`:738`) | Envelope, CRC, P0/P1/P2-Politiken | kein Capabilitybezug (`grep capabilit` in `IpcTestMain.cpp`: nur eine fremde Grundliste in `:6101`) |
| A4 `cargo test` (`tools/beweise.ps1:512-518`) | Coordinator, P2-Annahme, Deskriptor | alle Heartbeat-Helfer senden `"binary_telemetry": "supported"` (`broker/tests/coordinator_model.rs:70`, `broker/src/transport/server_v3/tests_abonnement.rs:664`, `tests_hilfe.rs:248`, `broker/tests/sonde012_sources_slice.rs:59`, `sonde013_verdrahtung.rs:76` …), das Produkt sendet `unsupported`; die produktnahe Kombination „unsupported + P2" kommt in keinem Test vor |

Es gibt am HEAD **keinen** Test, der den gemeldeten Wert mit dem tatsächlich benutzten
Transport vergleicht, und **keinen**, der den C++-Schreiber mit dem Bericht vergleicht; die
Gleichheit beider hält nur, weil zwei Handlisten (A13, `heartbeat-wire-v1.json`) zufällig
dasselbe festschreiben.

### e) Urteil: PRÄZISIERT

Die drei Tatsachen der Befundzeile tragen am HEAD ohne Lücke: Meldung `unsupported`
(Station 1–2), binärer P2-Versand (Station 3–4), Annahme ohne Bitprüfung (Station 6–7).
Anders als die Befundzeile und die Skeptikerzeile:

1. **Die Quellen schweigen nicht, sie widersprechen sich** (c). „Keine Zusage"
   (`BEFUNDE.md:387`, `:521`) übersieht §53.6:3981-3982 und §33.1:2012-2013/`:2035` samt
   `reservierte-nachrichten-v1.json:284-286`. Der Rückfall „reduzierte JSON-Kadenz" ist unter
   dem Vertrag nicht baubar; er ist kein offenes Produktmerkmal, sondern eine
   Entwurfszeile, die der Vertrag überholt hat.
2. **Die Bedingung des Berichts ist abgelaufen** (Station 11). `unsupported` war an „bis
   SONDE-010" gebunden; SONDE-010 ist seit dem 30.08.2026 abgenommen, der Schreiber entstand
   am 31.08. als Abschrift, der Produzent am 01.09. Der Bericht ist historischer Stand
   (`gemessen_am` 22.08.2026, `:4`), sein Text `:100` ist veraltet.
3. **Die Capabilities reisen im Heartbeat**, nicht im Hello (Station 2) — wichtig für jeden
   Riegel am Empfänger.
4. **Das Verhältnis zur Evidenz ist geregelt**, anders als W31 (`:834`) annimmt: Evidenz
   ist P1-JSON, P2 zählt nie als Evidenz (Station 9).

Kein späteres Ticket hat die Stellen verändert (Diffs im Kopf).

**Irreführende Kommentare und Texte, die der Befund berührt:**
`Intern.h:390-391` („acht feste Fallbacks") — für `binary_telemetry` wird der Rückfall nie
ausgeführt; `host-capabilities-fl-v1.json:100` („bis dahin unsupported") — die Bedingung ist
erfüllt; `host-capabilities-fl-v1.json:15-16` und `pruefe_host_capabilities.py:254-257` („Die
Bits, die die Rohdaten TRAGEN … alles andere ist unsupported") — rahmen alle zehn Bits als
Hostmessung, obwohl zwei „kein Hostbit" sind; im Audit selbst nennt
`roh/phase-10-skeptiker.md:131` das Bit „`binary_transport`" (Schreibfehler).

### f) Kategorie und Schwere

**DEFEKT gegen §53.6 (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3981-3982`)** — der
Capabilitysatz meldet für ein bewiesenes Buildmerkmal einen Wert, der keine Tatsache ist;
dazu ein **nicht registrierter Widerspruch** Entwurf §53.6:3994 ↔ §33.1/Vertrag
(`CLAUDE.md:61-63`). Kein Produktentscheid: welche Lesart gilt, ändert für den User nichts,
solange niemand P2 an das Bit bindet (Regelfrage 1).

**Schwere low** (Audit: medium): heute wertet kein Produktweg den Wert aus; wirksam wird er
erst, wenn Gen daraus die zugesagte Capability-Degradation ableitet (`:3997`; U27-Ergebnis
`docs/plan/fragen.json:678` „Sichtbar bleiben … Capability-Degradation am Zielumschalter")
oder jemand einen Empfangsriegel daraufsetzt — dann verlöre Gen still die Livedaten aller
Sonden. Deshalb Vorbedingung vor jedem Leser (B26, `BEFUNDE.md:96`: „Vor neuer
EQ-Telemetrie muss das bestehende Capabilitysignal verbindlich sein").

### g) Kleinster Fix

**Regel (Technik, vom Dirigenten zu setzen):** `binary_telemetry` ist eine
**Build-Tatsache** — „dieser Build überträgt P2 als geprüftes FlatBuffers-`FeatureBatch`" —,
bewiesen durch die SONDE-010-Beine A21 und A22. Das Bit ist keine Transportzulassung;
zugelassen wird P2 weiter über Kopplung, Routing, Adresse und Envelope (`senke.rs:125-164`).

**C++-Seite (Schreiber und Sender).**
- `Intern.h:401` auf `supported`, Kommentar `:390-392` berichtigen (zwei Hostmessungen, ein
  Buildbeweis, sieben feste Rückfälle).
- Sender und Meldung an **eine** Regel binden: ein Test (B3c `EqCopSchemaTest` oder B10)
  liest `host-capabilities-fl-v1.json:24-35` und vergleicht mit `capabilitiesJson()`; damit
  können Bericht und Schreiber nicht mehr auseinanderlaufen.
- `heartbeat-wire-v1.json` (bytegleich, `-text` nach `.gitattributes`) im selben Satz
  nachziehen; A3 und `sonde013_verdrahtung.rs:4611` lesen sie.

**Broker-Seite (Auswertung).** Kein Riegel. Den heutigen Zustand ausdrücklich machen: der
Kommentar `liveness.rs:648-652` und der Kopf von `senke.rs::p2` nennen die Regel, und ein
A4-Test mit produktnahem `"binary_telemetry": "unsupported"` im Heartbeat plus P2 hält
fest, dass die Annahme **nicht** vom Bit abhängt — sonst kann ein künftiger Riegel die
Liveansicht still abschalten, ohne dass ein Test fällt (alle Helfer senden `supported`,
d). Wer trotzdem einen Riegel will, braucht: eine Regel für P2 vor dem ersten Heartbeat
(Station 2), einen neuen Grund im **geschlossenen** Katalog `p2_reject`
(`eq-ipc-v3.schema.json:764-772`, Schemaschritt samt Gen-Leser), eine Regel für
„Reconnect mit anderem Satz" — und nimmt in Kauf, dass jeder Client mit `unsupported`
(heute alle) keine Livedaten mehr liefert.

**Bericht und Hostnachweis.** `host-capabilities-fl-v1.json:33` auf `supported`; Beleg
`:97-102`: `beweis_nach_53_6` bleibt, `termin` bleibt „keiner (kein Hostbit)", `rohfeld`
nennt statt „Beweis entsteht mit SONDE-010 … bis dahin unsupported" die Beine A21/A22 und
SONDE-010 T2 PASS 30.08.2026; `:16` erhält die Ausnahme für die zwei Nicht-Hostbits;
`:130` (Zusammenfassung) nachziehen. **Keine erfundene Hostfähigkeit:** der Eintrag
behauptet weiter keine FL-Messung. `remote_control` bleibt `unsupported` (Beweis SONDE-016/017
steht aus).

**A13.** `pruefe_host_capabilities.py:237-239`: `supported` ohne Termin nur für Bits mit
`termin` „keiner (kein Hostbit)" und benanntem Kanonbeweis; `:258-265` nachziehen;
Behauptungszeile `tools/beweise.ps1:584` entsprechend.

**Capabilityvertrag.** Unverändert: nur ein **Wert** ändert sich, kein Name
(`README.md:390-391`). Die PF11-04-Option „getrennter Evidenzstatus" (`BEFUNDE.md:236`) als
zusätzliches Feld wäre ein elftes Merkmal und damit ein Major-Schritt
(`eq-ipc-v3.schema.json:99`, `CLAUDE.md:180-181`) — nicht nötig, weil Evidenz ohnehin
getrennt ist (Station 9).

**Entwurf und Register.** §53.6:3994 als Erratum auf den Vertrag bringen (etwa: „ohne
Beweis: keine P2-Livetelemetrie; Zustand und Evidenz laufen über P0/P1-JSON weiter, kein
P0-Verlust") und den Widerspruch als Zeile in `docs/offene-punkte.md` führen
(`CLAUDE.md:61-63`).

**Verworfene Wege.** (i) `unsupported` behalten und den Rückfall wörtlich nehmen: P2
abschalten — Gen verlöre alle Livedaten der Sonden; das wäre eine Produktänderung und nur
mit dem User zu entscheiden, empfohlen ist sie nicht. (ii) `unsupported` als „historischen
Stand" behalten: verletzt §53.6:3981 weiter und lässt jeden künftigen Leser lügen.

**Mitzuprüfende Beziehungen:** **melden↔senden** in beiden Richtungen (Probeeq sendet P2,
Gen empfängt P2 über `subscription.rs:375-413`; beide melden über denselben Schreiber);
**verbinden↔trennen mit anderem Capability-Satz** — der Deskriptor folgt dem jüngsten
Heartbeat (`liveness.rs:471-479`); ein Reconnect nach einem Update mit geändertem Wert muss
im Snapshot den neuen Wert tragen und darf die P2-Annahme nicht unterbrechen.

### h) Ort des Rotbeweises

Die Zeile, die die Zusage trägt, ist `Intern.h:401` (der gemeldete Wert) gegen
`host-capabilities-fl-v1.json:33` (die dokumentierte Tatsache).

- **Schreiber gegen Bericht:** neuer Fall in B3c oder B10, der den Bericht liest und mit
  `capabilitiesJson()` vergleicht. Mutation `Intern.h:401` zurück auf `unsupported` (nach
  dem Fix) → rot. Gegenfall: `remote_control` bleibt in beiden `unsupported`.
- **Melden↔senden:** A22 (`tools/eq-copilot/pruefe_ipc_last.py`, 32 echte C++-Paare über
  die Probe-Pipe) oder B10: jeder Client, der P2-Frames veröffentlicht hat, meldet in seinen
  Heartbeats `binary_telemetry: supported`. Dieselbe Mutation → rot. Heute wäre der Fall rot,
  weil das Produkt `unsupported` meldet und sendet.
- **Kein stiller Riegel:** A4-Fall „Heartbeat mit `unsupported`, danach P2" → angenommen,
  Messsicht gefüllt. Eine Mutation, die in `senke.rs:125-164` eine Bitprüfung einzieht,
  → rot.
- A13 wird nach dem Fix mit dem geänderten Bericht grün und fällt, wenn `:33` ohne
  benannten Kanonbeweis auf `supported` steht.

---

## 2. T3-14-04 — mehrere erfolgreiche Festhalten-Exporte ersetzen dieselbe Datei (W33)

*Befundzeile: `BEFUNDE.md:591` · Phase-14-Urteil und Beleggrenze: `:580`, `:584` ·
Abdeckungssatz: `:597` · Folgeprüfbereich 3: `:603` · Arbeitsübersicht W33: `:836` ·
Rohbeleg: `roh/phase-14-blindspots.md:22`, `:33`, `:75-83` · keine Skeptikerzeile (Phase 14
lief nach Phase 10)*

### a) Behauptung des Audits

Mehrere erfolgreiche Festhalten-Exporte können dieselbe Datei ersetzen, weil alle Instanzen
denselben Ordner nutzen, der Name nur aus Kalendersekunde und höchstens 40 bereinigten
Labelzeichen besteht und `replaceWithText` vorhandenen Inhalt ersetzt; eine Erhaltungs- oder
Fehlermelderegel fehlt (`BEFUNDE.md:591`).

### b) Quellkette am HEAD

**Station 1 — der Handgriff und seine Meldung.**
`eq-copilot/plugin/src/PluginEditor.cpp:132-146`:

```
    initKnopf (festhaltenKnopf, "Festhalten", [this]
    {
        juce::String meldung;
        const auto m = processor.messSnapshot();
        if (processor.schreibeSnapshotDatei (meldung))
        {
            vergleichRohDb = m.ltasKompositDb;   // Roh gemerkt — Glättung wirkt
            vergleichAktiv = m.ltasGueltig;      // beim Zeichnen auf BEIDE gleich
            statusMeldung = "Festgehalten (Vergleichslinie + Datei): " + meldung;
        }
        else
            statusMeldung = u8 ("Festhalten nicht möglich — ") + meldung;
        statusMeldungBisMs = juce::Time::getMillisecondCounter() + 8000;
        uiDirty = true;
    });
```

Sichtbar ist der Knopf auf der Legacy-Fläche, also in jeder Gen-Instanz, deren Rolle nicht
`hub` ist oder deren Stand nur lesbar ist: `:955-958`
(`return processor.holeRolle() == "hub" && ! processor.stateNurLesen();`) und `:981-987`
(`festhaltenKnopf` in der Liste, `c->setVisible (! sollMain);`); die Rolle folgt aus der
Klasse, `eq-copilot/plugin/state/NakamaState.cpp:2303-2313`. Probeeq hat keinen Editor
(`eq-copilot/plugin/sonde/SondeProcessor.h:219-220`) und exportiert nicht.
**Erreichbarkeit: Stufe 1.**

**Station 2 — Inhalt.** `eq-copilot/plugin/src/prozessor/Analyse.cpp:1395-1408`: eigener
Snapshot (`:1397`, der zweite nach `PluginEditor.cpp:135`), ohne Messdaten ehrlicher Abbruch
„noch keine Messdaten" (`:1398-1402`), Inhalt aus `snapshotObjektBauen` mit `created_utc`
aus einem eigenen `getCurrentTime()` (`:1408`).

**Station 3 — ein Ordner für alle.** `:1410-1417`:

```
    auto ordner = juce::File::getSpecialLocation (juce::File::windowsLocalAppData)
                      .getChildFile ("evenacadia").getChildFile ("EQ-Copilot")
                      .getChildFile ("snapshots");
    if (! ordner.createDirectory())
```

Kein Instanz-, Prozess- oder Projektmerkmal. Derselbe Ordner nimmt auch die
Broker-Aggregate auf (`broker/src/aggregat.rs:137-143`, eigener Präfix `aggregat-`).

**Station 4 — der Name.** `:1418-1428`:

```
    juce::String labelTeil;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        for (const auto z : zustand.common.label)
            if (juce::CharacterFunctions::isLetterOrDigit (z) || z == '-')
                labelTeil += z;
    }
    const auto zeit = juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S");
    const auto datei = ordner.getChildFile ("snapshot-" + zeit
                                            + (labelTeil.isEmpty() ? "" : "-" + labelTeil.substring (0, 40))
                                            + ".json");
```

**Station 5 — das Schreiben ersetzt.** `:1429-1435`:

```
    if (! datei.replaceWithText (juce::JSON::toString (juce::var (wurzel), false), false, false, "\n"))
    {
        pfadOderFehler = "Schreiben fehlgeschlagen: " + datei.getFullPathName();
        return false;
    }
    pfadOderFehler = datei.getFullPathName();
    return true;
```

Was `replaceWithText` tut, steht in der JUCE-Quelle (Bauartefakt
`eq-copilot/build/_deps/juce-src/`, JUCE 8.0.9 laut `host-capabilities-fl-v1.json:11`):
`modules/juce_core/files/juce_File.cpp:798-803` schreibt in eine versteckte Temp-Datei neben
dem Ziel und ruft `overwriteTargetFileWithTemporary()`; der Rückgabewert von `appendText`
wird in `:801` **nicht** geprüft. `juce_TemporaryFile.cpp:94-114` versucht bis zu fünfmal,
mit je 100 ms Pause (`:103`, `:108`), `replaceFileIn`; `juce_File.cpp:323-335` ersetzt ein
vorhandenes Ziel über `ReplaceFile` (`native/juce_Files_windows.cpp:372-378`) und meldet
Erfolg. Die Temp-Namen tragen eine Zufallszahl (`juce_TemporaryFile.cpp:61-70`), zwei
Schreiber stören sich dort also nicht — am Ziel gewinnt der letzte.

**Station 6 — die Erfolgsmeldung.** `PluginEditor.cpp:140` zeigt den vollen Pfad aus
`Analyse.cpp:1434`, acht Sekunden lang (`:144`). Bei ersetzter Datei steht derselbe Text wie
bei einer neuen; kein Hinweis auf die verdrängte Datei.

**Herkunft.** Die Zeilen `:1409-1436` kamen am 09.09.2026 mit `84226d46` (NAK-225,
Herauslösung aus `PluginProcessor.cpp`) unverändert hierher; NAK-286 (`a2887462`, 14.09.)
änderte nur `:1404-1408`. Seit dem Phase-14-Stand `e008811e` ist `Analyse.cpp` unverändert.

**Zahlenränder (alle an der Quelle hergeleitet, nicht gemessen):**

- **Kalendersekunde.** `formatted` rechnet in **Ortszeit** (JUCE `juce_Time.cpp:341-345`,
  `millisToLocal` über `_localtime64_s`, `:40-47`), auf die Sekunde, ohne Zeitzone. Zwei
  Exporte derselben Sekunde mit gleichem Labelteil treffen denselben Namen. Schärfster Fall:
  beim Zurückstellen der Uhr (Ende der Sommerzeit, NTP- oder Handkorrektur) wiederholen sich
  Namen; ein Export kann dann eine **ältere, andere** Datei derselben Wanduhrsekunde
  ersetzen. Nebenbei stammen Name (`:1425`) und `created_utc` (`:1408`) aus zwei
  verschiedenen Uhrabfragen und können eine Sekunde auseinanderliegen.
- **40 Zeichen.** `substring (0, 40)` nach dem Filtern: Labels, die sich erst ab dem 41.
  behaltenen Zeichen unterscheiden, kollidieren. Labels dürfen bis 120 Zeichen lang sein
  (`eq-copilot/schemas/state/nakama-state-v2.md:20`).
- **Leeres Label.** Ohne Labelteil heißt die Datei `snapshot-<zeit>.json`; alle unbenannten
  Instanzen teilen sich je Sekunde einen Namen. Dasselbe gilt für Labels aus reinen
  Sonderzeichen (`"!!!"`) — und, weil Leerzeichen und Unterstrich herausfallen, für
  `"Bass 1"`, `"Bass_1"` und `"Bass1"`.
- **Unicode im Label.** `isLetterOrDigit (juce_wchar)` ruft
  `iswalnum ((wint_t) character)` (JUCE `juce_CharacterFunctions.cpp:109-112`); unter MSVC
  ist `wint_t` 16 Bit, Zeichen außerhalb der BMP (etwa Emojis) werden vor der Einordnung
  abgeschnitten und damit zufällig behalten oder verworfen. Welche übrigen Nicht-ASCII-Zeichen
  als Buchstaben gelten, entscheidet die MSVC-Laufzeit — nicht gemessen. Sicher ist: NTFS
  unterscheidet Groß- und Kleinschreibung standardmäßig nicht; `"Bass"` und `"bass"` ergeben
  zwei JUCE-Namen, aber **eine** Datei.
- **Zwei Instanzen gleichzeitig.** Beide schreiben ihre eigene Temp-Datei, beide ersetzen das
  Ziel, beide melden Erfolg mit demselben Pfad; die Datei enthält den Messstand der späteren.
  Gleiche Labels entstehen gewöhnlich: ein FL-Klon trägt dieselben Bytes und damit dasselbe
  Label (`nakama-state-v2.md:319`). Zwei FL-Prozesse desselben Windows-Kontos teilen den
  Ordner ebenfalls.
- **Fehlgeschlagener Write.** (1) Ordner nicht anlegbar → ehrliche Meldung (`:1413-1417`).
  (2) Ersetzen scheitert fünfmal → ehrliche Meldung (`:1429-1433`), nach bis zu rund 0,5 s
  Blockade des Message-Threads (`juce_TemporaryFile.cpp:103-108`). (3) **Teil-Schreibfehler**
  (etwa voller Datenträger mitten im Schreiben): `appendText` scheitert, sein Ergebnis wird
  verworfen (`juce_File.cpp:801`), die angefangene Temp-Datei existiert und ersetzt das Ziel;
  `replaceWithText` meldet Erfolg. Folge: Erfolgsmeldung, abgeschnittene JSON-Datei **und**
  eine gleichnamige Vorgängerdatei zerstört.

### c) Zusage wörtlich

Eine Zusage zum Erhalt oder Überschreiben von Exportdateien gibt es **nicht**. Gelesen:

- NAK-286 M-39, `docs/beweise/NAK-286.md:563`: „Datei im Ordner
  `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\` mit Namen
  `snapshot-<zeit>[-<label>].json`; Inhalt nach Maskierung von `created_utc` bytegleich zur
  Referenz … der Test entfernt seine eigene Datei wieder | ein Aufruf". Die Zeile schützt
  unter R-286-2 (`:368`, Verhaltensneutralität des Briefkastens) den **einzelnen** Aufruf
  samt Namensschema, keine Folge von Aufrufen. F-5 (`:358`): „der Knopfweg schreibt danach
  bytegleich wie vorher". Eine Korrektur muss dieses Namensschema bewusst mitführen.
- NAK-286 F-3 und F-8 (`:356`, `:361`): für die Antworten des **Briefkastens** gilt „Keine
  Suffixleiter, kein Deckel … scheitert das Umbenennen dennoch an einem vorhandenen Namen,
  ist das ein gescheiterter Schreibversuch nach F-13, nie ein Ersetzen" und „(5) ohne
  Ersetzen in den Antwortnamen umbenennen (Windows: `MoveFileExW` ohne
  `MOVEFILE_REPLACE_EXISTING`)". Das ist eine Technikregel des Dirigenten für einen anderen
  Pfad, kein Userwort zum Export.
- `CLAUDE.md:44-45` (User 24.08.2026): „jedes sichtbare Element führt einen Handgriff aus
  oder meldet ehrlich einen Zustand." Das trifft die Erfolgsmeldung (Station 6): im Fall
  „zwei Instanzen" nennt die Meldung der ersten Instanz eine Datei, die den Messstand der
  zweiten enthält.
- `CLAUDE.md:28-32` („Grundgesetz: nichts Ungefragtes. Verarbeitung nur, wenn der User sie
  einschaltet …") spricht wörtlich über Audioverarbeitung, nicht über Dateien. Dem Sinn nach
  berührt es den Fall: ein Klick auf „Festhalten" ist kein Auftrag, eine frühere Datei zu
  löschen.
- `design/docs/funktions-und-bedien-blueprint.md:516-518` führt für P9 „Export, Retention,
  Delete und redigiertes Diagnosepaket" und „keine Queue, Registry- oder WAL-Datei wächst
  unbegrenzt" — Plan, kein Entscheid zum heutigen Knopf.
- `docs/plugin-wissen.md:233-236`: „Kein Befundarchiv im Plugin." — beschreibt Befunde, nicht
  Exportdateien; der Zeiger dort (`PluginProcessor.cpp:697-857`) ist seit NAK-225 veraltet.
- Der Satz „nicht automatisch unbegrenzte Archivierung einführen" steht in der
  Arbeitsübersicht, `BEFUNDE.md:836`, nicht im Register; `docs/offene-punkte.md:317` sagt nur
  „Archiv- oder Überschreibregel W33". Ebenso ist „keine heimliche Änderung vorhandener
  Dateien" eine Invariante des Audits (`:836`), kein Userzitat.
- **Technischer Präzedenzfall im selben Ordner:** der Broker schreibt seine Aggregate seit
  `2f3ee24e` (27.08.2026) ohne Ersetzen — `broker/src/aggregat.rs:146-174` versucht zuerst
  `aggregat-{ms}.json`, bei Kollision einen Namen mit UUID-Zusatz, bis zu 32-mal, über
  `persistenz::atomar_neu` (`broker/src/persistenz.rs:155-158`: „Veröffentlicht `daten` nur
  dann, wenn `ziel` noch nicht existiert. … kein bestehendes Byte wird dabei ersetzt"); der
  Archivbericht dazu, `docs/archiv/ultracode-2026-08-27-sondenplan-debug/packets/03-targets-lifecycle-installer.md:97-110`
  (B03-01), nennt ausdrücklich „zwei und zwölf Snapshots derselben Millisekunde". Das ist
  Verlauf und Technik, keine Produktentscheidung.

Eine datierte Userentscheidung fand ich weder in `docs/plan/fragen.json` (offen, geparkt,
beantwortet) noch in `design/abnahmen/`, `docs/ZIELBILD.md` oder im Register. Die Karte
U17.2 (`docs/plan/fragen.json:616-623`, „Das Kurvenbild laesst sich festhalten") meint ein
anderes Merkmal — das Festhalten der Spektrumspitzen — und entscheidet nichts über Dateien.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| B30 `EqCopBriefkastenTest`, Behauptung `tools/beweise.ps1:777`; Fall `festhaltenBytegleich` `BriefkastenTestMain.cpp:367-403` | **ein** Aufruf von `schreibeSnapshotDatei` (`:272-277`), Datei lesen und löschen (`:278-283`), Ordner gleich `snapshotOrdner()` und Name nach `nameWieKnopfweg` (`:383-387`), Inhalt bytegleich zur Referenz | keine zweite Datei, keine zweite Instanz, keine festgehaltene Uhr, kein Schreibfehler |
| `nameWieKnopfweg`, `BriefkastenTestMain.cpp:313-327` | Name = `snapshot-` + 15 Zeichen `JJJJMMTT-HHMMSS` + `-` + Testlabel + `.json` | schreibt das heutige Schema fest; ein Zusatz hinter dem Label würde hier rot |
| B15 `EqCopShot` | statische Darstellung | kein Klick |
| A4, `broker/src/aggregat.rs:298-319` (`gleicher_zeitstempel_ueberschreibt_keinen_snapshot`) und `:321-364` (`parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten`) | genau die fehlenden Fälle — aber für den Broker-Schreiber im selben Ordner, in einem Temp-Ordner | nicht den Plugin-Export |

**Nebenbefund zur Testhygiene:** B30 schreibt in die **echte** Nutzerablage
(`BriefkastenTestMain.cpp:307-311` löst `windowsLocalAppData` auf, `:383` verlangt genau
diesen Ordner) und löscht danach die eigene Datei (`:283`). Trifft ein Lauf denselben Namen
wie eine vorhandene Datei — praktisch nur ein zweiter gleichzeitiger B30-Lauf in derselben
Sekunde, das Testlabel ist `NAK286Referenz` (`:152`) —, ersetzt und löscht der Test sie. Ein
Kollisionstest darf dort nicht laufen.

### e) Urteil: BESTÄTIGT, mit drei Ergänzungen

Alle vier Glieder der Befundzeile tragen am HEAD: gemeinsamer Ordner (`:1410-1412`), Name aus
Ortszeitsekunde und höchstens 40 bereinigten Labelzeichen (`:1418-1428`), ersetzendes
Schreiben mit Erfolgsrückgabe (`:1429-1435`, JUCE `juce_File.cpp:798-803`), einzige Prüfung
genau ein Aufruf (B30). Kein Ticket seit dem Auditstand hat daran etwas geändert.

Ergänzungen:

1. **Wo der Schaden liegt.** Beim Doppelklick derselben Instanz trennen die zwei Messstände
   Sekundenbruchteile; verloren geht eine fast gleiche Datei. Schädlich sind zwei Instanzen mit
   gleichem bereinigtem Namen (Klon, gleiches oder leeres Label, Groß-/Kleinschreibung) und
   die zurückgestellte Uhr, die eine ältere, andere Datei ersetzt.
2. **Die Meldung wird unwahr.** Im Fall „zwei Instanzen" zeigt die erste Instanz acht Sekunden
   lang „Festgehalten … <Pfad>", während der Pfad den Messstand der zweiten enthält — gegen
   `CLAUDE.md:44-45`.
3. **Teil-Schreibfehler.** Über `juce_File.cpp:801` meldet ein abgebrochenes Schreiben Erfolg
   und zerstört dabei eine gleichnamige Vorgängerdatei. Das geht über die Befundzeile hinaus.

**Irreführende Kommentare und Texte, die der Befund berührt:** im Code keiner —
`Analyse.cpp:1404-1407` beschreibt den NAK-286-Stand zutreffend („Ordner, Name und
Schreibweise bleiben"). Irreführend sind die Meldung `PluginEditor.cpp:140` („Festgehalten
(Vergleichslinie + Datei)"), die weder ein Ersetzen noch zwei getrennte Snapshots erkennen
lässt, und der veraltete Zeiger `docs/plugin-wissen.md:233` (`PluginProcessor.cpp:697-857`;
die Funktion steht seit NAK-225 in `Analyse.cpp:1395-1436`).

### f) Kategorie und Schwere

**LÜCKE mit Produktentscheid** für die Reaktion auf eine Kollision: Weiterschreiben unter
anderem Namen, ehrliches Ersetzen oder Ablehnen ändert, was ein Musiker nach dem Klick
vorfindet; keine Quelle entscheidet es (c). **Unabhängig von der Karte DEFEKT** gegen
`CLAUDE.md:44-45` sind zwei Anteile: die Erfolgsmeldung, die eine fremd überschriebene Datei
nennt, und der Teil-Schreibfehler mit Erfolgsmeldung. Beide sind Technik und in jeder
Kartenvariante zu beheben.

**Schwere low** (Audit: medium): der Fall braucht dieselbe Sekunde und denselben bereinigten
Namen, verliert höchstens eine Exportdatei je Kollision und berührt weder Audio noch State
noch Projekt; der schärfste Fall (ältere Datei nach Uhrrückstellung) ist noch seltener.

### g) Kleinster Fix

**Gemeinsamer Teil, unabhängig von der Kartenantwort** — in
`EqCopilotProcessor::schreibeSnapshotDatei` (`Analyse.cpp:1395-1436`):

- `replaceWithText` (`:1429`) durch ein **exklusives Veröffentlichen** ersetzen: Temp-Datei im
  Zielordner exklusiv anlegen, schreiben, spülen, Größe prüfen, **ohne Ersetzen** umbenennen —
  dieselbe Form wie der Briefkasten (`eq-copilot/plugin/core/diagnose/Briefkasten.cpp:586-618`,
  `CreateFileW` mit `CREATE_NEW` und `MoveFileExW` ohne `MOVEFILE_REPLACE_EXISTING`,
  `Briefkasten.h:173-174`) oder wie `persistenz::atomar_neu`. Damit ist die Kollision atomar
  auch zwischen Instanzen und Prozessen erkannt, NTFS-Groß-/Kleinschreibung eingeschlossen, und
  ein Teil-Schreibfehler wird zum ehrlichen Fehler statt zur Erfolgsmeldung.
- Der erste Versuch behält exakt den heutigen Namen: M-39 bleibt im Einzelfall bytegleich,
  `nameWieKnopfweg` (`BriefkastenTestMain.cpp:313-327`) hält für die erste Datei.
- Die Meldung (`PluginEditor.cpp:140`) nennt immer die tatsächlich entstandene oder ersetzte
  Datei; das Ergebnis von `schreibeSnapshotDatei` braucht dafür eine Unterscheidung „neu /
  ersetzt / abgelehnt / Fehler" statt `bool`.
- Nahtstellen für den Test: Uhr und Zielordner injizierbar, damit B30 nie die Nutzerablage
  berührt.

**Je Option der Regel:**

- **Weg 1 — eindeutiger Name mit Zusatz.** Bei Kollision ein neuer Name mit einem Zusatz aus
  einem Zeichen, das der Labelfilter nie durchlässt (`_` oder `~`; der Filter behält nur
  Buchstaben, Ziffern und `-`, `:1422`), also etwa `snapshot-<zeit>-<label>_2.json` — dann ist
  der Zusatz nie mit einem Label wie `Bass-2` verwechselbar. Versuche begrenzen (der Broker
  nimmt 32), danach ehrlicher Fehler. Meldung: „Festgehalten … <tatsächlicher Dateiname>".
- **Weg 2 — Überschreiben mit ehrlicher Meldung.** Erster Versuch exklusiv; bei Kollision
  bewusst ersetzen und melden „ersetzt <Datei> vom <Zeitpunkt>". Grenze: die Instanz, deren
  Datei später von einer anderen ersetzt wird, erfährt davon nichts; ihre Meldung bleibt für
  acht Sekunden unwahr. Weg 2 kann `CLAUDE.md:44-45` im Fall „zwei Instanzen" nicht voll
  halten.
- **Weg 3 — Ablehnung des zweiten Exports.** Bei Kollision nichts schreiben, Meldung „In dieser
  Sekunde gibt es schon einen Export mit diesem Namen — gleich noch einmal". Die
  Vergleichslinie bleibt unverändert; das tut der Handgriff schon, weil er sie nur bei Erfolg
  setzt (`PluginEditor.cpp:136-141`).

**Mitzuprüfende Beziehungen:** **exportieren↔bestätigen** — Meldung und Datei gehören
zusammen; derselbe Handgriff liest heute zwei Snapshots (`PluginEditor.cpp:135` für die Linie,
`Analyse.cpp:1397` für die Datei), obwohl die Meldung „Vergleichslinie + Datei" sagt
(Folgeprüfbereich 3, `BEFUNDE.md:603`); wer die Meldung umbaut, sollte einen Snapshot für
beide nehmen. **speichern↔laden** ist nicht berührt (die Datei ist kein Hoststate).

### h) Ort des Rotbeweises

**Bein B30** (`EqCopBriefkastenTest`, Behauptungszeile `tools/beweise.ps1:777`), neuer Fall
neben `festhaltenBytegleich` (`BriefkastenTestMain.cpp:367-403`), mit Temp-Ordner und
festgehaltener Uhr über die neue Nahtstelle:

- zwei Aufrufe derselben Instanz in derselben Sekunde mit unterscheidbaren Messständen →
  Weg 1: zwei Dateien, beide Inhalte erhalten, zwei verschiedene gemeldete Pfade; Weg 3: eine
  Datei mit dem ersten Inhalt, zweite Meldung „abgelehnt"; Weg 2: eine Datei, zweite Meldung
  „ersetzt". **Heute rot:** eine Datei mit dem zweiten Inhalt, zweimal dieselbe
  Erfolgsmeldung;
- zwei Prozessorinstanzen mit den Labels `"Bass 1"` und `"Bass_1"` (gleich nach dem Filter),
  dazu `"Bass"`/`"bass"` und leer/`"!!!"`;
- eine Dateisystem-Nahtstelle, die das Schreiben mittendrin scheitern lässt → keine
  Erfolgsmeldung, eine vorhandene gleichnamige Datei bleibt bytegleich;
- **Gegenfall:** ein einzelner Aufruf erzeugt weiterhin genau den heutigen Namen (M-39).

**Die Mutation, die die Zusage bricht,** ist die Rückkehr zum ersetzenden Schreiben an der
Stelle von `Analyse.cpp:1429` (oder ein `MOVEFILE_REPLACE_EXISTING` im neuen Weg): Weg 1 und 3
werden rot, weil die erste Datei verschwindet. Die Behauptungszeile `tools/beweise.ps1:777`
ist um den Satz zur Kollision zu ergänzen.

---

## Gemeinsame Ursachen

**Eine gemeinsame Denkfigur, kein gemeinsamer Code.** Beide Befunde melden einen Zustand, der
nicht aus der Wirkung abgeleitet ist: das Capabilitybit behauptet `unsupported`, während der
Build binär überträgt; die Erfolgsmeldung nennt einen Pfad, dessen Inhalt nicht mehr der
eigene sein muss. Beide werden von handgeschriebenen Erwartungen festgehalten, die den
heutigen Stand zementieren — A13 `erwartete_bits` und `heartbeat-wire-v1.json` hier,
`nameWieKnopfweg` und der Ein-Aufruf-Fall in B30 dort. Die Prüfregel ist in beiden Fällen
dieselbe: das Orakel vergleicht die Meldung mit der Wirkung (gesendetes Format; erhaltene
Datei), nicht mit einer Handliste und nicht mit einem Rückgabewert. Die Reparaturen sind
getrennt und gehören in getrennte Änderungssätze.

**Berührung mit Teil B (W30):**

- **T3-03-07** (zusätzliche VTable-Slots in der geschlossenen FlatBuffers-Adresse) sitzt am
  selben Empfang: `senke.rs:126` ruft `crate::telemetrie::fuer_broker`, dessen Adressprüfung
  T3-03-07 verschärft. Mit dem empfohlenen Weg für T3-03-10 (kein Riegel) gibt es keine
  gemeinsame Codeänderung; will der Dirigent doch einen Riegel, treffen beide dieselbe
  Funktion und denselben geschlossenen `p2_reject`-Katalog (`eq-ipc-v3.schema.json:764-772`)
  — dann **ein** versionierter Schritt für beide.
- **T3-03-02** (Integer als `1.0`/`1e0`, positives Fixture `heartbeat-integer-als-1punkt0.json`)
  betrifft den Heartbeat, der die Capabilities trägt, und damit denselben Eingang
  `liveness.rs` (`heartbeat_kontakt`); die bytegleiche Datei `heartbeat-wire-v1.json` lesen A3
  und `broker/tests/sonde013_verdrahtung.rs:4611`. Änderungen daran abstimmen.
- **T3-03-09** (doppelte JSON-Schlüssel): ein Heartbeat mit zweimal `binary_telemetry` behält im
  generischen Pfad den letzten Wert — heute folgenlos, weil niemand den Wert liest.
- Die v3-Fixtures unter `eq-copilot/fixtures/v3/` tragen viele Capabilityobjekte als
  Vertragsinstanzen; für den Wertwechsel des Produkts müssen sie nicht geändert werden, nur die
  Byteinstanz `heartbeat-wire-v1.json`.

**Berührung mit Teil A (W10), nur als Hinweis:** T3-02-04 (Panel-Abbau verliert Label) speist
dasselbe Label, aus dem der Exportname entsteht; ein verlorenes Label vermehrt die Kollisionen
unbenannter Instanzen.

---

## Regelfragen und Karten

### Frage 1 — `binary_telemetry`: Quellenlage, Technik oder Produkt

**Welche Messungen ohne binäre Fähigkeit zur Evidenz zählen:** die Frage ist beantwortet.
Evidenz ist P1-JSON (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2012`, Kadenz `:2065`,
`SondeProcessor.cpp:912-917`, Empfang `senke.rs:106-108`); P2 speist nur die Messsicht und den
Push an Gen (Station 6, 7, 9). Das Bit berührt keine Evidenz, egal wie es steht.

**Wer das Bit heute liest:** der C++-Schreiber schreibt eine Konstante (`Intern.h:401`); der
Broker kopiert das Objekt ungeprüft in den Deskriptor (`liveness.rs:409`, `:471-479`,
`:648-652`) und wertet es beim P2-Empfang nicht aus (`senke.rs:125-164`); Gen prüft nur die
Form (`SourcesModel.cpp:178-198`); A13 liest nur den Bericht und schreibt `unsupported` fest
(`pruefe_host_capabilities.py:237-239`, `:258-265`).

**Historischer Hostnachweis gegen produktive P2-Fähigkeit:** der Bericht
`host-capabilities-fl-v1.json` ist der S4-Stand vom 22.08.2026 (`:3-4`); sein Eintrag
`binary_telemetry` verschob den Beweis ausdrücklich auf SONDE-010 (`:100`), das am 30.08.2026
abgenommen wurde. Der Schreiber entstand am 31.08. als Abschrift, der P2-Produzent am 01.09.
Der Bericht ist für dieses Bit seitdem veraltet; der Riegel A13 kann ein Nicht-Hostbit nie auf
`supported` stellen.

**Technik oder Produkt: Technik.** Die möglichen Lesarten unterscheiden sich für den User
nicht, solange P2 nicht an das Bit gebunden wird. Der einzige Weg mit Produktwirkung — P2
abschalten, um den Rückfall wörtlich zu erfüllen — nähme Gen die Livedaten aller Sonden; ihn
schlage ich nicht vor. **PF11-04 (`BEFUNDE.md:232-237`) braucht keine Karte.** Zu entscheiden
hat der Dirigent die Regel aus 1g (Build-Tatsache, Wert `supported`, Beweis A21/A22) samt
Erratum und Registerzeile.

### Frage 2 — Archiv- oder Überschreibregel beim Export: Kartenentwurf

**Ist eine Option schon durch eine Zusage entschieden?** Nein (2c). Die Karte entfällt nicht.
Der Präzedenzfall im selben Ordner (Broker, `aggregat.rs:146-174`) und die Briefkastenregel
„nie ein Ersetzen" (NAK-286 F-8) sind Technik; sie machen Weg 1 billig, entscheiden aber nicht
für den User.

**Kartenentwurf** (Form wie U61–U64; die Nummer vergibt der Dirigent — am HEAD ist U65 die
nächste freie, Teil A und B schlagen womöglich ebenfalls Karten vor):

```json
{
 "id": "U65",
 "dringlichkeit": "wenn du dazu kommst",
 "art": "Produktentscheid (Verhalten)",
 "titel": "Zweimal „Festhalten“ in derselben Sekunde: sollen beide Messstände erhalten bleiben?",
 "was": "Im Fenster einer Gen-Instanz, die nicht als Hauptprogramm läuft (etwa als Messpunkt aus einem älteren Projekt), sichert der Knopf „Festhalten“ den aktuellen Messstand als Datei in einen gemeinsamen Ordner – nicht zu verwechseln mit dem geplanten Festhalten des Spektrumbilds. Der Dateiname besteht nur aus Datum, Uhrzeit auf die Sekunde genau und dem Namen der Instanz. Entsteht in derselben Sekunde ein zweiter Export mit demselben Namen – ein schneller Doppelklick, zwei Instanzen mit gleichem Namen (etwa eine geklonte Spur) oder zwei ohne Namen –, ersetzt die zweite Datei heute die erste ohne jeden Hinweis; beide Male heißt es „Festgehalten“ mit demselben Dateinamen. (1) Empfohlen: beide bleiben erhalten; die zweite Datei bekommt einen kleinen Zusatz im Namen, und die Meldung nennt genau die Datei, die entstanden ist. Gelöscht wird weiterhin nichts automatisch – das ist schon heute so; in diesem seltenen Fall liegt nur eine Datei mehr im Ordner. (2) Die zweite ersetzt die erste, aber die Meldung sagt es ausdrücklich („ersetzt den Export von 19:15:07“). Einschränkung: speichern zwei Instanzen gleichzeitig, erfährt nur die zweite davon. (3) Der zweite Export wird abgelehnt: „In dieser Sekunde gibt es schon einen Export mit diesem Namen – bitte gleich noch einmal.“ Die vorhandene Datei bleibt unberührt, der zweite Klick zählt nicht. Welche Variante willst du?",
 "warum": "Audit-Befund T3-14-04 (Arbeitsübersicht W33), an der Quelle bestätigt (Validierung Teil C). Keine Zusage entscheidet es: es gibt keine Regel von dir, ob ein Export eine vorhandene Datei ersetzen darf. Nakamas Hintergrunddienst legt seine eigenen Berichte im selben Ordner schon heute nach Weg 1 ab. In allen drei Wegen wird ein abgebrochenes Schreiben künftig als Fehler gemeldet statt als Erfolg. Ohne Antwort baut der Dirigent Weg 1 (Planschritt S25l, Ticket NAK-313).",
 "wo": "docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md T3-14-04, Arbeitsübersicht W33; docs/beweise/roh/NAK-313-quellvalidierung-teil-C.md; docs/offene-punkte.md NAK-313",
 "seit": "23.09.",
 "status": "offen"
}
```

**Empfehlung: Weg 1**, aus vier Gründen:

1. **„Nichts Ungefragtes" (`CLAUDE.md:28`) dem Sinn nach:** der Klick verlangt einen Export,
   nicht das Löschen eines früheren. Weg 1 und Weg 3 löschen nichts; Weg 2 löscht mit Ansage.
2. **„Keine heimliche Änderung vorhandener Dateien" und ehrliche Meldung (`CLAUDE.md:44-45`):**
   Weg 1 und Weg 3 halten beide in jedem Fall; Weg 2 kann die Meldung der früheren Instanz im
   Fall „zwei Instanzen" nicht wahr halten (2g).
3. **Keine neue Archivierung:** der heutige Ordner wächst ohnehin um eine Datei je
   Export-Sekunde und löscht nie; Weg 1 fügt nur im Kollisionsfall eine Datei hinzu.
   Aufbewahrung und Löschen sind ein P9-Thema (`funktions-und-bedien-blueprint.md:516-518`),
   das die Karte nicht vorwegnimmt.
4. **Verträglich mit M-39 und dem Präzedenzfall:** der erste Export behält Namen und Inhalt
   bytegleich; die Technik steht im selben Ordner schon (Broker) und im Repo als Muster
   (Briefkasten). Gegenüber Weg 3 verliert der User auch den zweiten Klick nicht.

**Trennung Technik und Produkt:**

| ID | Was der Dirigent als Technikregel entscheidet | Was nur der User entscheidet | Was unabhängig von der Antwort baubar ist |
|---|---|---|---|
| T3-03-10 | alles: Bedeutung des Bits (Build-Tatsache), Wert, Beweiszuordnung A21/A22, A13-Regel, Erratum §53.6:3994, Registerzeile, kein Empfangsriegel | nichts (nur falls jemand P2 abschalten wollte — nicht empfohlen) | alles aus 1g und 1h |
| T3-14-04 | exklusives Veröffentlichen, Form des Zusatzes, Fehlerpfad, Nahtstellen, Meldungstext im Rahmen der gewählten Regel | **ja:** was bei einer Kollision mit der zweiten Datei geschieht (Weg 1, 2 oder 3) | exklusives Veröffentlichen, ehrlicher Teil-Schreibfehler, Meldung mit tatsächlichem Pfad, B30 im Temp-Ordner, Rotbeweis-Gerüst |

---

## Reihenfolge

1. **Sofort, ohne Produktdiff:** die Registerzeile für den Widerspruch §53.6:3994 ↔
   §33.1/Vertrag (`CLAUDE.md:61-63`) und die Karte für W33 anlegen — beides ist Registerarbeit.
   Produktfragen stellt der Dirigent nach seiner Zeitregel.
2. **T3-03-10 nach den Reader-Fixes aus Teil B**, wie B26 es vorsieht (`BEFUNDE.md:141`: „Nach
   B12, B13"), weil der Heartbeat-Eingang (`liveness.rs`) und die Byteinstanz
   `heartbeat-wire-v1.json` gemeinsam berührt werden. Ein Änderungssatz: Bericht, A13,
   Schreiber, Byteinstanz, Vergleichstest, A4-Wächter, Erratum. Keine Karte nötig.
3. **T3-14-04, gemeinsamer Teil — unabhängig von der Karte und von T3-03-10:** exklusives
   Veröffentlichen, ehrlicher Teil-Schreibfehler, Meldung mit tatsächlichem Pfad,
   Nahtstellen und B30 im Temp-Ordner. Laut Arbeitsübersicht „hinter Audio-/State-/
   Recoveryarbeiten" (`BEFUNDE.md:836`).
4. **T3-14-04, Reaktion auf die Kollision — erst nach der Kartenantwort** (oder mit Weg 1 als
   Vorgabe, falls der Dirigent nach Kartenmuster „ohne Antwort Weg 1" verfährt). Der Aufwand
   zwischen den Wegen ist klein; das Gerüst aus Schritt 3 trägt alle drei.

---

## Nicht geprüft

- **Keine Messung, kein Lauf.** Bau, Testlauf, Kanon, Skript und FL waren verboten. Alle
  Aussagen sind Quellenherleitungen; weder die P2-Annahme bei `unsupported` noch eine
  Exportkollision ist gefahren.
- **JUCE ist Bauartefakt.** `eq-copilot/build/_deps/juce-src/` gehört nicht ins Repo; die
  dortigen Zeilennummern (`juce_File.cpp:798-803`, `:323-335`, `juce_TemporaryFile.cpp:61-70`,
  `:94-114`, `juce_Files_windows.cpp:372-378`, `juce_Time.cpp:40-47`, `:341-345`,
  `juce_CharacterFunctions.cpp:109-112`) sind nicht dauerhaft; der Rotbeweis muss die Wirkung
  am eigenen Prozessor messen.
- **MSVC-Zeichenklassen** für Nicht-ASCII-Zeichen innerhalb der BMP und die
  Groß-/Kleinschreibung einzelner Verzeichnisse (NTFS kennt eine abschaltbare
  Unterscheidung je Ordner) sind nicht gemessen; belastbar ist nur der Standard.
- **Ob A21 und A22 den §53.6-Beweis „v3-CRC/Fuzz/Lasttest" formal erfüllen**, hat keine Quelle
  entschieden; es ist meine Lesart nach Wortlaut. Die Zuordnung ist Technik und liegt beim
  Dirigenten.
- **Der Broker bei P2 vor dem ersten Heartbeat** ist nur gelesen (`flush.rs:289-298` verlangt
  keinen Heartbeat), nicht gefahren.
- **Die Zählung in `senke.rs:257-260`:** jeder angenommene P2-Frame erhöht
  `evidence_angenommen` beziehungsweise `evidence_gesperrt`, obwohl er nie Evidenz wird. Das
  gehört nicht zu den zwei IDs und ist nicht weiter bewertet.
- **Nutzung der Exportdateien:** ob und wie der User die Dateien verwendet, liegt keiner Quelle
  vor; die Schwere „low" setzt voraus, dass sie kein Archiv sind, auf das er sich verlässt.
- **Die IDs aus Teil A und B** waren nicht Gegenstand; ihre Berührung ist nur aus meiner Seite
  gelesen. Die Sicherheitsbefunde T3-12-\* sind auftragsgemäß ausgenommen.

FERTIG Teil C, 1 bestätigt, 1 präzisiert, 0 widerlegt.
