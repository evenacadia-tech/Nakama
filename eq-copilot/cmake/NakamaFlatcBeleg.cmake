# Configure-Zeit reicht nicht: Danach kann ein anderer Commit ausgecheckt und
# flatc ohne erneutes Configure neu gebaut werden. Das alte Commit-Literal
# stuende dann neben dem Hash des neuen Binaries und koennte falsches Drift 0 belegen.

cmake_minimum_required(VERSION 3.22)

foreach(_pflicht IN ITEMS
        NAKAMA_FLATC_BINARY NAKAMA_FLATC_QUELLE NAKAMA_FLATC_PIN
        NAKAMA_FLATC_BELEG)
    if(NOT DEFINED ${_pflicht} OR "${${_pflicht}}" STREQUAL "")
        message(FATAL_ERROR "Nakama-FlatBuffers: POST_BUILD ohne ${_pflicht}")
    endif()
endforeach()

# Ein fehlgeschlagener Neubau darf keinen alten Sidecar als scheinbar aktuellen
# Beleg hinterlassen. Erst nach allen Messungen entsteht die Nachbardatei neu.
set(_temporaer "${NAKAMA_FLATC_BELEG}.tmp")
file(REMOVE "${NAKAMA_FLATC_BELEG}" "${_temporaer}")

if(NOT EXISTS "${NAKAMA_FLATC_BINARY}")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: das soeben gebaute flatc fehlt: ${NAKAMA_FLATC_BINARY}")
endif()

if(NOT IS_DIRECTORY "${NAKAMA_FLATC_QUELLE}")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: flatc-Quellverzeichnis fehlt: ${NAKAMA_FLATC_QUELLE}")
endif()

string(TOLOWER "${NAKAMA_FLATC_PIN}" _pin)
string(LENGTH "${_pin}" _pin_laenge)
if(NOT _pin_laenge EQUAL 40 OR _pin MATCHES "[^0-9a-f]")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: ungueltiger flatc-Pin im POST_BUILD: '${_pin}'")
endif()

find_package(Git REQUIRED)
execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${NAKAMA_FLATC_QUELLE}"
    RESULT_VARIABLE _commit_exit
    OUTPUT_VARIABLE _commit_ausgabe
    ERROR_VARIABLE _commit_fehler
    OUTPUT_STRIP_TRAILING_WHITESPACE)
string(TOLOWER "${_commit_ausgabe}" _commit)
string(LENGTH "${_commit}" _commit_laenge)
if(NOT _commit_exit EQUAL 0
   OR NOT _commit_laenge EQUAL 40
   OR _commit MATCHES "[^0-9a-f]")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: flatc-Commit zur Bauzeit nicht belegbar.\n"
        "  Quelle: ${NAKAMA_FLATC_QUELLE}\n"
        "  git rev-parse HEAD: Exit ${_commit_exit}, Ausgabe '${_commit_ausgabe}'\n"
        "  Fehler: ${_commit_fehler}")
endif()
if(NOT _commit STREQUAL _pin)
    message(FATAL_ERROR
        "Nakama-FlatBuffers: flatc-Commit zur Bauzeit weicht vom Pin ab.\n"
        "  Pin:      ${_pin}\n"
        "  Checkout: ${_commit}")
endif()

execute_process(
    COMMAND "${GIT_EXECUTABLE}" status --porcelain
    WORKING_DIRECTORY "${NAKAMA_FLATC_QUELLE}"
    RESULT_VARIABLE _status_exit
    OUTPUT_VARIABLE _status_ausgabe
    ERROR_VARIABLE _status_fehler
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _status_exit EQUAL 0)
    message(FATAL_ERROR
        "Nakama-FlatBuffers: flatc-Checkout zur Bauzeit nicht pruefbar.\n"
        "  Quelle: ${NAKAMA_FLATC_QUELLE}\n"
        "  git status --porcelain: Exit ${_status_exit}\n"
        "  Fehler: ${_status_fehler}")
endif()
if(NOT _status_ausgabe STREQUAL "")
    message(FATAL_ERROR
        "Nakama-FlatBuffers: flatc-Checkout ist zur Bauzeit schmutzig.\n"
        "  Quelle: ${NAKAMA_FLATC_QUELLE}\n"
        "  git status --porcelain:\n${_status_ausgabe}\n"
        "Ein veraenderter Quellstand ist kein gepinnter Quellstand.")
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

# Erst vollstaendig in eine Nachbardatei schreiben, dann ersetzen.
file(WRITE "${_temporaer}" "commit ${_commit}\nsha256 ${_hash}\n")
file(RENAME "${_temporaer}" "${NAKAMA_FLATC_BELEG}")
message(STATUS
    "Nakama-FlatBuffers: Binary-Beleg geschrieben (${_commit}, ${_hash})")
