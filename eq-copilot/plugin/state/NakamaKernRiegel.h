/*  S8 / SONDE-007a - Kompilier-Riegel des gemeinsamen Kerns.

    ABSICHTLICH OHNE INCLUDE-GUARD: Jede Kern-Uebersetzungseinheit bindet
    diesen reinen Praeprozessor-Riegel zuerst und als letzte Zeile erneut ein.
    Die zweite Auswertung sieht damit auch JucePlugin_*-Makros, die erst ein
    eigener oder generierter, danach eingebundener Header definiert und bis
    zum TU-Ende definiert laesst. Ein `#pragma once` wuerde genau diese
    Endpruefung still ausschalten. Ein Makro, das vor dem TU-Ende wieder
    entfernt wird, sieht dieser Riegel dagegen NICHT: dafuer K1b (Quelltext-
    Token) und, sofern Bytes entstehen, K3 (gebautes Artefakt).

    WOGEGEN DAS SCHUETZT (Entwurf §53.4, Static-Lib-Randbedingung):

      "Der gemeinsame Kern sieht KEINE JucePlugin_*-Konstanten; Identitaet
       kommt ausschliesslich aus plugin-identities-v1.json ueber die duennen
       Target-Schichten. Andernfalls erhalten zwei der drei Bundles die
       Identitaetskonstanten des dritten - genau die Werte, die §44.1
       einfriert."

    Das ist kein theoretisches Risiko, sondern eine Eigenschaft von JUCEs
    CMake: `juce_add_plugin` haengt die Identitaet als PUBLIC-Defines an das
    Shared-Code-Ziel (gemessen 22.08.2026 in
    _deps/juce-src/extras/Build/CMake/JUCEUtils.cmake:1543 -
    `target_compile_definitions(${target} PUBLIC JucePlugin_ManufacturerCode=...
    JucePlugin_PluginCode=... JucePlugin_Name=...)`). PUBLIC heisst: wer dieses
    Ziel linkt, erbt die Identitaet EINES Bundles. Ein Kern, der so uebersetzt
    wuerde, traegt "Eqcp" fuer alle drei Apps in seinem Objektcode - und der
    Identitaets-Golden aus S2 faellt, sobald Probeeq oder Suna gebaut werden.

    ACHT RIEGEL, ARBEITSTEILUNG (Manifest docs/beweise/SONDE-007a.md):

      K1 - dieser hier. Namentlich, im Uebersetzer, VOR dem Linken; ausgewertet
           am Anfang und Ende jeder Kern-Uebersetzungseinheit. Schnell und mit
           klarer Fehlermeldung, aber nur fuer die Makros, die unten stehen:
           der Praeprozessor kann nicht auf ein Praefix pruefen - und er sieht
           NICHT, was vor dem TU-Ende wieder entfernt wurde. Dafuer K1b.
      K1b - der Quelltext-Riegel in tools/eq-copilot/pruefe_kern_identitaetsfrei.py
           (S8/SONDE-007a Runde 5, 29.08.2026). Er scannt die TATSAECHLICHEN
           Compiler-Eingaben unter plugin/ - alle Dateien aus dem frisch
           geschriebenen CL.read.1.tlog, also auch erzwungene Includes und
           vorkompilierte Koepfe - plus die literale Include-Huelle als
           Gegenprobe. Verboten ist das Token JucePlugin_ im Quelltext,
           unabhaengig von #define/#undef; Kommentare werden vorher entfernt,
           Stringliterale nicht. Einzige Ausnahme: diese Datei hier, gemessen
           und namentlich - seit Runde 15 kein Ueberspringen mehr, sondern ein
           Abgleich gegen die Makroliste, die K1 unten fuehrt.
           NACHTRAG RUNDE 7 (29.08.2026), er korrigiert das "unter plugin/"
           im Satz weiter oben: geprueft wird die KOMPLEMENTMENGE des
           Leseprotokolls, nicht ein Verzeichnis - JEDE gelesene Datei AUSSER
           denen aus den JUCE-Modulen, der abgeleiteten MSVC-Toolchain und dem
           abgeleiteten Windows-SDK. Das ist plugin/ UND alles Uebrige; genau
           ein per /FI erzwungener Kopf ausserhalb plugin/ war der Weg, den
           Runde 7 geschlossen hat. Die namentlich erlaubten Systemdateien
           unter %SystemRoot% werden dabei nicht als C++ geparst, sondern ROH
           in ASCII und UTF-16LE durchsucht; laesst sich eine der drei
           Ausschlusswurzeln nicht ableiten, bildet K1b gar keine Menge,
           sondern klagt (fail-closed). Nicht gesehen wird nur der Inhalt
           dieser drei Wurzeln - dafuer der Tlog-Riegel und der
           JUCE-Baum-Riegel.
      Tlog-Riegel - im selben Skript: aus welchen Orten der Compiler wirklich
           gelesen hat. Erlaubt sind plugin/, juce-src/modules/ OHNE
           juce_audio_plugin_client (dort liegen alle `#define JucePlugin_` der
           JUCE-Module) und die aus dem Bau abgeleiteten MSVC-/SDK-Wurzeln;
           alles andere ist ROT. Er sieht den INHALT der gelesenen Dateien
           nicht.
      JUCE-Baum-Riegel - im selben Skript: juce-src ist der gepinnte Tag plus
           genau der eine Nakama-VST3-Patch. Damit ist auch eine manipulierte
           Kopie eines JUCE-Modulheaders abgedeckt, die definiert, benutzt und
           wieder entfernt. Er sieht Loeschungen ausserhalb modules/ als
           benannte Duldung und die Toolchain-/SDK-Header ausserhalb des Repos
           gar nicht (ausdrueckliche Nichtzusage).
      K2 - der Konfigurier-Riegel in cmake/NakamaKern.cmake. Laeuft die
           rekursive Linkhuelle je Konfiguration ab und faellt auf jedes
           compilerwirksame `JucePlugin_` aus Definitions- oder /D-/D-
           Optionseigenschaften; unbekannte relevante Generatorausdruecke
           sind ROT statt unsichtbar.
      K2b - haelt die wirksamen JUCE-Konfigurationsdefines von Kern und jedem
           Verbraucher je Konfiguration als echte Mengengleichheit zusammen.
      K2c - haelt die rekursive Quelle der JUCE-Empfehlungsschalter je
           Konfiguration zusammen.
      K3 - dasselbe Skript misst das GEBAUTE NakamaKern.lib gegen Text-,
           Viercode-Integer- und CID-Bytes der eingefrorenen Identitaetswerte.
           Erst das ist eine Aussage ueber das Artefakt statt die
           Baubeschreibung. Seit Runde 5 laesst es die Lib dafuer im selben
           Lauf vollstaendig neu erzeugen, statt die Frische einer vorhandenen
           nachzurechnen.

    Der Riegel gilt nur beim Uebersetzen der Kern-Uebersetzungseinheiten
    (NAKAMA_KERN_UEBERSETZUNG, gesetzt von cmake/NakamaKern.cmake). Dieselben
    Kopfdateien werden von den Plugin-Zielen mitgelesen, und DORT sind die
    Konstanten voellig in Ordnung - sie sind ja der Zweck der Target-Schicht.
*/

