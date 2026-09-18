cmake_minimum_required(VERSION 3.22)
project(NakamaAudit16 VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(AUDIT_REPO "C:/na-audit16-aff2d818" CACHE PATH "Unveraenderte isolierte Produktquelle")
add_subdirectory("${AUDIT_REPO}/eq-copilot" "${CMAKE_BINARY_DIR}/produkt" EXCLUDE_FROM_ALL)
set(P "${AUDIT_REPO}/eq-copilot/plugin")
set(AUDIT_PROCESSOR_SOURCES
    "${P}/sonde/SondeProcessor.cpp"
    "${P}/src/PluginProcessor.cpp"
    "${P}/src/prozessor/Hostbruecke.cpp"
    "${P}/src/prozessor/State.cpp"
    "${P}/src/prozessor/Ipc.cpp"
    "${P}/src/prozessor/Analyse.cpp"
    "${P}/src/PluginEditor.cpp"
    "${P}/src/AnalyseEngine.cpp"
    "${P}/src/DiagnoseAntwort.cpp"
    "${P}/src/Diagnose.cpp"
    "${P}/src/SourcesModel.cpp"
    "${P}/core/ipc/PipeToken.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/PipeClientIsoliert.cpp")
function(audit_processor_target target main)
    juce_add_console_app(${target} PRODUCT_NAME "${target}")
    target_sources(${target} PRIVATE "${main}" ${AUDIT_PROCESSOR_SOURCES})
    target_include_directories(${target} PRIVATE "${P}/src" "${P}/sonde"
        "${P}/state" "${P}/core" "${P}/core/ipc" "${P}/hostbridge"
        "${P}/vertrag" "${P}/dsp" "${P}/tests"
        "${CMAKE_BINARY_DIR}/produkt/plugin/generated/nakama")
    target_compile_definitions(${target} PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        JUCE_DISPLAY_SPLASH_SCREEN=0 NAKAMA_BUNDLE_MAIN=1 NAKAMA_SONDE_AKTIV=1
        NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3=1)
    target_compile_options(${target} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    target_link_libraries(${target} PRIVATE NakamaKern juce::juce_audio_utils
        juce::juce_dsp juce::juce_cryptography flatbuffers Advapi32
        juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endfunction()
audit_processor_target(Audit16State "${CMAKE_CURRENT_LIST_DIR}/state_probe.cpp")
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../e/last_probe.cpp")
    audit_processor_target(Audit16Last "${CMAKE_CURRENT_LIST_DIR}/../e/last_probe.cpp")
endif()
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../e/resources_probe.cpp")
    audit_processor_target(Audit16LastResources "${CMAKE_CURRENT_LIST_DIR}/../e/resources_probe.cpp")
endif()
