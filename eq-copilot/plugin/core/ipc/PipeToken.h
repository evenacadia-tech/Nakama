// SONDE-010 — v3-Pipename als reine Funktion (Entwurf §48.3), C++-Fassung.
//
// base32(first_128_bits(SHA-256("evenacadia.nakama|v3|" + uppercase_sid_utf8)))
// ergibt den Token, daraus wird \\.\pipe\evenacadia.nakama.v3.<token>.
// RFC-4648-Base32, grossgeschrieben, ohne Padding.
//
// Der Token verbirgt die rohe Windows-User-SID im Pipenamen. Er ist KEIN
// Geheimnis und ersetzt keine DACL — die Zugriffsentscheidung trifft weiter
// die Sicherheitsbeschreibung der Pipe (§48.4).
//
// Dieselbe Funktion und dasselbe Golden stehen in
// broker/src/transport/pipetoken.rs. Das Golden bindet drei Dinge fest, die
// man einzeln nicht sieht: die UTF-8-Bytefolge des Praefix, die
// Digestreihenfolge (erste 128 Bit = erste 16 Bytes) und das Alphabet.
//
// NICHT TEIL VON NakamaKern (S14-15/SONDE-010, 29.08.2026): der Namensraum
// `evenacadia.nakama|v3|` traegt den eingefrorenen HERSTELLERNAMEN, und A14
// hat ihn beim ersten Lauf im gebauten Kern gefunden. Statt den Riegel
// aufzuweichen ist die Datei aus dem geteilten Boden genommen: WELCHE Pipe
// eine Instanz waehlt, ist dieselbe Art Aussage wie WELCHE Identitaet sie
// traegt - beides gehoert in die duenne Zielschicht. Der Kern (ControlClient,
// TelemetryClient) bekommt den Pipenamen deshalb als Zeichenkette und rechnet
// ihn nie selbst aus.
//
// SHA-256 steht hier von Hand, statt juce_cryptography anzuhaengen:
// der Kern ist bewusst JUCE-frei, und ein neues JUCE-Modul im geteilten Boden
// waere ein groesserer Eingriff als 60 Zeilen mit vier Golden-Vektoren.
#pragma once

#include <cstdint>
#include <string>

namespace nakama::ipc
{

/// Praefix des Digests. Aendert er sich, aendern sich ALLE Pipenamen — das ist
/// eine Protokollaenderung, keine Kosmetik.
inline constexpr const char* kPipeDomaene = "evenacadia.nakama|v3|";

/// Namensraum der v3-Pipes.
inline constexpr const char* kPipePraefixV3 = "\\\\.\\pipe\\evenacadia.nakama.v3.";

/// Namensraum der PROBE-Pipes. Alles darunter gehoert Tests und
/// Beweislaeufen; ein produktiver v3-Name kann nie darin liegen, weil ein
/// SID-Token ein 26-stelliges Base32-Wort ist und nie mit `probe.` beginnt.
inline constexpr const char* kPipePraefixProbe =
    "\\\\.\\pipe\\evenacadia.nakama.v3.probe.";

/// Darf ein Probeprogramm diesen Pipenamen oeffnen?
///
/// Die Frage ist bewusst als ERLAUBNIS gestellt und nicht als Sperrliste: eine
/// Sperrliste kennt nur die Namen, an die jemand gedacht hat — sie liess den
/// produktiven v3-Namensraum aus §48.3 durch (T2-Befund 7 vom 2026-08-29).
///
/// Sie steht `inline` im Header, weil das Lastbein sie braucht, ohne die
/// SID-Maschinerie aus `PipeToken.cpp` mitzunehmen: es ist eine reine Aussage
/// ueber eine Zeichenkette.
inline bool istProbePipename (const std::string& name) noexcept
{
    const std::string praefix (kPipePraefixProbe);
    // Der Rest hinter dem Praefix darf nicht leer sein, damit `probe.` allein
    // keine Sammelpipe wird.
    return name.size() > praefix.size() && name.compare (0, praefix.size(), praefix) == 0;
}

/// SHA-256 ueber beliebige Bytes; schreibt 32 Bytes nach `digest`.
void sha256 (const std::uint8_t* daten, std::size_t laenge, std::uint8_t digest[32]) noexcept;

/// RFC-4648-Base32, Grossbuchstaben, ohne Padding.
std::string base32 (const std::uint8_t* daten, std::size_t laenge);

/// Pipe-Token zu einer Logon-SID.
std::string pipeToken (const std::string& sid);

/// Vollstaendiger v3-Pipename zu einer Logon-SID.
std::string pipeNameV3 (const std::string& sid);

/// Logon-SID des aktuellen Prozesstokens; leer, wenn sie nicht ermittelbar ist.
/// Der Aufrufer entscheidet, was ein leerer Wert bedeutet — geraten wird nie.
std::string aktuelleLogonSid();

} // namespace nakama::ipc
