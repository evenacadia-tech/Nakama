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
# simpel." Das war richtig fuer EIN Bundle. Fuer mehrere Bundles ist es
# falsch: dieselbe Quelle waere dann je Ziel erneut uebersetzt, und jede
# dieser Uebersetzungen koennte eine fremde Identitaet einatmen.
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
# messen musste. Mit mehreren Zielen waeren daraus je vier Literale geworden.
# Jetzt liest das Bauskript das Manifest; der Test misst weiterhin das GEBAUTE
# moduleinfo.json gegen dieselbe Datei — zwei Wege zur selben Zahl, sonst
# waere er eine Tautologie.
nakama_identitaet_lesen(main NAKAMA_MAIN)

# SONDE-011 Phase B / NAK-13: Der Plugin-Worker verifiziert nicht gegen eine
# zweite Laufzeitdatei. CMake bindet die drei Startwerte unmittelbar aus dem
# Installer-Manifest in einen generierten Header; unvollstaendige oder
# mehrdeutige Brokerangaben sind ein Konfigurationsfehler statt ein spaeterer
# unsicherer Vorgabewert.
set(NAKAMA_INSTALLER_MANIFEST
    "${CMAKE_CURRENT_SOURCE_DIR}/../install/nakama-installer-v1.json")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
             "${NAKAMA_INSTALLER_MANIFEST}")
file(READ "${NAKAMA_INSTALLER_MANIFEST}" NAKAMA_INSTALLER_JSON)
string(JSON NAKAMA_BROKER_VERZEICHNIS GET "${NAKAMA_INSTALLER_JSON}"
       ziele broker_verzeichnis)
string(JSON NAKAMA_ARTEFAKT_ANZAHL LENGTH "${NAKAMA_INSTALLER_JSON}" artefakte)
set(NAKAMA_BROKER_GEFUNDEN 0)
math(EXPR NAKAMA_ARTEFAKT_LETZTES "${NAKAMA_ARTEFAKT_ANZAHL} - 1")
foreach(NAKAMA_ARTEFAKT_INDEX RANGE 0 ${NAKAMA_ARTEFAKT_LETZTES})
    string(JSON NAKAMA_ARTEFAKT_ART GET "${NAKAMA_INSTALLER_JSON}"
           artefakte ${NAKAMA_ARTEFAKT_INDEX} art)
    if(NAKAMA_ARTEFAKT_ART STREQUAL "broker")
        math(EXPR NAKAMA_BROKER_GEFUNDEN "${NAKAMA_BROKER_GEFUNDEN} + 1")
        string(JSON NAKAMA_BROKER_NAME GET "${NAKAMA_INSTALLER_JSON}"
               artefakte ${NAKAMA_ARTEFAKT_INDEX} name)
        string(JSON NAKAMA_BROKER_SHA256 GET "${NAKAMA_INSTALLER_JSON}"
               artefakte ${NAKAMA_ARTEFAKT_INDEX} sha256)
    endif()
endforeach()
if(NOT NAKAMA_BROKER_GEFUNDEN EQUAL 1)
    message(FATAL_ERROR
        "nakama-installer-v1.json muss genau ein Broker-Artefakt enthalten")
endif()
string(LENGTH "${NAKAMA_BROKER_SHA256}" NAKAMA_BROKER_SHA256_LAENGE)
if(NOT NAKAMA_BROKER_SHA256_LAENGE EQUAL 64
   OR NOT NAKAMA_BROKER_SHA256 MATCHES "^[0-9A-Fa-f]+$")
    message(FATAL_ERROR "Broker-SHA-256 im Installer-Manifest ist ungueltig")
endif()
string(JSON NAKAMA_BROKER_THUMBPRINT_TYP TYPE "${NAKAMA_INSTALLER_JSON}"
       signatur authenticode_thumbprint)
if(NAKAMA_BROKER_THUMBPRINT_TYP STREQUAL "NULL")
    set(NAKAMA_BROKER_THUMBPRINT "")
