cmake_minimum_required(VERSION 3.22)
project(NakamaAudit16D LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT DEFINED NAKAMA_SOURCE OR NOT DEFINED JUCE_SOURCE)
  message(FATAL_ERROR "Pass NAKAMA_SOURCE and JUCE_SOURCE explicitly; no product files are edited")
endif()
add_subdirectory("${JUCE_SOURCE}" "${CMAKE_BINARY_DIR}/juce" EXCLUDE_FROM_ALL)
add_executable(audit16d dsp_probe.cpp
  "${NAKAMA_SOURCE}/eq-copilot/plugin/dsp/DspProgramm.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/dsp/DspBankPool.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/dsp/DspKern.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/state/NakamaParameter.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/state/NakamaKanon.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/state/NakamaTransaktion.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/state/NakamaPreset.cpp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/vertrag/NakamaVertrag.cpp")
target_include_directories(audit16d PRIVATE
  "${NAKAMA_SOURCE}/eq-copilot/plugin/dsp"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/state"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/vertrag"
  "${NAKAMA_SOURCE}/eq-copilot/plugin/core/ipc")
target_compile_definitions(audit16d PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_DISPLAY_SPLASH_SCREEN=0)
target_link_libraries(audit16d PRIVATE juce::juce_core juce::juce_data_structures juce::juce_events juce::juce_cryptography)
if(MSVC)
  target_compile_options(audit16d PRIVATE /utf-8 /fp:precise)
endif()
