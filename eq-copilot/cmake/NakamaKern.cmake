# S8 / SONDE-007a — der gemeinsame Kern der drei Nakama-Apps.
#
# ── Was hier geloest wird ───────────────────────────────────────────────────
#
# Bis heute stand im Kopf von plugin/CMakeLists.txt: "Probe und Tests
# kompilieren die geteilten Quellen erneut, statt eine static-lib mit fremden
# JUCE-Moduldefinitionen zu teilen — bewusst simpel." Das war richtig, solange
# es EIN Bundle gab. Mit Gen, Probeeq und Suna wird es falsch, denn dann
# vervielfacht sich nicht nur die Bauzeit, sondern auch die Zahl der Stellen,
# an denen der Kern eine fremde Identitaet einatmen kann.
#
# Entwurf §53.4 (Static-Lib-Randbedingung) erlaubt den Wechsel unter EINER
# harten Regel: "Der gemeinsame Kern sieht KEINE JucePlugin_*-Konstanten;
# Identitaet kommt ausschliesslich aus plugin-identities-v1.json ueber die
# duennen Target-Schichten. Andernfalls erhalten zwei der drei Bundles die
# Identitaetskonstanten des dritten."
#
# ── Warum eine Kopf-Fassade und nicht `target_link_libraries(... juce::…)` ──
#
# JUCE-Module sind INTERFACE-Bibliotheken, deren .cpp als INTERFACE_SOURCES
# haengen (_deps/juce-src/extras/Build/CMake/JUCEModuleSupport.cmake:505). Wer
# sie linkt, KOMPILIERT sie mit hinein. Eine Static-Lib, die juce::juce_core
# linkt, traegt also ihre eigene Kopie von juce_core.obj — und jedes Ziel, das
# beides linkt, haette zwei. Drei Messungen am 22.08.2026 (Wegwerfprojekt,
# VS 17 2022 / CMake 3.31.6):
#
#   1. `$<COMPILE_ONLY:iface>` streift INTERFACE_SOURCES NICHT ab — die
#      Modulquelle wurde trotzdem in die Lib kompiliert. Weg tot.
#   2. Beide Kopien nebeneinander LINKEN sauber (Exit 0), weil MSVC ein
#      .lib-Mitglied nur zieht, wenn sonst ein Symbol offen bliebe. "Geht
#      heute" — aber es haelt nur, solange beide Kopien deckungsgleich
#      uebersetzt werden. Fuer einen Kern, der drei Bundles traegt, zu wenig.
#   3. Eine Fassade, die Includes und Defines aus dem Modul-Target ABLEITET,
#      seine Quellen aber nicht erbt: Kern sieht die Defines (ein `#error`-
#      Gegentest schwieg), die Modulquelle wurde GENAU EINMAL uebersetzt,
#      Link sauber. Das ist der Weg.
#
# Der Kern uebersetzt damit gegen JUCE-KOEPFE; die Implementierung kommt aus
# der einen Kopie, die das verbrauchende Ziel ohnehin schon baut.
#
# ── Riegel ─────────────────────────────────────────────────────────────────
#
#   K1  plugin/state/NakamaKernRiegel.h — namentlich, am Anfang UND Ende jeder
#       Kern-Uebersetzungseinheit.
#   K2  nakama_kern_riegel_pruefen() hier — alle compilerwirksamen Defines des
#       Kernziels und seiner Usage-Requirements-Huelle; Verbraucher sind keine
#       Quellen des Kerns und liegen nur bei einer fehlerhaften Rueckkante darin.
#   K2b nakama_kern_konfig_pruefen() — Mengengleichheit der getrennt berechneten
#       JUCE-Konfigurationsdefines von Kern und genau einem Verbraucher.
#   K2c nakama_kern_schalter_pruefen() — Vergleich der getrennten, rekursiven
#       Linkhuellen von Kern und genau einem Verbraucher je Konfiguration.
#   K3  tools/eq-copilot/pruefe_kern_identitaetsfrei.py — misst das GEBAUTE
#       NakamaKern.lib gegen Text-, Viercode-Integer- und CID-Bytes aus
#       eq-copilot/identity/plugin-identities-v1.json und erzeugt die Lib
#       dafuer im selben Lauf neu (NAK-100: nur noch dieses Gate-Mass).
#
# K1/K2/K2b/K2c reden ueber Quell- und Baubeschreibung, K3 ueber das Artefakt.
# Erst zusammen sind sie eine Aussage. Linkkanten werden bis zu direkten
# Zielnamen, debug/optimized/general-Kanten und den unten benannten bedingten
# bzw. zielbezogenen Generatorausdruecken aufgeloest. String-transformierende
# Generatorausdruecke (LOWER_CASE, UPPER_CASE, MAKE_C_IDENTIFIER, JOIN, ...)
# werden in Linkkanten nicht aufgeloest und sind dort ROT; in einer
# Define-/Options-Eigenschaft ist seit NAK-84 jeder unbekannte Operator ROT,
# auch wenn sein Rohtext kein JUCE_/JucePlugin_ zeigt. Dasselbe gilt fuer
# importierte Ziele mit gesetztem MAP_IMPORTED_CONFIG_* in der Linkhuelle: Die
# Konfigurationsabbildung wird nicht nachgebildet, sondern fail-closed abgewiesen.

include_guard(GLOBAL)

