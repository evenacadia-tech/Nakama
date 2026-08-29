#include "PipeToken.h"

#include <cstring>
#include <vector>

#if defined(_WIN32)
 #define WIN32_LEAN_AND_MEAN
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
 #include <sddl.h>
#endif

namespace nakama::ipc
{
namespace
{
constexpr std::uint32_t kK[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u,
    0x923f82a4u, 0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u,
    0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u,
    0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
    0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au,
    0x5b9cca4fu, 0x682e6ff3u, 0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
};

inline std::uint32_t rotr (std::uint32_t x, int n) noexcept
{
    return (x >> n) | (x << (32 - n));
}

void block (std::uint32_t h[8], const std::uint8_t* p) noexcept
{
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i)
        w[i] = (static_cast<std::uint32_t> (p[i * 4]) << 24)
             | (static_cast<std::uint32_t> (p[i * 4 + 1]) << 16)
             | (static_cast<std::uint32_t> (p[i * 4 + 2]) << 8)
             |  static_cast<std::uint32_t> (p[i * 4 + 3]);
    for (int i = 16; i < 64; ++i)
    {
        const std::uint32_t s0 = rotr (w[i - 15], 7) ^ rotr (w[i - 15], 18) ^ (w[i - 15] >> 3);
        const std::uint32_t s1 = rotr (w[i - 2], 17) ^ rotr (w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
    std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i)
    {
        const std::uint32_t S1 = rotr (e, 6) ^ rotr (e, 11) ^ rotr (e, 25);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t t1 = hh + S1 + ch + kK[i] + w[i];
        const std::uint32_t S0 = rotr (a, 2) ^ rotr (a, 13) ^ rotr (a, 22);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t t2 = S0 + maj;
        hh = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
}

constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
} // namespace

void sha256 (const std::uint8_t* daten, std::size_t laenge, std::uint8_t digest[32]) noexcept
{
    std::uint32_t h[8] = { 0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                           0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u };
    std::size_t i = 0;
    for (; i + 64 <= laenge; i += 64)
        block (h, daten + i);

    std::uint8_t rest[128] = {};
    const std::size_t restLaenge = laenge - i;
    if (restLaenge > 0)
        std::memcpy (rest, daten + i, restLaenge);
    rest[restLaenge] = 0x80;
    const std::size_t gesamt = (restLaenge + 9 <= 64) ? 64u : 128u;
    const std::uint64_t bits = static_cast<std::uint64_t> (laenge) * 8u;
    for (int b = 0; b < 8; ++b)
        rest[gesamt - 1 - static_cast<std::size_t> (b)] =
            static_cast<std::uint8_t> ((bits >> (8 * b)) & 0xFF);
    block (h, rest);
    if (gesamt == 128)
        block (h, rest + 64);

    for (int j = 0; j < 8; ++j)
    {
        digest[j * 4]     = static_cast<std::uint8_t> ((h[j] >> 24) & 0xFF);
        digest[j * 4 + 1] = static_cast<std::uint8_t> ((h[j] >> 16) & 0xFF);
        digest[j * 4 + 2] = static_cast<std::uint8_t> ((h[j] >> 8) & 0xFF);
        digest[j * 4 + 3] = static_cast<std::uint8_t> (h[j] & 0xFF);
    }
}

std::string base32 (const std::uint8_t* daten, std::size_t laenge)
{
    std::string aus;
    aus.reserve (((laenge + 4) / 5) * 8);
    std::uint32_t puffer = 0;
    int bits = 0;
    for (std::size_t i = 0; i < laenge; ++i)
    {
        puffer = (puffer << 8) | daten[i];
        bits += 8;
        while (bits >= 5)
        {
            bits -= 5;
            aus.push_back (kAlphabet[(puffer >> bits) & 0x1Fu]);
        }
    }
    if (bits > 0)
    {
        // Restbits linksbuendig auffuellen — genau das tut RFC 4648, bevor das
        // Padding drankaeme; das Padding lassen wir weg.
        aus.push_back (kAlphabet[(puffer << (5 - bits)) & 0x1Fu]);
    }
    return aus;
}

std::string pipeToken (const std::string& sid)
{
    std::string eingabe = kPipeDomaene;
    // Die SID wird VOR dem Hashen grossgeschrieben — sonst haetten zwei
    // Schreibweisen derselben SID zwei verschiedene Pipes. SIDs sind reines
    // ASCII (S-R-I-S...), deshalb reicht die ASCII-Regel.
    for (char c : sid)
        eingabe.push_back ((c >= 'a' && c <= 'z') ? static_cast<char> (c - 'a' + 'A') : c);

    std::uint8_t digest[32];
    sha256 (reinterpret_cast<const std::uint8_t*> (eingabe.data()), eingabe.size(), digest);
    return base32 (digest, 16);
}

std::string pipeNameV3 (const std::string& sid)
{
    return std::string (kPipePraefixV3) + pipeToken (sid);
}

std::string aktuelleLogonSid()
{
#if defined(_WIN32)
    HANDLE token = nullptr;
    if (! OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &token))
        return {};

    DWORD noetig = 0;
    GetTokenInformation (token, TokenUser, nullptr, 0, &noetig);
    if (noetig == 0)
    {
        CloseHandle (token);
        return {};
    }
    std::vector<std::uint8_t> puffer (noetig);
    if (! GetTokenInformation (token, TokenUser, puffer.data(), noetig, &noetig))
    {
        CloseHandle (token);
        return {};
    }
    CloseHandle (token);

    auto* nutzer = reinterpret_cast<TOKEN_USER*> (puffer.data());
    LPWSTR breit = nullptr;
    if (! ConvertSidToStringSidW (nutzer->User.Sid, &breit))
        return {};

    std::string sid;
    for (const wchar_t* p = breit; *p != 0; ++p)
        sid.push_back (static_cast<char> (*p & 0x7F)); // SIDs sind reines ASCII
    LocalFree (breit);
    return sid;
#else
    return {};
#endif
}

} // namespace nakama::ipc
