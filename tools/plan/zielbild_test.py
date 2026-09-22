#!/usr/bin/env python3
"""Proben fuer tools/plan/zielbild.py — hermetisch, auf einem kleinen Beispiel-Repo im
Temp-Ordner, nie auf dem echten Zielbild.

Aufruf vom Repo-Root:  py -3.13 tools/plan/zielbild_test.py
Jede Probe bricht an genau der Zusage, die sie traegt: Aktualitaet (neue oder geaenderte
Karte, Abnahme, Produktzeile, Planschritt), Verweigerung bei unvollstaendigem Text,
Zeilenenden, Umbruchbreite und Planmarken.
"""
from __future__ import annotations

import contextlib
import io
import json
import pathlib
import re
import sys
import tempfile
import unittest

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import zielbild  # noqa: E402

PLAN = {"phasen": [
    {"phase": "P0", "titel": "Hostgrenzen", "schritte": [
        # Echte Kennungen tragen Leerzeichen („Termin A"); der Fussblock muss sie tragen.
        {"id": "Termin A", "leitungsname": "Nebenwege praktisch bestaetigen"}]},
    {"phase": "P3", "titel": "Landkarte", "schritte": [
        {"id": "S18–19", "leitungsname": "Quellen zeigen"}]},
    {"phase": "P4–P5", "titel": "Ursachen", "schritte": [
        {"id": "S20–22", "leitungsname": "Vergleiche"},
        {"id": "S23–25", "leitungsname": "Befunde"}]},
    {"phase": "P6–P7", "titel": "EQ", "schritte": [
        {"id": "S26–28", "leitungsname": "EQ in der Sonde"}]},
]}
PLANSTAND = """# Planstand

## Alle Schritte

- ■ **Termin A** `FL, du` — Nebenwege (abgenommen · gemessen (Rohdaten))
- ■ **S18–19** `SONDE-012` — Quellen (abgenommen · T2 PASS 2026-09-01)
- ■ **S20–22** `SONDE-013` — Vergleiche (abgenommen · T2 PASS 2026-09-05)
- ▣ **S23–25** `SONDE-014` — Befunde (gebaut · T2 steht aus)
- □ **S26–28** `SONDE-015` — EQ (offen)
"""
FRAGEN = {
    "offen": [{"id": "U11", "titel": "Termin A2"}],
    "geparkt": [{"id": "U23", "titel": "Namen der Flaechen"}],
    "beantwortet": {"U24": {"wahl": "A", "text": "passt", "datum": "2026-09-15 01:33",
                            "karte": {"titel": "Zwoelf Kernfunktionen"}}},
}
REGISTER = """# Register

| ID | Datum | Text |
|---|---|---|
| NAK-30 | 08-21 | **[Planarbeit · S34–35]** **Umbenennung** Text |
| NAK-331 | 09-19 | **[Produktentscheid · Audio]** **Schwelle je Band umschaltbar** Text |
"""
ZIELBILD = """# Nakama, wenn es fertig ist

<!-- Pflege: nur fuer Sessions. -->

> Kasten mit **fettem** Wort.

## Auf einen Blick

- Mix-Landkarte <!-- plan: P3 -->
- Ursachenfinder <!-- plan: P4-P5 -->

### Hörbarer Beweis
<!-- plan: P4–P5 · P6–P7 -->

Ein sehr langer Absatz, der sicher umbrochen werden muss, damit die gewuenschte Breite
eingehalten wird, auch wenn er **fette Stellen ueber mehrere Woerter** enthaelt und dann
noch weitergeht, bis er mehrere Zeilen fuellt.

## Noch offen

- Termin A2 (U11)
- Geparkt: Namen (U23)
"""
ANSI = re.compile(r"\x1b\[[0-9;]*m")


class Beispielrepo:
    def __init__(self, wurzel: pathlib.Path, zeilenende: str = "\n"):
        self.wurzel = wurzel
        for rel, inhalt in (
            ("docs/plan/plan.json", json.dumps(PLAN, ensure_ascii=False)),
            ("docs/plan/fragen.json", json.dumps(FRAGEN, ensure_ascii=False)),
            ("docs/PLAN-STAND.md", PLANSTAND),
            ("docs/offene-punkte.md", REGISTER),
            ("design/abnahmen/2026-09-15-zwoelf-kernfunktionen-u24.md", "# U24\n"),
            ("design/abnahmen/LIES-MICH.md", "# Leseblatt\n"),
            ("docs/ZIELBILD.md", ZIELBILD.replace("\n", zeilenende)),
        ):
            self.schreiben(rel, inhalt)

    def schreiben(self, rel: str, inhalt: str):
        pfad = self.wurzel / rel
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(inhalt.encode("utf-8"))

    def lesen(self, rel: str) -> bytes:
        return (self.wurzel / rel).read_bytes()

    def fragen(self, aendern):
        daten = json.loads(json.dumps(FRAGEN))
        aendern(daten)
        self.schreiben("docs/plan/fragen.json", json.dumps(daten, ensure_ascii=False))

    def lauf(self, *argumente: str) -> tuple[int, str]:
        puffer = io.StringIO()
        with contextlib.redirect_stdout(puffer), contextlib.redirect_stderr(puffer):
            code = zielbild.main([*argumente, "--wurzel", str(self.wurzel)])
        return code, puffer.getvalue()


