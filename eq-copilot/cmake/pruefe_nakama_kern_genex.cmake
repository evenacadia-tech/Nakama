cmake_minimum_required(VERSION 3.22)

# Bauloser Regressionstest fuer den Configure-Zeit-Auswerter in
# NakamaKern.cmake. Ziel-Eigenschaften werden in cmake -P ueber dieselbe
# Lesefunktion eingespeist, die im echten Configure get_target_property nutzt.
if(NOT DEFINED NAKAMA_TEST_CONFIG)
    set(NAKAMA_TEST_CONFIG Debug)
endif()
if(NOT DEFINED NAKAMA_TEST_IS_SYNTH)
    set(NAKAMA_TEST_IS_SYNTH FALSE)
endif()
set(CMAKE_SYSTEM_NAME Windows)
set(NAKAMA_TEST_JUCE_DEBUG_CONFIG "$<OR:$<CONFIG:Debug>>")

set(NAKAMA_KERN_TEST_PROPERTY_EqCopilot_JUCE_IS_SYNTH "${NAKAMA_TEST_IS_SYNTH}")
set(NAKAMA_KERN_TEST_PROPERTY_EqCopilot_JUCE_COMPANY_NAME Nakama)
set(NAKAMA_KERN_TEST_PROPERTY_EqCopilot_COMPILE_DEFINITIONS
    "JUCE_USE_CURL=0;JucePlugin_Build_VST3=1;JUCE_SHARED_CODE=1;JucePlugin_Name=Nakama")
set(NAKAMA_KERN_TEST_PROPERTY_juce_core_INTERFACE_COMPILE_DEFINITIONS
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
    $<IF:${NAKAMA_TEST_JUCE_DEBUG_CONFIG},DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
    $<$<PLATFORM_ID:Android>:JUCE_ANDROID=1>
    JUCE_MODULE_AVAILABLE_juce_core=1)
set(NAKAMA_KERN_TEST_PROPERTY_juce_core_INTERFACE_LINK_LIBRARIES
    "$<$<CONFIG:Debug>:EqCopilot>")
set(NAKAMA_KERN_TEST_PROPERTY_juce_vst3_headers_INTERFACE_COMPILE_DEFINITIONS
    "$<$<TARGET_EXISTS:juce_vst3_sdk>:JUCE_CUSTOM_VST3_SDK=1>")
set(NAKAMA_KERN_TEST_PROPERTY_juce_module_exact_INTERFACE_COMPILE_DEFINITIONS
    "JUCE_MODULE_AVAILABLE_juce_module_exact=1")
set(NAKAMA_KERN_TEST_PROPERTY_BrokenInner_INTERFACE_COMPILE_DEFINITIONS
    "$<$<NAKAMA_UNBEKANNTER_INNERER_OPERATOR:1>:JUCE_NIE_GRUEN=1>")
# NAK-84: Der Prueferfall. JOIN erzeugt JUCE_USE_CURL=1, ohne dass der Rohtext
# je "JUCE_" zusammenhaengend zeigt oder ein Ziel nennt.
set(NAKAMA_KERN_TEST_PROPERTY_JoinDefine_INTERFACE_COMPILE_DEFINITIONS
    "$<JOIN:JUCE$<SEMICOLON>_USE_CURL=1,>")
set(NAKAMA_KERN_TEST_PROPERTY_CycleA_INTERFACE_COMPILE_DEFINITIONS
    "$<TARGET_PROPERTY:CycleB,INTERFACE_COMPILE_DEFINITIONS>")
set(NAKAMA_KERN_TEST_PROPERTY_CycleB_INTERFACE_COMPILE_DEFINITIONS
    "$<TARGET_PROPERTY:CycleA,INTERFACE_COMPILE_DEFINITIONS>")
set(NAKAMA_KERN_TEST_PROPERTY_DiamondLeaf_INTERFACE_COMPILE_DEFINITIONS
    "JUCE_DIAMANT=1;$<$<CONFIG:Debug>:JUCE_DIAMANT_DEBUG=1>")
set(NAKAMA_KERN_TEST_PROPERTY_DiamondLeft_INTERFACE_COMPILE_DEFINITIONS
    "$<TARGET_PROPERTY:DiamondLeaf,INTERFACE_COMPILE_DEFINITIONS>;JUCE_DIAMANT_LINKS=1")
set(NAKAMA_KERN_TEST_PROPERTY_DiamondRight_INTERFACE_COMPILE_DEFINITIONS
    "$<TARGET_PROPERTY:DiamondLeaf,INTERFACE_COMPILE_DEFINITIONS>;JUCE_DIAMANT_RECHTS=1")
