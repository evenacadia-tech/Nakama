# B9 unabhaengig: ungewichteter Energieabgleich (Vergleichspegel.h:230-232, Summe x^2 ueber Kanaele)
# gegen K-gewichtete Lautheit (BS.1770-5 Tab. 1/2, 48 kHz).
# Teil A: Frequenzbereichsrechnung (Leistungsintegral), Teil B: Zeitbereichs-Gegenprobe mit Rauschen,
# Teil C: was das Produkt heute tatsaechlich misst: trocken gegen Hoermarkierung (HoerMarkierung.h).
import numpy as np
from scipy import signal

fs = 48000.0
ITU_S = (np.array([1.53512485958697, -2.69169618940638, 1.19839281085285]), np.array([1.0, -1.69065929318241, 0.73248077421585]))
ITU_H = (np.array([1.0, -2.0, 1.0]), np.array([1.0, -1.99004745483398, 0.99007225036621]))

def mag2(b, a, f):
    _, h = signal.freqz(b, a, worN=f, fs=fs)
    return np.abs(h)**2

def rbj_lowshelf(f0, gdb, S=1.0):
    A = 10**(gdb/40); w0 = 2*np.pi*f0/fs; c = np.cos(w0); s = np.sin(w0)
    al = s/2*np.sqrt((A + 1/A)*(1/S - 1) + 2)
    b = np.array([A*((A+1)-(A-1)*c+2*np.sqrt(A)*al), 2*A*((A-1)-(A+1)*c), A*((A+1)-(A-1)*c-2*np.sqrt(A)*al)])
    a = np.array([(A+1)+(A-1)*c+2*np.sqrt(A)*al, -2*((A-1)+(A+1)*c), (A+1)+(A-1)*c-2*np.sqrt(A)*al])
    return b/a[0], a/a[0]

def rbj_peak_oct(f0, bw, gdb):     # HoerMarkierung.h:62-92
    A = 10**(gdb/40); w0 = 2*np.pi*f0/fs; s = np.sin(w0); al = s*np.sinh(0.5*np.log(2)*bw*w0/s); c = np.cos(w0)
    a0 = 1 + al/A
    return np.array([1+al*A, -2*c, 1-al*A])/a0, np.array([1.0, -2*c/a0, (1-al/A)/a0])

def rbj_hp(fc, q):
    w0 = 2*np.pi*fc/fs; al = np.sin(w0)/(2*q); c = np.cos(w0); a0 = 1+al
    return np.array([(1+c)/2, -(1+c), (1+c)/2])/a0, np.array([1.0, -2*c/a0, (1-al)/a0])

def rbj_lp(fc, q):
    w0 = 2*np.pi*fc/fs; al = np.sin(w0)/(2*q); c = np.cos(w0); a0 = 1+al
    return np.array([(1-c)/2, (1-c), (1-c)/2])/a0, np.array([1.0, -2*c/a0, (1-al)/a0])

f = np.geomspace(20, 20000, 40000); w = np.gradient(f)
K = mag2(*ITU_S, f)*mag2(*ITU_H, f)

def delta(psd, eq2):
    du = 10*np.log10(np.sum(psd*eq2*w)/np.sum(psd*w))           # was der Vergleichspegel sieht
    dk = 10*np.log10(np.sum(psd*eq2*K*w)/np.sum(psd*K*w))       # Lautheitsaenderung (LUFS)
    return du, dk

spektren = {"rosa (-3 dB/Okt)": -3.0103, "-4,5 dB/Okt": -4.5, "braun (-6 dB/Okt)": -6.0206}
eqs = {
    "Brickwall -6 dB <120 Hz (Vorpruefer)": np.where(f < 120, 10**(-0.6), 1.0),
    "RBJ-Low-Shelf -6 dB @120 Hz, S=1":     mag2(*rbj_lowshelf(120, -6.0), f),
    "RBJ-Low-Shelf +4 dB @80 Hz, S=1":      mag2(*rbj_lowshelf(80, 4.0), f),
    "Glocke -4 dB @3 kHz, 1 Okt":           mag2(*rbj_peak_oct(3000, 1.0, -4.0), f),
}
print("A) Frequenzbereich. du = Aenderung ungewichtete Energie (= -match_gain), dk = Aenderung LUFS;")
print("   nach Energieabgleich liegt der Kandidat um (dk - du) LU neben der Baseline.")
for sn, sl in spektren.items():
    psd = (f/1000.0)**(sl/(10*np.log10(2)))
    for en, e2 in eqs.items():
        du, dk = delta(psd, e2)
        print("   %-18s %-38s du %+6.2f dB  dk %+6.2f LU  -> Kandidat nach Abgleich %+5.2f LU" % (sn, en, du, dk, dk-du))

# B) Zeitbereich: -4,5 dB/Okt-Rauschen, 20 s, Low-Shelf -6 dB @120 Hz
rng = np.random.default_rng(11)
n = int(fs*20)
X = np.fft.rfft(rng.standard_normal(n)); fr = np.fft.rfftfreq(n, 1/fs); fr[0] = fr[1]
x = np.fft.irfft(X*(fr/1000.0)**(-4.5/(20*np.log10(2)))*((fr >= 20) & (fr <= 20000)), n)
x *= 0.1/np.sqrt(np.mean(x**2))
y = signal.lfilter(*rbj_lowshelf(120, -6.0), x)
kw = lambda s: signal.lfilter(*ITU_H, signal.lfilter(*ITU_S, s))
gain_vp = 10*np.log10(np.sum(y**2)/np.sum(x**2))        # Vergleichspegel.h:230-232 (B relativ zu A)
dl = 10*np.log10(np.mean(kw(y)[4800:]**2)/np.mean(kw(x)[4800:]**2))
print("\nB) Zeitbereich -4,5 dB/Okt, Shelf -6 dB @120 Hz: Vergleichspegel gainDb %+.2f dB, LUFS-Differenz %+.2f LU, Fehlabgleich %+.2f LU"
      % (gain_vp, dl, dl-gain_vp))

# C) Was heute gemessen wird: trocken gegen gefaerbt (HoerMarkierung.h:188-244), -4,5 dB/Okt
psd = (f/1000.0)**(-4.5/(10*np.log10(2)))
q1, q2 = 0.54119610, 1.30656296
def solo_zone(von, bis):
    return mag2(*rbj_hp(von, q1), f)*mag2(*rbj_hp(von, q2), f)*mag2(*rbj_lp(bis, q1), f)*mag2(*rbj_lp(bis, q2), f)
print("\nC) Produktpfad heute: match_gain = 10log10(E_nass/E_trocken) mit Hoermarkierung; ohne Markierung exakt 0 dB")
for nm, e2 in [("Solo Zone 120-300 Hz", solo_zone(120, 300)), ("Solo Zone 2-5 kHz", solo_zone(2000, 5000)),
               ("Puls voll +4,5 dB, 120-300 Hz", mag2(*rbj_peak_oct(np.sqrt(120*300), np.log2(300/120), 4.5), f)),
               ("Puls voll +4,5 dB, 2-5 kHz", mag2(*rbj_peak_oct(np.sqrt(2000*5000), np.log2(5000/2000), 4.5), f))]:
    du, dk = delta(psd, e2)
    print("   %-30s gainDb %+6.2f dB, Lautheitsdifferenz %+6.2f LU, Abweichung %+5.2f LU%s" % (nm, du, dk, dk-du,
          "  (|gain| > 6 dB: Deckel aus §41.2 wuerde greifen)" if abs(du) > 6 else ""))