class Proben(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.repo = Beispielrepo(pathlib.Path(self._tmp.name))

    def tearDown(self):
        self._tmp.cleanup()

    def abgeglichen(self):
        code, ausgabe = self.repo.lauf("abgleichen")
        self.assertEqual(code, 0, ausgabe)

    # --- Aktualitaet --------------------------------------------------------

    def test_nie_abgeglichen_ist_veraltet(self):
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("noch nie abgeglichen", ausgabe)

    def test_abgleich_macht_aktuell_und_ist_idempotent(self):
        self.abgeglichen()
        vorher = self.repo.lesen("docs/ZIELBILD.md")
        self.assertIn(b"ZIELBILD-ABGLEICH", vorher)
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 0, ausgabe)
        code, ausgabe = self.repo.lauf("abgleichen")
        self.assertEqual(code, 0)
        self.assertIn("bereits abgeglichen", ausgabe)
        self.assertEqual(self.repo.lesen("docs/ZIELBILD.md"), vorher)

    def test_neu_beantwortete_karte_macht_veraltet(self):
        self.abgeglichen()
        self.repo.fragen(lambda d: d["beantwortet"].update(
            {"U40": {"wahl": "1", "text": "1", "datum": "2026-09-15 14:06",
                     "karte": {"titel": "Advisor auf leisem Material"}}}))
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Karte U40 beantwortet: Advisor auf leisem Material", ausgabe)

    def test_erneut_beantwortete_karte_macht_veraltet(self):
        self.abgeglichen()
        self.repo.fragen(lambda d: d["beantwortet"]["U24"].update(datum="2026-09-30 10:00"))
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Karte U24 neu beantwortet", ausgabe)

    def test_neue_offene_karte_fehlt_im_text_und_abgleich_verweigert(self):
        self.abgeglichen()
        self.repo.fragen(lambda d: d["offen"].append({"id": "U58", "titel": "Bypass loslassen"}))
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Karte U58 neu offen", ausgabe)
        self.assertIn("offene Karte U58 fehlt im Text", ausgabe)
        vorher = self.repo.lesen("docs/ZIELBILD.md")
        code, ausgabe = self.repo.lauf("abgleichen")
        self.assertEqual(code, 4)
        self.assertIn("ABGLEICH VERWEIGERT", ausgabe)
        self.assertEqual(self.repo.lesen("docs/ZIELBILD.md"), vorher)

    def test_karte_im_fussblock_zaehlt_nicht_als_genannt(self):
        # Der Abgleichblock fuehrt alle Kennungen; genannt ist eine Karte nur im Lesetext.
        self.abgeglichen()
        text = self.repo.lesen("docs/ZIELBILD.md").decode("utf-8").replace("- Termin A2 (U11)\n", "")
        self.repo.schreiben("docs/ZIELBILD.md", text)
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("offene Karte U11 fehlt im Text", ausgabe)

    def test_neue_abnahme_macht_veraltet(self):
        self.abgeglichen()
        self.repo.schreiben("design/abnahmen/2026-09-24-neuer-entscheid-u59.md", "# U59\n")
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Abnahme neu: 2026-09-24-neuer-entscheid-u59", ausgabe)

    def test_registerzeilen_nur_mit_produktklasse(self):
        self.abgeglichen()
        neu = REGISTER.replace("**[Produktentscheid · Audio]**", "**[Geschlossen 09-30 · Produktentscheid · Audio]**")
        neu += "| NAK-400 | 09-30 | **[Produktfrage · Karte U60]** **Neue Produktfrage** Text |\n"
        neu += "| NAK-401 | 09-30 | **[Werkzeug]** **Nur Werkzeug** Text |\n"
        self.repo.schreiben("docs/offene-punkte.md", neu)
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Registerzeile NAK-331 Klasse geändert", ausgabe)
        self.assertIn("Registerzeile NAK-400 neu: Neue Produktfrage", ausgabe)
        self.assertNotIn("NAK-401", ausgabe)
        self.assertNotIn("NAK-30 ", ausgabe)

    def test_planschritt_neu_und_umbenannt(self):
        self.abgeglichen()
        plan = json.loads(json.dumps(PLAN))
        plan["phasen"][3]["schritte"].append({"id": "S28b", "leitungsname": "Master-EQ"})
        plan["phasen"][1]["schritte"][0]["leitungsname"] = "Quellen mit Frische zeigen"
        plan["phasen"][0]["schritte"][0]["leitungsname"] = "Nebenwege neu gemessen"
        self.repo.schreiben("docs/plan/plan.json", json.dumps(plan, ensure_ascii=False))
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 4)
        self.assertIn("Planschritt S28b neu: Master-EQ", ausgabe)
        self.assertIn("Planschritt S18–19 geändert", ausgabe)
        self.assertIn("Planschritt Termin A geändert", ausgabe)
        self.assertNotIn("Termin A neu", ausgabe)

    def test_kennung_mit_leerzeichen_ueberlebt_den_fussblock(self):
        self.abgeglichen()
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 0, ausgabe)
        self.assertIn(b"Termin%20A=", self.repo.lesen("docs/ZIELBILD.md"))

    def test_unbekannte_planmarke(self):
        text = ZIELBILD.replace("<!-- plan: P3 -->", "<!-- plan: P9 -->")
        self.repo.schreiben("docs/ZIELBILD.md", text)
        code, ausgabe = self.repo.lauf("abgleichen")
        self.assertEqual(code, 4)
        self.assertIn("Planmarke „P9\"", ausgabe)

    def test_zwei_abgleichbloecke_sind_werkzeugfehler(self):
        self.abgeglichen()
        text = self.repo.lesen("docs/ZIELBILD.md").decode("utf-8")
        block = zielbild.BLOCK_RE.search(text).group(0)
        self.repo.schreiben("docs/ZIELBILD.md", text + "\n" + block + "\n")
        code, ausgabe = self.repo.lauf("pruefen")
        self.assertEqual(code, 2)
        self.assertIn("2 Abgleich-Bloecke", ausgabe)

    def test_zeilenenden_bleiben(self):
        with tempfile.TemporaryDirectory() as tmp:
            repo = Beispielrepo(pathlib.Path(tmp), zeilenende="\r\n")
            code, ausgabe = repo.lauf("abgleichen")
            self.assertEqual(code, 0, ausgabe)
            daten = repo.lesen("docs/ZIELBILD.md")
            self.assertEqual(daten.count(b"\n"), daten.count(b"\r\n"))

    # --- Anzeige ------------------------------------------------------------

    def test_kurzzeile(self):
        code, ausgabe = self.repo.lauf("anzeige", "--kurz")
        self.assertEqual((code, ausgabe.strip()), (4, "noch nie abgeglichen"))
        self.abgeglichen()
        code, ausgabe = self.repo.lauf("anzeige", "--kurz")
        self.assertEqual(code, 0)
        self.assertTrue(ausgabe.startswith("aktuell, Stand "), ausgabe)

    def test_umbruch_haelt_die_breite_auch_mit_farbe(self):
        self.abgeglichen()
        code, ausgabe = self.repo.lauf("anzeige", "--breite", "50", "--farbe")
        self.assertEqual(code, 0, ausgabe)
        kopf, rumpf = ausgabe.split("\n\n", 1)
        for zeile in kopf.splitlines():
            self.assertLessEqual(len(ANSI.sub("", zeile)), 50, zeile)
        for zeile in rumpf.splitlines():
            self.assertLessEqual(len(ANSI.sub("", zeile)), 49, zeile)
        self.assertIn("\x1b[32m✓", ausgabe)   # Farbe kommt nach dem Umbruch
        self.assertNotIn("\x01", ausgabe)
        self.assertNotIn("\x02", ausgabe)
        self.assertNotIn("Pflege", ausgabe)
        self.assertNotIn("ZIELBILD-ABGLEICH", ausgabe)

    def test_planmarken_werden_gerechnet(self):
        self.abgeglichen()
        code, ausgabe = self.repo.lauf("anzeige", "--breite", "120")
        self.assertEqual(code, 0, ausgabe)
        self.assertIn("• Mix-Landkarte · ✓ P3", ausgabe)
        self.assertIn("• Ursachenfinder · ◐ P4–P5", ausgabe)
        self.assertIn("Plan: ◐ P4–P5 läuft (1 von 2 abgenommen) · ○ P6–P7 offen", ausgabe)

    def test_veraltet_steht_in_der_kopfzeile(self):
        self.abgeglichen()
        self.repo.schreiben("design/abnahmen/2026-09-24-neuer-entscheid-u59.md", "# U59\n")
        code, ausgabe = self.repo.lauf("anzeige", "--breite", "120")
        self.assertEqual(code, 4)
        kopf = ausgabe.split("\n\n", 1)[0].replace("\n", " ")
        self.assertIn("! 1 Punkt(e) seit dem Stand vom", kopf)
        self.assertIn("Abnahme neu: 2026-09-24-neuer-entscheid-u59", kopf)


if __name__ == "__main__":
    unittest.main(verbosity=2)
