#!/usr/bin/env python3
"""Envelope-Fixture-Korpus fuer SONDE-010 (Entwurf §33.1, §66.2).

Schreibt `eq-copilot/fixtures/v3/envelope/{gueltig,ungueltig}/*.bin` und
daneben `MANIFEST.json` mit dem ERWARTETEN Urteil je Fixture — bei Ablehnung
samt der vollstaendigen, kanonisch sortierten Verstossmenge.

WARUM DIE ERWARTUNG VON HAND STEHT
----------------------------------
Dasselbe Argument wie beim JSON- und FlatBuffers-Korpus: das Manifest ist die
dritte Partei zwischen C++ (`plugin/core/ipc/WireEnvelope`) und Rust
(`broker/src/transport/v3.rs`). Waere es aus einer der beiden Fassungen
erzeugt, waere der Vergleich zirkulaer. Unten steht deshalb JEDE Erwartung als
handgeschriebene Zeile neben den Bytes, die sie ausloest.

WIE DIE BYTES ENTSTEHEN
-----------------------
Durch einen kleinen, hier stehenden Encoder — bewusst eine DRITTE
Implementierung des Kopfes. Beim Envelope ist das der Punkt: das Format ist
16 Bytes gross und vollstaendig im Entwurf beschrieben; wer es dreimal
unabhaengig hinschreibt und dreimal dasselbe misst, hat es bewiesen. Fuer
FlatBuffers waere das anders (dort erzeugt der gepinnte `flatc`), und deshalb
traegt mindestens ein gueltiges P2-Fixture den ECHTEN Puffer aus dem
SONDE-005b-Korpus als Nutzlast statt erfundener Bytes.

Aufruf:
    py -3.13 tools/eq-copilot/erzeuge_envelope_fixtures.py
    py -3.13 tools/eq-copilot/erzeuge_envelope_fixtures.py --pruefen

Exitcodes: 0 gruen · 2 Abweichung · 3 Voraussetzung fehlt.
"""

from __future__ import annotations

import hashlib
import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
ZIEL = WURZEL / "eq-copilot/fixtures/v3/envelope"
FB_KORPUS = WURZEL / "eq-copilot/fixtures/v3/flatbuffers/gueltig"

MAX_FRAME_BYTES = 262_144
KOPF_BYTES = 16
MAX_PAYLOAD_BYTES = MAX_FRAME_BYTES - KOPF_BYTES
SCHEMA_MAJOR = 3

JSON_ENC, FB_ENC = 0, 1
P0, P1, P2 = 0, 1, 2

# Die geschlossene Regelmenge. Sie steht identisch in
# broker/src/transport/v3.rs (enum Verstoss) und in
# eq-copilot/plugin/core/ipc/WireEnvelope.h (enum class Verstoss).
REGELN = [
    "praefix_unvollstaendig",
    "frame_len_null",
    "frame_len_unter_kopf",
    "frame_len_ueber_grenze",
    "frame_unvollstaendig",
    "ueberlaenge",
    "unterlaenge",
    "crc_bei_json_nicht_null",
    "crc_falsch",
    "encoding_passt_nicht_zur_familie",
    "encoding_unbekannt",
    "flags_reserviert",
    "message_family_unbekannt",
    "schema_major_unbekannt",
]


def crc32c(daten: bytes) -> int:
    """CRC32C (Castagnoli), reflektiert — dritte unabhaengige Fassung."""
    poly = 0x82F63B78
    crc = 0xFFFFFFFF
    for b in daten:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ (poly if crc & 1 else 0)
    return crc ^ 0xFFFFFFFF


