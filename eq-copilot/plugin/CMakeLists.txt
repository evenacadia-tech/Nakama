# Transparentes Sammler-VST3 + Konsolen-Probe + Nulltest + Golden-Test.
#
# Vier Targets, ein Quellstand:
#  - EqCopilot (VST3)     — das Plugin selbst (M1: mit AnalyseEngine)
#  - EqCopPipeProbe       — Konsole: spielt einen Sensor gegen den laufenden Broker
#  - EqCopNullTest        — Konsole: Bitgleichheits-/NaN-/Latenz-Beweis ohne Host
#  - EqCopGoldenTest      — Konsole: M1-Kreuzvalidierung gegen fixtures/golden-referenz.json
#
# GETEILTE QUELLEN, Stand S8 (SONDE-007a, 22.08.2026): Bis hierher stand an
# dieser Stelle "Probe und Tests kompilieren die geteilten Quellen erneut,
# statt eine static-lib mit fremden JUCE-Moduldefinitionen zu teilen — bewusst
# simpel." Das war richtig fuer EIN Bundle. Fuer Gen, Probeeq und Suna ist es
# falsch: dieselbe Quelle waere dann bis zu dreimal je Ziel uebersetzt, und
# jede dieser Uebersetzungen koennte eine fremde Identitaet einatmen.
#
# Ab jetzt gilt der Kern NakamaKern (cmake/NakamaKern.cmake): eine Static-Lib,
# einmal uebersetzt, ohne JucePlugin_*-Konstante — die "fremden JUCE-
# Moduldefinitionen" aus dem alten Satz sind der Grund, WARUM er sie nicht
# sehen darf, nicht ein Grund gegen die Lib (Entwurf §53.4).
# Der uebrige Produktcode (src/) bleibt vorerst je Ziel uebersetzt; sein Umzug
# hinter die §53.4-Grenzen ist ausdruecklich inkrementell und gehoert nicht in
# dieses Ticket.

# ── S9/SONDE-007b: Identitaet aus dem Manifest, nicht aus dieser Datei ──────
# NAK-52 geschlossen. Bis 23.08. standen COMPANY_NAME, PLUGIN_CODE und
# PRODUCT_NAME hier als Literale — eine zweite Wahrheit neben
# identity/plugin-identities-v1.json, die EqCopIdentityTest gegeneinander
# messen musste. Mit drei Zielen waeren daraus zwoelf Literale geworden.
# Jetzt liest das Bauskript das Manifest; der Test misst weiterhin das GEBAUTE
# moduleinfo.json gegen dieselbe Datei — zwei Wege zur selben Zahl, sonst
# waere er eine Tautologie.
nakama_identitaet_lesen(main NAKAMA_MAIN)

