/* Bausteine — Verhalten. Ohne Abhängigkeiten, läuft per Doppelklick (file://).
   Alle Werte sind lokaler Blattzustand (Demo), kein Plugin-Datenpfad. */
"use strict";

const FEHLER = [];
window.addEventListener("error", e => { FEHLER.push(e.message); standband(); });

const $  = (s, w) => (w || document).querySelector(s);
const $$ = (s, w) => [...(w || document).querySelectorAll(s)];

const SWEEP = 270, START = -135;          // Drehweg aller Knobs
const svgNS = "http://www.w3.org/2000/svg";

function klemme(v, a, b) { return Math.min(b, Math.max(a, v)); }

/* ---------- Wertdarstellung ---------- */
function wertText(el) {
  if (document.body.dataset.zustand === "deaktiviert") return "—";
  const w = +el.dataset.wert;
  if (el.dataset.skala) {
    const worte = el.dataset.skala.split(",");
    return worte[Math.round(w / 100 * (worte.length - 1))].trim();
  }
  if (el.dataset.einheit === "dB") {
    const db = (w - 50) / 50 * 12;
    return (db >= 0 ? "+" : "−") + Math.abs(db).toFixed(1) + " dB";
  }
  if (el.dataset.max) return Math.round(w / 100 * +el.dataset.max) + " " + (el.dataset.einheit || "");
  return Math.round(w) + (el.dataset.einheit ? " " + el.dataset.einheit : "");
}

/* =========================================================
   Drehregler
   ========================================================= */
function bogenPfad(cx, cy, r, von, bis) {
  const a = (g) => [cx + r * Math.sin(g * Math.PI / 180), cy - r * Math.cos(g * Math.PI / 180)];
  const [x1, y1] = a(von), [x2, y2] = a(bis);
  const gross = bis - von > 180 ? 1 : 0;
  return `M ${x1.toFixed(2)} ${y1.toFixed(2)} A ${r} ${r} 0 ${gross} 1 ${x2.toFixed(2)} ${y2.toFixed(2)}`;
}

