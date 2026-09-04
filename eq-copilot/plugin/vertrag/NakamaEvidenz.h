#pragma once

/*  Erzeuger des `evidence_snapshot` (SONDE-013 M-05, Etappe B).

    Der Evidenzpfad war bis zu diesem Ticket VERTRAGLICH fertig und ohne
    jede Zeile Code: an `ed9bbf7` liefert `grep` auf `evidence_snapshot` in
    `broker/src` und `eq-copilot/plugin` keinen einzigen Treffer ausserhalb
    der Schemadatei. §33.2 ordnet die 221 Baender, die Verteilungspunkte,
    Abdeckung, Konvergenz und die Ereignisse aber ausdruecklich DIESEM
    Snapshot zu und nicht dem 10-Hz-Liveframe — ohne ihn haetten sie keinen
    Weg (Manifest §7.1, E-05).

    WARUM DIESER BAUER EINE EIGENE UEBERSETZUNGSEINHEIT IST

    Er ist ein VERTRAGSSERIALISIERER, dieselbe Rolle wie `NakamaTelemetrie`
    fuer die FlatBuffers-Seite: er nimmt eine Messung und macht daraus genau
    die Form, die das committete Schema verlangt. Er liegt deshalb neben ihr
    und nicht im Sondenprozessor — sonst haetten Produktpfad und Test zwei
    Kopien derselben JSON-Form, und genau daran ist in diesem Projekt schon
    einmal eine Falltabelle in drei Fassungen auseinandergelaufen
    (T2-Runde 2, BF-5).

    WAS ER NICHT TUT

    Er entscheidet keine Konfidenzklasse und erfindet keine Adresse; beides
    bekommt er. Er sendet auch nicht — der Weg auf die Leitung gehoert dem
    `ControlClient`, und ein Serialisierer, der sendet, waere im Audiopfad
    nicht mehr pruefbar.
*/

#include "../core/analysis/FeatureEngine.h"
#include "../core/ipc/ControlClient.h"

#include <string>

namespace nakama::evidenz
{

/** Was ein Snapshot ausser dem Frame braucht.

    `evidenceId` ist je Snapshot NEU: der Snapshot ist Evidenz mit eigener
    Identitaet, keine Sicht auf ein Objekt. Genau deshalb wird er auf der
    Leitung auch nicht koalesziert — zwei Snapshots derselben Quelle sind
    zwei Belege, nicht zweimal derselbe. */
struct Snapshotkopf
{
    std::string        evidenceId;   ///< hex32, je Snapshot neu gezogen
    nakama::ipc::Adresse adresse {};
    /// `stark` | `mittel` | `schwach` | `unbrauchbar` (§34.3). Die Klasse
    /// wird NICHT aus einem Mittelwert gerettet; sie kommt von dem, der die
    /// harten Maengel kennt.
    std::string        klasse { "mittel" };
};

/** Der Ereignisstrom EINES Evidenzfensters, wie ihn der Snapshot traegt.

    Die Eintraege kommen aus dem festen Ring der `FeatureEngine`
    (`kEreignisPlaetze` = 64, drop-oldest). `verloren` ist die Zahl der
    Ereignisse, die es GAB, die aber nicht reisen — Verdraengung im Ring plus
    alles, was der Bauer wegen fremder Epoche oder fremdem Segment aussortiert.
    Eine Zahl, eine Bedeutung: „nicht alle Ereignisse haben dich erreicht".

    Ein leerer Strom OHNE Verlust laesst das Feld ganz weg: Abwesenheit heisst
    „diese Quelle liefert keine Ereignisse". Ein leerer Strom MIT Verlust
    reist dagegen — sonst waere der Verlust genau die Nachricht, die
    verschwindet. */
struct Ereignisstrom
{
    const nakama::analyse::Ereignis* eintraege { nullptr };
    int           anzahl   { 0 };
    std::uint64_t verloren { 0 };
};

/** Die bandweise Stereoevidenz eines Evidenzfensters (SONDE-013 M-11).

    Sie kommt nicht aus dem `FeatureFrame`, sondern direkt aus der Engine —
    elf Bandsaetze zu 221 `float` sind rund 11 KiB, und der Frame liegt auf
    dem Stack (Begruendung bei `analyse::StereoBandwert`). Der Sondenprozessor
    holt sie mit `merkmale.stereoBand(b)` und `merkmale.stereoSkalare()`,
    genau wie den Ereignisring daneben.

    `baender` zeigt auf `Gitter::evidenzBaender` Eintraege oder ist
    `nullptr`. Ist es `nullptr`, entsteht das `stereo`-Feld GAR NICHT — ein
    Satz aus 221 leeren Baendern waeren 11 KiB Schweigen auf der Leitung. */
struct Stereosicht
{
    const nakama::analyse::StereoBandwert* baender { nullptr };
    nakama::analyse::StereoSkalare         skalare {};
};

/** Baut den `evidence_snapshot`-JSON-Text.

    Fail-closed: `false`, wenn der Frame keinen Snapshot traegt
    (`evidenzFrisch == false`), wenn Pflichtangaben fehlen (Adresse,
    `evidence_id`, Konfidenzklasse) oder wenn der Transportstempel seine
    NAK-29-Feldpflichten verletzt. In allen diesen Faellen bleibt `aus`
    unveraendert — lieber kein Snapshot als einer, der sich selbst
    widerspricht.

    Die Verteilung ist PFLICHT (§33.2), sie wird also mitgeschrieben, auch
    wenn kein Band ein Bit traegt: eine leere Bitmap ist die ehrliche
    Aussage „hier war nichts zu messen", ein fehlendes Feld waere eine
    Vertragsverletzung.

    @returns true bei Erfolg; `aus` traegt dann genau ein JSON-Objekt. */
bool evidenceSnapshotAlsJson (const nakama::analyse::FeatureFrame& frame,
                              const Snapshotkopf& kopf,
                              const Ereignisstrom& ereignisse,
                              const Stereosicht& stereo,
                              std::string& aus);

} // namespace nakama::evidenz
