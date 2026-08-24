(function registerNakamaContract(root) {
  "use strict";

  const Nakama = (root.Nakama = root.Nakama || {});

  Nakama.CONTRACT_VERSION = 1;
  Nakama.MOTION = Object.freeze({
    directMs: 0,
    feedbackMs: 100,
    stateMs: 140,
    pageMs: 180,
    measuringPulseMs: 1600,
    spectrumDataHz: 20,
    easing: "cubic-bezier(0.22, 1, 0.36, 1)",
  });

  Nakama.ACTION = Object.freeze({
    NAVIGATE_PAGE: "NAVIGATE_PAGE",
    SELECT_SOURCE: "SELECT_SOURCE",
    SELECT_COMPARISON_SOURCE: "SELECT_COMPARISON_SOURCE",
    SELECT_BAND: "SELECT_BAND",
    SET_BAND_ENABLED: "SET_BAND_ENABLED",
    SET_BAND_PARAMETER: "SET_BAND_PARAMETER",
    SET_GLOBAL_PARAMETER: "SET_GLOBAL_PARAMETER",
    DRAG_BAND: "DRAG_BAND",
    SET_HOVERED_CONTROL: "SET_HOVERED_CONTROL",
    PREVIEW_BEGIN: "PREVIEW_BEGIN",
    PREVIEW_END: "PREVIEW_END",
    SEND_DRAFT: "SEND_DRAFT",
    TEMPORARY_APPLY: "TEMPORARY_APPLY",
    CONFIRM_APPLY: "CONFIRM_APPLY",
    REJECT_DRAFT: "REJECT_DRAFT",
    UNDO: "UNDO",
    SET_BYPASS: "SET_BYPASS",
    TOGGLE_PANEL: "TOGGLE_PANEL",
    SET_CONNECTION: "SET_CONNECTION",
    SET_TRANSPORT: "SET_TRANSPORT",
    SET_MEASUREMENT: "SET_MEASUREMENT",
    EXTERNAL_AUTOMATION: "EXTERNAL_AUTOMATION",
    LOAD_FIXTURE: "LOAD_FIXTURE",
    TICK: "TICK",
  });

  Nakama.clamp = function clamp(value, minimum, maximum) {
    const numeric = Number(value);
    if (!Number.isFinite(numeric)) return minimum;
    return Math.min(maximum, Math.max(minimum, numeric));
  };

  Nakama.deepClone = function deepClone(value) {
    if (typeof structuredClone === "function") return structuredClone(value);
    return JSON.parse(JSON.stringify(value));
  };

  Nakama.frequencyToX = function frequencyToX(frequency, width) {
    const hz = Nakama.clamp(frequency, 20, 20000);
    return (Math.log10(hz / 20) / 3) * width;
  };

  Nakama.xToFrequency = function xToFrequency(x, width) {
    const normalized = Nakama.clamp(x / width, 0, 1);
    return 20 * Math.pow(1000, normalized);
  };

  Nakama.gainToY = function gainToY(gain, height) {
    return ((12 - Nakama.clamp(gain, -12, 12)) / 24) * height;
  };

  Nakama.yToGain = function yToGain(y, height) {
    return 12 - Nakama.clamp(y / height, 0, 1) * 24;
  };

  Nakama.formatFrequency = function formatFrequency(value) {
    const hz = Math.round(Number(value));
    return hz >= 1000 ? `${(hz / 1000).toFixed(hz >= 10000 ? 1 : 2).replace(/\.0+$/, "")} kHz` : `${hz} Hz`;
  };

  Nakama.formatDb = function formatDb(value, digits) {
    const numeric = Number(value);
    const precision = Number.isInteger(digits) ? digits : 1;
    if (Math.abs(numeric) < Math.pow(10, -precision) / 2) return `${(0).toFixed(precision)} dB`;
    return `${numeric < 0 ? "−" : "+"}${Math.abs(numeric).toFixed(precision)} dB`;
  };

  Nakama.PARAMETER_DRAG = Object.freeze({
    frequency: Object.freeze({ mode: "exponential", pixelsPerDouble: 96, step: 1 }),
    gain: Object.freeze({ mode: "linear", unitsPerPixel: 0.1, fineUnitsPerPixel: 0.02, step: 0.1, fineStep: 0.01 }),
    q: Object.freeze({ mode: "exponential", pixelsPerDouble: 120, step: 0.01 }),
    threshold: Object.freeze({ mode: "linear", unitsPerPixel: 0.25, fineUnitsPerPixel: 0.05, step: 0.1, fineStep: 0.01 }),
    range: Object.freeze({ mode: "linear", unitsPerPixel: 0.1, fineUnitsPerPixel: 0.02, step: 0.1, fineStep: 0.01 }),
    attack: Object.freeze({ mode: "exponential", pixelsPerDouble: 100, step: 0.1 }),
    hold: Object.freeze({ mode: "linear", unitsPerPixel: 1, fineUnitsPerPixel: 0.1, step: 1, fineStep: 0.1 }),
    release: Object.freeze({ mode: "exponential", pixelsPerDouble: 100, step: 1 }),
  });

  Nakama.adjustParameterFromVerticalDrag = function adjustParameterFromVerticalDrag(
    parameter,
    startValue,
    upwardPixels,
    fine,
  ) {
    const rule = Nakama.PARAMETER_DRAG[parameter];
    const start = Number(startValue);
    const delta = Number(upwardPixels);
    if (!rule || !Number.isFinite(start) || !Number.isFinite(delta)) return start;

    let value;
    if (rule.mode === "exponential") {
      const sensitivity = rule.pixelsPerDouble * (fine ? 4 : 1);
      value = start * Math.pow(2, delta / sensitivity);
    } else {
      const unitsPerPixel = fine ? rule.fineUnitsPerPixel : rule.unitsPerPixel;
      value = start + delta * unitsPerPixel;
    }

    const step = fine && rule.fineStep ? rule.fineStep : rule.step;
    return Math.round(value / step) * step;
  };

  Nakama.applyProtectedFrequency = function applyProtectedFrequency(
    currentFrequency,
    requestedFrequency,
    protectedRanges,
  ) {
    let result = Nakama.clamp(requestedFrequency, 20, 20000);
    let collision = null;

    for (const range of protectedRanges || []) {
      if (result > range.startHz && result < range.endHz) {
        const fromBelow = currentFrequency <= range.startHz;
        const fromAbove = currentFrequency >= range.endHz;
        if (fromBelow) result = range.startHz;
        else if (fromAbove) result = range.endHz;
        else {
          result = Math.abs(result - range.startHz) <= Math.abs(range.endHz - result)
            ? range.startHz
            : range.endHz;
        }
        collision = range.id;
      }
    }

    return { frequency: result, collision };
  };
})(globalThis);