function bauKnob(k) {
  const kd = parseFloat(getComputedStyle(k).getPropertyValue("--kd"));
  const orbit  = k.classList.contains("knob--orbit");
  const zeiger = k.classList.contains("knob--pointer");
  const skala  = k.classList.contains("knob--skala");

  const rPunkte = kd / 2 + 12;
  const rBogen  = zeiger ? kd / 2 : orbit ? kd * 0.62 : kd / 2 + 22;
  const pad = Math.ceil(Math.max(rPunkte, rBogen) - kd / 2 + (skala ? 26 : 8));
  const S = kd + pad * 2, M = S / 2;

  const buehne = document.createElement("div");
  buehne.className = "k-buehne";
  buehne.style.width = buehne.style.height = S + "px";

  const svg = document.createElementNS(svgNS, "svg");
  svg.setAttribute("viewBox", `0 0 ${S} ${S}`);
  svg.setAttribute("width", S); svg.setAttribute("height", S);

  // Punktkranz — nur Hero/Orbit; der Pedal-Knopf hat in der Vorlage keinen.
  // Dichte kommt aus der Asset-Werkstatt (Grob/Normal/Fein).
  if (!skala && !zeiger) {
    const dichteFaktor = { grob: 0.6, normal: 1, fein: 1.6 }[document.documentElement.dataset.ticks || "normal"];
    const n = Math.round((kd > 120 ? 40 : kd > 90 ? 28 : 18) * dichteFaktor);
    for (let i = 0; i <= n; i++) {
      const g = START + i * SWEEP / n;
      const c = document.createElementNS(svgNS, "circle");
      c.setAttribute("cx", M + rPunkte * Math.sin(g * Math.PI / 180));
      c.setAttribute("cy", M - rPunkte * Math.cos(g * Math.PI / 180));
      c.setAttribute("r", kd > 90 ? 1.3 : 1.1);
      c.setAttribute("class", "kranz-punkte");
      svg.appendChild(c);
    }
  }

  // Bogen: Rest + Wert (Hero/Orbit); Pointer bekommt keinen Bogen
  let bogenWert = null;
  if (!zeiger) {
    const rest = document.createElementNS(svgNS, "path");
    rest.setAttribute("d", bogenPfad(M, M, rBogen, START, START + SWEEP));
    rest.setAttribute("class", "bogen-rest");
    rest.setAttribute("fill", "none");
    rest.setAttribute("stroke-width", orbit ? 2 : 3.5);
    rest.setAttribute("stroke-linecap", "round");
    svg.appendChild(rest);

    bogenWert = document.createElementNS(svgNS, "path");
    bogenWert.setAttribute("class", "bogen-wert");
    bogenWert.setAttribute("fill", "none");
    bogenWert.setAttribute("stroke-width", orbit ? 2.5 : 4);
    bogenWert.setAttribute("stroke-linecap", "round");
    svg.appendChild(bogenWert);
  }

  // feste Skala-Worte (BALANCE)
  let skalaWorte = [];
  if (skala) {
    const worte = k.dataset.skala.split(",").map(s => s.trim());
    worte.forEach((wort, i) => {
      const g = START + i * SWEEP / (worte.length - 1);
      const r = kd / 2 + 18;
      const x = M + r * Math.sin(g * Math.PI / 180);
      const y = M - r * Math.cos(g * Math.PI / 180);
      const t = document.createElementNS(svgNS, "text");
      t.setAttribute("x", x); t.setAttribute("y", y + 3);
      t.setAttribute("class", "skala-wort");
      t.setAttribute("text-anchor", Math.abs(x - M) < 8 ? "middle" : x < M ? "end" : "start");
      t.textContent = wort;
      svg.appendChild(t);
      skalaWorte.push(t);
    });
  }

  const teller = document.createElement("div");
  teller.className = "teller";
  teller.tabIndex = 0;
  teller.setAttribute("role", "slider");
  teller.setAttribute("aria-label", k.dataset.label);
  const kappe = document.createElement("div");
  kappe.className = "kappe";
  kappe.innerHTML = '<span class="punkt"></span>';
  teller.appendChild(kappe);
  if (zeiger) {
    const z = document.createElement("span");
    z.className = "zeiger";
    teller.appendChild(z);
  }
  buehne.append(svg, teller);
  if (orbit) {
    const kugel = document.createElement("span");
    kugel.className = "orbit-kugel";
    buehne.appendChild(kugel);
  }

  const label = document.createElement("div");
  label.className = "k-label";
  label.textContent = k.dataset.label;
  const wert = document.createElement("div");
  wert.className = "k-wert";
  k.append(buehne, label, wert);

  k._zeichne = () => {
    const w = +k.dataset.wert;
    const winkel = START + w / 100 * SWEEP;
    k.style.setProperty("--winkel", winkel + "deg");
    teller.setAttribute("aria-valuenow", Math.round(w));
    teller.setAttribute("aria-valuemin", 0);
    teller.setAttribute("aria-valuemax", 100);
    if (bogenWert) {
      if (w <= 0.5) bogenWert.setAttribute("d", "");
      else bogenWert.setAttribute("d", bogenPfad(M, M, rBogen, START, winkel));
    }
    skalaWorte.forEach((t, i) => t.classList.toggle("aktiv",
      i === Math.round(w / 100 * (skalaWorte.length - 1))));
    wert.textContent = wertText(k);
  };

  dreheAn(teller, k);
  k._zeichne();
}

function dreheAn(griff, k) {
  const setz = (w) => { k.dataset.wert = klemme(w, 0, 100); k._zeichne(); };
  let startY = 0, startW = 0;
  griff.addEventListener("pointerdown", e => {
    if (document.body.dataset.zustand === "deaktiviert") return;
    griff.setPointerCapture(e.pointerId);
    startY = e.clientY; startW = +k.dataset.wert;
    e.preventDefault();
  });
  griff.addEventListener("pointermove", e => {
    if (!griff.hasPointerCapture(e.pointerId)) return;
    setz(startW + (startY - e.clientY) * 0.55);
  });
  griff.addEventListener("dblclick", () => setz(k.dataset.bipolar ? 50 : 0));
  griff.addEventListener("wheel", e => {
    e.preventDefault();
    setz(+k.dataset.wert + (e.deltaY < 0 ? 1.5 : -1.5));
  }, { passive: false });
  griff.addEventListener("keydown", e => {
    const s = e.shiftKey ? 10 : 2;
    if (e.key === "ArrowUp" || e.key === "ArrowRight") { setz(+k.dataset.wert + s); e.preventDefault(); }
    if (e.key === "ArrowDown" || e.key === "ArrowLeft") { setz(+k.dataset.wert - s); e.preventDefault(); }
  });
}