#if defined (NAKAMA_KERN_UEBERSETZUNG)

 #if defined (JucePlugin_Name)                       \
  || defined (JucePlugin_Desc)                       \
  || defined (JucePlugin_Manufacturer)               \
  || defined (JucePlugin_ManufacturerWebsite)        \
  || defined (JucePlugin_ManufacturerEmail)          \
  || defined (JucePlugin_ManufacturerCode)           \
  || defined (JucePlugin_PluginCode)                 \
  || defined (JucePlugin_IsSynth)                    \
  || defined (JucePlugin_IsMidiEffect)               \
  || defined (JucePlugin_WantsMidiInput)             \
  || defined (JucePlugin_ProducesMidiOutput)         \
  || defined (JucePlugin_EditorRequiresKeyboardFocus)\
  || defined (JucePlugin_Version)                    \
  || defined (JucePlugin_VersionString)              \
  || defined (JucePlugin_VersionCode)                \
  || defined (JucePlugin_VSTUniqueID)                \
  || defined (JucePlugin_VSTCategory)                \
  || defined (JucePlugin_VSTNumMidiInputs)           \
  || defined (JucePlugin_VSTNumMidiOutputs)          \
  || defined (JucePlugin_Vst3Category)               \
  || defined (JucePlugin_AUMainType)                 \
  || defined (JucePlugin_AUSubType)                  \
  || defined (JucePlugin_AUExportPrefix)             \
  || defined (JucePlugin_AUExportPrefixQuoted)       \
  || defined (JucePlugin_AUManufacturerCode)         \
  || defined (JucePlugin_CFBundleIdentifier)         \
  || defined (JucePlugin_AAXIdentifier)              \
  || defined (JucePlugin_AAXManufacturerCode)        \
  || defined (JucePlugin_AAXProductId)               \
  || defined (JucePlugin_AAXCategory)                \
  || defined (JucePlugin_AAXDisableBypass)           \
  || defined (JucePlugin_AAXDisableMultiMono)        \
  || defined (JucePlugin_Enable_ARA)                 \
  || defined (JucePlugin_ARAFactoryID)               \
  || defined (JucePlugin_ARADocumentArchiveID)       \
  || defined (JucePlugin_ARACompatibleArchiveIDs)    \
  || defined (JucePlugin_ARAContentTypes)            \
  || defined (JucePlugin_ARATransformationFlags)     \
  || defined (JucePlugin_Build_VST)                  \
  || defined (JucePlugin_Build_VST3)                 \
  || defined (JucePlugin_Build_AU)                   \
  || defined (JucePlugin_Build_AUv3)                 \
  || defined (JucePlugin_Build_AAX)                  \
  || defined (JucePlugin_Build_Standalone)           \
  || defined (JucePlugin_Build_Unity)                \
  || defined (JucePlugin_Build_LV2)                  \
  || defined (JUCE_SHARED_CODE)
  #error "S8/SONDE-007a: Der gemeinsame Kern sieht eine JucePlugin_*-Konstante. \
Damit traegt sein Objektcode die Identitaet EINES Bundles, und alle drei Apps \
(Gen, Probeeq, Suna) erben sie - der Identitaets-Golden aus S2 faellt. Ursache \
ist fast immer eine Linkkante von NakamaKern zu einem juce_add_plugin-Ziel: \
dessen JucePlugin_*-Defines sind PUBLIC (JUCEUtils.cmake:1543). Identitaet \
gehoert in die duenne Target-Schicht, nicht in den Kern (Entwurf §53.4)."
 #endif

#endif // NAKAMA_KERN_UEBERSETZUNG