juce_add_plugin(EqCopilot
    COMPANY_NAME "${NAKAMA_MAIN_HERSTELLER}"
    PRODUCT_NAME "${NAKAMA_MAIN_PRODUKTNAME}"
    PLUGIN_MANUFACTURER_CODE ${NAKAMA_MAIN_HERSTELLERCODE}
    PLUGIN_CODE ${NAKAMA_MAIN_PLUGINCODE}
    FORMATS VST3
    VST3_CATEGORIES ${NAKAMA_MAIN_KATEGORIEN}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(EqCopilot PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/PluginFactory.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)

# PUBLIC ist hier Absicht und kein Versehen: JUCE haengt das VST3-Ziel
# (EqCopilot_VST3) PRIVATE an dieses Ziel, also erreichen nur die INTERFACE-
# Eigenschaften den gepatchten Wrapper. Genau so kommt schon heute
# JUCE_VST3_CAN_REPLACE_VST2=0 dort an (vom Identitaetstest bewiesen) —
# NAKAMA_HOST_BRIDGE=1 nimmt denselben Weg.
target_compile_definitions(EqCopilot PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NAKAMA_HOST_BRIDGE=1
)

target_include_directories(EqCopilot PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/hostbridge")

# ── S8/SONDE-007a: der gemeinsame Kern als eigene Static-Lib ────────────────
# Inhalt: State-Schema 2 + Parameterbestand + RFC-8785-Kanon (SONDE-006) und
# der v3-Textriegel (SONDE-005a). Beides ist JUCE-core-Code ohne eine einzige
# JucePlugin_*-Konstante — genau die Menge, die Gen, Probeeq und Suna teilen
# duerfen. Bis 22.08. wurde sie je Ziel neu uebersetzt (sieben Kopien);
# ab hier einmal.
#
# Die drei Riegel dazu stehen in cmake/NakamaKern.cmake (K1/K2) und in
# tools/eq-copilot/pruefe_kern_identitaetsfrei.py (K3).
# S9/SONDE-007b Abschnitt 3: NakamaLebenslauf.cpp kommt dazu - die
# Lifecycle-Klassifikation (§53.5) ist reine Regelarbeit ueber dem State und
# gilt fuer alle drei Bundles. Sie gehoert damit genau in die Menge, die Gen,
# Probeeq und Suna teilen duerfen; das Kanon-Bein A14 fuehrt die Objektliste
# handgeschrieben und hat den Zuwachs beim ersten Lauf gemeldet.
set(NAKAMA_KERN_QUELLEN
    state/NakamaKanon.cpp
    state/NakamaLebenslauf.cpp
    state/NakamaParameter.cpp
    state/NakamaState.cpp
    vertrag/NakamaVertrag.cpp
)

# Kopf-Fassade: JUCE-Koepfe ohne die JUCE-Modulquellen. Warum das noetig ist
# und was daran gemessen wurde, steht im Kopf von cmake/NakamaKern.cmake.
nakama_kern_juce_fassade(NakamaKernJuce
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_cryptography)

# Die JUCE-Konfigurationsschrauben des Projekts. Sie stehen hier, weil der Kern
# dieselben Kopfdateien uebersetzt wie die Ziele: waere JUCE_USE_CURL hier
# ungesetzt, saehe der Kern den Vorgabewert 1 aus juce_core.h und das Plugin
# die 0 — derselbe Header, zwei Bedeutungen. Dass die drei hier vollstaendig
# sind, misst K2b unten gegen das Plugin-Ziel.
target_compile_definitions(NakamaKernJuce INTERFACE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

add_library(NakamaKern STATIC ${NAKAMA_KERN_QUELLEN})
target_include_directories(NakamaKern PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/state" "${CMAKE_CURRENT_SOURCE_DIR}/vertrag")
target_link_libraries(NakamaKern PRIVATE NakamaKernJuce)
# T2-Befund 23.08.: Diese beiden hingen bis dahin NICHT am Kern. Vor S8 lagen
# die vier Quellen per target_sources() IN den Verbraucherzielen und erbten
# deren PUBLIC-Empfehlungsschalter — allen voran /W4. Die Kopf-Fassade leitet
# nur aus den MODUL-Zielen ab, und die Empfehlungsziele sind keine Module: der
# geteilte Boden aller drei Apps uebersetzte damit als einziger Code im Baum
# unter der cl-Vorgabe /W1. Nachgezogen; K2c unten haelt es fest.
# Kein juce_recommended_lto_flags: dessen /GL-Objekte gingen in Konsolenziele,
# die kein -LTCG linken.
target_link_libraries(NakamaKern PRIVATE
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
# Schaltet K1 scharf (plugin/state/NakamaKernRiegel.h). Nur PRIVATE: dieselben
# Kopfdateien liest auch die Target-Schicht, und DORT sind die Konstanten der
# Zweck und kein Fehler.
target_compile_definitions(NakamaKern PRIVATE NAKAMA_KERN_UEBERSETZUNG=1)

# K2 laeuft, sobald alle Kanten des KERNS stehen — also nach den
# target_link_libraries oben.
nakama_kern_riegel_pruefen(NakamaKern)
# K2b und K2c stehen NICHT hier: sie lesen die Eigenschaften der VERBRAUCHER,
# und die entstehen erst weiter unten. Hier gerufen saehen sie eine leere
# Menge und waeren still gruen. Der Aufruf steht am Dateiende.

# Verbraucherseite. Der Kern ist gegen JUCE-KOEPFE uebersetzt; die
# Implementierung (juce_core.cpp, SHA-256 aus juce_cryptography, ...) muss vom
# verbrauchenden Ziel kommen. Deshalb stehen die Module hier und nicht am Kern:
# so gibt es sie im ganzen Baum genau einmal je Ziel statt zweimal.
function(nakama_kern_anbinden ziel)
    target_link_libraries(${ziel} PRIVATE
        NakamaKern
        juce::juce_core
        juce::juce_events
        juce::juce_data_structures
        juce::juce_cryptography)
    # Jeder Verbraucher traegt sich ein. K2b und K2c messen am Ende dieser
    # Datei gegen JEDEN davon - offene Frage aus SONDE-007a B8 ("ob der Kern
    # gegen alle drei geprueft wird oder die drei untereinander gleich
    # konfiguriert sein muessen"). Gegen jeden ist strikt staerker und
    # braucht keine Annahme darueber, ob die drei sich einig sind.
    set_property(GLOBAL APPEND PROPERTY NAKAMA_KERN_VERBRAUCHER "${ziel}")
endfunction()
nakama_kern_anbinden(EqCopilot)

target_link_libraries(EqCopilot
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# ── S9/SONDE-007b: die beiden neuen Sonden-Bundles ─────────────────────────
# Zwei duenne Target-Schichten ueber EINER geteilten Quelle (plugin/sonde/).
# Identisch ist alles ausser drei Dingen, und genau die machen die Ziele aus:
#   1. die Identitaet - aus dem Manifest, nicht aus dieser Datei
#   2. die Produktklasse - als Define, weil der geteilte Code sie nicht wissen
#      darf (er wuerde sonst fuer beide Bundles dasselbe behaupten)
#   3. die VST3-Kategorie - Suna analysiert, Probeeq ist ein EQ (User 21.08.)
#
# Beide sind heute audio-neutral: Suna dauerhaft (sie beraet nur), Probeeq
# bis seine DSP in P6 kommt. Ein Bundle, das schon Parameter meldete, ohne
# dass sie etwas taeten, waere eine Oberflaeche, die luegt.
function(nakama_sonde_ziel ziel identitaet klassendefine)
    nakama_identitaet_lesen(${identitaet} SONDE)

    juce_add_plugin(${ziel}
        COMPANY_NAME "${SONDE_HERSTELLER}"
        PRODUCT_NAME "${SONDE_PRODUKTNAME}"
        PLUGIN_MANUFACTURER_CODE ${SONDE_HERSTELLERCODE}
        PLUGIN_CODE ${SONDE_PLUGINCODE}
        FORMATS VST3
        VST3_CATEGORIES ${SONDE_KATEGORIEN}
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    target_sources(${ziel} PRIVATE sonde/SondeProcessor.cpp sonde/SondeFactory.cpp)
    target_include_directories(${ziel} PRIVATE sonde)

    # JUCE_VST3_CAN_REPLACE_VST2=0 gehoert zur eingefrorenen Identitaet, nicht
    # zu den Bauoptionen: mit 1 leitete JUCE die Class-ID aus einer ANDEREN
    # Quelle ab (vst2PluginId statt jucePluginId) - dieselben Codes ergaeben
    # eine andere ID. Siehe identity/plugin-identities-v1.json, "ableitung".
    target_compile_definitions(${ziel} PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        ${klassendefine}=1
    )

    target_link_libraries(${ziel}
        PRIVATE juce::juce_audio_utils
        PUBLIC  juce::juce_recommended_config_flags
                juce::juce_recommended_lto_flags
                juce::juce_recommended_warning_flags)

    nakama_kern_anbinden(${ziel})
endfunction()

nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)
nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)

# Passthrough-Beweis je Produktklasse. Das Grundgesetz sagt "sampleidentisch,
# 0 Samples Latenz, kein Tail" — fuer das Main-Bundle misst das seit M0 der
# EqCopNullTest; zwei neue Bundles tragen jetzt ebenfalls Audio, und eine
# Zusage ohne Messung ist hier keine Zusage. ZWEI Ziele, weil die
# Produktklasse ein Uebersetzungsschalter ist: ein Lauf kann nicht beide
# messen.
function(nakama_sonde_nulltest ziel klassendefine)
    juce_add_console_app(${ziel} PRODUCT_NAME "${ziel}")
    target_sources(${ziel} PRIVATE tests/SondeNullTestMain.cpp sonde/SondeProcessor.cpp)
    target_include_directories(${ziel} PRIVATE sonde)
    target_compile_definitions(${ziel} PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        ${klassendefine}=1)
    target_link_libraries(${ziel} PRIVATE juce::juce_audio_utils
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
    nakama_kern_anbinden(${ziel})
endfunction()

nakama_sonde_nulltest(EqCopSunaNullTest    NAKAMA_SONDE_PASSIV)
nakama_sonde_nulltest(EqCopProbeeqNullTest NAKAMA_SONDE_AKTIV)

# ── Konsolen-Probe: Ende-zu-Ende-Handshake ohne FL ──────────────────────────
juce_add_console_app(EqCopPipeProbe PRODUCT_NAME "eqcop-pipe-probe")
target_sources(EqCopPipeProbe PRIVATE
    probe/PipeProbeMain.cpp
    src/PipeClient.cpp
)
target_include_directories(EqCopPipeProbe PRIVATE src)
target_compile_definitions(EqCopPipeProbe PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(EqCopPipeProbe PRIVATE juce::juce_core
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── Nulltest: Passthrough-Beweis ohne Host ──────────────────────────────────
juce_add_console_app(EqCopNullTest PRODUCT_NAME "eqcop-nulltest")
target_sources(EqCopNullTest PRIVATE
    tests/NullTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopNullTest PRIVATE src)
target_compile_definitions(EqCopNullTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopNullTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopNullTest)

# ── Golden-Test: M1-Kreuzvalidierung + M3-Diagnose (Engine pur, ohne Pipe) ──
juce_add_console_app(EqCopGoldenTest PRODUCT_NAME "eqcop-goldentest")
target_sources(EqCopGoldenTest PRIVATE
    tests/GoldenTestMain.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopGoldenTest PRIVATE src)
target_compile_definitions(EqCopGoldenTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(EqCopGoldenTest PRIVATE
    juce::juce_audio_formats juce::juce_cryptography juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── Markierungstest: Hör-Markierung T2–T10 (Konzept v2 §8) ──────────────────
juce_add_console_app(EqCopMarkierungTest PRODUCT_NAME "eqcop-markierungtest")
target_sources(EqCopMarkierungTest PRIVATE
    tests/MarkierungTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopMarkierungTest PRIVATE src)
target_compile_definitions(EqCopMarkierungTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopMarkierungTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopMarkierungTest)

# ── Shot-Test: rendert den Editor offscreen als PNG (Sichtprüfung ohne FL) ──
juce_add_console_app(EqCopShot PRODUCT_NAME "eqcop-shot")
target_sources(EqCopShot PRIVATE
    tests/ShotTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopShot PRIVATE src)
target_compile_definitions(EqCopShot PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopShot PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopShot)

# ── Paint-Benchmark: reine paint()-Kosten offscreen (FPS-Beweis) ───────────
juce_add_console_app(EqCopPaintBench PRODUCT_NAME "eqcop-paintbench")
target_sources(EqCopPaintBench PRIVATE
    tests/PaintBenchMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopPaintBench PRIVATE src)
target_compile_definitions(EqCopPaintBench PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopPaintBench PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopPaintBench)

# ── SONDE-004a: Wegwerf-Messgeraet fuer Aux/PDC/Recall (User-Termin A) ──────
# Eigener Plugin-Code NkSp — bewusst NICHT Teil der eingefrorenen Identitaet
# (§31.2 reserviert Eqcp, NkPr, NkAc). Das Ziel wird nach dem Capabilityreport
# (S4) nicht weitergepflegt und teilt keine Quelle mit dem Produkt.
juce_add_plugin(EqCopAuxSpike
    COMPANY_NAME "evenacadia"
    PRODUCT_NAME "EqCop-Aux-Spike"
    PLUGIN_MANUFACTURER_CODE Evna
    PLUGIN_CODE NkSp
    FORMATS VST3
    VST3_CATEGORIES Fx Analyzer
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
target_sources(EqCopAuxSpike PRIVATE
    spike/AuxSpikeProcessor.cpp
    spike/AuxSpikeEditor.cpp
    spike/AuxSpikeFactory.cpp
)
target_include_directories(EqCopAuxSpike PRIVATE spike)
target_compile_definitions(EqCopAuxSpike PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
target_link_libraries(EqCopAuxSpike
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── Selbsttest: beweist das Lineal VOR dem FL-Termin ───────────────────────
juce_add_console_app(EqCopAuxSpikeTest PRODUCT_NAME "eqcop-auxspiketest")
target_sources(EqCopAuxSpikeTest PRIVATE
    tests/AuxSpikeTestMain.cpp
    spike/AuxSpikeProcessor.cpp
    spike/AuxSpikeEditor.cpp
)
target_include_directories(EqCopAuxSpikeTest PRIVATE spike)
target_compile_definitions(EqCopAuxSpikeTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopAuxSpikeTest PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── SONDE-001/002: Identitaets-Golden + Schema-1-State-Goldens ─────────────
# Misst das GEBAUTE Bundle gegen eq-copilot/identity/plugin-identities-v1.json
# und rechnet die reservierten CIDs nach, statt sie zu glauben.
juce_add_console_app(EqCopIdentityTest PRODUCT_NAME "eqcop-identitytest")
target_sources(EqCopIdentityTest PRIVATE
    tests/IdentityTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopIdentityTest PRIVATE src)
target_compile_definitions(EqCopIdentityTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopIdentityTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopIdentityTest)

# ── SONDE-006: State-Schema 2, Parameterbestand, Migration (Kanon B2) ──────
# Misst gegen die handgeschriebenen Vertraege (schemas/state/) und den
# Fixture-Korpus (fixtures/state/) - dieselben Dateien lesen Rust und Python.
# Linkt den ECHTEN Produktcode (plugin/state/ + PluginProcessor), keine Testkopie.
juce_add_console_app(EqCopStateMigrationTest PRODUCT_NAME "eqcop-statemigrationtest")
target_sources(EqCopStateMigrationTest PRIVATE
    tests/StateMigrationTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopStateMigrationTest PRIVATE src)
target_compile_definitions(EqCopStateMigrationTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopStateMigrationTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopStateMigrationTest)

# ── S9/SONDE-007b Abschnitt 3: Lifecycle-Klassifikation (Kanon B8) ─────────
# Zwei Ebenen in einem Ziel: der Automat pur (NakamaLebenslauf, im Kern) UND
# derselbe Automat verdrahtet im echten EqCopilotProcessor. Die zweite Ebene
# misst AUDIO - dieselbe Markierung, die EqCopMarkierungTest faerben laesst,
# muss hier stumm bleiben, solange nicht klassifiziert ist. Ein Automat ohne
# diese Haelfte waere ein Riegel, der nichts verriegelt.
juce_add_console_app(EqCopLebenslaufTest PRODUCT_NAME "eqcop-lebenslauftest")
target_sources(EqCopLebenslaufTest PRIVATE
    tests/LebenslaufTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopLebenslaufTest PRIVATE src)
target_compile_definitions(EqCopLebenslaufTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopLebenslaufTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopLebenslaufTest)

# ── S3b: Termin-B-Messgeraet (Wegwerfware wie SONDE-004a) ──────────────────
# Erstes Ziel, das die Hostbruecke BENUTZT: sein Processor ist eine Senke.
# Eigener Plugin-Code NkHp — bewusst ausserhalb der eingefrorenen Identitaet
# (§31.2: Eqcp/NkPr/NkAc) und ausserhalb des Aux-Spikes (NkSp). Wird nach dem
# Capabilityreport (S4) nicht weitergepflegt.
juce_add_plugin(EqCopHostProbe
    COMPANY_NAME "evenacadia"
    PRODUCT_NAME "EqCop-Host-Probe"
    PLUGIN_MANUFACTURER_CODE Evna
    PLUGIN_CODE NkHp
    FORMATS VST3
    VST3_CATEGORIES Fx Analyzer
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
target_sources(EqCopHostProbe PRIVATE
    hostprobe/HostProbeProcessor.cpp
    hostprobe/HostProbeEditor.cpp
    hostprobe/HostProbeFactory.cpp
)
target_include_directories(EqCopHostProbe PUBLIC hostprobe "${CMAKE_CURRENT_SOURCE_DIR}/hostbridge")
target_compile_definitions(EqCopHostProbe PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    NAKAMA_HOST_BRIDGE=1
)
target_link_libraries(EqCopHostProbe
    PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── Selbsttest: beweist das Lineal VOR dem Termin ──────────────────────────
juce_add_console_app(EqCopHostProbeTest PRODUCT_NAME "eqcop-hostprobetest")
target_sources(EqCopHostProbeTest PRIVATE
    tests/HostProbeTestMain.cpp
    hostprobe/HostProbeProcessor.cpp
    hostprobe/HostProbeEditor.cpp
)
target_include_directories(EqCopHostProbeTest PRIVATE hostprobe hostbridge)
target_compile_definitions(EqCopHostProbeTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopHostProbeTest PRIVATE juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── SONDE-003: Wrapper-Unitfixture der Hostbruecke ─────────────────────────
# Zwei Haelften in einem Ziel: das Quellhash-Gate misst die GEPATCHTE
# JUCE-Datei auf Platte, die Bruecken-Pruefungen fahren die Abbildung gegen den
# ECHTEN Steinberg::Vst::ProcessContext (kein Nachbau, der still falsch sein
# koennte). Der gepinnte Quellstand kommt aus cmake/NakamaBruecke.cmake und
# wird ueber eine generierte Kopfdatei hereingereicht — eine Wahrheit, zwei Leser.
set(NAKAMA_JUCE_WRAPPER_ABS "${juce_SOURCE_DIR}/${NAKAMA_JUCE_WRAPPER_REL}")
set(NAKAMA_BRUECKE_PATCH_ABS "${NAKAMA_WURZEL}/third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/hostbridge/NakamaBrueckeStand.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/nakama-generiert/NakamaBrueckeStand.h"
    @ONLY)

juce_add_console_app(EqCopHostContextTest PRODUCT_NAME "eqcop-hostcontexttest")
target_sources(EqCopHostContextTest PRIVATE tests/HostContextTestMain.cpp)
target_include_directories(EqCopHostContextTest PRIVATE
    hostbridge
    "${CMAKE_CURRENT_BINARY_DIR}/nakama-generiert"
    "${juce_SOURCE_DIR}/modules/juce_audio_processors/format_types/VST3_SDK")
target_compile_definitions(EqCopHostContextTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopHostContextTest PRIVATE juce::juce_core juce::juce_cryptography
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── SONDE-005a: v3-Vertrag (Schemas, Bandgitter, Cross-Language-Fixtures) ──
# Dritte Partei zwischen C++ und Rust: beide Engines messen gegen dasselbe
# handgeschriebene eq-copilot/fixtures/v3/MANIFEST.json. Das Ziel linkt den
# ECHTEN Quellpfad (plugin/vertrag/), keine Testkopie - ab SONDE-010 ist das
# die Eingangspruefung des C++-IPC-Clients.
juce_add_console_app(EqCopSchemaTest PRODUCT_NAME "eqcop-schematest")
# NakamaVertrag.cpp kommt seit S8 aus NakamaKern (eine Uebersetzung statt zwei);
# NakamaTelemetrie.cpp bleibt hier, weil es die FlatBuffers-Laufzeit braucht und
# heute nur dieses Ziel es liest — sein Umzug in den Kern gehoert zu SONDE-010.
target_sources(EqCopSchemaTest PRIVATE
    tests/SchemaTestMain.cpp
    vertrag/NakamaTelemetrie.cpp
)
target_include_directories(EqCopSchemaTest PRIVATE vertrag)
target_compile_definitions(EqCopSchemaTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
# SONDE-005b: die FlatBuffers-Laufzeit stammt aus DEMSELBEN gepinnten Commit
# wie der flatc, der den Code erzeugt hat. Der erzeugte Header traegt dazu von
# sich aus ein static_assert auf die Version - ein dritter, unabhaengiger
# Riegel derselben Aussage.
target_link_libraries(EqCopSchemaTest PRIVATE juce::juce_core flatbuffers
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopSchemaTest)

# ── S9/SONDE-007b Abschnitt 3: welchen Bundlevertrag src/ laedt ────────────
# Bis 23.08. stand `Bundle::eqcp()` als Literal in PluginProcessor.cpp — an
# genau der Stelle, an der die beiden neuen Ziele ihre Klasse laengst aus der
# duennen Target-Schicht bekommen (Manifest SONDE-007b §3). Jetzt sagt auch
# hier die Schicht, welches Bundle uebersetzt wird; der geteilte Quelltext
# behauptet es nicht selbst.
#
# Der Riegel dazu ist ein #error in PluginProcessor.h: ein kuenftiges Ziel,
# das src/ uebersetzt, ohne sich zu erklaeren, ist nicht baubar — statt still
# den Vertrag des Main-Bundles zu erben. Genau dieselbe Form wie
# NAKAMA_SONDE_PASSIV/AKTIV in sonde/SondeProcessor.h.
foreach(ziel EqCopilot
             EqCopNullTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest)
    target_compile_definitions(${ziel} PRIVATE NAKAMA_BUNDLE_MAIN=1)
endforeach()

# Die Quellen sind UTF-8 (deutsche UI-Texte). Ohne /utf-8 rät MSVC die
# Codepage — Literale wie „Klavier-Bus“ hingen dann an der Systemlocale.
foreach(ziel NakamaKern
             EqCopilot EqCopPipeProbe EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopAuxSpike EqCopAuxSpikeTest EqCopHostContextTest
             EqCopSchemaTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopHostProbe EqCopHostProbeTest
             NakamaSuna NakamaProbeeq EqCopSunaNullTest EqCopProbeeqNullTest)
    target_compile_options(${ziel} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endforeach()

# ── K2b/K2c am Dateiende: jetzt stehen alle Verbraucher ────────────────────
# Erst hier ist die Frage stellbar. SONDE-007a liess offen, ob der Kern gegen
# EINE Referenz oder gegen alle geprueft wird (B8, "Fuer S9 offen"); mit drei
# Bundles ist die Antwort: gegen jeden einzelnen. Waeren zwei Verbraucher
# untereinander uneins, faende eine Stichprobe das nicht.
get_property(NAKAMA_KERN_VERBRAUCHER GLOBAL PROPERTY NAKAMA_KERN_VERBRAUCHER)
list(LENGTH NAKAMA_KERN_VERBRAUCHER NAKAMA_KERN_VERBRAUCHER_ANZAHL)
if(NAKAMA_KERN_VERBRAUCHER_ANZAHL EQUAL 0)
    message(FATAL_ERROR
        "S8/SONDE-007a: kein einziger Verbraucher von NakamaKern eingetragen. Entweder wurde\n"
        "nakama_kern_anbinden() nie gerufen, oder jemand hat den Kern an der Funktion vorbei\n"
        "angebunden — dann messen K2b und K2c ins Leere und melden still gruen.")
endif()
foreach(verbraucher IN LISTS NAKAMA_KERN_VERBRAUCHER)
    nakama_kern_konfig_pruefen(NakamaKern ${verbraucher})
    nakama_kern_schalter_pruefen(NakamaKern ${verbraucher})
endforeach()
message(STATUS
    "Nakama-Kern: K2b/K2c gegen alle ${NAKAMA_KERN_VERBRAUCHER_ANZAHL} Verbraucher gemessen.")
