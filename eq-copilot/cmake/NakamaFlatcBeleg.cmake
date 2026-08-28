cmake_minimum_required(VERSION 3.22)

foreach(_pflicht IN ITEMS
        NAKAMA_FLATC_BINARY NAKAMA_FLATC_COMMIT NAKAMA_FLATC_BELEG)
    if(NOT DEFINED ${_pflicht} OR "${${_pflicht}}" STREQUAL "")
        message(FATAL_ERROR "Nakama-FlatBuffers: POST_BUILD ohne ${_pflicht}")
    endif()
endforeach()

if(NOT EXISTS "${NAKAMA_FLATC_BINARY}")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: das soeben gebaute flatc fehlt: ${NAKAMA_FLATC_BINARY}")
endif()

string(TOLOWER "${NAKAMA_FLATC_COMMIT}" _commit)
string(LENGTH "${_commit}" _commit_laenge)
if(NOT _commit_laenge EQUAL 40 OR _commit MATCHES "[^0-9a-f]")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: ungueltiger gemessener Commit im POST_BUILD: '${_commit}'")
endif()

# Bewusst cmake -E sha256sum statt eines plattformspezifischen Hashwerkzeugs.
# Die Ausgabe enthaelt zusaetzlich den Dateinamen; in den Sidecar kommt nur der
# Digest, damit der externe Pruefer ein kleines, plattformneutrales Format hat.
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E sha256sum "${NAKAMA_FLATC_BINARY}"
    RESULT_VARIABLE _hash_exit
    OUTPUT_VARIABLE _hash_ausgabe
    ERROR_VARIABLE _hash_fehler
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX MATCH "^([0-9a-fA-F]+)[ \t]+" _hash_treffer "${_hash_ausgabe}")
set(_hash "${CMAKE_MATCH_1}")
string(LENGTH "${_hash}" _hash_laenge)
if(NOT _hash_exit EQUAL 0
   OR NOT _hash_laenge EQUAL 64
   OR _hash MATCHES "[^0-9a-fA-F]")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: SHA-256 fuer ${NAKAMA_FLATC_BINARY} fehlgeschlagen.\n"
        "  cmake -E sha256sum: Exit ${_hash_exit}, Ausgabe '${_hash_ausgabe}'\n"
        "  Fehler: ${_hash_fehler}")
endif()
string(TOLOWER "${_hash}" _hash)

# Erst vollstaendig in eine Nachbardatei schreiben, dann ersetzen. Ein
# abgebrochener POST_BUILD hinterlaesst dadurch hoechstens den alten Beleg;
# dessen Commit oder Hash laesst der Drift-Pruefer rot werden.
set(_temporaer "${NAKAMA_FLATC_BELEG}.tmp")
file(WRITE "${_temporaer}" "commit ${_commit}\nsha256 ${_hash}\n")
file(RENAME "${_temporaer}" "${NAKAMA_FLATC_BELEG}")
message(STATUS
    "Nakama-FlatBuffers: Binary-Beleg geschrieben (${_commit}, ${_hash})")