set(NAKAMA_KERN_TEST_PROPERTY_DiamondRoot_INTERFACE_COMPILE_DEFINITIONS
    "$<TARGET_PROPERTY:DiamondLeft,INTERFACE_COMPILE_DEFINITIONS>;$<TARGET_PROPERTY:DiamondRight,INTERFACE_COMPILE_DEFINITIONS>")
set(NAKAMA_KERN_TEST_PROPERTY_ListLiteral_INTERFACE_COMPILE_DEFINITIONS
    "JUCE_LISTE_A=1;JUCE_LISTE_LITERAL=a\\;b;JUCE_LISTE_B=1")
set(NAKAMA_KERN_TEST_ALIAS_JuceCoreAlias juce_core)
set(NAKAMA_KERN_TEST_TARGETS
    EqCopilot juce_core juce_vst3_headers juce_module_exact BrokenInner
    CycleA CycleB DiamondLeaf DiamondLeft DiamondRight DiamondRoot ListLiteral
    JoinDefine)
set(NAKAMA_KERN_TEST_COMPILE_LANGUAGE CXX)
set(NAKAMA_KERN_TEST_CXX_COMPILER_ID MSVC)

include("${CMAKE_CURRENT_LIST_DIR}/NakamaKern.cmake")

if(NAKAMA_TEST_ZYKLUS_ROT)
    _nakama_kern_wert_auswerten(
        "$<TARGET_PROPERTY:CycleA,INTERFACE_COMPILE_DEFINITIONS>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest echter Property-Zyklus" _darf_nie_gruen)
    message(FATAL_ERROR "Echter Property-Zyklus blieb unerwartet gruen: ${_darf_nie_gruen}")
endif()

if(NAKAMA_TEST_UNBEKANNT_ROT)
    _nakama_kern_wert_auswerten(
        "JUCE_RIEGEL_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest kontrollierter Bruch" _darf_nie_gruen)
    message(FATAL_ERROR "Kontrollierter Bruch blieb unerwartet gruen: ${_darf_nie_gruen}")
endif()

if(NAKAMA_TEST_INNERER_OPERATOR_ROT)
    _nakama_kern_wert_auswerten(
        "$<TARGET_PROPERTY:BrokenInner,INTERFACE_COMPILE_DEFINITIONS>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest innerster Operator" _darf_nie_gruen)
    message(FATAL_ERROR "Unbekannter innerer Operator blieb unerwartet gruen: ${_darf_nie_gruen}")
endif()

# NAK-84 (T3-Runde 3): Zusammengesetzter Definename direkt in einer
# Define-Eigenschaft. Ohne den Fix ist dieser Aufruf GRUEN und liefert "".
if(NAKAMA_TEST_JOIN_DEFINE_ROT)
    _nakama_kern_wert_auswerten(
        "$<JOIN:JUCE$<SEMICOLON>_USE_CURL=1,>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest zusammengesetzter Definename"
        _darf_nie_gruen)
    message(FATAL_ERROR
        "Zusammengesetzter Definename blieb unerwartet gruen: '${_darf_nie_gruen}'")
endif()

# Derselbe Fall ueber die Huelle, genau in der Form des Pruefers:
# target_compile_definitions(NakamaKern PRIVATE "$<JOIN:...>").
if(NAKAMA_TEST_JOIN_PROPERTY_ROT)
    _nakama_kern_wert_auswerten(
        "$<TARGET_PROPERTY:JoinDefine,INTERFACE_COMPILE_DEFINITIONS>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest JOIN in Huellen-Property"
        _darf_nie_gruen)
    message(FATAL_ERROR
        "JOIN in einer Huellen-Define-Property blieb unerwartet gruen: '${_darf_nie_gruen}'")
endif()

# NAK-84: In einer Define-Eigenschaft darf der Rohtext gar nicht mehr
# mitreden. Frueher war genau dieser Ausdruck "irrelevant" und still leer.
if(NAKAMA_TEST_IRRELEVANT_DEFINE_ROT)
    _nakama_kern_wert_auswerten(
        "NAKAMA_TEST=$<NAKAMA_UNBEKANNTER_OPERATOR:ohne_ziel>"
        "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest unbekannter Operator ohne Rohtextspur"
        _darf_nie_gruen)
    message(FATAL_ERROR
        "Unbekannter Operator in Define-Eigenschaft blieb unerwartet gruen: '${_darf_nie_gruen}'")
endif()

if(NAKAMA_TEST_STRINGOPERATOR_ROT)
    _nakama_kern_wert_auswerten(
        "$<LOWER_CASE:NakamaIdentitaetsIface>"
        "${NAKAMA_TEST_CONFIG}" LINK "Selbsttest Stringtransformation in Linkkante"
        _darf_nie_gruen)
    message(FATAL_ERROR "Stringtransformation in Linkkante blieb unerwartet gruen: ${_darf_nie_gruen}")
endif()

set(_nakama_test_anzahl 0)