def rahmen(
    payload: bytes,
    *,
    encoding: int,
    familie: int,
    major: int = SCHEMA_MAJOR,
    minor: int = 0,
    flags: int = 0,
    frame_len: int | None = None,
    payload_len: int | None = None,
    crc: int | None = None,
) -> bytes:
    """Baut einen Frame. Jeder Kopfwert ist einzeln uebersteuerbar — genau so
    entstehen die ungueltigen Faelle, ohne dass ein zweiter Encoder noetig
    waere."""
    p_len = len(payload) if payload_len is None else payload_len
    f_len = (KOPF_BYTES + len(payload)) if frame_len is None else frame_len
    if crc is None:
        crc = 0 if encoding == JSON_ENC else crc32c(payload)
    kopf = bytes([encoding & 0xFF, familie & 0xFF, major & 0xFF, minor & 0xFF])
    kopf += flags.to_bytes(4, "little")
    kopf += p_len.to_bytes(4, "little")
    kopf += crc.to_bytes(4, "little")
    return f_len.to_bytes(4, "little") + kopf + payload


def fb_nutzlast() -> bytes:
    """Ein ECHTER FlatBuffers-Puffer aus dem SONDE-005b-Korpus. Damit haengt
    der Envelope-Korpus an der Nutzlast, die im Betrieb wirklich fliesst —
    statt an erfundenen Bytes."""
    quelle = FB_KORPUS / "live-64-band.bin"
    if not quelle.exists():
        raise SystemExit(f"VORAUSSETZUNG FEHLT: {quelle} (SONDE-005b-Korpus)")
    return quelle.read_bytes()


def gueltige() -> list[tuple[str, bytes, str]]:
    fb = fb_nutzlast()
    return [
        (
            "p0-json-leeres-objekt",
            rahmen(b"{}", encoding=JSON_ENC, familie=P0),
            "Grundform P0: JSON, CRC per Vertrag exakt 0",
        ),
        (
            "p0-json-welcome",
            rahmen(
                json.dumps(
                    {
                        "type": "welcome",
                        "protocol": 3,
                        "broker_version": "0.1.0",
                        "broker_epoch": "0" * 32,
                        "link_id": "1" * 32,
                        "challenge": "2" * 32,
                    },
                    separators=(",", ":"),
                ).encode("utf-8"),
                encoding=JSON_ENC,
                familie=P0,
            ),
            "die erste Antwort des Brokers, so wie sie wirklich gerahmt wird (§53.9)",
        ),
        (
            "p1-json-state-report",
            rahmen(b'{"type":"state_report"}', encoding=JSON_ENC, familie=P1),
            "P1 traegt wie P0 JSON; die Familie unterscheidet nur die Prioritaet",
        ),
        (
            "p0-payload-laenge-null",
            rahmen(b"", encoding=JSON_ENC, familie=P0),
            "payload_len 0 und frame_len 16 sind envelopeseitig gueltig — ob der "
            "leere Payload eine gueltige NACHRICHT ist, entscheidet erst der "
            "Payloadparser. Der Envelope urteilt nie ueber Inhalt",
        ),
        (
            "p0-schema-minor-unbekannt",
            rahmen(b"{}", encoding=JSON_ENC, familie=P0, minor=200),
            "unbekannte MINOR-Version ist additiv und wird angenommen (§33.1); "
            "nur unbekannte Major-Versionen werden abgelehnt",
        ),
        (
            "p2-flatbuffers-echter-batch",
            rahmen(fb, encoding=FB_ENC, familie=P2),
            "P2 mit dem echten live-64-band-Puffer aus dem SONDE-005b-Korpus und "
            "der darueber gerechneten CRC32C",
        ),
        (
            "p2-flatbuffers-kurz",
            rahmen(bytes(range(32)), encoding=FB_ENC, familie=P2),
            "kurzer P2-Frame: die CRC deckt genau die 32 Payloadbytes, nicht "
            "Praefix und nicht Kopf",
        ),
        (
            "p2-payload-an-der-paketgrenze",
            rahmen(bytes(MAX_PAYLOAD_BYTES), encoding=FB_ENC, familie=P2),
            "frame_len == 262144 ist die GRENZE und noch erlaubt; erst darueber "
            "faellt der Frame",
        ),
    ]


