# EQ-Copilot — Bauwurzel (Plan §10). JUCE 8, Version gepinnt; Zielformat VST3.
cmake_minimum_required(VERSION 3.22)
project(EqCopilotSuite VERSION 0.3.0 LANGUAGES C CXX)

# ── Versions-Riegel (Kontext-Inventur 21.08.2026) ───────────────────────────
# Das Bundle meldet dem Host PROJECT_VERSION (moduleinfo.json), der Heartbeat
# meldet kPluginVersion aus plugin/src/EqCopilotIds.h. Vom 16. bis 21.08. sagten
# beide etwas anderes (0.1.0 vs. 0.3.0) — FL zeigte 0.1.0, alle Docs 0.3.0.
# Ab hier bricht der Configure, sobald die beiden Zahlen auseinanderlaufen.
file(READ "${CMAKE_CURRENT_LIST_DIR}/plugin/src/EqCopilotIds.h" _eqcop_ids_quelle)
string(REGEX MATCH "kPluginVersion[ \t]*=[ \t]*\"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _eqcop_ids_treffer "${_eqcop_ids_quelle}")
if(NOT _eqcop_ids_treffer)
    message(FATAL_ERROR "Versions-Riegel: kPluginVersion in plugin/src/EqCopilotIds.h nicht gefunden.")
endif()
if(NOT CMAKE_MATCH_1 STREQUAL PROJECT_VERSION)
    message(FATAL_ERROR
        "Versions-Riegel: kPluginVersion = ${CMAKE_MATCH_1} (EqCopilotIds.h), "
        "PROJECT_VERSION = ${PROJECT_VERSION} (CMakeLists.txt). Beide muessen eins sagen — "
        "sonst meldet das Bundle dem Host eine andere Version als der Heartbeat dem Broker.")
endif()
unset(_eqcop_ids_quelle)
unset(_eqcop_ids_treffer)

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

# ── SONDE-005b: gepinntes FlatBuffers-Werkzeug ──────────────────────────────
# Holt Header UND Compiler aus demselben Commit und bricht ab, wenn der geholte
# Stand eine andere Version traegt. Details im Modul.
include(${CMAKE_CURRENT_LIST_DIR}/cmake/NakamaFlatBuffers.cmake)
nakama_flatbuffers_bereitstellen(
    "${CMAKE_CURRENT_LIST_DIR}/schemas/v3/flatbuffers/WERKZEUG.json")

add_subdirectory(plugin)