/* =========================================================
   Schieberegler
   ========================================================= */
function bauSlider(s) {
  const kopf = document.createElement("div");
  kopf.className = "s-kopf";
  kopf.innerHTML = `<span class="k-label">${s.dataset.label}</span><span class="k-wert"></span>`;
  const ticks = document.createElement("div");
  ticks.className = "s-ticks";
  for (let i = 0; i < 11; i++) ticks.appendChild(document.createElement("i"));
  const bahn = document.createElement("div");
  bahn.className = "s-bahn";
  bahn.tabIndex = 0;
  bahn.setAttribute("role", "slider");
  bahn.setAttribute("aria-label", s.dataset.label);
  bahn.innerHTML = '<div class="s-linie"></div><div class="s-voll"></div><div class="s-griff"></div>';
  s.append(kopf, ticks, bahn);

  const voll = $(".s-voll", bahn), griff = $(".s-griff", bahn), wert = $(".k-wert", kopf);
  s._zeichne = () => {
    const w = +s.dataset.wert;
    griff.style.left = w + "%";
    voll.style.width = w + "%";
    bahn.setAttribute("aria-valuenow", Math.round(w));
    wert.textContent = wertText(s);
  };

  const setzAusEvent = e => {
    const r = bahn.getBoundingClientRect();
    s.dataset.wert = klemme((e.clientX - r.left) / r.width * 100, 0, 100);
    s._zeichne();
  };
  bahn.addEventListener("pointerdown", e => {
    if (document.body.dataset.zustand === "deaktiviert") return;
    bahn.setPointerCapture(e.pointerId); setzAusEvent(e); e.preventDefault();
  });
  bahn.addEventListener("pointermove", e => { if (bahn.hasPointerCapture(e.pointerId)) setzAusEvent(e); });
  bahn.addEventListener("dblclick", () => { s.dataset.wert = s.dataset.bipolar ? 50 : 0; s._zeichne(); });
  bahn.addEventListener("keydown", e => {
    const st = e.shiftKey ? 10 : 2;
    if (e.key === "ArrowRight") { s.dataset.wert = klemme(+s.dataset.wert + st, 0, 100); s._zeichne(); e.preventDefault(); }
    if (e.key === "ArrowLeft")  { s.dataset.wert = klemme(+s.dataset.wert - st, 0, 100); s._zeichne(); e.preventDefault(); }
  });
  s._zeichne();
}

/* Doppelgriff-Bereich (LPASS/HPASS) */
function bauRange(r) {
  r.innerHTML = `
    <div class="r-koepfe">
      <span class="k-label">${r.dataset.labelVon}</span>
      <span class="k-label">${r.dataset.labelBis}</span>
    </div>
    <div class="s-ticks"></div>
    <div class="s-bahn" tabindex="0" role="group" aria-label="${r.dataset.labelVon} / ${r.dataset.labelBis}">
      <div class="s-linie"></div><div class="s-voll"></div>
      <div class="s-griff g-von"></div><div class="s-griff g-bis"></div>
    </div>
    <div class="r-fuss">
      <span class="k-wert">${r.dataset.minText}</span>
      <span class="k-wert r-mitte"></span>
      <span class="k-wert">${r.dataset.maxText}</span>
    </div>`;
  const ticks = $(".s-ticks", r);
  for (let i = 0; i < 13; i++) ticks.appendChild(document.createElement("i"));
  const bahn = $(".s-bahn", r), voll = $(".s-voll", r);
  const gV = $(".g-von", r), gB = $(".g-bis", r), mitte = $(".r-mitte", r);

  r._zeichne = () => {
    const v = +r.dataset.von, b = +r.dataset.bis;
    gV.style.left = v + "%"; gB.style.left = b + "%";
    voll.style.left = v + "%"; voll.style.width = (b - v) + "%";
    mitte.textContent = document.body.dataset.zustand === "deaktiviert"
      ? "—"
      : Math.round(v / 100 * 200) / 10 + " – " + Math.round(b / 100 * 200) / 10 + " kHz";
  };

  let ziel = null;
  const setzAusEvent = e => {
    const rect = bahn.getBoundingClientRect();
    const p = klemme((e.clientX - rect.left) / rect.width * 100, 0, 100);
    if (ziel === "von") r.dataset.von = Math.min(p, +r.dataset.bis - 3);
    else r.dataset.bis = Math.max(p, +r.dataset.von + 3);
    r._zeichne();
  };
  bahn.addEventListener("pointerdown", e => {
    if (document.body.dataset.zustand === "deaktiviert") return;
    const rect = bahn.getBoundingClientRect();
    const p = (e.clientX - rect.left) / rect.width * 100;
    ziel = Math.abs(p - r.dataset.von) <= Math.abs(p - r.dataset.bis) ? "von" : "bis";
    bahn.setPointerCapture(e.pointerId); setzAusEvent(e); e.preventDefault();
  });
  bahn.addEventListener("pointermove", e => { if (bahn.hasPointerCapture(e.pointerId)) setzAusEvent(e); });
  r._zeichne();
}