# Faehrt dieses Skript als Unterprozess mit dem genannten ROT-Schalter und
# verlangt: Exitcode ungleich 0 UND jedes uebergebene Diagnosefragment in der
# Ausgabe. Ein roter Riegel, dessen Meldung den Ausdruck nicht nennt, hilft
# niemandem beim Aufraeumen - deshalb sind die Fragmente Teil der Zusage.
function(_nakama_genex_rotprobe schalter was pass_text)
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            "-DNAKAMA_TEST_CONFIG=${NAKAMA_TEST_CONFIG}"
            "-DNAKAMA_TEST_IS_SYNTH=${NAKAMA_TEST_IS_SYNTH}"
            "-D${schalter}=ON"
            -P "${CMAKE_CURRENT_LIST_FILE}"
        RESULT_VARIABLE _exit
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
    set(_ausgabe "${_stdout}\n${_stderr}")
    string(REGEX REPLACE "[ \r\n\t]+" " " _einzeilig "${_ausgabe}")
    if(_exit EQUAL 0)
        message(FATAL_ERROR "${was} blieb unerwartet gruen. Ausgabe=${_einzeilig}")
    endif()
    foreach(_fragment IN LISTS ARGN)
        string(FIND "${_einzeilig}" "${_fragment}" _position)
        if(_position EQUAL -1)
            message(FATAL_ERROR
                "${was}: Diagnose nennt '${_fragment}' nicht. Ausgabe=${_einzeilig}")
        endif()
    endforeach()
    message(STATUS "PASS ${pass_text}")
endfunction()

function(_nakama_genex_erwarte name ausdruck erwartung)
    _nakama_kern_text_auswerten(
        "${ausdruck}" "${NAKAMA_TEST_CONFIG}" IDENTITAET "Selbsttest ${name}"
        _ist _bekannt)
    if(NOT _bekannt)
        message(FATAL_ERROR "${name}: Ausdruck wurde unerwartet als unbekannt abgewiesen: ${ausdruck}")
    endif()
    if(NOT "${_ist}" STREQUAL "${erwartung}")
        message(FATAL_ERROR
            "${name}: Erwartung != Ergebnis\n"
            "  Ausdruck: ${ausdruck}\n"
            "  Erwartet: ${erwartung}\n"
            "  Ist:      ${_ist}")
    endif()
    math(EXPR _anzahl "${_nakama_test_anzahl} + 1")
    set(_nakama_test_anzahl ${_anzahl} PARENT_SCOPE)
    message(STATUS "PASS ${name}: ${_ist}")
endfunction()

_nakama_kern_wahrheitswert("${NAKAMA_TEST_IS_SYNTH}" _synth_wahr)
if(_synth_wahr)
    set(_synth_erwartet 1)
else()
    set(_synth_erwartet 0)
endif()

if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_debug_define JUCE_DEBUG=1)
    set(_debug_kante JUCE_USE_CURL=0)
    set(_and_erwartet 1)
else()
    set(_debug_define JUCE_DEBUG=0)
    set(_debug_kante "")
    set(_and_erwartet 0)
endif()

_nakama_genex_erwarte(
    "01 TARGET_PROPERTY"
    "$<TARGET_PROPERTY:EqCopilot,JUCE_COMPANY_NAME>"
    "Nakama")
_nakama_genex_erwarte(
    "02 JucePlugin_IsSynth/BOOL"
    "JucePlugin_IsSynth=$<BOOL:$<TARGET_PROPERTY:EqCopilot,JUCE_IS_SYNTH>>"
    "JucePlugin_IsSynth=${_synth_erwartet}")
_nakama_genex_erwarte(
    "03 IF und CONFIG:X"
    "$<IF:$<CONFIG:Debug>,JUCE_DEBUG=1,JUCE_DEBUG=0>"
    "${_debug_define}")
_nakama_genex_erwarte(
    "04 STREQUAL"
    "$<STREQUAL:$<TARGET_PROPERTY:EqCopilot,JUCE_COMPANY_NAME>,Nakama>"
    "1")
_nakama_genex_erwarte("05 NOT" "$<NOT:$<BOOL:OFF>>" "1")
_nakama_genex_erwarte(
    "06 AND"
    "$<AND:$<BOOL:1>,$<STREQUAL:a,a>,$<CONFIG:Debug>>"
    "${_and_erwartet}")
_nakama_genex_erwarte(
    "07 OR"
    "$<OR:$<BOOL:0>,$<CONFIG:Release>,$<STREQUAL:x,x>>"
    "1")
_nakama_genex_erwarte("08 CONFIG-Wert" "$<CONFIG>" "${NAKAMA_TEST_CONFIG}")
_nakama_genex_erwarte(
    "09 bedingte Kurzform"
    "$<$<CONFIG:Debug>:JUCE_USE_CURL=0>"
    "${_debug_kante}")
