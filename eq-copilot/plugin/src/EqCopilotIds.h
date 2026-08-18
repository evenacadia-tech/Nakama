// Zentrale Konstanten des EQ-Copilot-Plugins. Der IPC-Vertrag selbst lebt in
// eq-copilot/schemas/eq-ipc.schema.json — dieser Header spiegelt nur die Werte,
// die C++ zum Sprechen des Vertrags braucht.
#pragma once

namespace eqcop
{
// v2 (M2): Heartbeat trägt den kompakten Messstand, der Broker quittiert mit
// heartbeat_ack (Konflikt-Flag). Der Broker nimmt v1 UND v2 an; ein alter
// v1-Broker lehnt dieses Plugin mit einem klaren reject ab (Plan §9.2).
inline constexpr int         kProtocolVersion   = 2;
// 0.3.0 (2026-08-16): Hör-Markierung (Einfärben) — Solo/Puls je Befundkarte,
// Lebenszeichen-Renderschutz, additives Heartbeat-Feld `hoermarkierung`.
inline constexpr const char* kPluginVersion     = "0.3.0";

// Fester Name; Zugriff wird broker-seitig per ACL auf den aktuellen Windows-User
// begrenzt (Plan §9.2). Das Session-Token kommt aus dem Welcome des Brokers.
inline constexpr const wchar_t* kPipeName       = L"\\\\.\\pipe\\evenacadia.eq-copilot.v1";

// Harte Paketgrenze aus dem Schema — größer beendet die Verbindung.
inline constexpr unsigned    kMaxFrameBytes     = 262144;

inline constexpr int         kHeartbeatMs       = 1000;
inline constexpr int         kBackoffStartMs    = 500;
inline constexpr int         kBackoffMaxMs      = 8000;
} // namespace eqcop