# ── Konfigurations- und Generatorausdruck-Laeufer ───────────────────────────
# CMake wertet Usage Requirements erst bei der Generierung aus. Die Riegel
# laufen absichtlich schon beim Configure und muessen deshalb die fuer sie
# relevanten Ausdruecke selbst, fail-closed, aufloesen. Gemessen werden alle
# Konfigurationen des Generators; bei einem Single-Config-Generator ohne
# CMAKE_BUILD_TYPE steht NOCONFIG fuer dessen tatsaechlich leere Konfiguration.
function(_nakama_kern_konfigurationen ausgabe)
    if(CMAKE_CONFIGURATION_TYPES)
        set(_konfigurationen ${CMAKE_CONFIGURATION_TYPES})
    elseif(CMAKE_BUILD_TYPE)
        set(_konfigurationen "${CMAKE_BUILD_TYPE}")
    else()
        set(_konfigurationen NOCONFIG)
    endif()
    list(REMOVE_DUPLICATES _konfigurationen)
    set(${ausgabe} "${_konfigurationen}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_ziel_aufloesen eingabe ausgabe)
    set(_ziel "${eingabe}")
    if(CMAKE_SCRIPT_MODE_FILE)
        string(MAKE_C_IDENTIFIER "${_ziel}" _test_alias_schluessel)
        set(_test_alias_variable "NAKAMA_KERN_TEST_ALIAS_${_test_alias_schluessel}")
        if(DEFINED ${_test_alias_variable})
            set(_ziel "${${_test_alias_variable}}")
        endif()
    endif()
    if(TARGET "${_ziel}")
        get_target_property(_alias "${_ziel}" ALIASED_TARGET)
        if(_alias)
            set(_ziel "${_alias}")
        endif()
    endif()
    set(${ausgabe} "${_ziel}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_ziel_existiert eingabe ausgabe)
    _nakama_kern_ziel_aufloesen("${eingabe}" _ziel)
    if(TARGET "${_ziel}")
        set(_existiert TRUE)
    elseif(CMAKE_SCRIPT_MODE_FILE AND "${_ziel}" IN_LIST NAKAMA_KERN_TEST_TARGETS)
        set(_existiert TRUE)
    else()
        set(_existiert FALSE)
    endif()
    set(${ausgabe} ${_existiert} PARENT_SCOPE)
endfunction()

function(_nakama_kern_importierte_konfigabbildung_pruefen ziel kontext)
    if(CMAKE_SCRIPT_MODE_FILE)
        return()
    endif()

    _nakama_kern_ziel_aufloesen("${ziel}" _ziel)
    if(NOT TARGET "${_ziel}")
        return()
    endif()
    get_target_property(_importiert "${_ziel}" IMPORTED)
    if(NOT _importiert)
        return()
    endif()

    _nakama_kern_konfigurationen(_projektkonfigurationen)
    foreach(_projektkonfiguration IN LISTS _projektkonfigurationen)
        string(TOUPPER "${_projektkonfiguration}" _konfiguration_gross)
        set(_abbildung "MAP_IMPORTED_CONFIG_${_konfiguration_gross}")
        get_property(_abbildung_gesetzt
            TARGET "${_ziel}" PROPERTY "${_abbildung}" SET)
        if(_abbildung_gesetzt)
            get_target_property(_abbildungswert "${_ziel}" "${_abbildung}")
            if(_abbildungswert STREQUAL "_abbildungswert-NOTFOUND"
               OR _abbildungswert STREQUAL "")
                set(_abbildungswert "<leer>")
            endif()
            message(FATAL_ERROR
                "S8/SONDE-007a: importiertes Ziel '${_ziel}' in ${kontext} setzt "
                "${_abbildung}=${_abbildungswert}. Der Riegelauswerter bildet "
                "MAP_IMPORTED_CONFIG_* nicht nach; die Linkhuelle bleibt deshalb "
                "fail-closed ROT statt mit der Projektkonfiguration falsch gruen.")
        endif()
    endforeach()
endfunction()

function(_nakama_kern_konfiguration_passt namen konfiguration ausgabe)
    string(TOUPPER "${konfiguration}" _gesucht)
    string(REPLACE "," ";" _namen "${namen}")
    set(_passt FALSE)
    foreach(_name IN LISTS _namen)
        string(TOUPPER "${_name}" _name_gross)
        if(_name_gross STREQUAL _gesucht)
            set(_passt TRUE)
        endif()
    endforeach()
    set(${ausgabe} ${_passt} PARENT_SCOPE)
endfunction()

# JUCE 8 setzt seine JucePlugin_*-Defines und weitergereichten Moduldefines mit
# TARGET_PROPERTY, TARGET_GENEX_EVAL, FILTER und BOOL. Seine Definitions-,
# Options- und Linkeigenschaften benutzen ausserdem IF, STREQUAL, NOT, OR,
# CONFIG, COMPILE_LANGUAGE, PLATFORM_ID und TARGET_EXISTS; das Nakama-Projekt
# fuegt CXX_COMPILER_ID hinzu. Der kleine Auswerter kennt diese Formen sowie
# EQUAL, GENEX_EVAL, AND und die von CMake in Usage Requirements verwendeten
# LINK_ONLY-/BUILD_INTERFACE-/INSTALL_INTERFACE-/COMPILE_ONLY- und
# TARGET_NAME-Formen. Er zerlegt ':' und ',' nur auf der jeweils aeussersten
# Genex-Ebene; damit bleiben beliebige Verschachtelungen intakt.
#
# TARGET_PROPERTY liest skalare Eigenschaften direkt. Listeneigenschaften wie
# *_COMPILE_DEFINITIONS, *_COMPILE_OPTIONS und *_LINK_LIBRARIES werden Element
# fuer Element rekursiv ausgewertet. Ein explizit als Wertargument gereichter
# Rekursionsstapel aus Ziel und Eigenschaft stoppt nur echte Zyklen im aktuellen
# Abstieg; ein spaeterer legitimer Wiederbesuch wird erneut ausgewertet. Aliase
# werden vor dem Lesen aufgeloest, importierte Ziele sind fuer CMake normale
# TARGETs. Ein fehlendes Ziel und ein Zyklus sind immer ROT. Fuer einen
# unbekannten oder nicht auswertbaren Ausdruck gilt seit NAK-84 (29.08.2026)
# nach ART der gelesenen Eigenschaft:
#   * Define-/Options-Eigenschaft (IDENTITAET, JUCE, *_OPTION): IMMER ROT, ohne
#     jede Ruecksicht auf den Rohtext. Ein zusammengesetzter Definename wie
#     "$<JOIN:JUCE$<SEMICOLON>_USE_CURL=1,>" zeigt sein Praefix im Rohtext nie
#     zusammenhaengend; eine Textheuristik kann ihn deshalb prinzipiell nicht
#     erkennen und wurde hier ersatzlos gestrichen.
#   * Linkkante (LINK, COMPILE_LINK): ROT bei relevantem Define ODER
#     irgendeiner Zielreferenz. Nur dort darf ein Ausdruck ohne beides
#     irrelevant bleiben - er kann kein Define tragen, sondern hoechstens ein
#     Ziel verbergen, und dafuer haelt zusaetzlich der Stringoperator-Riegel.

function(_nakama_kern_genex_ende text anfang ausgabe bekannt)
    string(LENGTH "${text}" _laenge)
    set(_i ${anfang})
    set(_tiefe 0)
    while(_i LESS _laenge)
        string(SUBSTRING "${text}" ${_i} 1 _zeichen)
        math(EXPR _naechstes "${_i} + 1")
        if(_zeichen STREQUAL "$" AND _naechstes LESS _laenge)
            string(SUBSTRING "${text}" ${_naechstes} 1 _folgezeichen)
            if(_folgezeichen STREQUAL "<")
                math(EXPR _tiefe "${_tiefe} + 1")
                math(EXPR _i "${_i} + 2")
                continue()
            endif()
        endif()
        if(_zeichen STREQUAL ">")
            math(EXPR _tiefe "${_tiefe} - 1")
            if(_tiefe EQUAL 0)
                set(${ausgabe} ${_i} PARENT_SCOPE)
                set(${bekannt} TRUE PARENT_SCOPE)
                return()
            elseif(_tiefe LESS 0)
                break()
            endif()
        endif()
        math(EXPR _i "${_i} + 1")
    endwhile()
    set(${ausgabe} -1 PARENT_SCOPE)
    set(${bekannt} FALSE PARENT_SCOPE)
endfunction()

function(_nakama_kern_genex_kopf innen operator rest hat_rest bekannt)
    string(LENGTH "${innen}" _laenge)
    set(_i 0)
    set(_tiefe 0)
    while(_i LESS _laenge)
        string(SUBSTRING "${innen}" ${_i} 1 _zeichen)
        math(EXPR _naechstes "${_i} + 1")
        if(_zeichen STREQUAL "$" AND _naechstes LESS _laenge)
            string(SUBSTRING "${innen}" ${_naechstes} 1 _folgezeichen)
            if(_folgezeichen STREQUAL "<")
                math(EXPR _tiefe "${_tiefe} + 1")
                math(EXPR _i "${_i} + 2")
                continue()
            endif()
        elseif(_zeichen STREQUAL ">")
            math(EXPR _tiefe "${_tiefe} - 1")
            if(_tiefe LESS 0)
                set(${bekannt} FALSE PARENT_SCOPE)
                return()
            endif()
        elseif(_zeichen STREQUAL ":" AND _tiefe EQUAL 0)
            string(SUBSTRING "${innen}" 0 ${_i} _operator)
            math(EXPR _rest_anfang "${_i} + 1")
            string(SUBSTRING "${innen}" ${_rest_anfang} -1 _rest)
            set(${operator} "${_operator}" PARENT_SCOPE)
            set(${rest} "${_rest}" PARENT_SCOPE)
            set(${hat_rest} TRUE PARENT_SCOPE)
            set(${bekannt} TRUE PARENT_SCOPE)
            return()
        endif()
        math(EXPR _i "${_i} + 1")
    endwhile()
    if(NOT _tiefe EQUAL 0)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()
    set(${operator} "${innen}" PARENT_SCOPE)
    set(${rest} "" PARENT_SCOPE)
    set(${hat_rest} FALSE PARENT_SCOPE)
    set(${bekannt} TRUE PARENT_SCOPE)
endfunction()

function(_nakama_kern_genex_argumente text ausgabe bekannt)
    string(LENGTH "${text}" _laenge)
    set(_i 0)
    set(_tiefe 0)
    set(_teil "")
    set(_teile "")
    while(_i LESS _laenge)
        string(SUBSTRING "${text}" ${_i} 1 _zeichen)
        math(EXPR _naechstes "${_i} + 1")
        if(_zeichen STREQUAL "$" AND _naechstes LESS _laenge)
            string(SUBSTRING "${text}" ${_naechstes} 1 _folgezeichen)
            if(_folgezeichen STREQUAL "<")
                string(APPEND _teil "$<")
                math(EXPR _tiefe "${_tiefe} + 1")
                math(EXPR _i "${_i} + 2")
                continue()
            endif()
        endif()
        if(_zeichen STREQUAL ">")
            math(EXPR _tiefe "${_tiefe} - 1")
            if(_tiefe LESS 0)
                set(${bekannt} FALSE PARENT_SCOPE)
                return()
            endif()
            string(APPEND _teil ">")
        elseif(_zeichen STREQUAL "," AND _tiefe EQUAL 0)
            string(REPLACE ";" "\\;" _teil_listenfest "${_teil}")
            list(APPEND _teile "${_teil_listenfest}")
            set(_teil "")
        else()
            string(APPEND _teil "${_zeichen}")
        endif()
        math(EXPR _i "${_i} + 1")
    endwhile()
    if(NOT _tiefe EQUAL 0)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()
    string(REPLACE ";" "\\;" _teil_listenfest "${_teil}")
    list(APPEND _teile "${_teil_listenfest}")
    set(${ausgabe} "${_teile}" PARENT_SCOPE)
    set(${bekannt} TRUE PARENT_SCOPE)
endfunction()

# CMake trennt unquoted Argumente an Leerzeichen, auch wenn sie gemeinsam
# einen Generatorausdruck bilden. JUCEs Standard-Defines stehen deshalb als
# benachbarte Property-Listenelemente wie
#   $<IF:cond,DEBUG=1 ; _DEBUG=1,NDEBUG=1 ; _NDEBUG=1>
# im get_target_property-Ergebnis. CMake wertet diese Fragmente gemeinsam aus;
# der Configure-Riegel muss vor seiner eigenen Auswertung dasselbe tun.
function(_nakama_kern_genex_text_vollstaendig text ausgabe)
    set(_rest "${text}")
    while(TRUE)
        string(FIND "${_rest}" "$<" _anfang)
        if(_anfang EQUAL -1)
            set(${ausgabe} TRUE PARENT_SCOPE)
            return()
        endif()
        _nakama_kern_genex_ende("${_rest}" ${_anfang} _ende _ende_bekannt)
        if(NOT _ende_bekannt)
            set(${ausgabe} FALSE PARENT_SCOPE)
            return()
        endif()
        math(EXPR _danach "${_ende} + 1")
        string(SUBSTRING "${_rest}" ${_danach} -1 _rest)
    endwhile()
endfunction()

function(_nakama_kern_eigenschaftseintraege_gruppieren wert ausgabe)
    set(_fragmente "${wert}")
    set(_gruppen "")
    set(_offen "")
    foreach(_fragment IN LISTS _fragmente)
        if(_offen STREQUAL "")
            set(_offen "${_fragment}")
        else()
            # Der Property-Listentrenner ist im Generatorausdruck ein
            # Listentrenner des jeweiligen IF-Zweigs.
            string(APPEND _offen ";${_fragment}")
        endif()
        _nakama_kern_genex_text_vollstaendig("${_offen}" _vollstaendig)
        if(_vollstaendig)
            string(REPLACE ";" "\\;" _gruppe_listenfest "${_offen}")
            list(APPEND _gruppen "${_gruppe_listenfest}")
            set(_offen "")
        endif()
    endforeach()
    if(NOT _offen STREQUAL "")
        # Nicht still verwerfen: Der nachgelagerte strenge Auswerter nennt das
        # unvollstaendige innerste Fragment samt aeusserem Ursprung.
        string(REPLACE ";" "\\;" _gruppe_listenfest "${_offen}")
        list(APPEND _gruppen "${_gruppe_listenfest}")
    endif()
    set(${ausgabe} "${_gruppen}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_liste_auswerten_mit_stapel
         wert konfiguration art kontext stapel ausgabe bekannt)
    set(_eingabe "${wert}")
    set(_ergebnis "")
    foreach(_eintrag IN LISTS _eingabe)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_eintrag}" "${konfiguration}" "${art}" "${kontext}" "${stapel}"
            _teil _teil_bekannt)
        if(NOT _teil_bekannt)
            set(${ausgabe} "" PARENT_SCOPE)
            set(${bekannt} FALSE PARENT_SCOPE)
            return()
        endif()
        foreach(_unterelement IN LISTS _teil)
            string(REPLACE ";" "\\;" _listenfest "${_unterelement}")
            list(APPEND _ergebnis "${_listenfest}")
        endforeach()
    endforeach()
    set(${ausgabe} "${_ergebnis}" PARENT_SCOPE)
    set(${bekannt} TRUE PARENT_SCOPE)
endfunction()