def ungueltige() -> list[tuple[str, bytes, list[str], str]]:
    fb_klein = bytes(range(32))
    gut_p2 = rahmen(fb_klein, encoding=FB_ENC, familie=P2)
    gut_p0 = rahmen(b"{}", encoding=JSON_ENC, familie=P0)

    p2_ein_bit = bytearray(gut_p2)
    p2_ein_bit[-1] ^= 0x01

    # v2-Frame, ungerahmt in den v3-Pruefer gegeben. Von Hand nachgerechnet:
    # der JSON-Text ist 37 Bytes, also frame_len = 37 (>= 16, <= Grenze). Die
    # Bytes 8..12 des "Kopfes" sind die Zeichen '"hel' = 0x6C656822, also
    # payload_len = 1 818 587 682 und erwartet = 1 818 587 698 != 37 —
    # Unterlaenge. Alle Stufe-3-Regeln kommen gar nicht mehr dran.
    v2_text = b'{"type":"hello","protocol_version":2}'
    assert len(v2_text) == 37, len(v2_text)
    v2_frame = len(v2_text).to_bytes(4, "little") + v2_text

    return [
        # ── Stufe 1: Rahmen ────────────────────────────────────────────────
        ("rahmen-praefix-drei-bytes", bytes([1, 2, 3]), ["praefix_unvollstaendig"],
         "drei Bytes sind noch kein Laengenpraefix"),
        ("rahmen-frame-len-null", (0).to_bytes(4, "little"), ["frame_len_null"],
         "frame_len 0 ist wie in v2 verboten — sonst gaebe es einen Frame ohne Ende"),
        ("rahmen-frame-len-fuenfzehn",
         (15).to_bytes(4, "little") + bytes(15), ["frame_len_unter_kopf"],
         "15 Bytes koennen den 16-Byte-Kopf nicht tragen"),
        ("rahmen-frame-len-ueber-grenze",
         (MAX_FRAME_BYTES + 1).to_bytes(4, "little") + bytes(16),
         ["frame_len_ueber_grenze"],
         "ein Byte ueber der Paketgrenze; die Ablehnung faellt VOR jeder "
         "Allokation der angekuendigten Groesse"),
        ("rahmen-payload-fehlt-ein-byte", gut_p2[:-1], ["frame_unvollstaendig"],
         "der Frame ist angekuendigt, aber ein Byte fehlt — im Strom heisst das "
         "'weiterlesen', als ganzer Puffer heisst es 'unvollstaendig'"),

        # ── Stufe 2: Kopfkonsistenz ────────────────────────────────────────
        ("kopf-ueberlaenge",
         rahmen(b"1234", encoding=JSON_ENC, familie=P0, frame_len=KOPF_BYTES + 5)
         + b"x",
         ["ueberlaenge"],
         "frame_len ist um eins groesser als 16 + payload_len"),
        ("kopf-unterlaenge",
         rahmen(b"12345", encoding=JSON_ENC, familie=P0, frame_len=KOPF_BYTES + 4),
         ["unterlaenge"],
         "frame_len ist um eins kleiner als 16 + payload_len"),
        ("kopf-payload-len-u32-ueberlauf",
         rahmen(b"", encoding=JSON_ENC, familie=P0, payload_len=0xFFFFFFFF,
                frame_len=KOPF_BYTES),
         ["unterlaenge"],
         "payload_len 0xFFFFFFFF: in u32 gerechnet waere 16 + payload_len == 15 "
         "und der Frame hiesse 'ueberlaenge'. Der Regelname unterscheidet die "
         "richtige u64-Rechnung von der naiven — deshalb ist das Fixture hier"),
        ("v2-frame-im-v3-pruefer", v2_frame, ["unterlaenge"],
         "die Gegenrichtung der v2-Isolation: ein echter v2-Frame ist fuer den "
         "v3-Pruefer kein gueltiger Envelope. Von Hand nachgerechnet (siehe "
         "Kommentar im Erzeuger)"),

        # ── Stufe 3: Feldwerte ─────────────────────────────────────────────
        ("encoding-zwei", rahmen(b"{}", encoding=2, familie=P0, crc=0),
         ["encoding_unbekannt"],
         "2 ist kein Encoding; 0 = JSON, 1 = FlatBuffers"),
        ("encoding-255", rahmen(b"{}", encoding=255, familie=P0, crc=0),
         ["encoding_unbekannt"],
         "oberer Rand des Bytes"),
        ("familie-drei", rahmen(b"{}", encoding=JSON_ENC, familie=3),
         ["message_family_unbekannt"],
         "3 ist keine Familie; 0 = P0, 1 = P1, 2 = P2"),
        ("familie-255", rahmen(b"{}", encoding=JSON_ENC, familie=255),
         ["message_family_unbekannt"],
         "oberer Rand des Bytes"),
        ("major-null", rahmen(b"{}", encoding=JSON_ENC, familie=P0, major=0),
         ["schema_major_unbekannt"],
         "Major 0 ist keine v3-Version"),
        ("major-zwei", rahmen(b"{}", encoding=JSON_ENC, familie=P0, major=2),
         ["schema_major_unbekannt"],
         "v2 kennt diesen Kopf gar nicht; als Major im v3-Kopf ist 2 unbekannt"),
        ("major-vier", rahmen(b"{}", encoding=JSON_ENC, familie=P0, major=4),
         ["schema_major_unbekannt"],
         "eine ZUKUENFTIGE Major-Version wird abgelehnt, nicht geraten (§33.1)"),
        ("flags-bit-null", rahmen(b"{}", encoding=JSON_ENC, familie=P0, flags=1),
         ["flags_reserviert"],
         "alle 32 Flagbits sind heute reserviert; ein gesetztes Bit bedeutet "
         "eine Semantik, die dieser Leser nicht kennt"),
        ("flags-hoechstes-bit",
         rahmen(b"{}", encoding=JSON_ENC, familie=P0, flags=0x80000000),
         ["flags_reserviert"],
         "oberes Ende des Flagworts"),
        ("crc-bei-json-nicht-null",
         rahmen(b"{}", encoding=JSON_ENC, familie=P0, crc=1),
         ["crc_bei_json_nicht_null"],
         "bei JSON ist die CRC per Vertrag EXAKT 0 — nicht 'egal'"),
        ("crc-p2-ein-bit-im-payload-gekippt", bytes(p2_ein_bit), ["crc_falsch"],
         "ein einziges gekipptes Payloadbit faellt an der Pflicht-CRC von P2"),
        ("crc-p2-null-statt-summe",
         rahmen(fb_klein, encoding=FB_ENC, familie=P2, crc=0),
         ["crc_falsch"],
         "0 ist bei FlatBuffers keine Ausnahme, sondern eine falsche Summe"),
        ("crc-p2-summe-ueber-kopf-statt-payload",
         rahmen(fb_klein, encoding=FB_ENC, familie=P2,
                crc=crc32c(gut_p2[:KOPF_BYTES + 4])),
         ["crc_falsch"],
         "die CRC deckt GENAU die payload_len Payloadbytes; Praefix und Kopf "
         "gehoeren nicht in den Digest (§33.1)"),
        ("familie-p0-mit-flatbuffers",
         rahmen(fb_klein, encoding=FB_ENC, familie=P0),
         ["encoding_passt_nicht_zur_familie"],
         "P0 verwendet JSON (§33.1); ein binaerer P0-Frame ist kein Steuerpaket"),
        ("familie-p2-mit-json",
         rahmen(b"{}", encoding=JSON_ENC, familie=P2, crc=0),
         ["encoding_passt_nicht_zur_familie"],
         "P2 verweist auf ein FeatureBatch; JSON dort waere die Zahlenexpansion, "
         "die der Binaerpfad gerade vermeidet"),

        # ── Mehrere Verstoesse: die Menge und ihre Sortierung ───────────────
        ("kopf-vier-feldfehler",
         rahmen(b"{}", encoding=9, familie=9, major=4, flags=1, crc=0),
         ["encoding_unbekannt", "flags_reserviert", "message_family_unbekannt",
          "schema_major_unbekannt"],
         "vier unabhaengige Feldfehler; die Menge ist kanonisch nach dem "
         "Regelnamen sortiert, damit sie nicht von der Auswertungsreihenfolge "
         "abhaengt"),
        ("familie-p2-mit-json-und-crc",
         rahmen(b"{}", encoding=JSON_ENC, familie=P2, crc=7),
         ["crc_bei_json_nicht_null", "encoding_passt_nicht_zur_familie"],
         "zwei Regeln greifen gleichzeitig — das Encoding IST bekannt, also "
         "gilt auch die CRC-Regel dieses Encodings"),
        ("encoding-unbekannt-mit-flags",
         rahmen(b"{}", encoding=7, familie=P0, flags=8, crc=99),
         ["encoding_unbekannt", "flags_reserviert"],
         "der Vorrang-Beweis: bei UNBEKANNTEM Encoding faellt keine CRC-Regel, "
         "obwohl die CRC hier weder 0 noch die Summe ist. Ohne diese Regel "
         "haetten beide Leser die freie Wahl zwischen zwei Antworten"),
        ("major-unbekannt-und-crc-falsch",
         rahmen(fb_klein, encoding=FB_ENC, familie=P2, major=4, crc=0),
         ["crc_falsch", "schema_major_unbekannt"],
         "unbekannte Major-Version hindert die CRC-Pruefung nicht: der Kopf ist "
         "in JEDER Major-Version derselbe, nur der Payload nicht"),
        ("alles-falsch-ausser-dem-rahmen",
         rahmen(b"xy", encoding=200, familie=200, major=200, flags=0xFFFFFFFF,
                crc=0xFFFFFFFF),
         ["encoding_unbekannt", "flags_reserviert", "message_family_unbekannt",
          "schema_major_unbekannt"],
         "Extremfall: jeder Feldwert ausserhalb, aber der Rahmen stimmt. Genau "
         "vier Regeln greifen — die CRC-Regeln nicht, weil das Encoding "
         "unbekannt ist, und die Familienregel nicht, weil beide unbekannt sind"),
    ]