/* =========================================================
   Tasten, Schalter, LEDs, Dock
   ========================================================= */
function bauTasten() {
  $$(".pille").forEach(p => p.addEventListener("click", () => {
    p.dataset.an = p.dataset.an === "1" ? "0" : "1";
    zeigWerte();
  }));

  $$(".segment").forEach(seg => {
    const spans = $$("span", seg);
    const mal = () => spans.forEach((sp, i) => sp.classList.toggle("aktiv", i === +seg.dataset.aktiv));
    seg.addEventListener("click", () => { seg.dataset.aktiv = 1 - +seg.dataset.aktiv; mal(); zeigWerte(); });
    seg.tabIndex = 0;
    seg.setAttribute("role", "switch");
    mal();
  });

  $$(".kipp").forEach(k => {
    const worte = k.dataset.skala.split(",").map(s => s.trim());
    k.innerHTML = `
      <div class="kipp-schacht"><div class="hebel"></div></div>
      <div class="kipp-skala">${worte.map(w => `<span>${w}</span>`).join("")}</div>
      <div class="k-label">${k.dataset.label}</div>`;
    const spans = $$(".kipp-skala span", k);
    const mal = () => spans.forEach((sp, i) => sp.classList.toggle("aktiv", i === +k.dataset.pos));
    k.tabIndex = 0;
    k.addEventListener("click", () => { k.dataset.pos = (+k.dataset.pos + 1) % worte.length; mal(); });
    k.addEventListener("keydown", e => {
      if (e.key === " " || e.key === "Enter") { k.click(); e.preventDefault(); }
    });
    mal();
  });

  $$(".power").forEach(p => {
    p.innerHTML = '<svg viewBox="0 0 24 24" fill="none" stroke-width="2" stroke-linecap="round"><path d="M12 3v8"/><path d="M6.3 6.5a8 8 0 1 0 11.4 0"/></svg>';
    p.addEventListener("click", () => { p.dataset.an = p.dataset.an === "1" ? "0" : "1"; zeigWerte(); });
  });

  $$(".stepper").forEach(st => {
    let anzeige = $(".step-anzeige", st);
    if (!anzeige) {
      anzeige = document.createElement("span");
      anzeige.className = "step-anzeige";
      st.insertBefore(anzeige, st.lastElementChild);
    }
    st.dataset.nr = st.dataset.nr || "2";
    const mal = () => anzeige.textContent =
      (st.classList.contains("stepper--kompakt") ? "" : "PRESET ") +
      String(st.dataset.nr).padStart(2, "0") + "/16";
    $$(".step", st).forEach(b => b.addEventListener("click", () => {
      st.dataset.nr = ((+st.dataset.nr - 1 + +b.dataset.dir + 16) % 16) + 1;
      mal();
    }));
    mal();
  });

  $$(".led").forEach(l => {
    if (l.classList.contains("led--stomp")) return;
    l.tabIndex = 0;
    l.setAttribute("role", "switch");
    l.addEventListener("click", () => { l.dataset.an = l.dataset.an === "1" ? "0" : "1"; zeigWerte(); });
    l.addEventListener("keydown", e => { if (e.key === " " || e.key === "Enter") { l.click(); e.preventDefault(); } });
  });

  $$(".stomp").forEach(s => s.addEventListener("click", () => {
    s.dataset.an = s.dataset.an === "1" ? "0" : "1";
    const led = $(".led--stomp", s.closest(".stomp-block"));
    if (led) led.dataset.an = s.dataset.an;
  }));

  const ICONS = {
    haus:  '<path d="M4 11 12 4l8 7"/><path d="M6 10v9h12v-9"/>',
    pfad:  '<circle cx="6" cy="6" r="2.2"/><circle cx="18" cy="6" r="2.2"/><circle cx="6" cy="18" r="2.2"/><circle cx="18" cy="18" r="2.2"/><path d="M8 6h7.8M6 8.2v7.6M18 8.2v7.6M8.2 18h7.6"/>',
    stab:  '<path d="M5 19 15 9"/><path d="m14 4 .8 2.2L17 7l-2.2.8L14 10l-.8-2.2L11 7l2.2-.8Z"/><path d="m19 12 .5 1.5L21 14l-1.5.5L19 16l-.5-1.5L17 14l1.5-.5Z"/>',
    raster:'<rect x="4" y="4" width="6.6" height="6.6" rx="1.4"/><rect x="13.4" y="4" width="6.6" height="6.6" rx="1.4"/><rect x="4" y="13.4" width="6.6" height="6.6" rx="1.4"/><rect x="13.4" y="13.4" width="6.6" height="6.6" rx="1.4"/>'
  };
  $$(".dock").forEach(d => {
    const tasten = $$(".dock-taste", d);
    tasten.forEach((t, i) => {
      t.innerHTML = `<svg viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round">${ICONS[t.dataset.icon] || ""}</svg>`;
      t.addEventListener("click", () => { d.dataset.aktiv = i; mal(); zeigWerte(); });
    });
    const mal = () => tasten.forEach((t, i) => t.classList.toggle("aktiv", i === +d.dataset.aktiv));
    mal();
  });
}