_nakama_genex_erwarte(
    "10 LINK_ONLY verschachtelt"
    "$<LINK_ONLY:$<IF:$<BOOL:1>,NakamaKernJuce,Niemals>>"
    "NakamaKernJuce")
if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_juce_core_erwartet
        "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1;DEBUG=1;_DEBUG=1;JUCE_MODULE_AVAILABLE_juce_core=1")
else()
    set(_juce_core_erwartet
        "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1;NDEBUG=1;_NDEBUG=1;JUCE_MODULE_AVAILABLE_juce_core=1")
endif()
_nakama_genex_erwarte(
    "11 Listen-TARGET_PROPERTY ueber Alias"
    "$<TARGET_PROPERTY:JuceCoreAlias,INTERFACE_COMPILE_DEFINITIONS>"
    "${_juce_core_erwartet}")
_nakama_genex_erwarte(
    "12 TARGET_GENEX_EVAL mit Definitionsliste"
    "$<TARGET_GENEX_EVAL:EqCopilot,$<TARGET_PROPERTY:EqCopilot,COMPILE_DEFINITIONS>>"
    "JUCE_USE_CURL=0;JucePlugin_Build_VST3=1;JUCE_SHARED_CODE=1;JucePlugin_Name=Nakama")
_nakama_genex_erwarte(
    "13 FILTER wie JUCEUtils.cmake"
    "$<FILTER:$<TARGET_GENEX_EVAL:EqCopilot,$<TARGET_PROPERTY:EqCopilot,COMPILE_DEFINITIONS>>,EXCLUDE,JucePlugin_Build_|JUCE_SHARED_CODE>"
    "JUCE_USE_CURL=0;JucePlugin_Name=Nakama")
if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_genex_eval_erwartet JUCE_GENEX_EVAL=1)
else()
    set(_genex_eval_erwartet "")
endif()
_nakama_genex_erwarte(
    "14 GENEX_EVAL"
    "$<GENEX_EVAL:$<$<CONFIG:Debug>:JUCE_GENEX_EVAL=1>>"
    "${_genex_eval_erwartet}")
_nakama_genex_erwarte(
    "15 COMPILE_LANGUAGE"
    "$<$<COMPILE_LANGUAGE:CXX>:JUCE_CXX=1>"
    "JUCE_CXX=1")
_nakama_genex_erwarte(
    "16 CXX_COMPILER_ID"
    "$<$<CXX_COMPILER_ID:MSVC>:JUCE_MSVC=1>"
    "JUCE_MSVC=1")
_nakama_genex_erwarte(
    "17 EQUAL"
    "$<IF:$<EQUAL:8,8>,JUCE_PTR=8,JUCE_PTR=0>"
    "JUCE_PTR=8")
_nakama_genex_erwarte(
    "18 PLATFORM_ID"
    "$<PLATFORM_ID:NakamaUnmoeglichesSystem>"
    "0")
_nakama_genex_erwarte(
    "19 TARGET_EXISTS"
    "$<TARGET_EXISTS:EqCopilot>"
    "1")
if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_linkliste_erwartet EqCopilot)
else()
    set(_linkliste_erwartet "")
endif()
_nakama_genex_erwarte(
    "20 Listen-INTERFACE_LINK_LIBRARIES ueber Alias"
    "$<TARGET_PROPERTY:JuceCoreAlias,INTERFACE_LINK_LIBRARIES>"
    "${_linkliste_erwartet}")
if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_diamant_erwartet
        "JUCE_DIAMANT=1;JUCE_DIAMANT_DEBUG=1;JUCE_DIAMANT_LINKS=1;JUCE_DIAMANT=1;JUCE_DIAMANT_DEBUG=1;JUCE_DIAMANT_RECHTS=1")
else()
    set(_diamant_erwartet
        "JUCE_DIAMANT=1;JUCE_DIAMANT_LINKS=1;JUCE_DIAMANT=1;JUCE_DIAMANT_RECHTS=1")
endif()
_nakama_genex_erwarte(
    "21 Property-Diamant mit legitimem Wiederbesuch"
    "$<TARGET_PROPERTY:DiamondRoot,INTERFACE_COMPILE_DEFINITIONS>"
    "${_diamant_erwartet}")
set(_listenliteral_erwartet
    "JUCE_LISTE_A=1;JUCE_LISTE_LITERAL=a\\;b;JUCE_LISTE_B=1")
_nakama_genex_erwarte(
    "22 Listenwert mit literalem Semikolon"
    "$<TARGET_PROPERTY:ListLiteral,INTERFACE_COMPILE_DEFINITIONS>"
    "${_listenliteral_erwartet}")