function(_nakama_kern_wahrheitswert wert ausgabe)
    string(TOUPPER "${wert}" _gross)
    if(_gross STREQUAL ""
       OR _gross MATCHES "^(0|FALSE|OFF|N|NO|IGNORE|NOTFOUND)$"
       OR _gross MATCHES "-NOTFOUND$")
        set(_wahr FALSE)
    else()
        set(_wahr TRUE)
    endif()
    set(${ausgabe} ${_wahr} PARENT_SCOPE)
endfunction()

# In cmake -P gibt es keine Ziele. Die Testdatei speist deshalb deklarierte
# Testziele, Aliase und Eigenschaften ueber NAKAMA_KERN_TEST_* ein. Im echten
# Configure wird ausschliesslich die Eigenschaft des Alias-aufgeloesten realen
# oder importierten Ziels gelesen. Eine nicht gesetzte Standard-Usage-Liste ist
# leer; eine beliebige andere Eigenschaft ist bewusst "unbekannt" und kann bei
# einem relevanten Define oder einer Zielreferenz nicht falsch gruen werden.
function(_nakama_kern_target_property_lesen ziel eigenschaft ausgabe bekannt)
    _nakama_kern_ziel_aufloesen("${ziel}" _ziel)
    _nakama_kern_ziel_existiert("${_ziel}" _ziel_existiert)
    if(NOT _ziel_existiert)
        set(${ausgabe} "" PARENT_SCOPE)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()

    string(MAKE_C_IDENTIFIER "${_ziel}_${eigenschaft}" _test_schluessel)
    set(_test_variable "NAKAMA_KERN_TEST_PROPERTY_${_test_schluessel}")
    if(CMAKE_SCRIPT_MODE_FILE AND DEFINED ${_test_variable})
        set(${ausgabe} "${${_test_variable}}" PARENT_SCOPE)
        set(${bekannt} TRUE PARENT_SCOPE)
        return()
    endif()

    if(CMAKE_SCRIPT_MODE_FILE)
        set(${ausgabe} "" PARENT_SCOPE)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()
    get_target_property(_wert "${_ziel}" "${eigenschaft}")
    if(_wert STREQUAL "_wert-NOTFOUND")
        # CMake definiert eine nicht gesetzte Usage-Requirements-Eigenschaft
        # als leere Menge. Das ist fuer die von der Fassade abgefragten
        # Standardlisten vollstaendig aufloesbar (z. B. am leeren
        # juce_atomic_wrapper). Beliebige Projekt-/JUCE-Eigenschaften bekommen
        # diese Ausnahme nicht: NAKAMA_UNBEKANNT an einem JUCE_-Define bleibt
        # deshalb fail-closed.
        set(_leere_standardlisten
            COMPILE_DEFINITIONS
            COMPILE_OPTIONS
            LINK_LIBRARIES
            INTERFACE_COMPILE_DEFINITIONS
            INTERFACE_COMPILE_OPTIONS
            INTERFACE_INCLUDE_DIRECTORIES
            INTERFACE_LINK_LIBRARIES)
        if("${eigenschaft}" IN_LIST _leere_standardlisten)
            set(${ausgabe} "" PARENT_SCOPE)
            set(${bekannt} TRUE PARENT_SCOPE)
            return()
        endif()
        set(${ausgabe} "" PARENT_SCOPE)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()
    set(${ausgabe} "${_wert}" PARENT_SCOPE)
    set(${bekannt} TRUE PARENT_SCOPE)
endfunction()

function(_nakama_kern_genex_fehler art operator innen kontext stapel)
    if(stapel)
        string(REPLACE ";" " -> " _property_pfad "${stapel}")
        set(_pfad_text "\n  Property-Pfad: ${_property_pfad}")
    else()
        set(_pfad_text "")
    endif()
    message(FATAL_ERROR
        "S8/SONDE-007a: ${art} ${operator} im innersten fehlgeschlagenen "
        "Generatorausdruck:\n"
        "  Teilausdruck: $<${innen}>\n"
        "  innerhalb: ${_NAKAMA_KERN_GENEX_URSPRUNG}\n"
        "  Kontext: ${kontext}${_pfad_text}")
endfunction()

