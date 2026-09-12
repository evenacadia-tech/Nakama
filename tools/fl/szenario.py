"""Szenario-Runner des Laufzeit-Arms (Plan S25e, Register NAK-286).

Fährt ein Szenario (JSON unter docs/gesundheit/szenarien/) direkt über die
Bibliothek des FL-Studio-MCP (loopMIDI -> Controller-Skript -> JSON), ohne
den MCP-Umweg, und schreibt jeden Schritt roh in eine Markdown-Datei.

Aufruf (aus dem MCP-Repo heraus, damit `fl_studio_mcp` importierbar ist):

    uv run --directory C:\\Users\\phili\\Projekte\\fl-studio-mcp --python 3.12 \
        python <repo>/tools/fl/szenario.py <szenario.json> [--roh <datei.md>]
    ... szenario.py --ping            # nur Ping, JSON auf stdout

Exitcodes: 0 bestanden · 3 Voraussetzung fehlt (kein Port, kein Ping) ·
4 mindestens eine Erwartung verfehlt · 2 Szenariodatei unbrauchbar.

Szenarioformat:

    {"id": "...", "titel": "...", "schritte": [
        {"aktion": "system.ping", "erwarte": {"program_title": {"enthaelt": "Nakama-Diagnose"}}},
        {"aktion": "mixer.getPeaks", "params": {"track": 1}, "erwarte": {"peak_max": {"min": 0.01}}},
        {"warte_s": 3}
    ]}

Erwartungen je Feld: gleich · enthaelt · min · max · nicht_leer (true).
Ein Timeout wird genau einmal wiederholt (FL verschluckt gelegentlich einen
MIDI-Trigger); ein zweiter Timeout ist ein verfehlter Schritt.
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

EXIT_OK, EXIT_DATEI, EXIT_VORAUSSETZUNG, EXIT_VERFEHLT = 0, 2, 3, 4


def _verbindung():
    try:
        from fl_studio_mcp.utils.connection import get_connection
    except ImportError as e:
        raise SystemExit(
            f"fl_studio_mcp nicht importierbar ({e}); aus dem MCP-Repo starten: "
            "uv run --directory <fl-studio-mcp> python szenario.py ..."
        ) from e
    conn = get_connection()
    try:
        conn.ensure_connected()
    except RuntimeError as e:
        return None, str(e)
    return conn, None


def _sende(conn, aktion: str, params: dict | None, frist: float) -> dict:
    antwort = conn.send_command(aktion, params or {}, frist)
    if not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):
        time.sleep(0.5)
        antwort = conn.send_command(aktion, params or {}, frist)
        antwort["_wiederholt"] = True
    return antwort


def _pruefe(erwartung: dict, antwort: dict) -> list[str]:
    """Liefert die Liste der verfehlten Erwartungen (leer = bestanden)."""
    fehler: list[str] = []
    if not antwort.get("success"):
        return [f"Antwort ohne Erfolg: {antwort.get('error', antwort)}"]
    for feld, regel in erwartung.items():
        wert = antwort.get(feld)
        for art, soll in regel.items():
            if art == "gleich" and wert != soll:
                fehler.append(f"{feld}: erwartet gleich {soll!r}, ist {wert!r}")
            elif art == "enthaelt" and (wert is None or str(soll) not in str(wert)):
                fehler.append(f"{feld}: erwartet enthält {soll!r}, ist {wert!r}")
            elif art == "min" and (not isinstance(wert, (int, float)) or wert < soll):
                fehler.append(f"{feld}: erwartet ≥ {soll}, ist {wert!r}")
            elif art == "max" and (not isinstance(wert, (int, float)) or wert > soll):
                fehler.append(f"{feld}: erwartet ≤ {soll}, ist {wert!r}")
            elif art == "nicht_leer" and soll and not wert:
                fehler.append(f"{feld}: erwartet nicht leer, ist {wert!r}")
            elif art not in ("gleich", "enthaelt", "min", "max", "nicht_leer"):
                fehler.append(f"{feld}: unbekannte Erwartungsart {art!r}")
    return fehler


def _kompakt(obj, limit: int = 400) -> str:
    text = json.dumps(obj, ensure_ascii=False, sort_keys=True)
    return text if len(text) <= limit else text[: limit - 1] + "…"


def fahre(szenario_pfad: Path, roh: list[str]) -> int:
    try:
        szenario = json.loads(szenario_pfad.read_text(encoding="utf-8"))
        schritte = szenario["schritte"]
    except (OSError, ValueError, KeyError) as e:
        roh.append(f"## Szenario `{szenario_pfad.name}`: UNBRAUCHBAR ({e})\n")
        return EXIT_DATEI

    conn, fehler = _verbindung()
    kopf = f"## Szenario `{szenario.get('id', szenario_pfad.stem)}` — {szenario.get('titel', '')}\n"
    roh.append(kopf)
    if conn is None:
        roh.append(f"VORAUSSETZUNG FEHLT: {fehler}\n")
        return EXIT_VORAUSSETZUNG

    roh.append("| # | Aktion | Parameter | Antwort | Urteil |\n|---|---|---|---|---|")
    verfehlt = 0
    ergebnis = EXIT_OK
    for nr, schritt in enumerate(schritte, 1):
        if "warte_s" in schritt:
            time.sleep(float(schritt["warte_s"]))
            roh.append(f"| {nr} | warte | {schritt['warte_s']} s | — | — |")
            continue
        aktion = schritt.get("aktion")
        if not aktion:
            roh.append(f"| {nr} | ? | — | — | VERFEHLT (kein `aktion`) |")
            verfehlt += 1
            continue
        params = schritt.get("params") or {}
        frist = float(schritt.get("frist_s", 4.0))
        antwort = _sende(conn, aktion, params, frist)
        maengel = _pruefe(schritt.get("erwarte") or {}, antwort)
        if aktion == "system.ping" and not antwort.get("success"):
            ergebnis = EXIT_VORAUSSETZUNG
        urteil = "ok" if not maengel else "VERFEHLT: " + "; ".join(maengel)
        if maengel:
            verfehlt += 1
        p = _kompakt(params).replace("|", "\\|")
        a = _kompakt(antwort).replace("|", "\\|")
        roh.append(f"| {nr} | `{aktion}` | `{p}` | `{a}` | {urteil} |")
        if ergebnis == EXIT_VORAUSSETZUNG:
            roh.append("\nAbbruch: kein Ping — FL läuft nicht oder der Controller antwortet nicht.\n")
            return ergebnis

    roh.append(f"\n**Ergebnis:** {len(schritte) - verfehlt} von {len(schritte)} Schritten bestanden.\n")
    return EXIT_VERFEHLT if verfehlt else EXIT_OK


def ping() -> int:
    conn, fehler = _verbindung()
    if conn is None:
        print(json.dumps({"success": False, "error": fehler}, ensure_ascii=False))
        return EXIT_VORAUSSETZUNG
    antwort = _sende(conn, "system.ping", {}, 3.0)
    antwort["port_name"] = conn.get_status().get("port_name")
    print(json.dumps(antwort, ensure_ascii=False))
    return EXIT_OK if antwort.get("success") else EXIT_VORAUSSETZUNG


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("szenarien", nargs="*", type=Path, help="Szenariodateien (JSON)")
    ap.add_argument("--roh", type=Path, help="Markdown-Datei, an die die Rohausgabe angehängt wird")
    ap.add_argument("--ping", action="store_true", help="nur Ping, JSON auf stdout")
    args = ap.parse_args(argv)

    if args.ping:
        return ping()
    if not args.szenarien:
        ap.error("mindestens eine Szenariodatei oder --ping")

    roh: list[str] = [f"\n<!-- szenario.py {datetime.now(timezone.utc).isoformat(timespec='seconds')} -->"]
    schlechtester = EXIT_OK
    for pfad in args.szenarien:
        code = fahre(pfad, roh)
        schlechtester = max(schlechtester, code) if code != EXIT_VORAUSSETZUNG else EXIT_VORAUSSETZUNG
        if code == EXIT_VORAUSSETZUNG:
            break

    text = "\n".join(roh) + "\n"
    if args.roh:
        args.roh.parent.mkdir(parents=True, exist_ok=True)
        with args.roh.open("a", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    print(f"SZENARIO EXIT={schlechtester}")
    return schlechtester


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
