// Reiner OS-Grenzadapter fuer die v2-Pipe im Audit. Der originale PipeClient
// wird unveraendert eingebunden. Gen startet v2 auch im vorhandenen NO_PRODUCT_V3-
// Testbau; deshalb werden genau seine beiden Win32-Verbindungsimporte hier
// auf einen PID-eigenen Probe-Namen umgeleitet. Keine Produktdatei wird editiert.
// Dies ist kein Nachweis des produktiven v2-Namens/Verbindungsaufbaus.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>

namespace audit16 {
const std::wstring& probeName() {
    static const auto name = std::wstring(L"\\\\.\\pipe\\evenacadia.eq-copilot.probe.audit16.v2.")
        + std::to_wstring(GetCurrentProcessId());
    return name;
}
HANDLE WINAPI createFile(LPCWSTR, DWORD access, DWORD share,
    LPSECURITY_ATTRIBUTES security, DWORD disposition, DWORD flags, HANDLE templ) {
    return ::CreateFileW(probeName().c_str(), access, share, security, disposition, flags, templ);
}
BOOL WINAPI waitPipe(LPCWSTR, DWORD timeout) {
    return ::WaitNamedPipeW(probeName().c_str(), timeout);
}
}
#define CreateFileW audit16::createFile
#define WaitNamedPipeW audit16::waitPipe
#include "PipeClient.cpp"
#undef WaitNamedPipeW
#undef CreateFileW
