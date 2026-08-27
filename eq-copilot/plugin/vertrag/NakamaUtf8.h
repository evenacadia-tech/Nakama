#pragma once

#include <cstddef>
#include <cstdint>

namespace nakama::utf8
{
/** Strikter RFC-3629-Riegel fuer eine Bytefolge ohne terminatorbasierte
    Abkuerzung. Prueft Fortsetzungsbytes, Overlong-Sequenzen, Surrogate und
    Werte oberhalb U+10FFFF. Rohes NUL/BOM sind Protokollregeln der Aufrufer. */
inline bool istGueltig (const void* daten, size_t laenge) noexcept
{
    if (daten == nullptr)
        return false;
    const auto* p = static_cast<const std::uint8_t*> (daten);
    size_t i = 0;
    const auto fortsetzung = [] (std::uint8_t b) { return b >= 0x80 && b <= 0xbf; };
    while (i < laenge)
    {
        const auto b0 = p[i++];
        if (b0 <= 0x7f)
            continue;
        if (b0 >= 0xc2 && b0 <= 0xdf)
        {
            if (i >= laenge || ! fortsetzung (p[i++])) return false;
            continue;
        }
        if (b0 == 0xe0)
        {
            if (i + 1 >= laenge || p[i] < 0xa0 || p[i] > 0xbf
                || ! fortsetzung (p[i + 1])) return false;
            i += 2;
            continue;
        }
        if ((b0 >= 0xe1 && b0 <= 0xec) || (b0 >= 0xee && b0 <= 0xef))
        {
            if (i + 1 >= laenge || ! fortsetzung (p[i]) || ! fortsetzung (p[i + 1]))
                return false;
            i += 2;
            continue;
        }
        if (b0 == 0xed)
        {
            if (i + 1 >= laenge || p[i] < 0x80 || p[i] > 0x9f
                || ! fortsetzung (p[i + 1])) return false;
            i += 2;
            continue;
        }
        if (b0 == 0xf0)
        {
            if (i + 2 >= laenge || p[i] < 0x90 || p[i] > 0xbf
                || ! fortsetzung (p[i + 1]) || ! fortsetzung (p[i + 2])) return false;
            i += 3;
            continue;
        }
        if (b0 >= 0xf1 && b0 <= 0xf3)
        {
            if (i + 2 >= laenge || ! fortsetzung (p[i])
                || ! fortsetzung (p[i + 1]) || ! fortsetzung (p[i + 2])) return false;
            i += 3;
            continue;
        }
        if (b0 == 0xf4)
        {
            if (i + 2 >= laenge || p[i] < 0x80 || p[i] > 0x8f
                || ! fortsetzung (p[i + 1]) || ! fortsetzung (p[i + 2])) return false;
            i += 3;
            continue;
        }
        return false;
    }
    return true;
}
} // namespace nakama::utf8