function(_nakama_kern_genex_auswerten innen konfiguration art kontext stapel ausgabe bekannt)
    _nakama_kern_genex_kopf("${innen}" _operator _rest _hat_rest _kopf_bekannt)
    if(NOT _kopf_bekannt)
        if(_NAKAMA_KERN_GENEX_STRENG)
            _nakama_kern_genex_fehler(
                "nicht lesbarer Operator" "<unbekannt>" "${innen}" "${kontext}" "${stapel}")
        endif()
        set(${ausgabe} "" PARENT_SCOPE)
        set(${bekannt} FALSE PARENT_SCOPE)
        return()
    endif()
    string(TOUPPER "${_operator}" _operator_gross)
    set(_operator_ist_bekannt TRUE)

    # Bedingte Kurzform, z. B. $<$<CONFIG:Release>:JUCE_USE_CURL=1>.
    if(_operator MATCHES "^\\$<")
        _nakama_kern_text_auswerten_mit_stapel(
            "${_operator}" "${konfiguration}" "${art}" "${kontext} [Bedingung]"
            "${stapel}" _bedingung _teil_bekannt)
        if(NOT _teil_bekannt)
            set(${ausgabe} "" PARENT_SCOPE)
            set(${bekannt} FALSE PARENT_SCOPE)
            return()
        endif()
        _nakama_kern_wahrheitswert("${_bedingung}" _aktiv)
        if(_aktiv)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [aktiver Zweig]"
                "${stapel}" _ergebnis _teil_bekannt)
        else()
            set(_ergebnis "")
            set(_teil_bekannt TRUE)
        endif()
    elseif((_operator STREQUAL "0" OR _operator STREQUAL "1") AND _hat_rest)
        if(_operator STREQUAL "1")
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [aktiver Zweig]"
                "${stapel}" _ergebnis _teil_bekannt)
        else()
            set(_ergebnis "")
            set(_teil_bekannt TRUE)
        endif()
    elseif(_operator_gross STREQUAL "TARGET_PROPERTY" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 2)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _ziel_roh)
            list(GET _argumente 1 _eigenschaft_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_ziel_roh}" "${konfiguration}" "${art}" "${kontext} [TARGET_PROPERTY-Ziel]"
                "${stapel}" _ziel _ziel_bekannt)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_eigenschaft_roh}" "${konfiguration}" "${art}"
                "${kontext} [TARGET_PROPERTY-Eigenschaft]" "${stapel}"
                _eigenschaft _eigenschaft_bekannt)
            if(NOT _ziel_bekannt OR NOT _eigenschaft_bekannt)
                set(_teil_bekannt FALSE)
            else()
                _nakama_kern_ziel_aufloesen("${_ziel}" _ziel)
                _nakama_kern_target_property_lesen(
                    "${_ziel}" "${_eigenschaft}" _eigenschaftswert _eigenschaft_gesetzt)
                if(_eigenschaft_gesetzt)
                    set(_besuch "${_ziel}::${_eigenschaft}")
                    if("${_besuch}" IN_LIST stapel)
                        set(_zyklus ${stapel} "${_besuch}")
                        string(REPLACE ";" " -> " _zykluspfad "${_zyklus}")
                        message(FATAL_ERROR
                            "S8/SONDE-007a: zyklische TARGET_PROPERTY-Auswertung; "
                            "Rekursionspfad: ${_zykluspfad}")
                    else()
                        # Der Abstieg bekommt eine Kopie des aktuellen Pfads.
                        # Nach seiner Rueckkehr ist `_abstieg` mit diesem
                        # Funktionsscope verschwunden: Geschwister duerfen
                        # dieselbe Eigenschaft deshalb normal erneut lesen.
                        set(_abstieg ${stapel})
                        list(APPEND _abstieg "${_besuch}")
                        set(_ergebnis "")
                        set(_teil_bekannt TRUE)
                        # Target-Usage-Properties sind Listen. Jedes Element
                        # wird einzeln mit derselben Semantik ausgewertet;
                        # dadurch koennen Listen selbst weitere Properties
                        # oder bedingte Defines enthalten, ohne zu einem
                        # unteilbaren Textwert zusammenzufallen.
                        _nakama_kern_eigenschaftseintraege_gruppieren(
                            "${_eigenschaftswert}" _eigenschaftseintraege)
                        foreach(_eigenschaftselement IN LISTS _eigenschaftseintraege)
                            _nakama_kern_text_auswerten_mit_stapel(
                                "${_eigenschaftselement}" "${konfiguration}" "${art}"
                                "${_ziel} [${_eigenschaft}]" "${_abstieg}"
                                _element _element_bekannt)
                            if(NOT _element_bekannt)
                                set(_teil_bekannt FALSE)
                                break()
                            endif()
                            if(NOT _element STREQUAL "")
                                # `_element` darf selbst eine Liste sein. Die
                                # beabsichtigten Semikolon-Trenner werden als
                                # einzelne Rueckgabeelemente uebernommen.
                                foreach(_unterelement IN LISTS _element)
                                    # Ein escaped Semikolon innerhalb EINES
                                    # CMake-Listenelements kommt aus `foreach`
                                    # als echtes Zeichen zurueck. Vor dem
                                    # erneuten APPEND muss es wieder escaped
                                    # werden, sonst entstehen zwei Elemente.
                                    string(REPLACE ";" "\\;" _listenfest "${_unterelement}")
                                    list(APPEND _ergebnis "${_listenfest}")
                                endforeach()
                            endif()
                        endforeach()
                    endif()
                else()
                    set(_teil_bekannt FALSE)
                endif()
            endif()
        endif()
    elseif(_operator_gross STREQUAL "BOOL" AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [BOOL]"
            "${stapel}" _wert _teil_bekannt)
        if(_teil_bekannt)
            _nakama_kern_wahrheitswert("${_wert}" _wahr)
            if(_wahr)
                set(_ergebnis 1)
            else()
                set(_ergebnis 0)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "IF" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 3)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _bedingung_roh)
            list(GET _argumente 1 _wahr_roh)
            list(GET _argumente 2 _falsch_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_bedingung_roh}" "${konfiguration}" "${art}" "${kontext} [IF-Bedingung]"
                "${stapel}" _bedingung _teil_bekannt)
            if(_teil_bekannt)
                _nakama_kern_wahrheitswert("${_bedingung}" _aktiv)
                if(_aktiv)
                    set(_zweig "${_wahr_roh}")
                else()
                    set(_zweig "${_falsch_roh}")
                endif()
                _nakama_kern_liste_auswerten_mit_stapel(
                    "${_zweig}" "${konfiguration}" "${art}" "${kontext} [IF-Zweig]"
                    "${stapel}" _ergebnis _teil_bekannt)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "STREQUAL" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 2)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _links_roh)
            list(GET _argumente 1 _rechts_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_links_roh}" "${konfiguration}" "${art}" "${kontext} [STREQUAL links]"
                "${stapel}" _links _links_bekannt)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rechts_roh}" "${konfiguration}" "${art}" "${kontext} [STREQUAL rechts]"
                "${stapel}" _rechts _rechts_bekannt)
            if(_links_bekannt AND _rechts_bekannt)
                if("${_links}" STREQUAL "${_rechts}")
                    set(_ergebnis 1)
                else()
                    set(_ergebnis 0)
                endif()
                set(_teil_bekannt TRUE)
            else()
                set(_teil_bekannt FALSE)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "EQUAL" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 2)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _links_roh)
            list(GET _argumente 1 _rechts_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_links_roh}" "${konfiguration}" "${art}" "${kontext} [EQUAL links]"
                "${stapel}" _links _links_bekannt)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rechts_roh}" "${konfiguration}" "${art}" "${kontext} [EQUAL rechts]"
                "${stapel}" _rechts _rechts_bekannt)
            if(_links_bekannt AND _rechts_bekannt
               AND "${_links}" MATCHES "^-?[0-9]+$"
               AND "${_rechts}" MATCHES "^-?[0-9]+$")
                if("${_links}" EQUAL "${_rechts}")
                    set(_ergebnis 1)
                else()
                    set(_ergebnis 0)
                endif()
                set(_teil_bekannt TRUE)
            else()
                set(_teil_bekannt FALSE)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "GENEX_EVAL" AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [GENEX_EVAL]"
            "${stapel}" _ergebnis _teil_bekannt)
    elseif(_operator_gross STREQUAL "TARGET_GENEX_EVAL" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 2)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _ziel_roh)
            list(GET _argumente 1 _ausdruck_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_ziel_roh}" "${konfiguration}" "${art}"
                "${kontext} [TARGET_GENEX_EVAL-Ziel]" "${stapel}"
                _ziel _ziel_bekannt)
            if(_ziel_bekannt)
                _nakama_kern_ziel_aufloesen("${_ziel}" _ziel)
                _nakama_kern_ziel_existiert("${_ziel}" _ziel_existiert)
            endif()
            if(NOT _ziel_bekannt OR NOT _ziel_existiert)
                set(_teil_bekannt FALSE)
            else()
                _nakama_kern_text_auswerten_mit_stapel(
                    "${_ausdruck_roh}" "${konfiguration}" "${art}"
                    "${kontext} [TARGET_GENEX_EVAL ${_ziel}]" "${stapel}"
                    _ergebnis _teil_bekannt)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "FILTER" AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR NOT _anzahl EQUAL 3)
            set(_teil_bekannt FALSE)
        else()
            list(GET _argumente 0 _liste_roh)
            list(GET _argumente 1 _modus_roh)
            list(GET _argumente 2 _regex_roh)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_liste_roh}" "${konfiguration}" "${art}" "${kontext} [FILTER-Liste]"
                "${stapel}" _liste _liste_bekannt)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_modus_roh}" "${konfiguration}" "${art}" "${kontext} [FILTER-Modus]"
                "${stapel}" _modus _modus_bekannt)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_regex_roh}" "${konfiguration}" "${art}" "${kontext} [FILTER-Regex]"
                "${stapel}" _regex _regex_bekannt)
            string(TOUPPER "${_modus}" _modus)
            if(NOT _liste_bekannt OR NOT _modus_bekannt OR NOT _regex_bekannt
               OR NOT (_modus STREQUAL "INCLUDE" OR _modus STREQUAL "EXCLUDE"))
                set(_teil_bekannt FALSE)
            else()
                set(_ergebnis "")
                foreach(_listenelement IN LISTS _liste)
                    if("${_listenelement}" MATCHES "${_regex}")
                        set(_passt TRUE)
                    else()
                        set(_passt FALSE)
                    endif()
                    if((_modus STREQUAL "INCLUDE" AND _passt)
                       OR (_modus STREQUAL "EXCLUDE" AND NOT _passt))
                        list(APPEND _ergebnis "${_listenelement}")
                    endif()
                endforeach()
                set(_teil_bekannt TRUE)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "NOT" AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [NOT]"
            "${stapel}" _wert _teil_bekannt)
        if(_teil_bekannt)
            _nakama_kern_wahrheitswert("${_wert}" _wahr)
            if(_wahr)
                set(_ergebnis 0)
            else()
                set(_ergebnis 1)
            endif()
        endif()
    elseif((_operator_gross STREQUAL "AND" OR _operator_gross STREQUAL "OR") AND _hat_rest)
        _nakama_kern_genex_argumente("${_rest}" _argumente _argumente_bekannt)
        list(LENGTH _argumente _anzahl)
        if(NOT _argumente_bekannt OR _anzahl EQUAL 0)
            set(_teil_bekannt FALSE)
        else()
            set(_teil_bekannt TRUE)
            if(_operator_gross STREQUAL "AND")
                set(_ergebnis 1)
            else()
                set(_ergebnis 0)
            endif()
            foreach(_argument IN LISTS _argumente)
                _nakama_kern_text_auswerten_mit_stapel(
                    "${_argument}" "${konfiguration}" "${art}" "${kontext} [${_operator_gross}]"
                    "${stapel}" _wert _wert_bekannt)
                if(NOT _wert_bekannt)
                    set(_teil_bekannt FALSE)
                    break()
                endif()
                _nakama_kern_wahrheitswert("${_wert}" _wahr)
                if(_operator_gross STREQUAL "AND" AND NOT _wahr)
                    set(_ergebnis 0)
                    break()
                elseif(_operator_gross STREQUAL "OR" AND _wahr)
                    set(_ergebnis 1)
                    break()
                endif()
            endforeach()
        endif()
    elseif(_operator_gross STREQUAL "CONFIG")
        if(_hat_rest)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [CONFIG]"
                "${stapel}" _namen _teil_bekannt)
            if(_teil_bekannt)
                _nakama_kern_konfiguration_passt("${_namen}" "${konfiguration}" _passt)
                if(_passt)
                    set(_ergebnis 1)
                else()
                    set(_ergebnis 0)
                endif()
            endif()
        else()
            if(konfiguration STREQUAL "NOCONFIG")
                set(_ergebnis "")
            else()
                set(_ergebnis "${konfiguration}")
            endif()
            set(_teil_bekannt TRUE)
        endif()
    elseif(_operator_gross STREQUAL "COMPILE_LANGUAGE" AND _hat_rest)
        if(CMAKE_SCRIPT_MODE_FILE AND DEFINED NAKAMA_KERN_TEST_COMPILE_LANGUAGE)
            set(_sprache "${NAKAMA_KERN_TEST_COMPILE_LANGUAGE}")
        else()
            # NakamaKern und seine registrierten Verbraucher uebersetzen die
            # hier gemessenen Properties fuer ihre C++-Quellen.
            set(_sprache CXX)
        endif()
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [COMPILE_LANGUAGE]"
            "${stapel}" _sprachen _teil_bekannt)
        if(_teil_bekannt)
            _nakama_kern_konfiguration_passt("${_sprachen}" "${_sprache}" _passt)
            if(_passt)
                set(_ergebnis 1)
            else()
                set(_ergebnis 0)
            endif()
        endif()
    elseif(_operator_gross STREQUAL "CXX_COMPILER_ID")
        if(CMAKE_SCRIPT_MODE_FILE AND DEFINED NAKAMA_KERN_TEST_CXX_COMPILER_ID)
            set(_compiler_id "${NAKAMA_KERN_TEST_CXX_COMPILER_ID}")
        else()
            set(_compiler_id "${CMAKE_CXX_COMPILER_ID}")
        endif()
        if(_hat_rest)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [CXX_COMPILER_ID]"
                "${stapel}" _compiler_ids _teil_bekannt)
            if(_teil_bekannt)
                _nakama_kern_konfiguration_passt("${_compiler_ids}" "${_compiler_id}" _passt)
                if(_passt)
                    set(_ergebnis 1)
                else()
                    set(_ergebnis 0)
                endif()
            endif()
        else()
            set(_ergebnis "${_compiler_id}")
            set(_teil_bekannt TRUE)
        endif()
    elseif(_operator_gross STREQUAL "LINK_ONLY" AND _hat_rest)
        if(art STREQUAL "COMPILE_LINK")
            set(_ergebnis "")
            set(_teil_bekannt TRUE)
        else()
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [LINK_ONLY]"
                "${stapel}" _ergebnis _teil_bekannt)
        endif()
    elseif((_operator_gross STREQUAL "COMPILE_ONLY" OR _operator_gross STREQUAL "BUILD_INTERFACE")
           AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [${_operator_gross}]"
            "${stapel}" _ergebnis _teil_bekannt)
    elseif(_operator_gross STREQUAL "INSTALL_INTERFACE" AND _hat_rest)
        set(_ergebnis "")
        set(_teil_bekannt TRUE)
    elseif(_operator_gross STREQUAL "PLATFORM_ID")
        if(_hat_rest)
            _nakama_kern_text_auswerten_mit_stapel(
                "${_rest}" "${konfiguration}" "${art}" "${kontext} [PLATFORM_ID]"
                "${stapel}" _namen _teil_bekannt)
            if(_teil_bekannt)
                _nakama_kern_konfiguration_passt("${_namen}" "${CMAKE_SYSTEM_NAME}" _passt)
                if(_passt)
                    set(_ergebnis 1)
                else()
                    set(_ergebnis 0)
                endif()
            endif()
        else()
            set(_ergebnis "${CMAKE_SYSTEM_NAME}")
            set(_teil_bekannt TRUE)
        endif()
    elseif(_operator_gross STREQUAL "TARGET_EXISTS" AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [TARGET_EXISTS]"
            "${stapel}" _ziel _teil_bekannt)
        if(_teil_bekannt)
            _nakama_kern_ziel_existiert("${_ziel}" _ziel_existiert)
            if(_ziel_existiert)
                set(_ergebnis 1)
            else()
                set(_ergebnis 0)
            endif()
        endif()
    elseif((_operator_gross STREQUAL "TARGET_NAME_IF_EXISTS" OR _operator_gross STREQUAL "TARGET_NAME")
           AND _hat_rest)
        _nakama_kern_text_auswerten_mit_stapel(
            "${_rest}" "${konfiguration}" "${art}" "${kontext} [${_operator_gross}]"
            "${stapel}" _ziel _teil_bekannt)
        if(_teil_bekannt)
            _nakama_kern_ziel_existiert("${_ziel}" _ziel_existiert)
        endif()
        if(_teil_bekannt AND _ziel_existiert)
            set(_ergebnis "${_ziel}")
        elseif(_operator_gross STREQUAL "TARGET_NAME_IF_EXISTS")
            set(_ergebnis "")
            set(_teil_bekannt TRUE)
        else()
            set(_teil_bekannt FALSE)
        endif()
    else()
        set(_ergebnis "")
        set(_teil_bekannt FALSE)
        set(_operator_ist_bekannt FALSE)
    endif()

    if(NOT _teil_bekannt AND _NAKAMA_KERN_GENEX_STRENG)
        if(_operator_ist_bekannt)
            set(_fehlerart "nicht auswertbarer Operator")
        else()
            set(_fehlerart "unbekannter Operator")
        endif()
        _nakama_kern_genex_fehler(
            "${_fehlerart}" "${_operator_gross}" "${innen}" "${kontext}" "${stapel}")
    endif()

    set(${ausgabe} "${_ergebnis}" PARENT_SCOPE)
    set(${bekannt} ${_teil_bekannt} PARENT_SCOPE)