/* Wertzeilen unter Tastengruppen — ehrlich, aus dem echten Zustand gelesen */
function zeigWerte() {
  const tot = document.body.dataset.zustand === "deaktiviert";
  $$(".wertzeile").forEach(z => {
    if (tot) { z.textContent = "—"; return; }
    const probe = z.closest(".probe");
    switch (z.dataset.fuer) {
      case "pillen": {
        const p = $$(".pille", probe).map(x => `${x.textContent} ${x.dataset.an === "1" ? "on" : "off"}`);
        const seg = $(".segment", probe);
        if (seg) p.splice(1, 0, $$("span", seg)[+seg.dataset.aktiv].textContent);
        z.textContent = p.join(" · ");
        break;
      }
      case "power":
        z.textContent = "POWER " + ($(".power", probe).dataset.an === "1" ? "ON" : "OFF");
        break;
      case "led": {
        z.textContent = $$(".led", probe).map(l => l.dataset.an === "1" ? "●" : "○").join(" ");
        break;
      }
      case "dock": {
        const d = $(".dock", probe);
        z.textContent = $$(".dock-taste", d)[+d.dataset.aktiv].getAttribute("aria-label").toUpperCase();
        break;
      }
    }
  });
}

/* =========================================================
   Vorlagen-Ausschnitte aus der Truhe
   ========================================================= */
const BILDER = {
  "presentation-1":           { datei: "../assets/figma/2026-08-24-ui-assets/presentation-1.png",           w: 2048, h: 2048 },
  "guitar-pedal":             { datei: "../assets/figma/2026-08-24-ui-assets/guitar-pedal.jpg",             w: 1350, h: 2048 },
  "toggle-nav-node-108-1650": { datei: "../assets/figma/2026-08-24-ui-assets/toggle-nav-node-108-1650.png", w: 2400, h: 1350 }
};

function vorlagen(an) {
  $$(".probe[data-quelle]").forEach(p => {
    const alt = $(".ref", p);
    if (alt) alt.remove();
    if (!an) return;
    const q = BILDER[p.dataset.quelle];
    if (!q) return;
    const [x, y, w, h] = p.dataset.crop.split(",").map(Number);
    const T = 150, sk = Math.min(T / w, T / h);
    const ref = document.createElement("div");
    ref.className = "ref";
    ref.title = "Vorlage: " + p.dataset.quelle + " (" + p.dataset.crop + ")";
    const bild = document.createElement("img");
    bild.src = q.datei;
    bild.alt = "";
    bild.style.width = q.w * sk + "px";
    bild.style.left = (-x * sk + (T - w * sk) / 2) + "px";
    bild.style.top = (-y * sk + (T - h * sk) / 2) + "px";
    ref.appendChild(bild);
    p.appendChild(ref);
  });
}