def als_text(inhalt) -> bytes:
    return (json.dumps(inhalt, indent=2, ensure_ascii=False) + "\n").encode("utf-8")


def baue() -> tuple[dict, dict[str, bytes]]:
    dateien: dict[str, bytes] = {}
    eintraege: list[dict] = []

    for name, roh, warum in gueltige():
        pfad = f"gueltig/{name}.bin"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        dateien[pfad] = roh
        eintraege.append({"datei": pfad, "urteil": "gueltig", "warum": warum,
                          "verstoesse": []})

    for name, roh, verstoesse, warum in ungueltige():
        pfad = f"ungueltig/{name}.bin"
        if pfad in dateien:
            raise SystemExit(f"doppelter Fixturename: {pfad}")
        unbekannt = [v for v in verstoesse if v not in REGELN]
        if unbekannt:
            raise SystemExit(f"{pfad}: Regeln ausserhalb der Menge: {unbekannt}")
        if sorted(verstoesse) != verstoesse:
            raise SystemExit(f"{pfad}: Verstossmenge ist nicht kanonisch sortiert")
        if not verstoesse:
            raise SystemExit(f"{pfad}: ungueltig ohne Verstoss ist widerspruechlich")
        dateien[pfad] = roh
        eintraege.append({"datei": pfad, "urteil": "ungueltig", "warum": warum,
                          "verstoesse": verstoesse})

    # Jede Regel braucht mindestens ein Negativfixture — sonst steht sie nur
    # im Code und niemand misst sie (Lehre aus SONDE-005a, Bein A5).
    gedeckt = {v for e in eintraege for v in e["verstoesse"]}
    fehlend = [r for r in REGELN if r not in gedeckt]
    if fehlend:
        raise SystemExit(f"Regeln ohne Negativfixture: {fehlend}")

    eintraege.sort(key=lambda e: e["datei"])
    manifest = {
        "$id": "evenacadia.nakama.fixtures.v3.envelope",
        "titel": "Envelope-Fixture-Korpus (SONDE-010)",
        "vertrag": "docs/FL-Nakama-Sonden-Design-Entwurf.md §33.1 (16-Byte-Kopf)",
        "zweck": (
            "Die dritte Partei zwischen C++ (plugin/core/ipc/WireEnvelope) und Rust "
            "(broker/src/transport/v3.rs). Beide Leser messen gegen DIESES Manifest; "
            "stimmen beide mit ihm ueberein, stimmen sie transitiv miteinander "
            "ueberein. Die Erwartungen sind von Hand geschrieben, nicht aus einem "
            "Leser erzeugt — sonst waere der Vergleich zirkulaer."
        ),
        "kopf": (
            "u32 frame_len (LE) | u8 encoding | u8 message_family | u8 schema_major "
            "| u8 schema_minor | u32 flags | u32 payload_len | u32 crc32c. Es gilt "
            "exakt frame_len == 16 + payload_len, gerechnet in u64."
        ),
        "stufen": (
            "Die Pruefung laeuft in drei Stufen, weil die Fragen voneinander "
            "abhaengen: (1) Rahmen — ohne gueltiges frame_len ist der Payload nicht "
            "auffindbar; (2) Kopfkonsistenz — ohne frame_len == 16 + payload_len ist "
            "die Payloadgrenze nicht bestimmt; (3) Feldwerte — hier wird ALLES "
            "gesammelt und kanonisch nach dem Regelnamen sortiert. Stufe 1 und 2 "
            "liefern deshalb immer genau einen Verstoss, Stufe 3 kann mehrere "
            "liefern. Innerhalb von Stufe 3 gilt eine einzige Abhaengigkeit: die "
            "CRC-Regeln greifen nur bei BEKANNTEM Encoding, die Familienregel nur "
            "bei bekanntem Encoding UND bekannter Familie."
        ),
        "regeln": REGELN,
        "anzahl_gueltig": sum(1 for e in eintraege if e["urteil"] == "gueltig"),
        "anzahl_ungueltig": sum(1 for e in eintraege if e["urteil"] == "ungueltig"),
        "fixtures": eintraege,
    }
    return manifest, dateien


