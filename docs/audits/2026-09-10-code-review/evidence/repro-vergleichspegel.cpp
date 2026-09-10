#include "Vergleichspegel.h"
#include <array>
#include <iostream>

int main() {
    std::array<float, 19200> a{}, b{};
    a.fill(0.25f);
    b.fill(0.125f);
    int violations = 0;
    for (int channels : {1, 2}) {
        for (int milliseconds : {199, 200, 399, 400}) {
            nakama::analyse::Vergleichspegel meter;
            meter.vorbereiten(48000.0);
            const int frames = milliseconds * 48;
            // Same per-channel call topology as PluginProcessor.cpp:777-783.
            // The meter implementation is the original production header.
            for (int channel = 0; channel < channels; ++channel)
                meter.speise(a.data(), b.data(), frames);
            const bool ready = meter.bereit();
            const bool frozen = meter.friereEin();
            const bool expected = milliseconds >= 400;
            const bool violation = ready != expected || frozen != expected;
            violations += violation;
            std::cout << "channels=" << channels
                      << " elapsed_ms=" << milliseconds
                      << " counted_samples=" << meter.gezaehlteSamples()
                      << " ready=" << ready << " frozen=" << frozen
                      << " expected=" << expected
                      << " violation=" << violation << '\n';
        }
    }
    std::cout << "violations=" << violations << '\n';
    return violations == 0 ? 0 : 1;
}
