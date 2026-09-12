# Paket 6 — Ausweitung auf Host-Lebenszyklus und Lieferung

**Auftrag:** Busannahme, Audioverarbeitung und Host-Lebenszyklus bis zur VST3-Grenze prüfen; Installation/Repair/Rückweg lesend einordnen. Keine Installation.

**Belegt:** F04 nimmt einen gültigen symmetrischen Vierkanalbus an, bearbeitet jedoch nur zwei Kanäle. F05 lässt bei setProcessing(false)/true Filterrestzustand hörbar weiterlaufen. Beide Ergebnisse liegen für direkte Prozessoraufrufe und das echte VST3-Modul vor. release/reprepare sind im direkten Kontrolllauf sauber.

**Provenienz:** Kleiner lokaler Testhost mit IHostApplication, IComponent und IAudioProcessor. Hostaufrufe melden Erfolg. Public-API-erzeugte Statefixtures, Quellen, Buildargumente und Logs unter evidence/host/. VST3-Hash in der Übergabe und Verifikationsnotiz.

**Nicht behauptet:** FL-Studio-Abnahme, Treiber-/XRun-Messung, Installation oder Upgradeprüfung. Unterschiedliche Installer-/Journalstände wurden gelesen, aber ohne abgeschlossenen Nachweis nicht als Defekt eingestuft. Ungültiges prepare ohne üblichen release-Pfad ebenfalls kein normaler Hostdefekt.

**Status:** Belegte Hostläufe abgeschlossen; restliche Arbeit auf User-Signal unterbrochen. Kein Produktcode geändert.