if(NAKAMA_TEST_CONFIG STREQUAL "Debug")
    set(_juce_debugzweig_erwartet "DEBUG=1 _DEBUG=1")
    set(_juce_standard_mitte_erwartet DEBUG=1)
    set(_juce_standard_drittes_erwartet _DEBUG=1)
else()
    set(_juce_debugzweig_erwartet "NDEBUG=1 _NDEBUG=1")
    set(_juce_standard_mitte_erwartet NDEBUG=1)
    set(_juce_standard_drittes_erwartet _NDEBUG=1)
endif()
_nakama_genex_erwarte(
    "23 JUCE debug_config und IF-Leerzeichenzweig wortgleich"
    "$<IF:${NAKAMA_TEST_JUCE_DEBUG_CONFIG},DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>"
    "${_juce_debugzweig_erwartet}")
_nakama_genex_erwarte(
    "24 JUCE TARGET_EXISTS-Define wortgleich"
    "$<TARGET_PROPERTY:juce_vst3_headers,INTERFACE_COMPILE_DEFINITIONS>"
    "")
_nakama_genex_erwarte(
    "25 JUCE MODULE_AVAILABLE-Define wortgleich"
    "$<TARGET_PROPERTY:juce_module_exact,INTERFACE_COMPILE_DEFINITIONS>"
    "JUCE_MODULE_AVAILABLE_juce_module_exact=1")
_nakama_genex_erwarte(
    "26 PLATFORM_ID Windows"
    "$<PLATFORM_ID:Windows>"
    "1")
_nakama_kern_text_auswerten(
    "$<TARGET_PROPERTY:ListLiteral,INTERFACE_COMPILE_DEFINITIONS>"
    "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest Listenstruktur"
    _listenstruktur _listenstruktur_bekannt)
list(LENGTH _listenstruktur _listenstruktur_anzahl)
list(GET _listenstruktur 1 _listenstruktur_mitte)
if(NOT _listenstruktur_bekannt
   OR NOT _listenstruktur_anzahl EQUAL 3
   OR NOT _listenstruktur_mitte STREQUAL "JUCE_LISTE_LITERAL=a;b")
    message(FATAL_ERROR
        "Listenstruktur ging verloren: Anzahl=${_listenstruktur_anzahl}, "
        "Mitte=${_listenstruktur_mitte}, Gesamt=${_listenstruktur}")
endif()
message(STATUS "PASS Listenstruktur: 3 Elemente, literales Semikolon bleibt im mittleren Element")
_nakama_kern_text_auswerten(
    "$<TARGET_PROPERTY:juce_core,INTERFACE_COMPILE_DEFINITIONS>"
    "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest wortgleiche JUCE-Standarddefines"
    _juce_standardliste _juce_standardliste_bekannt)
list(LENGTH _juce_standardliste _juce_standardliste_anzahl)
list(GET _juce_standardliste 1 _juce_standardliste_mitte)
list(GET _juce_standardliste 2 _juce_standardliste_drittes)
if(NOT _juce_standardliste_bekannt
   OR NOT _juce_standardliste_anzahl EQUAL 4
   OR NOT _juce_standardliste_mitte STREQUAL "${_juce_standard_mitte_erwartet}"
   OR NOT _juce_standardliste_drittes STREQUAL "${_juce_standard_drittes_erwartet}")
    message(FATAL_ERROR
        "Wortgleiche JUCE-Standarddefines falsch ausgewertet: "
        "Anzahl=${_juce_standardliste_anzahl}, Mitte=${_juce_standardliste_mitte}, "
        "Drittes=${_juce_standardliste_drittes}, "
        "Gesamt=${_juce_standardliste}")
endif()
message(STATUS
    "PASS JUCE-Standarddefines: OR/CONFIG, IF-Leerzeichenzweig und PLATFORM_ID wortgleich")
_nakama_kern_eigenschaft_defines(
    juce_core INTERFACE_COMPILE_DEFINITIONS "${NAKAMA_TEST_CONFIG}" JUCE
    _juce_standarddefines_direkt)
set(_juce_standarddefines_direkt_erwartet
    "JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1;JUCE_MODULE_AVAILABLE_juce_core=1")
if(NOT "${_juce_standarddefines_direkt}" STREQUAL
       "${_juce_standarddefines_direkt_erwartet}")
    message(FATAL_ERROR
        "Direkter Define-Sammler wertete die zerlegte JUCE-Property falsch aus: "
        "Erwartet=${_juce_standarddefines_direkt_erwartet}; "
        "Ist=${_juce_standarddefines_direkt}")
endif()
message(STATUS
    "PASS direkter Define-Sammler: zerlegte JUCE-Property ist vollstaendig aufloesbar")