else()
    string(JSON NAKAMA_BROKER_THUMBPRINT GET "${NAKAMA_INSTALLER_JSON}"
           signatur authenticode_thumbprint)
    string(LENGTH "${NAKAMA_BROKER_THUMBPRINT}"
           NAKAMA_BROKER_THUMBPRINT_LAENGE)
    if((NOT NAKAMA_BROKER_THUMBPRINT_LAENGE EQUAL 40
        AND NOT NAKAMA_BROKER_THUMBPRINT_LAENGE EQUAL 64)
       OR NOT NAKAMA_BROKER_THUMBPRINT MATCHES "^[0-9A-Fa-f]+$")
        message(FATAL_ERROR "authenticode_thumbprint im Installer-Manifest ist ungueltig")
    endif()
endif()
set(NAKAMA_BROKER_INSTALL_PFAD
    "${NAKAMA_BROKER_VERZEICHNIS}/${NAKAMA_BROKER_NAME}")
set(NAKAMA_BROKER_BINDING_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/nakama")
file(MAKE_DIRECTORY "${NAKAMA_BROKER_BINDING_DIR}")
configure_file(core/ipc/BrokerInstallBinding.h.in
               "${NAKAMA_BROKER_BINDING_DIR}/BrokerInstallBinding.h" @ONLY)

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
# JucePlugin_*-Konstante — genau die Menge, die Gen und Probeeq teilen
# duerfen. Bis 22.08. wurde sie je Ziel neu uebersetzt (sieben Kopien);
# ab hier einmal.
#
# Die fuenf Riegel dazu: K1 steht am Anfang und Ende jeder Kernquelle;
# K2/K2b/K2c stehen in cmake/NakamaKern.cmake, K3/A14 in
# tools/eq-copilot/pruefe_kern_identitaetsfrei.py. Ihre exakt gemessene
# Arbeitsteilung steht in der Riegeluebersicht von SONDE-007a.
# S9/SONDE-007b Abschnitt 3: NakamaLebenslauf.cpp kommt dazu - die
# Lifecycle-Klassifikation (§53.5) ist reine Regelarbeit ueber dem State und
# gilt fuer jedes Bundle. Sie gehoert damit genau in die Menge, die Gen und
# Probeeq teilen duerfen; das Kanon-Bein A14 fuehrt die Objektliste
# handgeschrieben und hat den Zuwachs beim ersten Lauf gemeldet.
# S14-15/SONDE-010: der v3-IPC-Weg kommt dazu. Envelope, Pipe-Mechanik und die
# beiden Clients gelten fuer JEDES Bundle, das spaeter mit dem Broker spricht -
# sie gehoeren damit in dieselbe Menge wie State und Vertrag. Die Quellen sind
# JUCE-frei (wie core/StampedAudioQueue.h); sie ziehen nur <windows.h> und die
# C++-Standardbibliothek. A14 hat den Zuwachs beim ersten Lauf gemeldet - genau
# dafuer ist seine Objektliste unabhaengig von dieser hier gefuehrt.
#
# NICHT im Kern: core/ipc/PipeToken.cpp. Es traegt den v3-Pipe-Namensraum
# `evenacadia.nakama|v3|` - und `evenacadia` ist der eingefrorene
# Herstellername aus plugin-identities-v1.json. A14 hat genau das beim ersten
# Lauf gefunden. Die Antwort ist nicht, den Riegel weicher zu machen, sondern
# die Schichtung ernst zu nehmen: WELCHE Pipe eine Instanz waehlt, ist dieselbe
# Art Aussage wie WELCHE Identitaet sie traegt - sie gehoert in die duenne
# Zielschicht, nicht in den geteilten Boden. Der Kern nimmt den Pipenamen
# deshalb als Zeichenkette entgegen und rechnet ihn nie selbst aus. Die Quelle
# bleibt EINE Datei; sie wird von den Zielen uebersetzt, die sie brauchen -
# dieselbe Form wie src/PipeClient.cpp.
set(NAKAMA_KERN_QUELLEN
    state/NakamaKanon.cpp
    state/NakamaLebenslauf.cpp
    state/NakamaParameter.cpp
    state/NakamaState.cpp
    vertrag/NakamaVertrag.cpp
    vertrag/NakamaTelemetrie.cpp
    core/ipc/WireEnvelope.cpp
    core/ipc/IpcVerbindung.cpp
    core/ipc/BrokerLifecycle.cpp
    core/ipc/ControlClient.cpp
    core/ipc/TelemetryClient.cpp
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
    "${CMAKE_CURRENT_SOURCE_DIR}/state" "${CMAKE_CURRENT_SOURCE_DIR}/vertrag"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/ipc")
target_link_libraries(NakamaKern PRIVATE NakamaKernJuce flatbuffers
    PUBLIC Advapi32 Bcrypt Crypt32 Wintrust)
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
# Schaltet K1 scharf (plugin/state/NakamaKernRiegel.h, bewusst ohne
# Include-Guard und am Anfang/Ende jeder Kernquelle eingebunden). Nur PRIVATE:
# dieselben Kopfdateien liest auch die Target-Schicht, und DORT sind die
# Konstanten der Zweck und kein Fehler.
target_compile_definitions(NakamaKern PRIVATE NAKAMA_KERN_UEBERSETZUNG=1)

# K2 wird hier registriert, laeuft aber erst am Ende dieses Verzeichnisses.
# Damit sieht er auch spaetere target_compile_definitions/-options/-libraries
# am Kern. Seine Zielmenge bleibt nur NakamaKern plus dessen compilerwirksame
# Usage-Requirements-Huelle; registrierte Verbraucher sind keine Kernquellen.
cmake_language(DEFER DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    CALL nakama_kern_riegel_pruefen NakamaKern)
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
    # gegen alle geprueft wird oder die Verbraucher untereinander gleich
    # konfiguriert sein muessen"). Gegen jeden ist strikt staerker und
    # braucht keine Annahme darueber, ob sie sich einig sind.
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

# ── S9/SONDE-007b: die duenne Target-Schicht der Sonden  ─────────────────────────
# Eine duenne Target-Schicht ueber der geteilten Quelle (plugin/sonde/).
# Identisch ist alles ausser drei Dingen, und genau die machen ein Ziel aus:
#   1. die Identitaet - aus dem Manifest, nicht aus dieser Datei
#   2. die Produktklasse - als Define, weil der geteilte Code sie nicht wissen
#      darf (er wuerde sonst fuer jedes Bundle dasselbe behaupten)
#   3. die VST3-Kategorie - Probeeq ist ein EQ (User 21.08.)
#
# Probeeq ist heute audio-neutral, bis seine DSP in P6 kommt. Ein Bundle, das
# schon Parameter meldete, ohne dass sie etwas taeten, waere eine Oberflaeche,
# die luegt.
#
# S9b/SONDE-007c (28.08.2026): die Funktion bleibt eine FUNKTION, obwohl sie
# heute nur einen Aufrufer hat. Sie ist die Stelle, an der ein Sondenziel
# entstehen DARF - EqCopIdentityTest friert genau diesen einen Block ein und
# misst, dass jedes Sondenziel ueber ihn entsteht. Ein zweiter, eigener
# juce_add_plugin-Block haette dort keinen Frost.
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

    target_sources(${ziel} PRIVATE
        sonde/SondeProcessor.cpp
        sonde/SondeFactory.cpp
        src/AnalyseEngine.cpp)
    target_sources(${ziel} PRIVATE core/ipc/PipeToken.cpp)
    target_include_directories(${ziel} PRIVATE sonde src core hostbridge
        "${NAKAMA_BROKER_BINDING_DIR}")

    # JUCE_VST3_CAN_REPLACE_VST2=0 gehoert zur eingefrorenen Identitaet, nicht
    # zu den Bauoptionen: mit 1 leitete JUCE die Class-ID aus einer ANDEREN
    # Quelle ab (vst2PluginId statt jucePluginId) - dieselben Codes ergaeben
    # eine andere ID. Siehe identity/plugin-identities-v1.json, "ableitung".
    target_compile_definitions(${ziel} PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        NAKAMA_HOST_BRIDGE=1
        ${klassendefine}=1
    )

    target_link_libraries(${ziel}
        PRIVATE juce::juce_audio_utils juce::juce_dsp Advapi32
        PUBLIC  juce::juce_recommended_config_flags
                juce::juce_recommended_lto_flags
                juce::juce_recommended_warning_flags)

    nakama_kern_anbinden(${ziel})
endfunction()

# ── STILLGELEGT 28.08.2026: das Ziel NakamaSuna (passive-probe, NkPr) ────
# Hier stand bis zum 28.08.2026 der Aufruf fuer das dritte Ziel - Bundle
# "Nakama Suna.vst3". Er ist ENTFERNT, nicht auskommentiert-vergessen:
#
#   Ticket    S9b / SONDE-007c
#   Entscheid design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md
#   User      "suna gibt es nichtmehr . nurnoch Nakama Gen mit 2 Oberflaechen
#              und Nakama Probeeq"  /  "ja stell das einfach richtig"
#
# Suna ist seit dem 23.08.2026 in Nakama Probeeq aufgegangen: dieselbe
# Instanz misst passiv auf ihrem Bus UND fuehrt bei zugeschaltetem EQ Gens
# Fernsteuerung aus. Eine eigene, rein passive Sonde hat damit keine
# Produktrolle mehr - und ein Bundle, das kein Produkt ist, gehoert nicht in
# die Auslieferung.
#
# Was NICHT passiert ist: die Kennung ist nicht geloescht. NkPr, beide
# Class-IDs und der Bundlename stehen weiter in
# identity/plugin-identities-v1.json und bleiben gesperrt (NAK-30). Wer den
# Aufruf hier wieder einsetzte, braeuchte den Eintrag - und
# nakama_identitaet_lesen() bricht bei einem stillgelegten Ziel ab. Der
# Riegel liegt damit am Manifest, nicht an dieser Zeile: eine geloeschte
# Zeile allein waere nur eine Meinung.
nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)

# Passthrough-Beweis je Produktklasse. Das Grundgesetz sagt "sampleidentisch,
# 0 Samples Latenz, kein Tail" — fuer das Main-Bundle misst das seit M0 der
# EqCopNullTest; das Sondenbundle traegt ebenfalls Audio, und eine Zusage
# ohne Messung ist hier keine Zusage. Die Produktklasse ist ein
# Uebersetzungsschalter: ein Lauf misst genau eine.
function(nakama_sonde_nulltest ziel klassendefine)
    juce_add_console_app(${ziel} PRODUCT_NAME "${ziel}")
    target_sources(${ziel} PRIVATE
        tests/SondeNullTestMain.cpp
        sonde/SondeProcessor.cpp
        src/AnalyseEngine.cpp)
    target_sources(${ziel} PRIVATE core/ipc/PipeToken.cpp)
    target_include_directories(${ziel} PRIVATE sonde src core hostbridge
        "${NAKAMA_BROKER_BINDING_DIR}")
    target_compile_definitions(${ziel} PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        ${klassendefine}=1 NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
    target_link_libraries(${ziel} PRIVATE juce::juce_audio_utils juce::juce_dsp Advapi32
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
    nakama_kern_anbinden(${ziel})
endfunction()

# STILLGELEGT 28.08.2026 (S9b/SONDE-007c): das Bein fuer die passive Klasse
# ist mit seinem Ziel gegangen. Es mass Nakama Suna, und Nakama Suna wird
# nicht mehr gebaut - ein Passthrough-Beweis fuer ein Bundle, das niemand
# ausliefert, beweist nichts. Im Kanon (tools/beweise.ps1) steht die Zeile
# A15 weiter, ausdruecklich als stillgelegt markiert: der Runner darf eine
# Pruefung nie STILL verlieren, sonst sinkt die Zahl und niemand weiss warum.
nakama_sonde_nulltest(EqCopProbeeqNullTest NAKAMA_SONDE_AKTIV)

# ── SONDE-012 B3a: Probeeq-Producer und lokaler ChannelContext ─────────────
# Zwei deklarierte Beine statt eines Sammeltests: Host-/Message-Thread und
# Audioqueue-/Analyseworker haben verschiedene Fehlermodi und muessen im Kanon
# einzeln rot werden koennen.
function(nakama_sonde012_test ziel quelle)
    juce_add_console_app(${ziel} PRODUCT_NAME "${ziel}")
    target_sources(${ziel} PRIVATE
        ${quelle}
        sonde/SondeProcessor.cpp
        src/AnalyseEngine.cpp
        core/ipc/PipeToken.cpp)
    target_include_directories(${ziel} PRIVATE sonde src core hostbridge vertrag
        "${NAKAMA_BROKER_BINDING_DIR}")
    target_compile_definitions(${ziel} PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
        NAKAMA_SONDE_AKTIV=1 NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
    target_link_libraries(${ziel} PRIVATE
        juce::juce_audio_utils juce::juce_dsp flatbuffers Advapi32
        PUBLIC juce::juce_recommended_config_flags
               juce::juce_recommended_warning_flags)
    nakama_kern_anbinden(${ziel})
endfunction()

nakama_sonde012_test(EqCopSonde012HostChannelContextTest
                     tests/Sonde012HostChannelContextTest.cpp)
nakama_sonde012_test(EqCopSonde012LoudnessSourceTest
                     tests/Sonde012LoudnessSourceTest.cpp)

# ── G3-SOAK: die C++-Haelfte des Dauerlaufbeins A24 ───────────────────────
# Kein Kanon-Bein fuer sich: tools/eq-copilot/pruefe_session_soak.py faehrt es
# gegen den echten Rust-Coordinator auf einer Probe-Pipe und besitzt
# Prozesslebenszyklus, Pipenamen-Riegel, Brokerneustart und Speicherkurve.
# Es braucht BEIDE Welten: den echten SondeProcessor am Ganzblockpfad (wie B12)
# und das echte Main-SourcesModel (wie A23). NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3
# haelt die eigenen v3-Clients des SondeProcessor aus - sonst oeffnete eine
# Testbinaerdatei die PRODUKTIONS-Pipe (Manifest G3-SOAK §2.2, §8).
juce_add_console_app(EqCopSessionSoak PRODUCT_NAME "eqcop-sessionsoak")
target_sources(EqCopSessionSoak PRIVATE
    tests/SessionSoakMain.cpp
    sonde/SondeProcessor.cpp
    src/AnalyseEngine.cpp
    src/SourcesModel.cpp
    core/ipc/PipeToken.cpp)
target_include_directories(EqCopSessionSoak PRIVATE sonde src core hostbridge vertrag
    "${NAKAMA_BROKER_BINDING_DIR}")
target_compile_definitions(EqCopSessionSoak PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
    NAKAMA_SONDE_AKTIV=1 NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
target_link_libraries(EqCopSessionSoak PRIVATE
    juce::juce_audio_utils juce::juce_dsp flatbuffers Advapi32
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopSessionSoak)

# ── SONDE-012 B3b: Main-Quellenmodell und Save/Load-Gegenpfad ─────────────
juce_add_console_app(EqCopSonde012SourcesModelTest
                     PRODUCT_NAME "eqcop-sonde012-sourcesmodeltest")
target_sources(EqCopSonde012SourcesModelTest PRIVATE
    tests/Sonde012SourcesModelTest.cpp
    src/SourcesModel.cpp)
target_include_directories(EqCopSonde012SourcesModelTest PRIVATE src vertrag core)
target_compile_definitions(EqCopSonde012SourcesModelTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
    NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
target_link_libraries(EqCopSonde012SourcesModelTest PRIVATE juce::juce_core flatbuffers
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopSonde012SourcesModelTest)

juce_add_console_app(EqCopSonde012ProjectReloadTest
                     PRODUCT_NAME "eqcop-sonde012-projectreloadtest")
target_sources(EqCopSonde012ProjectReloadTest PRIVATE
    tests/Sonde012ProjectReloadTest.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp)
target_include_directories(EqCopSonde012ProjectReloadTest PRIVATE src)
target_compile_definitions(EqCopSonde012ProjectReloadTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopSonde012ProjectReloadTest PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopSonde012ProjectReloadTest)

# B3c / E-L06: von A23 gegen den echten Rust-Coordinator auf einer Probe-Pipe
# gestartet. Das Programm ist kein isoliertes Kanonbein; der Python-Runner
# besitzt Prozesslebenszyklus, Pipenamen-Riegel und das p95-Urteil.
juce_add_console_app(EqCopSonde012SourcesLatencyTest
                     PRODUCT_NAME "eqcop-sonde012-sourceslatencytest")
target_sources(EqCopSonde012SourcesLatencyTest PRIVATE
    tests/Sonde012SourcesLatencyTest.cpp
    src/SourcesModel.cpp
    core/ipc/PipeToken.cpp)
target_include_directories(EqCopSonde012SourcesLatencyTest PRIVATE src core vertrag)
target_compile_definitions(EqCopSonde012SourcesLatencyTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0
    NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
target_link_libraries(EqCopSonde012SourcesLatencyTest PRIVATE
    juce::juce_core flatbuffers Advapi32
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopSonde012SourcesLatencyTest)

# ── Konsolen-Probe: Ende-zu-Ende-Handshake ohne FL ──────────────────────────
juce_add_console_app(EqCopPipeProbe PRODUCT_NAME "eqcop-pipe-probe")
target_sources(EqCopPipeProbe PRIVATE
    probe/PipeProbeMain.cpp
    src/PipeClient.cpp
)
target_include_directories(EqCopPipeProbe PRIVATE src "${NAKAMA_BROKER_BINDING_DIR}")
target_compile_definitions(EqCopPipeProbe PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopPipeProbe PRIVATE juce::juce_core
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopPipeProbe)

# Adversarialer Client-Lifecycle ohne Produktionspipe: blockierte Reads,
# Reconnect-Generation, SQOS, UTF-8/NUL, strikte ACK-Sequenz und absolute
# Frame-Deadline.
juce_add_console_app(EqCopPipeClientTest PRODUCT_NAME "eqcop-pipeclienttest")
target_sources(EqCopPipeClientTest PRIVATE
    tests/PipeClientLifecycleTestMain.cpp
    src/PipeClient.cpp
)
target_include_directories(EqCopPipeClientTest PRIVATE src)
target_compile_definitions(EqCopPipeClientTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopPipeClientTest PRIVATE juce::juce_core Advapi32
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopPipeClientTest)

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

# ── S10-11/SONDE-008: Ganzblock-Queue, Quarantaene, Blockgroessen-Stress ───
# Kanon B4. Zwei Ebenen in EINEM Ziel, wie bei B8:
#   1. Queue und Quarantaene pur (core/StampedAudioQueue.h) - Ganzblockaufnahme,
#      Drop, Wrap, Oversize, Kontinuitaet.
#   2. Derselbe Weg VERDRAHTET im echten EqCopilotProcessor - inklusive
#      Blockgroessen-Stress ueber maximumExpectedSamplesPerBlock hinaus (FL
#      zerteilt Puffer bis auf 1 Sample, Capabilityreport S4) und dem Beweis,
#      dass der Audiothread dabei nicht alloziert.
# Ohne Ebene 2 waere es ein huebscher Ring, der im Produkt nichts verriegelt.
juce_add_console_app(EqCopQueueStressTest PRODUCT_NAME "eqcop-queuestresstest")
target_sources(EqCopQueueStressTest PRIVATE
    tests/QueueStressTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopQueueStressTest PRIVATE src)
target_compile_definitions(EqCopQueueStressTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopQueueStressTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopQueueStressTest)

# ── S14-15/SONDE-010: v3-Envelope, Pipetoken, Backpressure und die Clients ─
# Kanon B10. Zwei Ebenen in EINEM Ziel, wie bei B4 und B8:
#   1. Envelope, Pipetoken und die drei client-seitigen Backpressure-Politiken
#      PUR - dort sind Grenzen, CRC-Fehler und Ueberlauf deterministisch
#      herbeifuehrbar, und dort liegt der Fixture-Korpus.
#   2. Die ECHTEN Clients aus NakamaKern gegen einen selbst gehosteten
#      v3-Server auf einer Testpipe (PID + Zaehler im Namen): Bootstrap,
#      Kopplung, Reconnect nach Serverneustart, Envelopeverstoss, P0-Ueberlauf.
# Ohne Ebene 2 waere es ein huebscher Parser, der im Produkt nichts verbindet.
# juce_core nur fuer das LESEN des Fixture-Manifests (juce::JSON, juce::File) -
# die gemessenen Quellen selbst sind JUCE-frei.
juce_add_console_app(EqCopIpcTest PRODUCT_NAME "eqcop-ipctest")
target_sources(EqCopIpcTest PRIVATE tests/IpcTestMain.cpp core/ipc/PipeToken.cpp)
target_compile_definitions(EqCopIpcTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopIpcTest PRIVATE juce::juce_core Advapi32
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopIpcTest)

# ── S14-15/SONDE-010: die C++-Haelfte des Ende-zu-Ende-Lastbeins ──────────
# Kein Kanon-Bein fuer sich: es wird von tools/eq-copilot/pruefe_ipc_last.py
# GEGEN den Rust-Probe-Broker gefahren (Kanon A22). Getrenntes Ziel, weil es
# einen externen Prozess braucht - EqCopIpcTest bleibt dadurch in sich
# geschlossen und schnell.
juce_add_console_app(EqCopIpcLast PRODUCT_NAME "eqcop-ipclast")
target_sources(EqCopIpcLast PRIVATE tests/IpcLastMain.cpp)
target_compile_definitions(EqCopIpcLast PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopIpcLast PRIVATE juce::juce_core Advapi32
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopIpcLast)

# ── S10-11/SONDE-008: EBU-Golden der fixed-memory Loudness ─────────────────
# Kanon B9. Misst den `LoudnessAccumulator` gegen die AUSGEBAUTE Rechnung:
# den unbegrenzten Vektor mit Zweitdurchgang, den dieses Ticket ersetzt. Das
# ist die einzige Referenz, die die Frage des Gate-Textes beantwortet ("RT-/
# EBU-Goldens gruen") - eine gegen sich selbst gerechnete Zahl waere eine
# Tautologie. Kein juce_dsp noetig: die Kopfdatei ist bibliotheksfrei.
juce_add_console_app(EqCopLoudnessGoldenTest PRODUCT_NAME "eqcop-loudnessgoldentest")
target_sources(EqCopLoudnessGoldenTest PRIVATE tests/LoudnessGoldenTestMain.cpp)
target_compile_definitions(EqCopLoudnessGoldenTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopLoudnessGoldenTest PRIVATE juce::juce_core
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)

# ── S12-13\SONDE-009: FeatureEngine v2 - Zeit, Validity, Events, Baender ───
# Kanon B5 (stand seit S0 als "geplant (ab P2)" im Runner). Entwurf §66.1:
# "EBU, Bandgitter, Stereo, Ereignisse, Epochgrenzen".
#
# Zwei Ebenen in EINEM Ziel, wie bei B4 und B8:
#   1. Die FeatureEngine pur (core/analysis/) - Bandgitter gegen die
#      eingefrorenen Fixtures, Quantisierung gegen alle 61 Vertragsvektoren,
#      und die Fenstertrennung je Grenzart als eigener Fall.
#   2. DERSELBE Weg verdrahtet im echten EqCopilotProcessor - Abschnitt K
#      faehrt `nakamaBlockEmpfangen()`, die Brueckenhaelfte des Zeitstempels,
#      die laut T2-Bericht zu SONDE-008 "kein Bein" hatte (NAK-56).
# Ohne Ebene 2 waere es eine huebsche Engine, die im Produkt nichts misst.
juce_add_console_app(EqCopAnalysisGoldenTest PRODUCT_NAME "eqcop-analysisgoldentest")
target_sources(EqCopAnalysisGoldenTest PRIVATE
    tests/AnalysisGoldenTestMain.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PipeClient.cpp
    src/AnalyseEngine.cpp
    src/Diagnose.cpp
)
target_include_directories(EqCopAnalysisGoldenTest PRIVATE src)
target_compile_definitions(EqCopAnalysisGoldenTest PRIVATE
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(EqCopAnalysisGoldenTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
nakama_kern_anbinden(EqCopAnalysisGoldenTest)

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
# Beide handgeschriebenen Vertragsleser kommen aus NakamaKern. Seit SONDE-012
# konsumiert auch Gen/Main die typisierte FlatBuffers-Kopie; eine zweite
# Uebersetzung nur fuer den Schematest waere wieder zwei Leserstaende.
target_sources(EqCopSchemaTest PRIVATE
    tests/SchemaTestMain.cpp
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
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopAnalysisGoldenTest
             EqCopSonde012ProjectReloadTest)
    target_compile_definitions(${ziel} PRIVATE NAKAMA_BUNDLE_MAIN=1)
endforeach()

# Das Quellenmodell ist Produktcode der Main-Schale. Jedes Ziel, das den
# echten Main-Prozessor uebersetzt, bekommt exakt dieselbe Uebersetzungseinheit.
foreach(ziel EqCopilot
             EqCopNullTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopAnalysisGoldenTest
             EqCopSonde012ProjectReloadTest)
    target_sources(${ziel} PRIVATE src/SourcesModel.cpp)
endforeach()

# Nur Ziele, die den Main-Prozessor wirklich uebersetzen, bekommen die duenne
# Produkt-Pipeidentitaet und den aus dem Installer-Manifest generierten
# Startvertrag. `PipeToken.cpp` bleibt bewusst ausserhalb von NakamaKern: sein
# Hersteller-Namensraum ist Zielidentitaet, keine geteilte Kernwahrheit.
foreach(ziel EqCopilot
             EqCopNullTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopAnalysisGoldenTest
             EqCopSonde012ProjectReloadTest)
    target_sources(${ziel} PRIVATE core/ipc/PipeToken.cpp)
    target_include_directories(${ziel} PRIVATE "${NAKAMA_BROKER_BINDING_DIR}")
    target_link_libraries(${ziel} PRIVATE Advapi32)
endforeach()

# Die Konsolentests instanziieren den echten Prozessor, duerfen dabei aber nie
# die Produktions-v3-Pipe beruehren oder einen installierten Broker starten.
# B8/B10 fahren den Lifecycle selbst mit Testhaken beziehungsweise PID-basierten
# Probe-Pipenamen; nur das eigentliche Bundle aktiviert die Produktthreads.
foreach(ziel EqCopNullTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopAnalysisGoldenTest
             EqCopSonde012ProjectReloadTest)
    target_compile_definitions(${ziel} PRIVATE
        NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
endforeach()

# ── S10-11/SONDE-008: Realtime-Core (plugin/core/) auf dem Suchpfad ─────────
# `core/StampedAudioQueue.h` und `core/analysis/LoudnessAccumulator.h` sind
# JUCE-freie Kopfdateien (dieselbe Form wie hostbridge/NakamaHostBridge.h):
# dadurch liegt die riskante Realtime-Logik in UNSEREM Repo und ist headless
# pruefbar. Jedes Ziel, das src/ uebersetzt, braucht beide Pfade - PluginProcessor
# ist seit diesem Ticket eine `Senke` der Hostbruecke (der Zeitstempel kommt von
# dort), und AnalyseEngine haelt den fixed-memory LoudnessAccumulator.
foreach(ziel EqCopilot
             EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopLoudnessGoldenTest EqCopAnalysisGoldenTest
             EqCopSonde012ProjectReloadTest EqCopSonde012SourcesModelTest)
    target_include_directories(${ziel} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/core"
        "${CMAKE_CURRENT_SOURCE_DIR}/hostbridge")
endforeach()

# Die Quellen sind UTF-8 (deutsche UI-Texte). Ohne /utf-8 rät MSVC die
# Codepage — Literale wie „Klavier-Bus“ hingen dann an der Systemlocale.
foreach(ziel NakamaKern
             EqCopilot EqCopPipeProbe EqCopPipeClientTest EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIpcTest EqCopIpcLast
             EqCopIdentityTest EqCopAuxSpike EqCopAuxSpikeTest EqCopHostContextTest
             EqCopSchemaTest EqCopStateMigrationTest EqCopLebenslaufTest
             EqCopQueueStressTest EqCopLoudnessGoldenTest EqCopAnalysisGoldenTest
             EqCopHostProbe EqCopHostProbeTest
             NakamaProbeeq EqCopProbeeqNullTest
             EqCopSonde012HostChannelContextTest EqCopSonde012LoudnessSourceTest
             EqCopSonde012SourcesModelTest EqCopSonde012ProjectReloadTest
             EqCopSonde012SourcesLatencyTest EqCopSessionSoak)
    target_compile_options(${ziel} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endforeach()

# ── K2b/K2c am Dateiende: jetzt stehen alle Verbraucher ────────────────────
# Erst hier ist die Frage stellbar. SONDE-007a liess offen, ob der Kern gegen
# EINE Referenz oder gegen alle geprueft wird (B8, "Fuer S9 offen"); mit drei
# Bundles ist die Antwort: gegen jeden einzelnen. Waeren zwei Verbraucher
# untereinander uneins, faende eine Stichprobe das nicht. K2b berechnet die
# compilerwirksamen Definehuellen von Kern und Verbraucher getrennt; K2c
# berechnet ihre Linkhuellen getrennt. Beide laufen rekursiv und je
# Baukonfiguration, ohne einen Verbraucher in die Kernhuelle hineinzuziehen.
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
