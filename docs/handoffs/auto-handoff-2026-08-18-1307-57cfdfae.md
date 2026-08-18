# Auto-Handoff — 2026-08-18 13:07

**Session:** `57cfdfae-386c-4615-8ccc-95fe74d9577f`  
**Grund:** prompt_input_exit  
**Dauer:** 912 min  
**Nakama-HEAD:** `7d22050` → `7f71473`  

## Nakama-Workspace (Code + Broker + Design + Wissen)

```
7f71473 Auto-Handoffs der Umzugs-Session eingesammelt
fdbcdb7 Verifikations-Befunde gefixt (frischer Subagent): Install-Skript skript-relativ (starb doppelt an FL-Pfaden), smf.py als lokale Kopie (Testsong-Werkzeuge wieder lauffaehig, py_compile gruen), analyze-track-Fallbacksuche mit ehrlicher Meldung, tote FL-Pfade in plugin-wissen/geschmacksprofil/design-stand/Uebergabe-Doc (§17-Befehle neu), Probe-Bin-Kommentar, getrackten __pycache__-Bytecode entfernt + Ignore-Regel
9bc31de Kontextgeruest auf eigenstaendigen Workspace umgestellt: CLAUDE.md neu (lokale Pfade, Broker-Betrieb, Design-Stand 18.08.), Primer/Handoff/Marker-Hooks lesen nur noch dieses Repo, plugin-wissen Broker-Abschnitt, NEXT-SESSION-Umzugsnotiz, offene Punkte NAK-12 (Sensoruebersicht heimatlos) + NAK-13 (Broker-Autostart fehlt)
31c4124 Nakama wird eigenstaendiger Workspace — Einzug aus dem FL-Studio-Repo (18.08.2026): eq-copilot/ komplett (Plugin, Schemas, Design, Docs, Install, Fixtures, Kalibration), tools/eq-copilot/, Recherche/Plan/Mockup/Sonden-Doc; NEU broker/ = aus der Hub-App herausgeloester Named-Pipe-Broker als eigene Crate (36/36 Tests gruen, eqcop-broker.exe + Probe-Bin, Sensoruebersicht als Svelte-Referenz); Beweise am neuen Ort: CMake-Build + NullTest OK + GoldenTest 239/239 + MarkierungTest OK. Git-Historie der Dateien: FL-Studio-Repo bis 7964777
a9ae463 Geschmacksprofil: Sicht-Proben-Urteil — Prisma-Objekt abgenommen (erste Optik-Abnahme), rohe Energie-Punktwolke als Dauerinhalt verworfen (Lesbarkeits-Massstab des Profis, woertlich)
c2b663b NEXT-SESSION: Sicht-Probe (P01 im Glasvolumen) steht — FL-Repo 6b5d4eb
34fd793 design-stand: Effektdarstellungs-Festlegungen des Users (Raum nutzen, Effekt im Prisma-Volumen, definiert statt Shaderzirkus)
4eec053 NEXT-SESSION: Prototyp-Seite als Denk-Leinwand vermerkt (FL-Repo 3ea4c89/6b8371b/5907c15)
bea5c0c Kreativ-Freigabe: Prototyp-Seite mit Drehfunktion (User-Wortlaut 18.08.); Effektdarstellung bleibt beim User in Ausarbeitung
54b8992 Prisma-Material-Urteil protokolliert: klar vorn, rauchig Reserve, Frost raus (woertlich im Geschmacksprofil); naechster Schritt Flaechen-/Buendel-Inhalt im Gespraech
9e10252 design-stand + NEXT-SESSION: Prisma-Material-Stills gerendert (drei Anmutungen, FL-Repo 76c8793/2107536), Cycles-Grenzen dokumentiert; naechster Schritt = User-Urteil
3419269 Kreativ-Freigabe auf Prisma-Material-Probestill fortgeschrieben (User-Zustimmung 17.08., Fortsetzung bestaetigt)
```