foreach(_wiederholungs_config Debug Release)
    _nakama_kern_text_auswerten(
        "$<TARGET_PROPERTY:juce_core,INTERFACE_COMPILE_DEFINITIONS>"
        "${_wiederholungs_config}" JUCE "Selbsttest Wiederbesuch je Konfiguration"
        _wiederholung _wiederholung_bekannt)
    if(NOT _wiederholung_bekannt)
        message(FATAL_ERROR
            "Legitimer juce_core-Wiederbesuch war fuer ${_wiederholungs_config} unbekannt.")
    endif()
endforeach()
message(STATUS "PASS Wiederbesuch: dieselbe juce_core-Eigenschaft in Debug und Release aufloesbar")
_nakama_kern_text_auswerten(
    "$<LINK_ONLY:$<IF:$<BOOL:1>,NakamaKernJuce,Niemals>>"
    "${NAKAMA_TEST_CONFIG}" COMPILE_LINK "Selbsttest LINK_ONLY Usage Requirements"
    _link_only_compile _link_only_compile_bekannt)
if(NOT _link_only_compile_bekannt OR NOT _link_only_compile STREQUAL "")
    message(FATAL_ERROR "LINK_ONLY wurde faelschlich in die Compile-Usage-Huelle gezogen.")
endif()
message(STATUS "PASS LINK_ONLY-Abgrenzung: keine Compile-Usage-Kante")

# Sensitivitaetskontrolle: Eine nicht gesetzte Eigenschaft darf nicht wie ein
# leerer, erfolgreich ausgewerteter Wert aussehen. Der oeffentliche Riegel
# macht daraus fuer das sichtbare JUCE_-Define einen FATAL_ERROR.
_nakama_kern_text_auswerten(
    "JUCE_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>"
    "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest unbekannte Eigenschaft"
    _unbekannt_ist _unbekannt_bekannt)
if(_unbekannt_bekannt)
    message(FATAL_ERROR "Unbekannte TARGET_PROPERTY wurde unerwartet als bekannt akzeptiert.")
endif()
_nakama_kern_wert_relevant(
    "JUCE_UNBEKANNT=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>" JUCE _unbekannt_relevant)
if(NOT _unbekannt_relevant)
    message(FATAL_ERROR "Unbekannte TARGET_PROPERTY mit JUCE_-Define wurde unerwartet ignoriert.")
endif()
message(STATUS "PASS Sensitivitaet: unbekannte relevante TARGET_PROPERTY bleibt ROT")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DNAKAMA_TEST_CONFIG=${NAKAMA_TEST_CONFIG}"
        "-DNAKAMA_TEST_IS_SYNTH=${NAKAMA_TEST_IS_SYNTH}"
        -DNAKAMA_TEST_ZYKLUS_ROT=ON
        -P "${CMAKE_CURRENT_LIST_FILE}"
    RESULT_VARIABLE _zyklus_exit
    OUTPUT_VARIABLE _zyklus_stdout
    ERROR_VARIABLE _zyklus_stderr)
set(_zyklus_ausgabe "${_zyklus_stdout}\n${_zyklus_stderr}")
string(REGEX REPLACE "[ \r\n\t]+" " " _zyklus_einzeilig "${_zyklus_ausgabe}")
set(_zykluspfad_erwartet
    "CycleA::INTERFACE_COMPILE_DEFINITIONS -> CycleB::INTERFACE_COMPILE_DEFINITIONS -> CycleA::INTERFACE_COMPILE_DEFINITIONS")
string(FIND "${_zyklus_einzeilig}" "${_zykluspfad_erwartet}" _zykluspfad_position)
if(_zyklus_exit EQUAL 0 OR _zykluspfad_position EQUAL -1)
    message(FATAL_ERROR
        "Echter Property-Zyklus war nicht kontrolliert ROT mit Pfad. "
        "Exit=${_zyklus_exit}; Ausgabe=${_zyklus_einzeilig}")
endif()
message(STATUS "PASS Sensitivitaet: Property-Zyklus ROT mit Pfad ${_zykluspfad_erwartet}")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DNAKAMA_TEST_CONFIG=${NAKAMA_TEST_CONFIG}"
        "-DNAKAMA_TEST_IS_SYNTH=${NAKAMA_TEST_IS_SYNTH}"
        -DNAKAMA_TEST_INNERER_OPERATOR_ROT=ON
        -P "${CMAKE_CURRENT_LIST_FILE}"
    RESULT_VARIABLE _innerer_operator_exit
    OUTPUT_VARIABLE _innerer_operator_stdout
    ERROR_VARIABLE _innerer_operator_stderr)
set(_innerer_operator_ausgabe
    "${_innerer_operator_stdout}\n${_innerer_operator_stderr}")
string(REGEX REPLACE "[ \r\n\t]+" " " _innerer_operator_einzeilig
    "${_innerer_operator_ausgabe}")