/* =========================================================
   Bedien-Panel (Chrome)
   ========================================================= */
function bauPanel() {
  $$("[data-thema]", $("#panel")).forEach(b => b.addEventListener("click", () => {
    document.documentElement.dataset.thema = b.dataset.thema;
    $$("[data-thema]", $("#panel")).forEach(x => x.setAttribute("aria-pressed", x === b));
    standband();
  }));

  const setzAkzent = f => {
    document.documentElement.style.setProperty("--akzent", f);
    $("#akzent-frei").value = /^#/.test(f) ? f : "#b04038";
    $$(".p-farbe").forEach(x => x.classList.toggle("aktiv", x.dataset.akzent === f));
  };
  $$(".p-farbe").forEach(b => b.addEventListener("click", () => setzAkzent(b.dataset.akzent)));
  $("#akzent-frei").addEventListener("input", e => setzAkzent(e.target.value));
  setzAkzent("#b04038");

  const binde = (id, wirkung) => {
    const inp = $("#" + id), out = inp.nextElementSibling;
    inp.addEventListener("input", () => { out.value = inp.value; wirkung(+inp.value); });
  };
  const setzVar = (name) => (v) => document.documentElement.style.setProperty(name, v);
  binde("p-relief", v => setzVar("--relief")(v / 100));
  binde("p-glanz",  v => setzVar("--glanz")(v / 100));
  binde("p-glut",   v => setzVar("--glut")(v / 100));
  binde("p-bogen",  v => setzVar("--bogen")(v + "px"));
  binde("p-metall", v => {
    document.documentElement.style.setProperty("--metallw", (v > 0 ? v : 0) + "%");
    document.documentElement.style.setProperty("--metallb", (v < 0 ? -v : 0) + "%");
  });

  // Punktkranz-Dichte: Knobs mit erhaltenem Wert neu aufbauen
  $$("#p-ticks .p-taste").forEach(b => b.addEventListener("click", () => {
    document.documentElement.dataset.ticks = b.dataset.ticks;
    $$("#p-ticks .p-taste").forEach(x => x.setAttribute("aria-pressed", x === b));
    $$(".knob").forEach(k => { k.innerHTML = ""; bauKnob(k); });
  }));

  $$("[data-zustand]", $("#panel")).forEach(b => b.addEventListener("click", () => {
    document.body.dataset.zustand = b.dataset.zustand;
    $$("[data-zustand]", $("#panel")).forEach(x => x.setAttribute("aria-pressed", x === b));
    alleZeichnen();
  }));

  $("#p-vorlagen").addEventListener("click", () => {
    const b = $("#p-vorlagen");
    const an = b.getAttribute("aria-pressed") !== "true";
    b.setAttribute("aria-pressed", an);
    vorlagen(an);
  });

  $("#p-mischen").addEventListener("click", () => {
    $$(".knob, .slider").forEach(el => { el.dataset.wert = Math.round(Math.random() * 100); });
    $$(".range").forEach(r => {
      const a = Math.random() * 60, b = a + 15 + Math.random() * (97 - a - 15);
      r.dataset.von = a.toFixed(0); r.dataset.bis = b.toFixed(0);
    });
    alleZeichnen();
  });
}

function alleZeichnen() {
  $$(".knob, .slider, .range").forEach(el => el._zeichne && el._zeichne());
  zeigWerte();
}

function standband() {
  const n = $$(".knob, .slider, .range, .pille, .segment, .kipp, .power, .step, .led, .stomp, .dock-taste").length;
  const band = $("#standband");
  band.textContent =
    `Stand 2026-08-24 · ${n} Bausteine, alle bedienbar · Thema ${document.documentElement.dataset.thema}` +
    ` · Werte = Demo (lokal) · ${FEHLER.length} JS-Fehler`;
  band.classList.toggle("warnt", FEHLER.length > 0);
}

/* ---------- Start ---------- */
document.body.dataset.zustand = "normal";
$$(".knob").forEach(bauKnob);
$$(".slider").forEach(bauSlider);
$$(".range").forEach(bauRange);
bauTasten();
bauPanel();
zeigWerte();
standband();