def main(argv: list[str]) -> int:
    if not FB_KORPUS.exists():
        print(f"VORAUSSETZUNG FEHLT: {FB_KORPUS.relative_to(WURZEL)}")
        return 3

    nur_pruefen = "--pruefen" in argv
    manifest, dateien = baue()
    print(f"{manifest['anzahl_gueltig']} gueltige, {manifest['anzahl_ungueltig']} "
          f"ungueltige Envelope-Fixtures; {len(REGELN)} Regeln, jede mit Negativfall")

    alle: list[tuple[pathlib.Path, bytes]] = [(ZIEL / "MANIFEST.json", als_text(manifest))]
    alle += [(ZIEL / p, b) for p, b in sorted(dateien.items())]

    if nur_pruefen:
        vorhanden = ({p.relative_to(ZIEL).as_posix()
                      for p in ZIEL.rglob("*") if p.is_file()} if ZIEL.exists() else set())
        erwartet = {p.relative_to(ZIEL).as_posix() for p, _ in alle}
        verwaist = sorted(vorhanden - erwartet)
        if verwaist:
            for w in verwaist:
                print(f"  ROT: verwaiste Datei {w}")
            return 2
        for pfad, inhalt in alle:
            if not pfad.exists():
                print(f"  ROT: {pfad.relative_to(WURZEL)} fehlt")
                return 3
            if pfad.read_bytes() != inhalt:
                print(f"  ROT: {pfad.relative_to(WURZEL)} weicht ab")
                return 2
        h = hashlib.sha256(als_text(manifest)).hexdigest()
        print(f"  bytegleich: {len(alle)} Dateien, MANIFEST sha256={h}")
        return 0

    for pfad, inhalt in alle:
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(inhalt)
    print(f"  geschrieben: {len(alle)} Dateien nach {ZIEL.relative_to(WURZEL)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
