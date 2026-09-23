"""Sichtpruefung zu Rechnung 5: Ausgaenge um den Wechsel (Szenario A und A', Sinus 1 kHz)."""
import numpy as np, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import nk
from r5_zeitvarianz import FS, N0, L, verlauf_df2t, verlauf_svf_gk, verlauf_svf_produkt, run_df2t, run_svf

fig, axs = plt.subplots(2, 1, figsize=(15, 8))
for ax, (alt, neu, titel) in zip(axs, [((1000.0, 6.0, 12.0), (3000.0, 6.0, -12.0), "A: 1k +12 Q6 -> 3k -12 Q6"),
                                       ((3000.0, 6.0, -12.0), (1000.0, 6.0, 12.0), "A': 3k -12 Q6 -> 1k +12 Q6")]):
    w = 2 * np.pi * 1000.0 / FS
    x = 0.5 * np.sin(w * np.arange(L))
    ca, cn = nk.rbj_bell(FS, *alt), nk.rbj_bell(FS, *neu)
    sa = nk.svf_bell(nk.svf_grundG(alt[0], FS), alt[1], alt[2]); sn = nk.svf_bell(nk.svf_grundG(neu[0], FS), neu[1], neu[2])
    ya, yn = nk.df2t_run(*ca, x), nk.df2t_run(*cn, x)
    t = np.zeros(L); t[N0:N0 + 256] = np.arange(256) / 256; t[N0 + 256:] = 1
    kalt = ya.copy(); nk_ = np.zeros(L); nk_[N0:] = nk.df2t_run(*cn, x[N0:])
    kalt[N0:] = (1 - t[N0:]) * ya[N0:] + t[N0:] * nk_[N0:]
    ideal = (1 - t) * ya + t * yn
    n = np.arange(L) - N0
    sl = slice(N0 - 200, N0 + 1200)
    ax.plot(n[sl], ideal[sl], "k", lw=3, alpha=.3, label="R1 ideal (Xfade eingeschwungen)")
    ax.plot(n[sl], kalt[sl], label="Nakama-Crossfade, neu kalt")
    ax.plot(n[sl], run_df2t(verlauf_df2t(ca, cn, 1), x)[sl], label="DF2T Sprung")
    ax.plot(n[sl], run_svf(verlauf_svf_gk(sa, sn, 1), x)[sl], label="SVF Sprung")
    ax.plot(n[sl], run_df2t(verlauf_df2t(ca, cn, 256), x)[sl], label="DF2T linear 256")
    ax.plot(n[sl], run_svf(verlauf_svf_produkt((nk.svf_grundG(alt[0], FS), alt[1], alt[2]), (nk.svf_grundG(neu[0], FS), neu[1], neu[2]), 256, 0), x)[sl], label="SVF Nakama-Steuerrate 256")
    ax.set_title(titel + ", Sinus 1 kHz -6 dBFS, fs 48 kHz (x: Samples ab Wechsel)"); ax.grid(True); ax.legend(fontsize=7, ncol=3)
fig.tight_layout(); fig.savefig("r5_zeitvarianz.png", dpi=80)
