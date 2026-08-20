# Transparentes Sammler-VST3 + Konsolen-Probe + Nulltest + Golden-Test.
#
# Vier Targets, ein Quellstand:
#  - EqCopilot (VST3)     — das Plugin selbst (M1: mit AnalyseEngine)
#  - EqCopPipeProbe       — Konsole: spielt einen Sensor gegen den laufenden Broker
#  - EqCopNullTest        — Konsole: Bitgleichheits-/NaN-/Latenz-Beweis ohne Host
#  - EqCopGoldenTest      — Konsole: M1-Kreuzvalidierung gegen fixtures/golden-referenz.json
# Probe und Tests kompilieren die geteilten Quellen erneut, statt eine
# static-lib mit fremden JUCE-Moduldefinitionen zu teilen — bewusst simpel.

juce_add_plugin(EqCopilot
    COMPANY_NAME "evenacadia"
    PRODUCT_NAME "EQ-Copilot"
    PLUGIN_MANUFACTURER_CODE Evna
    PLUGIN_CODE Eqcp
    FORMATS VST3
    VST3_CATEGORIES Fx Analyzer
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

target_link_libraries(EqCopilot
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

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

# Die Quellen sind UTF-8 (deutsche UI-Texte). Ohne /utf-8 rät MSVC die
# Codepage — Literale wie „Klavier-Bus“ hingen dann an der Systemlocale.
foreach(ziel EqCopilot EqCopPipeProbe EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench
             EqCopIdentityTest EqCopAuxSpike EqCopAuxSpikeTest EqCopHostContextTest)
    target_compile_options(${ziel} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endforeach()