endfunction()

function(_nakama_kern_text_auswerten_mit_stapel
         wert konfiguration art kontext stapel ausgabe bekannt)
    set(_rest "${wert}")
    set(_ergebnis "")
    while(TRUE)
        string(FIND "${_rest}" "$<" _anfang)
        if(_anfang EQUAL -1)
            # `_rest` gehoert zum aktuell ausgewerteten Listenelement. Ein
            # darin enthaltenes Semikolon war in der Quellliste escaped und
            # muss es fuer die Rueckgabeliste wieder sein. Semikolons aus
            # rekursiven Genex-Ergebnissen werden dagegen unten als echte
            # Listentrenner uebernommen.
            string(REPLACE ";" "\\;" _rest_listenfest "${_rest}")
            string(APPEND _ergebnis "${_rest_listenfest}")
            set(${ausgabe} "${_ergebnis}" PARENT_SCOPE)
            set(${bekannt} TRUE PARENT_SCOPE)
            return()
        endif()
        if(_anfang GREATER 0)
            string(SUBSTRING "${_rest}" 0 ${_anfang} _vorher)
            string(REPLACE ";" "\\;" _vorher_listenfest "${_vorher}")
            string(APPEND _ergebnis "${_vorher_listenfest}")
        endif()
        _nakama_kern_genex_ende("${_rest}" ${_anfang} _ende _ende_bekannt)
        if(NOT _ende_bekannt)
            if(_NAKAMA_KERN_GENEX_STRENG)
                message(FATAL_ERROR
                    "S8/SONDE-007a: nicht abgeschlossener innerster Generatorausdruck:\n"
                    "  Teilausdruck: ${_rest}\n"
                    "  innerhalb: ${_NAKAMA_KERN_GENEX_URSPRUNG}\n"
                    "  Kontext: ${kontext}")
            endif()
            set(${ausgabe} "" PARENT_SCOPE)
            set(${bekannt} FALSE PARENT_SCOPE)
            return()
        endif()
        math(EXPR _innen_anfang "${_anfang} + 2")
        math(EXPR _innen_laenge "${_ende} - ${_innen_anfang}")
        string(SUBSTRING "${_rest}" ${_innen_anfang} ${_innen_laenge} _innen)
        _nakama_kern_genex_auswerten(
            "${_innen}" "${konfiguration}" "${art}" "${kontext}" "${stapel}"
            _teil _teil_bekannt)
        if(NOT _teil_bekannt)
            set(${ausgabe} "" PARENT_SCOPE)
            set(${bekannt} FALSE PARENT_SCOPE)
            return()
        endif()
        string(APPEND _ergebnis "${_teil}")
        math(EXPR _danach "${_ende} + 1")
        string(SUBSTRING "${_rest}" ${_danach} -1 _rest)
    endwhile()
endfunction()

# Oeffentlicher Einstieg: Jeder unabhaengige Wert beginnt mit einem leeren
# Rekursionspfad. Nur die internen Aufrufe oben duerfen einen Stack erben.
function(_nakama_kern_text_auswerten wert konfiguration art kontext ausgabe bekannt)
    _nakama_kern_text_auswerten_mit_stapel(
        "${wert}" "${konfiguration}" "${art}" "${kontext}" ""
        _ergebnis _bekannt)
    set(${ausgabe} "${_ergebnis}" PARENT_SCOPE)
    set(${bekannt} ${_bekannt} PARENT_SCOPE)
endfunction()

function(_nakama_kern_wert_relevant wert art ausgabe)
    set(_relevant FALSE)
    set(_wert "${wert}")
    string(REGEX REPLACE "^SHELL:" "" _wert "${_wert}")
    set(_zielreferenz FALSE)
    if(_wert MATCHES "\\$<(TARGET_PROPERTY|TARGET_GENEX_EVAL|TARGET_EXISTS|TARGET_NAME_IF_EXISTS|TARGET_NAME):")
        set(_zielreferenz TRUE)
    endif()
    string(REGEX MATCHALL "[A-Za-z_][A-Za-z0-9_.:+-]*" _teile "${_wert}")
    foreach(_teil IN LISTS _teile)
        _nakama_kern_ziel_existiert("${_teil}" _teil_ist_ziel)
        if(_teil_ist_ziel)
            set(_zielreferenz TRUE)
        endif()
    endforeach()
    if(_zielreferenz)
        # Auch ein Ausdruck ohne sichtbares JUCE_-Praefix muss expandieren,
        # sobald er ein Ziel nennt: dessen Usage-Properties koennen die
        # relevanten Defines erst indirekt beisteuern.
        set(_relevant TRUE)
    endif()

    if(art STREQUAL "IDENTITAET")
        if(_wert MATCHES "(^|[:>,])-?/?D?JucePlugin_")
            set(_relevant TRUE)
        endif()
    elseif(art STREQUAL "JUCE")
        if(_wert MATCHES "(^|[:>,])-?/?D?JUCE_")
            set(_relevant TRUE)
        endif()
    elseif(art STREQUAL "IDENTITAET_OPTION")
        if(_wert MATCHES "(^|[ :>,])[-/]D([ ]*)JucePlugin_")
            set(_relevant TRUE)
        endif()
    elseif(art STREQUAL "JUCE_OPTION")
        if(_wert MATCHES "(^|[ :>,])[-/]D([ ]*)JUCE_")
            set(_relevant TRUE)
        endif()
    endif()
    set(${ausgabe} ${_relevant} PARENT_SCOPE)
endfunction()

