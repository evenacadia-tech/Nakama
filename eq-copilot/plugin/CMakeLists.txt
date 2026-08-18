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

target_compile_definitions(EqCopilot PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

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

# Die Quellen sind UTF-8 (deutsche UI-Texte). Ohne /utf-8 rät MSVC die
# Codepage — Literale wie „Klavier-Bus“ hingen dann an der Systemlocale.
foreach(ziel EqCopilot EqCopPipeProbe EqCopNullTest EqCopGoldenTest EqCopMarkierungTest EqCopShot EqCopPaintBench)
    target_compile_options(${ziel} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endforeach()