foreach(_diagnose_fragment
        "unbekannter Operator NAKAMA_UNBEKANNTER_INNERER_OPERATOR"
        "$<NAKAMA_UNBEKANNTER_INNERER_OPERATOR:1>"
        "$<TARGET_PROPERTY:BrokenInner,INTERFACE_COMPILE_DEFINITIONS>")
    string(FIND "${_innerer_operator_einzeilig}" "${_diagnose_fragment}"
        _diagnose_position)
    if(_diagnose_position EQUAL -1)
        message(FATAL_ERROR
            "Innere Genex-Diagnose nennt '${_diagnose_fragment}' nicht. "
            "Ausgabe=${_innerer_operator_einzeilig}")
    endif()
endforeach()
if(_innerer_operator_exit EQUAL 0)
    message(FATAL_ERROR
        "Unbekannter innerer Operator blieb unerwartet gruen. "
        "Ausgabe=${_innerer_operator_einzeilig}")
endif()
message(STATUS
    "PASS Diagnose: Operator, innerster Teilausdruck und aeusserer Ursprung werden genannt")
_nakama_kern_text_auswerten(
    "$<TARGET_PROPERTY:NichtVorhanden,INTERFACE_COMPILE_DEFINITIONS>"
    "${NAKAMA_TEST_CONFIG}" JUCE "Selbsttest unbekanntes Ziel"
    _fehlziel_ist _fehlziel_bekannt)
if(_fehlziel_bekannt)
    message(FATAL_ERROR "Nicht existierendes TARGET_PROPERTY-Ziel wurde unerwartet akzeptiert.")
endif()
message(STATUS "PASS Sensitivitaet: nicht existierendes Ziel bleibt ROT")
_nakama_kern_wert_relevant(
    "NAKAMA_TEST=$<TARGET_PROPERTY:EqCopilot,NAKAMA_UNBEKANNT>" JUCE _zielreferenz_relevant)
if(NOT _zielreferenz_relevant)
    message(FATAL_ERROR "Zielreferenz ohne sichtbares JUCE_-Define wurde unerwartet ignoriert.")
endif()
message(STATUS "PASS Sensitivitaet: jede Zielreferenz muss expandieren")
# NAK-84 (T3-Runde 3, 29.08.2026): Die Relevanzheuristik entscheidet nur noch
# ueber Linkkanten. In einer Define-/Options-Eigenschaft ist jeder unbekannte
# Operator ROT - dort waere jede Textprobe erschleichbar (JOIN).
foreach(_definemengen_art IN ITEMS IDENTITAET JUCE IDENTITAET_OPTION JUCE_OPTION)
    _nakama_kern_art_ist_definemenge("${_definemengen_art}" _ist_definemenge)
    if(NOT _ist_definemenge)
        message(FATAL_ERROR
            "Define-/Options-Art wurde nicht als Definemenge erkannt: ${_definemengen_art}")
    endif()
endforeach()
foreach(_linkart IN ITEMS LINK COMPILE_LINK)
    _nakama_kern_art_ist_definemenge("${_linkart}" _ist_definemenge)
    if(_ist_definemenge)
        message(FATAL_ERROR "Linkkanten-Art wurde faelschlich als Definemenge erkannt: ${_linkart}")
    endif()
endforeach()
message(STATUS
    "PASS Arteinteilung: vier Define-/Options-Arten fail-closed, zwei Linkarten heuristisch")

# Die verbliebene Abgrenzung gilt nur noch fuer Linkkanten: ein Ausdruck ohne
# Zielreferenz kann dort kein Ziel verbergen. Ein Define kann er nicht tragen.
_nakama_kern_wert_auswerten(
    "NAKAMA_TEST=$<NAKAMA_UNBEKANNTER_OPERATOR:ohne_ziel>"
    "${NAKAMA_TEST_CONFIG}" LINK "Selbsttest irrelevante Linkkante ohne Ziel" _fremde_kante)
if(NOT _fremde_kante STREQUAL "")
    message(FATAL_ERROR "Linkkante ohne Zielreferenz wurde unerwartet beansprucht.")
endif()
message(STATUS "PASS Abgrenzung: Linkkante ohne Zielreferenz bleibt irrelevant")

# Derselbe Ausdruck in einer Define-Eigenschaft muss dagegen ROT sein.
_nakama_genex_rotprobe(
    NAKAMA_TEST_IRRELEVANT_DEFINE_ROT
    "unbekannter Operator in Define-Eigenschaft"
    "Sensitivitaet: unbekannter Operator ohne Rohtextspur bleibt ROT"
    "unbekannter Operator NAKAMA_UNBEKANNTER_OPERATOR"
    "$<NAKAMA_UNBEKANNTER_OPERATOR:ohne_ziel>")