# NAK-84 (T3-Runde 3, 29.08.2026): Diese vier Arten bezeichnen
# Define-/Options-Eigenschaften. Ihr Inhalt IST die Definemenge des Kerns bzw.
# seiner Huelle - deshalb darf dort kein Rohtext mehr darueber entscheiden, ob
# ein nicht aufloesbarer Ausdruck rot wird. LINK/COMPILE_LINK bezeichnen
# dagegen Linkkanten; dort entscheidet die Heuristik ueber Zielnamen, nicht
# ueber Definetext, und der Stringoperator-Riegel darunter haelt zusaetzlich.
function(_nakama_kern_art_ist_definemenge art ausgabe)
    if(art STREQUAL "IDENTITAET" OR art STREQUAL "JUCE"
       OR art STREQUAL "IDENTITAET_OPTION" OR art STREQUAL "JUCE_OPTION")
        set(${ausgabe} TRUE PARENT_SCOPE)
    else()
        set(${ausgabe} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(_nakama_kern_linkkante_stringoperator wert ausgabe)
    set(_operator "")
    string(REGEX MATCH
        "\\$<(LOWER_CASE|UPPER_CASE|MAKE_C_IDENTIFIER|JOIN|REMOVE_DUPLICATES|LIST|PATH|SHELL_PATH):"
        _fund "${wert}")
    if(_fund)
        set(_operator "${CMAKE_MATCH_1}")
    endif()
    set(${ausgabe} "${_operator}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_wert_auswerten wert konfiguration art kontext ausgabe)
    if(art STREQUAL "LINK" OR art STREQUAL "COMPILE_LINK")
        _nakama_kern_linkkante_stringoperator("${wert}" _stringoperator)
        if(_stringoperator)
            message(FATAL_ERROR
                "S8/SONDE-007a: String-transformierender Generatorausdruck "
                "${_stringoperator} in Linkkante wird nicht aufgeloest; stilles "
                "Verwerfen koennte ein Ziel und dessen Defines verbergen und ist "
                "deshalb fail-closed ROT.\n"
                "  Ausdruck: ${wert}\n"
                "  Kontext: ${kontext}")
        endif()
    endif()
    _nakama_kern_wert_relevant("${wert}" "${art}" _relevant)
    # NAK-84: In einer Define-/Options-Eigenschaft ist JEDER unbekannte oder
    # nicht auswertbare Operator ROT - unabhaengig vom Rohtext. Der Prueferfall
    # war "$<JOIN:JUCE$<SEMICOLON>_USE_CURL=1,>": das erzeugt JUCE_USE_CURL=1,
    # der Rohtext enthaelt aber weder "JUCE_" noch ein Ziel. Die Heuristik hielt
    # ihn fuer irrelevant und verwarf ihn still; K2b blieb trotz Wertwiderspruch
    # gruen. Ein zusammengesetzter Name kann per Konstruktion nicht am Rohtext
    # erkannt werden - also faellt die Heuristik hier ersatzlos weg.
    _nakama_kern_art_ist_definemenge("${art}" _art_ist_definemenge)
    if(_art_ist_definemenge)
        set(_relevant TRUE)
    endif()
    # Diese beiden Variablen sind absichtlich read-only und an den Scope genau
    # dieses Auswertungsaufrufs gebunden. Verschachtelte Funktionen erben sie,
    # ohne globalen Zustand oder PARENT_SCOPE-Schreibzugriffe zu benutzen.
    set(_NAKAMA_KERN_GENEX_STRENG ${_relevant})
    set(_NAKAMA_KERN_GENEX_URSPRUNG "${wert}")
    _nakama_kern_text_auswerten(
        "${wert}" "${konfiguration}" "${art}" "${kontext}" _ergebnis _bekannt)
    if(NOT _bekannt)
        if(_relevant)
            message(FATAL_ERROR
                "S8/SONDE-007a: Generatorausdruck in ${kontext} ist fuer "
                "${konfiguration} nicht aufloesbar; stilles Verwerfen waere falsch gruen:\n"
                "  ${wert}")
        endif()
        set(_ergebnis "")
    endif()
    set(${ausgabe} "${_ergebnis}" PARENT_SCOPE)
endfunction()

# Sammelt rekursiv alle in `konfiguration` erreichbaren Ziele ab `start`.
# LINK_LIBRARIES und INTERFACE_LINK_LIBRARIES werden beide abgelaufen; Aliase
# werden auf ihr echtes (auch importiertes) Ziel aufgeloest. Systembibliotheken
# und reine Linkerflags sind keine Ziele und bleiben draussen.
function(_nakama_kern_huelle start konfiguration ausgabe)
    set(_gesehen "")
    set(_offen "${start}")

    while(_offen)
        list(POP_FRONT _offen _ziel)
        _nakama_kern_ziel_aufloesen("${_ziel}" _ziel)

        if(NOT TARGET "${_ziel}")
            continue()
        endif()
        if("${_ziel}" IN_LIST _gesehen)
            continue()
        endif()
        _nakama_kern_importierte_konfigabbildung_pruefen(
            "${_ziel}" "Linkhuelle von '${start}' [${konfiguration}]")
        list(APPEND _gesehen "${_ziel}")

        foreach(_eigenschaft INTERFACE_LINK_LIBRARIES LINK_LIBRARIES)
            get_target_property(_kanten "${_ziel}" ${_eigenschaft})
            if(NOT _kanten)
                continue()
            endif()

            _nakama_kern_eigenschaftseintraege_gruppieren(
                "${_kanten}" _kanteneintraege)
            set(_linkart general)
            foreach(_kante IN LISTS _kanteneintraege)
                if(_kante STREQUAL "debug" OR _kante STREQUAL "optimized" OR _kante STREQUAL "general")
                    set(_linkart "${_kante}")
                    continue()
                endif()

                string(TOUPPER "${konfiguration}" _konfig_gross)
                set(_aktiv TRUE)
                if(_linkart STREQUAL "debug" AND NOT _konfig_gross STREQUAL "DEBUG")
                    set(_aktiv FALSE)
                elseif(_linkart STREQUAL "optimized" AND _konfig_gross STREQUAL "DEBUG")
                    set(_aktiv FALSE)
                endif()
                set(_linkart general)
                if(NOT _aktiv)
                    continue()
                endif()

                _nakama_kern_wert_auswerten(
                    "${_kante}" "${konfiguration}" LINK
                    "${_ziel} [${_eigenschaft}]" _ziele)
                foreach(_naechstes IN LISTS _ziele)
                    list(APPEND _offen "${_naechstes}")
                endforeach()
            endforeach()
        endforeach()
    endwhile()

    set(${ausgabe} "${_gesehen}" PARENT_SCOPE)
endfunction()

# Usage-Requirements-Huelle fuer die Uebersetzung von `start`: Am Wurzelziel
# wirken dessen LINK_LIBRARIES; danach propagieren nur INTERFACE_LINK_LIBRARIES
# weiter. LINK_ONLY-Kanten werden dabei absichtlich nicht verfolgt. Der breite
# Linkhuellen-Laeufer oben bleibt fuer K2c zustaendig, das Linkquellen misst.
function(_nakama_kern_nutzungshuelle start konfiguration ausgabe)
    _nakama_kern_ziel_aufloesen("${start}" _wurzel)
    set(_gesehen "")
    set(_offen "${_wurzel}")

    while(_offen)
        list(POP_FRONT _offen _ziel)
        _nakama_kern_ziel_aufloesen("${_ziel}" _ziel)
        if(NOT TARGET "${_ziel}" OR "${_ziel}" IN_LIST _gesehen)
            continue()
        endif()
        _nakama_kern_importierte_konfigabbildung_pruefen(
            "${_ziel}" "Usage-Requirements-Huelle von '${start}' [${konfiguration}]")
        list(APPEND _gesehen "${_ziel}")

        if(_ziel STREQUAL _wurzel)
            set(_eigenschaft LINK_LIBRARIES)
        else()
            set(_eigenschaft INTERFACE_LINK_LIBRARIES)
        endif()
        get_target_property(_kanten "${_ziel}" "${_eigenschaft}")
        if(NOT _kanten)
            continue()
        endif()

        _nakama_kern_eigenschaftseintraege_gruppieren(
            "${_kanten}" _kanteneintraege)
        set(_linkart general)
        foreach(_kante IN LISTS _kanteneintraege)
            if(_kante STREQUAL "debug" OR _kante STREQUAL "optimized" OR _kante STREQUAL "general")
                set(_linkart "${_kante}")
                continue()
            endif()

            string(TOUPPER "${konfiguration}" _konfig_gross)
            set(_aktiv TRUE)
            if(_linkart STREQUAL "debug" AND NOT _konfig_gross STREQUAL "DEBUG")
                set(_aktiv FALSE)
            elseif(_linkart STREQUAL "optimized" AND _konfig_gross STREQUAL "DEBUG")
                set(_aktiv FALSE)
            endif()
            set(_linkart general)
            if(NOT _aktiv)
                continue()
            endif()

            _nakama_kern_wert_auswerten(
                "${_kante}" "${konfiguration}" COMPILE_LINK
                "${_ziel} [${_eigenschaft}]" _ziele)
            foreach(_naechstes IN LISTS _ziele)
                list(APPEND _offen "${_naechstes}")
            endforeach()
        endforeach()
    endwhile()

    set(${ausgabe} "${_gesehen}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_option_defines ausgabe)
    set(_defines "")
    set(_erwartet_wert FALSE)
    foreach(_option IN LISTS ARGN)
        string(REGEX REPLACE "^SHELL:" "" _option "${_option}")
        separate_arguments(_teile NATIVE_COMMAND "${_option}")
        foreach(_teil IN LISTS _teile)
            if(_erwartet_wert)
                list(APPEND _defines "${_teil}")
                set(_erwartet_wert FALSE)
            elseif(_teil MATCHES "^[-/]D$")
                set(_erwartet_wert TRUE)
            elseif(_teil MATCHES "^[-/]D(.+)$")
                list(APPEND _defines "${CMAKE_MATCH_1}")
            endif()
        endforeach()
    endforeach()
    if(_erwartet_wert)
        message(FATAL_ERROR
            "S8/SONDE-007a: leere -D-/D-Option; ihre Definemenge ist unbestimmt.")
    endif()
    set(${ausgabe} "${_defines}" PARENT_SCOPE)
endfunction()

function(_nakama_kern_eigenschaft_defines ziel eigenschaft konfiguration fokus ausgabe)
    _nakama_kern_target_property_lesen(
        "${ziel}" "${eigenschaft}" _roh _eigenschaft_gesetzt)
    set(_defines "")
    set(_optionen "")
    if(_eigenschaft_gesetzt AND _roh)
        if(eigenschaft MATCHES "COMPILE_OPTIONS$")
            set(_art "${fokus}_OPTION")
        else()
            set(_art "${fokus}")
        endif()

        _nakama_kern_eigenschaftseintraege_gruppieren(
            "${_roh}" _eigenschaftseintraege)
        foreach(_eintrag IN LISTS _eigenschaftseintraege)
            _nakama_kern_wert_auswerten(
                "${_eintrag}" "${konfiguration}" "${_art}"
                "${ziel} [${eigenschaft}]" _werte)
            foreach(_wert IN LISTS _werte)
                if(eigenschaft MATCHES "COMPILE_OPTIONS$")
                    list(APPEND _optionen "${_wert}")
                else()
                    string(REGEX REPLACE "^[-/]D" "" _wert "${_wert}")
                    list(APPEND _defines "${_wert}")
                endif()
            endforeach()
        endforeach()
    endif()
    if(_optionen)
        _nakama_kern_option_defines(_aus_option ${_optionen})
        list(APPEND _defines ${_aus_option})
    endif()

    set(_gefiltert "")
    foreach(_define IN LISTS _defines)
        if((fokus STREQUAL "IDENTITAET" AND _define MATCHES "JucePlugin_")
           OR (fokus STREQUAL "JUCE" AND _define MATCHES "^JUCE_"))
            list(APPEND _gefiltert "${_define}")
        endif()
    endforeach()
    set(${ausgabe} "${_gefiltert}" PARENT_SCOPE)
endfunction()

# Compilerwirksam sind die eigenen COMPILE_*-Eigenschaften und die INTERFACE_*
# Usage Requirements jedes transitiv gelinkten Ziels. Private Eigenschaften
# eines gelinkten Ziels wirken dagegen nur auf dessen eigene Uebersetzung.
function(_nakama_kern_wirksame_defines ziel konfiguration fokus ausgabe)
    _nakama_kern_ziel_aufloesen("${ziel}" _wurzel)
    _nakama_kern_nutzungshuelle("${_wurzel}" "${konfiguration}" _huelle)

    set(_alle "")
    foreach(_eigenschaft COMPILE_DEFINITIONS COMPILE_OPTIONS)
        _nakama_kern_eigenschaft_defines(
            "${_wurzel}" "${_eigenschaft}" "${konfiguration}" "${fokus}" _teil)
        list(APPEND _alle ${_teil})
    endforeach()

    foreach(_h IN LISTS _huelle)
        if(_h STREQUAL _wurzel)
            continue()
        endif()
        foreach(_eigenschaft INTERFACE_COMPILE_DEFINITIONS INTERFACE_COMPILE_OPTIONS)
            _nakama_kern_eigenschaft_defines(
                "${_h}" "${_eigenschaft}" "${konfiguration}" "${fokus}" _teil)
            list(APPEND _alle ${_teil})
        endforeach()
    endforeach()

    list(REMOVE_DUPLICATES _alle)
    list(SORT _alle)
    set(${ausgabe} "${_alle}" PARENT_SCOPE)
endfunction()

# ── K2: der Konfigurier-Riegel ──────────────────────────────────────────────
# Faellt, BEVOR eine einzige Uebersetzungseinheit laeuft, sobald die
# compilerwirksamen Usage Requirements des Kerns ein `JucePlugin_` enthalten.
# Der haeufigste Weg dorthin ist eine Linkkante zu einem juce_add_plugin-Ziel —
# dessen Identitaetsdefines sind PUBLIC (JUCEUtils.cmake:1543).
#
# Erfasst werden COMPILE_DEFINITIONS/INTERFACE_COMPILE_DEFINITIONS und
# /D-/D-Eintraege aus COMPILE_OPTIONS/INTERFACE_COMPILE_OPTIONS ueber die
# rekursive, konfigurationsgenau ausgewertete Usage-Requirements-Huelle des
# Kernziels. Die registrierten Pluginziele werden hier nicht als Verbraucher
# hineingerechnet; nur eine echte Rueckkante vom Kern zu einem Pluginziel macht
# dessen PUBLIC-Defines compilerwirksam und damit sichtbar. Ein unbekannter
# Generatorausdruck in einer Define-/Options-Eigenschaft ist ein
# Configure-Fehler, keine Messluecke - seit NAK-84 fail-closed ohne
# Rohtextheuristik.
function(nakama_kern_riegel_pruefen ziel)
    _nakama_kern_konfigurationen(_konfigurationen)
    set(_funde "")
    set(_ziele "")
    foreach(_konfiguration IN LISTS _konfigurationen)
        _nakama_kern_nutzungshuelle("${ziel}" "${_konfiguration}" _huelle)
        list(APPEND _ziele ${_huelle})
        _nakama_kern_wirksame_defines(
            "${ziel}" "${_konfiguration}" IDENTITAET _defs)
        foreach(_d IN LISTS _defs)
            list(APPEND _funde "${_konfiguration}: ${_d}")
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES _ziele)

    list(LENGTH _ziele _anzahl)
    if(_funde)
        string(REPLACE ";" "\n    " _liste "${_funde}")
        message(FATAL_ERROR
            "S8/SONDE-007a K2: Der gemeinsame Kern '${ziel}' sieht JucePlugin_-Konstanten.\n"
            "  ${_liste}\n"
            "Damit traegt sein Objektcode die Identitaet EINES Bundles, und Gen, Probeeq und\n"
            "Suna erben sie gemeinsam — der Identitaets-Golden aus S2 faellt (Entwurf §53.4).\n"
            "Ursache ist fast immer eine Linkkante zu einem juce_add_plugin-Ziel: dessen\n"
            "JucePlugin_*-Defines sind PUBLIC (JUCEUtils.cmake:1543). Identitaet gehoert in\n"
            "die duenne Target-Schicht, nicht in den Kern.")
    endif()

    string(REPLACE ";" ", " _konfig_text "${_konfigurationen}")
    message(STATUS
        "Nakama-Kern: K2 gruen — ${_anzahl} Ziele in der Usage-Requirements-Huelle von '${ziel}', "
        "keine compilerwirksame JucePlugin_-Konstante aus Defines oder -D-/D-Optionen "
        "(${_konfig_text}).")
endfunction()

# ── Die Kopf-Fassade ───────────────────────────────────────────────────────
# Leitet Includes und Defines aus den Modulzielen ab (inklusive deren eigener
# Linkhuelle, damit z. B. juce_atomic_wrapper nicht durchfaellt) und laesst die
# Modulquellen liegen. Nichts hier ist abgeschrieben: aendert JUCE seine
# Moduldefines, wandert die Aenderung von selbst mit.
function(nakama_kern_juce_fassade name)
    add_library(${name} INTERFACE)

    set(_module ${ARGN})
    set(_alle "")
    _nakama_kern_konfigurationen(_konfigurationen)
    foreach(_konfiguration IN LISTS _konfigurationen)
        foreach(_m IN LISTS _module)
            _nakama_kern_huelle("${_m}" "${_konfiguration}" _teil)
            list(APPEND _alle ${_teil})
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES _alle)

    foreach(_m IN LISTS _alle)
        target_include_directories(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_INCLUDE_DIRECTORIES>)
        target_compile_definitions(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_COMPILE_DEFINITIONS>)
        target_compile_options(${name} INTERFACE
            $<TARGET_PROPERTY:${_m},INTERFACE_COMPILE_OPTIONS>)
    endforeach()

    list(LENGTH _alle _anzahl)
    message(STATUS "Nakama-Kern: JUCE-Kopffassade '${name}' aus ${_anzahl} Modulzielen abgeleitet.")
endfunction()

# ── K2b: die JUCE-Projektkonfiguration darf nicht auseinanderlaufen ─────────
# Der Kern uebersetzt DIESELBEN JUCE-Kopfdateien wie seine Verbraucher, aber
# mit seiner eigenen Definemenge. Solange eine JUCE_*-Konfigurationsschraube
# hier anders steht als dort, koennte derselbe Header zwei verschiedene Dinge
# bedeuten - genau die Voraussetzung einer ODR-Verletzung.
#
# ⚠️ BERICHTIGT 24.08.2026 (G1-Nacharbeit). Hier stand seit dem 22.08.:
# "heute weicht genau eine ab (JUCE_USE_CURL; der Kern saehe den Vorgabewert 1
# aus juce_core.h:152 statt der 0 des Projekts), und sie ist folgenlos."
#
# Das ist GEMESSEN FALSCH, und der Fehler ist lehrreich: die Abweichung war nie
# eine Eigenschaft des Baus, sondern eine Eigenschaft des BLICKS. Die
# Kopf-Fassade traegt ihre Defines als Generatorausdruck; zur Konfigurierzeit
# las der alte Riegel davon nur unaufgeloesten Text und sah deshalb KEIN
# JUCE_USE_CURL am Kern. Beim Uebersetzen expandiert derselbe Ausdruck sehr
# wohl - der Kern hatte die 0 also immer. Aus einer Messluecke wurde eine
# Aussage ueber das Produkt, und die stand hier zwei Tage als Befund.
# Seit die Fassade aufgeloest wird, meldet der Riegel beide Seiten literal:
# JUCE_DISPLAY_SPLASH_SCREEN=0, JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1,
# JUCE_USE_CURL=0, JUCE_WEB_BROWSER=0 - vier Defines, beide Seiten gleich.
#
# ⚠️ Und der zweite Teil desselben Befunds (G1 §4.4): der Satz "haelt die
# beiden Mengen zusammen" stand hier, seit es den Riegel gibt — er war eine
# Zusage auf MENGENGLEICHHEIT, und der Code darunter leistete sie nicht. Die
# tragende Schleife lief NUR ueber die Referenz und fragte nur "fehlt es am
# Kern?".
# Damit war unsichtbar:
#
#   * ein Define, das es NUR am Kern gibt        -> derselbe Header, andere
#                                                   Bedeutung, kein Alarm
#   * ein Name mit ZWEI Werten am Kern           -> der erwartete Wert stand ja
#                                                   drin, nur nicht allein
#
# Das ist genau die Befundklasse, fuer die S8 gebaut wurde: ein Riegel, der
# weniger kann, als er behauptet, und dessen Schweigen deshalb nichts beweist.
# Er misst jetzt in BEIDE Richtungen und zusaetzlich auf Wertwiderspruch; die
# Zusage oben stimmt seitdem.
#
# K2b vergleicht DEFINES, einschliesslich der als /D oder -D geschriebenen
# Compile-Optionen. Kern und Referenz werden je als eigenes Wurzelziel mit ihrer
# compilerwirksamen Usage-Requirements-Huelle berechnet; das Pluginziel wird
# nicht in die Kernhuelle gezogen. Seine JucePlugin_*-Defines gehoeren nicht zur
# JUCE_-Vergleichsmenge, koennen aber wegen der gemeinsamen Auswertungsroutine
# vollstaendig und fail-closed gelesen werden. Andere Uebersetzungsschalter
# liegen ausserhalb von K2b - dafuer steht K2c darunter (T2-Befund 23.08.).
#
# Nicht verglichen wird, was legitim verschieden ist:
#   JUCE_MODULE_AVAILABLE_*  - der Kern nutzt weniger Module, das ist sein Sinn
#   JUCE_SHARED_CODE / JUCE_STANDALONE_APPLICATION / JUCE_VST3_CAN_REPLACE_VST2
#                            - exakt diese drei vollstaendigen Makronamen (ohne
#                              Wert oder mit =Wert), nicht gleich beginnende
#                              Zusatzdefines; Sache der Plugin-Huelle. K1 nennt davon nur
#                              JUCE_SHARED_CODE namentlich; die anderen beiden
#                              haelt K2 auf, denn ihre WERTE tragen den Praefix
#                              (JUCE_STANDALONE_APPLICATION=JucePlugin_Build_Standalone,
#                              nachzulesen in der Rohausgabe B3 des Manifests).
#                              Bis 23.08. stand hier "K1 verbietet die ersten
#                              beiden" - das war falsch; der Ausschluss haelt,
#                              aber ein anderer Riegel traegt ihn.
# Sammelt die WIRKSAMEN JUCE-Defines eines Ziels: eigene Definitions- und
# /D-/D-Optionseigenschaften plus die INTERFACE-Eigenschaften der ganzen
# rekursiven Linkhuelle, getrennt je Konfiguration. Beide Seiten des Vergleichs
# werden damit GLEICH gerechnet — das ist die Voraussetzung dafuer, dass ein
# Mengenvergleich in beide Richtungen ueberhaupt aussagekraeftig ist.
#
# ⚠️ Vor dem 24.08.2026 war genau das nicht so: die Referenzseite wurde nur aus
# COMPILE_DEFINITIONS gelesen, die Kernseite zusaetzlich aus der Fassade. Zwei
# verschieden gerechnete Mengen kann man nur in EINE Richtung vergleichen, ohne
# Fehlalarme zu ernten — und genau in eine Richtung wurde verglichen.
function(_nakama_kern_juce_define_ist_ausgenommen define aus)
    set(_ausgenommen FALSE)
    if("${define}" MATCHES "^JUCE_MODULE_AVAILABLE_"
       OR "${define}" STREQUAL "JUCE_SHARED_CODE"
       OR "${define}" MATCHES "^JUCE_SHARED_CODE="
       OR "${define}" STREQUAL "JUCE_STANDALONE_APPLICATION"
       OR "${define}" MATCHES "^JUCE_STANDALONE_APPLICATION="
       OR "${define}" STREQUAL "JUCE_VST3_CAN_REPLACE_VST2"
       OR "${define}" MATCHES "^JUCE_VST3_CAN_REPLACE_VST2=")
        set(_ausgenommen TRUE)
    endif()
    set(${aus} ${_ausgenommen} PARENT_SCOPE)
endfunction()

function(_nakama_kern_juce_defines ziel konfiguration aus)
    _nakama_kern_wirksame_defines("${ziel}" "${konfiguration}" JUCE _alle)
    # Nur JUCE-Konfiguration, und nur das, was legitim verschieden sein DARF,
    # faellt raus (Begruendung im Kopf dieses Abschnitts).
    set(_gefiltert "")
    foreach(_d IN LISTS _alle)
        if(NOT "${_d}" MATCHES "^JUCE_")
            continue()
        endif()
        _nakama_kern_juce_define_ist_ausgenommen("${_d}" _ausgenommen)
        if(_ausgenommen)
            continue()
        endif()
        list(APPEND _gefiltert "${_d}")
    endforeach()
    list(REMOVE_DUPLICATES _gefiltert)
    list(SORT _gefiltert)
    set(${aus} "${_gefiltert}" PARENT_SCOPE)
endfunction()

# Findet Namen, die in EINER Menge mit ZWEI verschiedenen Werten stehen.
#
# ⚠️ Diese Klasse war vor dem 24.08. voellig unsichtbar, und zwar aus einem
# subtilen Grund: der einseitige Test fragte "steht der erwartete Wert in der
# Kernmenge?". Bei `JUCE_USE_CURL=0` UND `JUCE_USE_CURL=1` am Kern lautet die
# Antwort JA — der erwartete Wert steht ja da, nur eben nicht allein. Der
# Uebersetzer sieht dann zwei -D fuer denselben Namen; welcher gewinnt, haengt
# an der Reihenfolge.
function(_nakama_kern_widersprueche menge aus)
    set(_namen "")
    set(_treffer "")
    foreach(_d IN LISTS menge)
        string(REGEX REPLACE "=.*$" "" _n "${_d}")
        if("${_n}" IN_LIST _namen)
            list(APPEND _treffer "${_n}")
        else()
            list(APPEND _namen "${_n}")
        endif()
    endforeach()
    if(_treffer)
        list(REMOVE_DUPLICATES _treffer)
    endif()
    set(${aus} "${_treffer}" PARENT_SCOPE)
endfunction()

function(nakama_kern_konfig_pruefen kern referenz)
    _nakama_kern_konfigurationen(_konfigurationen)
    set(_berichte "")
    foreach(_konfiguration IN LISTS _konfigurationen)
        _nakama_kern_juce_defines("${referenz}" "${_konfiguration}" _ref)
        _nakama_kern_juce_defines("${kern}" "${_konfiguration}" _haben)

        # Richtung 1: hat der Kern alles, was der Verbraucher hat?
        set(_fehlt "")
        foreach(_d IN LISTS _ref)
            if(NOT "${_d}" IN_LIST _haben)
                list(APPEND _fehlt "${_d}")
            endif()
        endforeach()

        # Richtung 2 (G1 §4.4): hat der Kern etwas, was der Verbraucher nicht
        # hat? Ein Define nur am Kern veraendert denselben Header ebenso.
        set(_zuviel "")
        foreach(_d IN LISTS _haben)
            if(NOT "${_d}" IN_LIST _ref)
                list(APPEND _zuviel "${_d}")
            endif()
        endforeach()

        # Richtung 3: derselbe Name zweimal mit verschiedenem Wert.
        _nakama_kern_widersprueche("${_haben}" _wider_kern)
        _nakama_kern_widersprueche("${_ref}" _wider_ref)

        set(_klagen "")
        if(_fehlt)
            string(REPLACE ";" "\n        " _l "${_fehlt}")
            list(APPEND _klagen "  fehlt am Kern (hat der Verbraucher, der Kern nicht):\n        ${_l}")
        endif()
        if(_zuviel)
            string(REPLACE ";" "\n        " _l "${_zuviel}")
            list(APPEND _klagen "  nur am Kern (hat der Kern, der Verbraucher nicht):\n        ${_l}")
        endif()
        if(_wider_kern)
            string(REPLACE ";" "\n        " _l "${_wider_kern}")
            list(APPEND _klagen "  widerspruechlich AM KERN (ein Name, zwei Werte):\n        ${_l}")
        endif()
        if(_wider_ref)
            string(REPLACE ";" "\n        " _l "${_wider_ref}")
            list(APPEND _klagen "  widerspruechlich AM VERBRAUCHER (ein Name, zwei Werte):\n        ${_l}")
        endif()

        if(_klagen)
            string(REPLACE ";" "\n" _text "${_klagen}")
            message(FATAL_ERROR
                "S8/SONDE-007a K2b [${_konfiguration}]: Der Kern '${kern}' uebersetzt die "
                "JUCE-Kopfdateien mit einer anderen Konfiguration als '${referenz}':\n"
                "${_text}\n"
                "Derselbe Header kann dort dann etwas anderes bedeuten als hier. Trag die\n"
                "Schraube an der Kopf-Fassade nach (plugin/CMakeLists.txt, NakamaKernJuce).")
        endif()

        # Der Riegel nennt je Konfiguration, WAS er verglichen hat. Schrumpft
        # die Menge still auf null, wird das in der Ausgabe sichtbar.
        list(LENGTH _ref _n)
        string(REPLACE ";" ", " _gemessen "${_ref}")
        list(APPEND _berichte "${_konfiguration}: ${_n} [${_gemessen}]")
    endforeach()

    string(REPLACE ";" " | " _bericht "${_berichte}")
    message(STATUS
        "Nakama-Kern: K2b gruen — Mengen von '${kern}' und '${referenz}' GLEICH "
        "(beide Richtungen, keine Wertwidersprueche, rekursive Defines plus -D-/D-Optionen); "
        "${_bericht}")
endfunction()

# ── K2c: dieselben JUCE-Empfehlungsschalter wie der Verbraucher ─────────────
# T2-Befund 23.08. K2b vergleicht COMPILE_DEFINITIONS und ist damit blind fuer
# Uebersetzungsschalter. Genau dort riss der Umbau ein Loch: die Helferziele
# juce_recommended_*_flags haengen PUBLIC an den Verbraucherzielen, aber die
# Kopf-Fassade leitet nur aus MODUL-Zielen ab. Bis 23.08. uebersetzte der Kern
# deshalb als einziger Code im Baum unter /W1 statt /W4 - gemessen an den
# erzeugten .vcxproj: 0 Zeilen <WarningLevel> gegen 4x <WarningLevel>Level4 bei
# jedem Verbraucher.
#
# Der Riegel fragt nicht nach einzelnen Schaltern (die sind versionsabhaengig),
# sondern nach ihrer QUELLE: Die volle Linkhuelle des Kernziels und die volle
# Linkhuelle genau eines registrierten Verbrauchers werden getrennt berechnet;
# jedes juce_recommended_*-Ziel der Referenz muss der Kern in derselben
# Konfiguration ebenfalls tragen. Kein Verbraucher wird dadurch zur Quelle des
# Kerns. CONFIG- und LINK_ONLY-Kanten werden ausgewertet; eine unbekannte
# zieltragende Generatorausdruck-Kante ist ROT.
#
# EINE Ausnahme, und sie ist begruendet, nicht bequem:
#   juce_recommended_lto_flags - setzt /GL und verlangt -LTCG am Endlink. Nur
#     EqCopilot linkt es; die Konsolenziele (NullTest, SchemaTest, ...) linken
#     den Kern OHNE -LTCG. /GL-Objekte im Kern wuerden dort auf einen Linker
#     ohne LTCG treffen. Der Kern muss dieses Ziel also NICHT tragen.
function(nakama_kern_schalter_pruefen kern referenz)
    _nakama_kern_konfigurationen(_konfigurationen)
    set(_ausgenommen juce_recommended_lto_flags)
    set(_berichte "")
    foreach(_konfiguration IN LISTS _konfigurationen)
        _nakama_kern_huelle("${kern}" "${_konfiguration}" _kern_huelle)
        _nakama_kern_huelle("${referenz}" "${_konfiguration}" _ref_huelle)

        set(_fehlt "")
        set(_gedeckt "")
        foreach(_t IN LISTS _ref_huelle)
            if(NOT "${_t}" MATCHES "^juce_recommended_")
                continue()
            endif()
            if("${_t}" IN_LIST _ausgenommen)
                continue()
            endif()
            if("${_t}" IN_LIST _kern_huelle)
                list(APPEND _gedeckt "${_t}")
            else()
                list(APPEND _fehlt "${_t}")
            endif()
        endforeach()

        if(_fehlt)
            string(REPLACE ";" "\n    " _liste "${_fehlt}")
            message(FATAL_ERROR
                "S8/SONDE-007a K2c [${_konfiguration}]: Der Kern '${kern}' uebersetzt ohne "
                "Empfehlungsschalter, die '${referenz}' traegt. Fehlend am Kern:\n"
                "    ${_liste}\n"
                "Vor S8 lagen die Kernquellen IN den Verbrauchern und erbten diese Schalter (PUBLIC).\n"
                "Als eigene Lib erbt der Kern sie nicht mehr: die Kopf-Fassade leitet nur aus\n"
                "MODUL-Zielen ab, und die Empfehlungsziele sind keine Module. Trag sie am Kern nach\n"
                "(plugin/CMakeLists.txt, target_link_libraries(NakamaKern PRIVATE …)).")
        endif()

        list(LENGTH _gedeckt _anzahl)
        string(REPLACE ";" ", " _namen "${_gedeckt}")
        list(APPEND _berichte "${_konfiguration}: ${_anzahl} [${_namen}]")
    endforeach()

    string(REPLACE ";" " | " _bericht "${_berichte}")
    message(STATUS
        "Nakama-Kern: K2c gruen — Empfehlungsschalter von '${referenz}' rekursiv auch am Kern: "
        "${_bericht}; ausgenommen: juce_recommended_lto_flags (/GL ohne -LTCG im Verbraucher).")
endfunction()
