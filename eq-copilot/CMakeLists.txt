# EQ-Copilot — Bauwurzel (Plan §10). JUCE 8, Version gepinnt; Zielformat VST3.
cmake_minimum_required(VERSION 3.22)
project(EqCopilotSuite VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
# Gepinnt auf den neuesten 8er-Tag (git ls-remote, 2026-08-13). Upgrade nur bewusst.
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG        8.0.9
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ── SONDE-003: Nakama-Hostbruecke in den gevendorten Wrapper patchen ─────────
# Idempotent und quellhash-gegatet. Laeuft VOR add_subdirectory(plugin), damit
# kein Ziel je eine ungepatchte Fassung kompiliert. Bricht bei fremdem
# Quellstand ab, statt blind zu patchen — Details im Modul.
set(NAKAMA_WURZEL "${CMAKE_CURRENT_LIST_DIR}/..")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/NakamaBruecke.cmake)
nakama_bruecke_anwenden(
    "${juce_SOURCE_DIR}"
    "${NAKAMA_WURZEL}/third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch")

add_subdirectory(plugin)