# NAK-84, der Prueferfall selbst: JOIN baut JUCE_USE_CURL=1 zusammen, ohne dass
# der Rohtext "JUCE_" oder ein Ziel zeigt. Vor dem Fix war beides gruen.
_nakama_genex_rotprobe(
    NAKAMA_TEST_JOIN_DEFINE_ROT
    "zusammengesetzter Definename per JOIN"
    "NAK-84: JOIN direkt in einer Define-Eigenschaft bleibt ROT"
    "unbekannter Operator JOIN"
    "$<JOIN:JUCE$<SEMICOLON>_USE_CURL=1,>")
_nakama_genex_rotprobe(
    NAKAMA_TEST_JOIN_PROPERTY_ROT
    "zusammengesetzter Definename per JOIN in der Huelle"
    "NAK-84: JOIN in einer Huellen-Define-Property bleibt ROT"
    "unbekannter Operator JOIN"
    "$<TARGET_PROPERTY:JoinDefine,INTERFACE_COMPILE_DEFINITIONS>")

foreach(_define IN ITEMS
        JUCE_SHARED_CODE JUCE_SHARED_CODE=1
        JUCE_STANDALONE_APPLICATION JUCE_STANDALONE_APPLICATION=0
        JUCE_VST3_CAN_REPLACE_VST2 JUCE_VST3_CAN_REPLACE_VST2=1)
    _nakama_kern_juce_define_ist_ausgenommen("${_define}" _ausgenommen)
    if(NOT _ausgenommen)
        message(FATAL_ERROR "Dokumentierte K2b-Ausnahme wurde nicht erkannt: ${_define}")
    endif()
endforeach()
foreach(_define IN ITEMS
        JUCE_SHARED_CODE_EXTRA=1
        JUCE_STANDALONE_APPLICATION_EXTRA=1
        JUCE_VST3_CAN_REPLACE_VST2_EXTRA=1)
    _nakama_kern_juce_define_ist_ausgenommen("${_define}" _ausgenommen)
    if(_ausgenommen)
        message(FATAL_ERROR "K2b-Ausnahme griff faelschlich als Praefix: ${_define}")
    endif()
endforeach()
message(STATUS "PASS K2b-Ausnahmen: drei Einzelmakros exakt, *_EXTRA bleibt sichtbar")

foreach(_operator IN ITEMS
        LOWER_CASE UPPER_CASE MAKE_C_IDENTIFIER JOIN REMOVE_DUPLICATES LIST PATH SHELL_PATH)
    _nakama_kern_linkkante_stringoperator("$<${_operator}:NakamaIdentitaetsIface>" _erkannt)
    if(NOT _erkannt STREQUAL _operator)
        message(FATAL_ERROR
            "Stringoperator in Linkkante nicht erkannt: erwartet=${_operator}, ist=${_erkannt}")
    endif()
endforeach()
message(STATUS "PASS Linkkanten-Abgrenzung: bekannte String-/Listenoperatoren werden erkannt")

execute_process(
    COMMAND "${CMAKE_COMMAND}"
        "-DNAKAMA_TEST_CONFIG=${NAKAMA_TEST_CONFIG}"
        "-DNAKAMA_TEST_IS_SYNTH=${NAKAMA_TEST_IS_SYNTH}"
        -DNAKAMA_TEST_STRINGOPERATOR_ROT=ON
        -P "${CMAKE_CURRENT_LIST_FILE}"
    RESULT_VARIABLE _stringoperator_exit
    OUTPUT_VARIABLE _stringoperator_stdout
    ERROR_VARIABLE _stringoperator_stderr)
set(_stringoperator_ausgabe "${_stringoperator_stdout}\n${_stringoperator_stderr}")
foreach(_diagnose_fragment
        "String-transformierender Generatorausdruck LOWER_CASE"
        "$<LOWER_CASE:NakamaIdentitaetsIface>"
        "fail-closed ROT")
    string(FIND "${_stringoperator_ausgabe}" "${_diagnose_fragment}"
        _diagnose_position)
    if(_diagnose_position EQUAL -1)
        message(FATAL_ERROR
            "Stringoperator-Diagnose nennt '${_diagnose_fragment}' nicht. "
            "Ausgabe=${_stringoperator_ausgabe}")
    endif()
endforeach()
if(_stringoperator_exit EQUAL 0)
    message(FATAL_ERROR "LOWER_CASE-Linkkante blieb unerwartet gruen.")
endif()
message(STATUS "PASS Sensitivitaet: LOWER_CASE-Linkkante bleibt kontrolliert ROT")

if(NOT _nakama_test_anzahl EQUAL 26)
    message(FATAL_ERROR "Interner Testfehler: ${_nakama_test_anzahl} statt 26 Ausdruecken geprueft.")
endif()
message(STATUS "Nakama-Kern-Genex-Selbsttest: 26/26 Ausdruecke korrekt.")