```
 .claude/kreativ-freigabe.md                        |   32 +-
 .gitignore                                         |   19 +
 CLAUDE.md                                          |  119 +-
 FL-EQ-Copilot-Recherche.md                         | 1935 +++++++++++++++
 Textdokument (neu).txt                             |    0
 broker/Cargo.lock                                  |  350 +++
 broker/Cargo.toml                                  |   23 +
 .../Sensoruebersicht.svelte                        |  475 ++++
 broker/src/aggregat.rs                             |  270 ++
 broker/src/bin/eqcop-broker-probe.rs               |   35 +
 broker/src/bindung.rs                              |   82 +
 broker/src/framing.rs                              |  118 +
 broker/src/lib.rs                                  |  851 +++++++
 broker/src/main.rs                                 |   68 +
 broker/src/protokoll.rs                            |  283 +++
 broker/src/server.rs                               |  740 ++++++
 docs/FL-EQ-Copilot-Mockup.html                     | 1682 +++++++++++++
 docs/FL-EQ-Copilot-Plan.md                         |   19 +
 docs/FL-Nakama-Sonden-Design-Entwurf.md            |  981 ++++++++
 docs/NEXT-SESSION.md                               |   43 +-
 docs/design-stand.md                               |   45 +-
 docs/fl-claudemd-auszug-2026-08-18.md              |  153 ++
 docs/geschmacksprofil.md                           |   37 +-
 .../auto-handoff-2026-08-17-2155-e4925865.md       |  122 +
 .../auto-handoff-2026-08-18-1219-2e0311c1.md       |   18 +
 .../auto-handoff-2026-08-18-1219-794445d8.md       |   20 +
 .../auto-handoff-2026-08-18-1219-8c001938.md       |   19 +
 .../auto-handoff-2026-08-18-1219-ce18fa57.md       |   21 +
 .../prisma-farbe-01-petrol-knochen-aprikose.png    |  Bin 0 -> 1281688 bytes
 .../prisma-farbe-02-kobalt-pfirsich-koralle.png    |  Bin 0 -> 1282328 bytes
 .../prisma-farbe-03-mint-koralle-signalrot.png     |  Bin 0 -> 1311117 bytes
 .../prisma-farbe-04-indigo-violett-eis.png         |  Bin 0 -> 1348987 bytes
 .../prisma-farbe-05-koralle-periwinkle-cyan.png    |  Bin 0 -> 1327455 bytes
 .../prisma-licht-01-spektrale-topografie.png       |  Bin 0 -> 1396725 bytes
 .../prisma-licht-02-drei-zustaende.png             |  Bin 0 -> 1516911 bytes
 .../prisma-licht-03-lichtgedaechtnis.png           |  Bin 0 -> 1235432 bytes
 .../prisma-licht-04-ein-strahl-drei-optiken.png    |  Bin 0 -> 1381267 bytes
 .../prisma-licht-05-perkolations-triptychon.png    |  Bin 0 -> 1181642 bytes
 .../prisma-tiefe-01-drei-keilraeume.png            |  Bin 0 -> 1554436 bytes
 .../prisma-tiefe-02-optische-kavernen.png          |  Bin 0 -> 1478033 bytes
 .../prisma-tiefe-03-inneres-lichtobjekt.png        |  Bin 0 -> 1606035 bytes
 .../prisma-tiefe-04-z-achse-ist-zeit.png           |  Bin 0 -> 1544663 bytes
 docs/leitstand-karte-eq-copilot.md                 |   39 +
 docs/offene-punkte.md                              |    2 +
 docs/plugin-wissen.md                              |   19 +-
 eq-copilot/.gitignore                              |    3 +
 eq-copilot/CMakeLists.txt                          |   18 +
 eq-copilot/README.md                               |  125 +
 eq-copilot/design/ASSET-KIT.md                     |   81 +
 eq-copilot/design/archive/NakamaGehaeuse-v1.h      |  283 +++
 eq-copilot/design/assets/nakama-symbols.svg        |  112 +
 .../design/concepts/nakama-field-alphabet-v2.html  |  426 ++++
 .../design/concepts/nakama-symbolalphabet-v1.png   |  Bin 0 -> 1645305 bytes
 eq-copilot/design/eq-copilot-material-preview.png  |  Bin 0 -> 76538 bytes
 .../design/eq-copilot-spectral-field-event.png     |  Bin 0 -> 704968 bytes
 eq-copilot/design/eq-copilot-spectral-field.html   |  842 +++++++
 eq-copilot/design/eq-copilot-spectral-field.png    |  Bin 0 -> 660468 bytes
 eq-copilot/design/fonts/Manrope-Variable.ttf       |  Bin 0 -> 165420 bytes
 eq-copilot/design/fonts/NakamaPhase-Regular.ttf    |  Bin 0 -> 7068 bytes
 eq-copilot/design/fonts/NakamaPhase-Regular.woff2  |  Bin 0 -> 2440 bytes
 eq-copilot/design/fonts/OFL-Manrope.txt            |   93 +
 eq-copilot/design/fonts/gen-nakama-phase.py        |  386 +++
 eq-copilot/design/gen-tokens.mjs                   |   43 +
 eq-copilot/design/nakama-kunstwerk-claude.html     |  993 ++++++++
 .../design/nakama-problem-effects-studie.html      |  544 ++++
 .../design/nakama-spectral-field-claude.html       | 2609 ++++++++++++++++++++
 .../design/nakama-spectral-field-vorentwurf.html   | 1692 +++++++++++++
 eq-copilot/design/prisma/prisma-material-still.py  |  369 +++
 eq-copilot/design/prisma/prisma-prototyp.html      |  532 ++++
 eq-copilot/design/prisma/pruefe-projektion.html    |   70 +
 eq-copilot/design/prisma/renders/debug-energie.png |  Bin 0 -> 409991 bytes
 .../design/prisma/renders/debug-kugeln-frei.png    |  Bin 0 -> 769202 bytes
 eq-copilot/design/prisma/renders/debug-kugeln.png  |  Bin 0 -> 845342 bytes
 .../design/prisma/renders/debug-nurstrahl.png      |  Bin 0 -> 115008 bytes
 eq-copilot/design/prisma/renders/debug-oben.png    |  Bin 0 -> 378317 bytes
 eq-copilot/design/prisma/renders/dreh/f000.webp    |  Bin 0 -> 26006 bytes
 eq-copilot/design/prisma/renders/dreh/f001.webp    |  Bin 0 -> 24314 bytes
 eq-copilot/design/prisma/renders/dreh/f002.webp    |  Bin 0 -> 19186 bytes
 eq-copilot/design/prisma/renders/dreh/f003.webp    |  Bin 0 -> 16388 bytes
 eq-copilot/design/prisma/renders/dreh/f004.webp    |  Bin 0 -> 16088 bytes
 eq-copilot/design/prisma/renders/dreh/f005.webp    |  Bin 0 -> 16542 bytes
 eq-copilot/design/prisma/renders/dreh/f006.webp    |  Bin 0 -> 15632 bytes
 eq-copilot/design/prisma/renders/dreh/f007.webp    |  Bin 0 -> 16160 bytes
 eq-copilot/design/prisma/renders/dreh/f008.webp    |  Bin 0 -> 16984 bytes
 eq-copilot/design/prisma/renders/dreh/f009.webp    |  Bin 0 -> 16580 bytes
 eq-copilot/design/prisma/renders/dreh/f010.webp    |  Bin 0 -> 17272 bytes
 eq-copilot/design/prisma/renders/dreh/f011.webp    |  Bin 0 -> 17308 bytes
 eq-copilot/design/prisma/renders/dreh/f012.webp    |  Bin 0 -> 19126 bytes
 eq-copilot/design/prisma/renders/dreh/f013.webp    |  Bin 0 -> 18294 bytes
 eq-copilot/design/prisma/renders/dreh/f014.webp    |  Bin 0 -> 19082 bytes
 eq-copilot/design/prisma/renders/dreh/f015.webp    |  Bin 0 -> 20712 bytes
 eq-copilot/design/prisma/renders/dreh/f016.webp    |  Bin 0 -> 21884 bytes
 eq-copilot/design/prisma/renders/dreh/f017.webp    |  Bin 0 -> 23892 bytes
 eq-copilot/design/prisma/renders/dreh/f018.webp    |  Bin 0 -> 24922 bytes
 eq-copilot/design/prisma/renders/dreh/f019.webp    |  Bin 0 -> 25602 bytes
 eq-copilot/design/prisma/renders/dreh/f020.webp    |  Bin 0 -> 24338 bytes
 eq-copilot/design/prisma/renders/dreh/f021.webp    |  Bin 0 -> 22678 bytes
 eq-copilot/design/prisma/renders/dreh/f022.webp    |  Bin 0 -> 22346 bytes
 eq-copilot/design/prisma/renders/dreh/f023.webp    |  Bin 0 -> 21556 bytes
 eq-copilot/design/prisma/renders/dreh/f024.webp    |  Bin 0 -> 20934 bytes
 eq-copilot/design/prisma/renders/dreh/f025.webp    |  Bin 0 -> 20462 bytes
 eq-copilot/design/prisma/renders/dreh/f026.webp    |  Bin 0 -> 20844 bytes
 eq-copilot/design/prisma/renders/dreh/f027.webp    |  Bin 0 -> 21052 bytes
 eq-copilot/design/prisma/renders/dreh/f028.webp    |  Bin 0 -> 21280 bytes
 eq-copilot/design/prisma/renders/dreh/f029.webp    |  Bin 0 -> 20984 bytes
 eq-copilot/design/prisma/renders/dreh/f030.webp    |  Bin 0 -> 20774 bytes
 eq-copilot/design/prisma/renders/dreh/f031.webp    |  Bin 0 -> 19812 bytes
 eq-copilot/design/prisma/renders/dreh/f032.webp    |  Bin 0 -> 20322 bytes
 eq-copilot/design/prisma/renders/dreh/f033.webp    |  Bin 0 -> 19690 bytes
 eq-copilot/design/prisma/renders/dreh/f034.webp    |  Bin 0 -> 20046 bytes
 eq-copilot/design/prisma/renders/dreh/f035.webp    |  Bin 0 -> 20490 bytes
 eq-copilot/design/prisma/renders/dreh/f036.webp    |  Bin 0 -> 20496 bytes
 eq-copilot/design/prisma/renders/dreh/f037.webp    |  Bin 0 -> 20032 bytes
 eq-copilot/design/prisma/renders/dreh/f038.webp    |  Bin 0 -> 19740 bytes
 eq-copilot/design/prisma/renders/dreh/f039.webp    |  Bin 0 -> 18490 bytes
 eq-copilot/design/prisma/renders/dreh/f040.webp    |  Bin 0 -> 17800 bytes
 eq-copilot/design/prisma/renders/dreh/f041.webp    |  Bin 0 -> 16792 bytes
 eq-copilot/design/prisma/renders/dreh/f042.webp    |  Bin 0 -> 16308 bytes
 eq-copilot/design/prisma/renders/dreh/f043.webp    |  Bin 0 -> 16904 bytes
 eq-copilot/design/prisma/renders/dreh/f044.webp    |  Bin 0 -> 17320 bytes
 eq-copilot/design/prisma/renders/dreh/f045.webp    |  Bin 0 -> 17818 bytes
 eq-copilot/design/prisma/renders/dreh/f046.webp    |  Bin 0 -> 17348 bytes
 eq-copilot/design/prisma/renders/dreh/f047.webp    |  Bin 0 -> 16610 bytes
 eq-copilot/design/prisma/renders/dreh/f048.webp    |  Bin 0 -> 16930 bytes
 eq-copilot/design/prisma/renders/dreh/f049.webp    |  Bin 0 -> 17272 bytes
 eq-copilot/design/prisma/renders/dreh/f050.webp    |  Bin 0 -> 17584 bytes
 eq-copilot/design/prisma/renders/dreh/f051.webp    |  Bin 0 -> 17918 bytes
 eq-copilot/design/prisma/renders/dreh/f052.webp    |  Bin 0 -> 17904 bytes
 eq-copilot/design/prisma/renders/dreh/f053.webp    |  Bin 0 -> 17700 bytes
 eq-copilot/design/prisma/renders/dreh/f054.webp    |  Bin 0 -> 17464 bytes
 eq-copilot/design/prisma/renders/dreh/f055.webp    |  Bin 0 -> 16936 bytes
 eq-copilot/design/prisma/renders/dreh/f056.webp    |  Bin 0 -> 16808 bytes
 eq-copilot/design/prisma/renders/dreh/f057.webp    |  Bin 0 -> 17168 bytes
 eq-copilot/design/prisma/renders/dreh/f058.webp    |  Bin 0 -> 19752 bytes
 eq-copilot/design/prisma/renders/dreh/f059.webp    |  Bin 0 -> 17504 bytes
 eq-copilot/design/prisma/renders/dreh/f060.webp    |  Bin 0 -> 17234 bytes
 eq-copilot/design/prisma/renders/dreh/f061.webp    |  Bin 0 -> 16846 bytes
 eq-copilot/design/prisma/renders/dreh/f062.webp    |  Bin 0 -> 17952 bytes
 eq-copilot/design/prisma/renders/dreh/f063.webp    |  Bin 0 -> 16646 bytes
 eq-copilot/design/prisma/renders/dreh/f064.webp    |  Bin 0 -> 18034 bytes
 eq-copilot/design/prisma/renders/dreh/f065.webp    |  Bin 0 -> 18556 bytes
 eq-copilot/design/prisma/renders/dreh/f066.webp    |  Bin 0 -> 17178 bytes
 eq-copilot/design/prisma/renders/dreh/f067.webp    |  Bin 0 -> 15804 bytes
 eq-copilot/design/prisma/renders/dreh/f068.webp    |  Bin 0 -> 16146 bytes
 eq-copilot/design/prisma/renders/dreh/f069.webp    |  Bin 0 -> 17050 bytes
 eq-copilot/design/prisma/renders/dreh/f070.webp    |  Bin 0 -> 22770 bytes
 eq-copilot/design/prisma/renders/dreh/f071.webp    |  Bin 0 -> 27392 bytes
 .../design/prisma/renders/entwurf-kadrierung.png   |  Bin 0 -> 218619 bytes
 eq-copilot/design/prisma/renders/entwurf-klar.png  |  Bin 0 -> 420408 bytes
 eq-copilot/design/prisma/renders/glas-ebene.png    |  Bin 0 -> 533245 bytes
 .../design/prisma/renders/prisma-frostig.png       |  Bin 0 -> 1930850 bytes
 eq-copilot/design/prisma/renders/prisma-klar.png   |  Bin 0 -> 1586274 bytes
 .../design/prisma/renders/prisma-rauchig.png       |  Bin 0 -> 1582730 bytes
 eq-copilot/design/prisma/winter-nexus-plate.png    |  Bin 0 -> 82568 bytes
 eq-copilot/design/proben/probe-01-perkolation.html |  373 +++
 .../design/proben/probe-02-lichtschleier.html      |  276 +++
 .../design/proben/probe-03-tropfenklingen.html     |  362 +++
 eq-copilot/design/proben/probe-04-interferenz.html |  334 +++
 eq-copilot/design/proben/render-probe.mjs          |   42 +
 .../design/proben/renders/bruecke-anleitung.png    |  Bin 0 -> 50037 bytes
 .../design/proben/renders/bruecke-winter-nexus.png |  Bin 0 -> 59177 bytes
 eq-copilot/design/proben/renders/p01-chorus.png    |  Bin 0 -> 972272 bytes
 eq-copilot/design/proben/renders/p01-intro.png     |  Bin 0 -> 571782 bytes
 eq-copilot/design/proben/renders/p01-spaet.png     |  Bin 0 -> 812686 bytes
 eq-copilot/design/proben/renders/p02-chorus.png    |  Bin 0 -> 642597 bytes
 eq-copilot/design/proben/renders/p02-intro.png     |  Bin 0 -> 587946 bytes
 eq-copilot/design/proben/renders/p03-chorus.png    |  Bin 0 -> 123316 bytes
 eq-copilot/design/proben/renders/p03-spaet.png     |  Bin 0 -> 122501 bytes
 eq-copilot/design/proben/renders/p04-chorus.png    |  Bin 0 -> 650373 bytes
 eq-copilot/design/proben/renders/p04-intro.png     |  Bin 0 -> 564009 bytes
 .../design/proben/renders/projektion-beweis.png    |  Bin 0 -> 511484 bytes
 .../proben/renders/prototyp-band1-tiefen.png       |  Bin 0 -> 354929 bytes
 .../proben/renders/prototyp-band2-mitten.png       |  Bin 0 -> 336038 bytes
 .../proben/renders/prototyp-band3-hoehen.png       |  Bin 0 -> 296013 bytes
 .../design/proben/renders/prototyp-debug.png       |  Bin 0 -> 239827 bytes
 eq-copilot/design/proben/renders/prototyp-neu.png  |  Bin 0 -> 163475 bytes
 .../proben/renders/prototyp-punkte-chorus.png      |  Bin 0 -> 355340 bytes
 .../design/proben/renders/prototyp-smoke.png       |  Bin 0 -> 110547 bytes
 .../design/proben/renders/prototyp-smoke2.png      |  Bin 0 -> 110547 bytes
 eq-copilot/design/proben/renders/prototyp-voll.png |  Bin 0 -> 87796 bytes
 eq-copilot/design/proben/renders/umzug-smoke.png   |  Bin 0 -> 252852 bytes
 .../referenz/297bed317cf6aec48684e238f3209989.webp |  Bin 0 -> 52532 bytes
 eq-copilot/design/referenz/LIES-MICH.md            |   19 +
 .../referenz/dce3a279c1ccaf2699fb4cf2e1ee0bbf.webp |  Bin 0 -> 42208 bytes
 .../referenz/e454a6b78d56eb8e17de5a3eebdde335.webp |  Bin 0 -> 41294 bytes
 .../original-755e3c8dfb6df32f74bca3b85a226c6a.webp |  Bin 0 -> 68230 bytes
 .../original-a4ffec040b1e944ac10fcbab276d58a9.webp |  Bin 0 -> 36136 bytes
 .../original-bea83c06a07272e994c2989267bcb12c.webp |  Bin 0 -> 46674 bytes
 .../original-f806632da13662974c3898ab306b1665.webp |  Bin 0 -> 39654 bytes
 eq-copilot/design/render-blatt.mjs                 |  149 ++
 eq-copilot/design/render-kunstwerk.mjs             |   66 +
 eq-copilot/design/sync-nakama-symbols.mjs          |   52 +
 eq-copilot/design/tokens.json                      |  113 +
 eq-copilot/design/unicorn-bruecke.html             |  282 +++
 .../design/unicorn/hintergrund-winter-nexus.json   |    1 +
 eq-copilot/design/vendor-build/package-lock.json   |   60 +
 eq-copilot/design/vendor-build/package.json        |    9 +
 .../design/vorentwurf-renders/01-belastbar.png     |  Bin 0 -> 651910 bytes
 eq-copilot/design/vorentwurf-renders/02-voll.png   |  Bin 0 -> 744923 bytes
 eq-copilot/design/vorentwurf-renders/03-zone.png   |  Bin 0 -> 737414 bytes
 eq-copilot/design/vorentwurf-renders/04-temp.png   |  Bin 0 -> 664336 bytes
 eq-copilot/design/vorentwurf-renders/05-bereit.png |  Bin 0 -> 457523 bytes
 eq-copilot/design/vorentwurf-renders/06-zukurz.png |  Bin 0 -> 635718 bytes
 .../design/vorentwurf-renders/07-kennung.png       |  Bin 0 -> 653142 bytes
 .../design/vorentwurf-renders/08-keinbefund.png    |  Bin 0 -> 543154 bytes
 .../design/vorentwurf-renders/09-simulation-8s.png |  Bin 0 -> 668217 bytes
 .../vorentwurf-renders/10-simulation-20s.png       |  Bin 0 -> 706206 bytes
 .../vorentwurf-renders/13-nakama-1920-graphit.png  |  Bin 0 -> 180454 bytes
 .../14-nakama-1200-papier-bericht.png              |  Bin 0 -> 80617 bytes
 .../vorentwurf-renders/15-nakama-720-signal.png    |  Bin 0 -> 41412 bytes
 .../16-nakama-1200-farbpakete.png                  |  Bin 0 -> 85736 bytes
 .../21-nakama-resonanz-effekte.png                 |  Bin 0 -> 146073 bytes
 .../22-nakama-maskierung-effekte.png               |  Bin 0 -> 167232 bytes
 .../23-nakama-resonanz-effekte.gif                 |  Bin 0 -> 1161697 bytes
 .../24-nakama-maskierung-effekte.gif               |  Bin 0 -> 1199177 bytes
 .../vorentwurf-renders/k1-01-garten-intro.png      |  Bin 0 -> 130947 bytes
 .../vorentwurf-renders/k1-02-garten-chorus.png     |  Bin 0 -> 134809 bytes
 .../vorentwurf-renders/k1-03-gewebe-chorus.png     |  Bin 0 -> 183701 bytes
 .../vorentwurf-renders/k1-04-gewebe-frueh.png      |  Bin 0 -> 91595 bytes
 .../vorentwurf-renders/k1-05-wesen-chorus.png      |  Bin 0 -> 73024 bytes
 .../vorentwurf-renders/k1-06-wesen-intro.png       |  Bin 0 -> 72446 bytes
 .../vorentwurf-renders/k1-07-garten-leer.png       |  Bin 0 -> 150259 bytes
 .../design/vorentwurf-renders/k1-08-gross.png      |  Bin 0 -> 185152 bytes
 .../vorentwurf-renders/k1-09-garten-solo.png       |  Bin 0 -> 102807 bytes
 .../vorentwurf-renders/n4-01-frueh-nebel.png       |  Bin 0 -> 979737 bytes
 .../vorentwurf-renders/n4-02-belastbar-licht.png   |  Bin 0 -> 949129 bytes
 .../vorentwurf-renders/n4-03-breite-fehlstelle.png |  Bin 0 -> 952480 bytes
 .../design/vorentwurf-renders/n4-04-transient.png  |  Bin 0 -> 950377 bytes
 .../vorentwurf-renders/n4-05-ueberlagerung.png     |  Bin 0 -> 974402 bytes
 .../vorentwurf-renders/n4-06-kein-befund.png       |  Bin 0 -> 946185 bytes
 .../design/vorentwurf-renders/n4-07-kompakt.png    |  Bin 0 -> 415805 bytes
 .../design/vorentwurf-renders/n4-08-mittel.png     |  Bin 0 -> 974402 bytes
 .../design/vorentwurf-renders/n4-09-gross.png      |  Bin 0 -> 2313517 bytes
 .../design/vorentwurf-renders/n4-10-tusche.png     |  Bin 0 -> 892867 bytes
 .../nakama-1920-graphite-palette.png               |  Bin 0 -> 187242 bytes
 .../design/vorentwurf-renders/t3-1-strich-plan.png |  Bin 0 -> 1165533 bytes
 .../design/vorentwurf-renders/t3-2-fahne-pool.png  |  Bin 0 -> 1177473 bytes
 .../design/vorentwurf-renders/t3-3-befundnebel.png |  Bin 0 -> 1170694 bytes
 .../design/vorentwurf-renders/t3-4-licht-tiefe.png |  Bin 0 -> 1182910 bytes
 .../design/vorentwurf-renders/t3-5-alles.png       |  Bin 0 -> 1196458 bytes
 .../design/vorentwurf-renders/t3-6-alles-kraft.png |  Bin 0 -> 1197504 bytes
 .../design/vorentwurf-renders/t3-7-ruhig.png       |  Bin 0 -> 1175949 bytes
 .../design/vorentwurf-renders/t3-8-nacht-alles.png |  Bin 0 -> 1268175 bytes
 .../docs/BENCHMARK-STUDIE-RESO-SMARTEQ-PROQ.md     |  382 +++
 eq-copilot/docs/CS1-BEFUND.md                      |  117 +
 eq-copilot/docs/EQ-COPILOT-TECHNIK-UX-UEBERGABE.md |  824 +++++++
 eq-copilot/docs/FL-TESTANLEITUNG.md                |  254 ++
 eq-copilot/docs/HOER-MARKIERUNG-KONZEPT.md         |  323 +++
 eq-copilot/docs/LIZENZ-NOTIZ.md                    |   36 +
 eq-copilot/docs/M0-BEFUND.md                       |   52 +
 eq-copilot/docs/M1-BEFUND.md                       |  104 +
 eq-copilot/docs/M2-BEFUND.md                       |  127 +
 eq-copilot/docs/M3-KERN-BEFUND.md                  |  101 +
 eq-copilot/docs/M3A-BEFUND.md                      |  141 ++
 eq-copilot/docs/NAKAMA-PAPIER-3D-EFFEKTE-PLAN.md   |  438 ++++
 eq-copilot/docs/NAKAMA-SPECTRAL-FIELD-BAUPLAN.md   |  783 ++++++
 .../docs/claude-spike/bare-lauf-2026-08-13.json    |    1 +
 eq-copilot/fixtures/README.md                      |   18 +
 eq-copilot/fixtures/golden-referenz.json           | 2050 +++++++++++++++
 eq-copilot/kalibration/KALIBRIER-PROTOKOLL.md      |  141 ++
 .../kalibration/referenz-light-of-nibel.json       |    1 +
 eq-copilot/kalibration/testsong-01/00_LIES-MICH.md |   46 +
 .../kalibration/testsong-01/01_Klavier-1_Haupt.mid |  Bin 0 -> 1821 bytes
 .../testsong-01/02_Klavier-2_Arpeggio.mid          |  Bin 0 -> 594 bytes
 .../kalibration/testsong-01/03_Pad-Streicher.mid   |  Bin 0 -> 484 bytes
 eq-copilot/kalibration/testsong-01/04_Pad-Chor.mid |  Bin 0 -> 472 bytes
 .../testsong-01/05_Pad-Elektronisch.mid            |  Bin 0 -> 216 bytes
 eq-copilot/kalibration/testsong-01/06_Bass.mid     |  Bin 0 -> 602 bytes
 .../kalibration/testsong-01/07_Schlagzeug_GM.mid   |  Bin 0 -> 861 bytes
 .../kalibration/testsong-01/08_Gitarre_Motiv.mid   |  Bin 0 -> 589 bytes
 eq-copilot/plugin/CMakeLists.txt                   |  140 ++
 eq-copilot/plugin/probe/PipeProbeMain.cpp          |  140 ++
 eq-copilot/plugin/src/AnalyseEngine.cpp            | 1141 +++++++++
 eq-copilot/plugin/src/AnalyseEngine.h              |  346 +++
 eq-copilot/plugin/src/Diagnose.cpp                 |  520 ++++
 eq-copilot/plugin/src/Diagnose.h                   |   83 +
 eq-copilot/plugin/src/EqCopilotAssetKit.h          |  477 ++++
 eq-copilot/plugin/src/EqCopilotIds.h               |   26 +
 eq-copilot/plugin/src/HoerMarkierung.h             |  449 ++++
 eq-copilot/plugin/src/LeitstandTokens.h            |  101 +
 eq-copilot/plugin/src/PipeClient.cpp               |  340 +++
 eq-copilot/plugin/src/PipeClient.h                 |  138 ++
 eq-copilot/plugin/src/PluginEditor.cpp             | 1163 +++++++++
 eq-copilot/plugin/src/PluginEditor.h               |  147 ++
 eq-copilot/plugin/src/PluginFactory.cpp            |    8 +
 eq-copilot/plugin/src/PluginProcessor.cpp          |  667 +++++
 eq-copilot/plugin/src/PluginProcessor.h            |  171 ++
 eq-copilot/plugin/src/ZonenRegeln.h                |   37 +
 eq-copilot/plugin/tests/GoldenTestMain.cpp         |  883 +++++++
 eq-copilot/plugin/tests/MarkierungTestMain.cpp     |  504 ++++
 eq-copilot/plugin/tests/NullTestMain.cpp           |   93 +
 eq-copilot/plugin/tests/PaintBenchMain.cpp         |  103 +
 eq-copilot/plugin/tests/ShotTestMain.cpp           |   77 +
 eq-copilot/schemas/eq-aggregat.schema.json         |   81 +
 eq-copilot/schemas/eq-ipc.schema.json              |  159 ++
 eq-copilot/schemas/eq-measurement.schema.json      |   56 +
 eq-copilot/schemas/eq-report.schema.json           |   58 +
 eq-copilot/schemas/eq-snapshot.schema.json         |  169 ++
 tools/eq-copilot/erzeuge_fixtures.py               |  243 ++
 tools/eq-copilot/erzeuge_testsong.py               |  277 +++
 tools/eq-copilot/smf.py                            |  144 ++
 tools/eq-copilot/verify_testsong.py                |  153 ++
 tools/hooks/depth-primer.sh                        |    2 +-
 tools/hooks/nakama-primer.sh                       |   45 +-
 tools/hooks/session-end-handoff.sh                 |   23 +-
 tools/hooks/session-start-marker.sh                |    4 +-
 306 files changed, 34958 insertions(+), 128 deletions(-)
```

## Uncommitted bei Session-Ende

```
?? docs/handoffs/auto-handoff-2026-08-18-1307-57cfdfae.md
```

---
_Auto-generiert von `session-end-handoff.sh`. Reichere Memos: Memory `project_session_*.md`._
