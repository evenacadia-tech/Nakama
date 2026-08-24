(function registerDemoAudio(root) {
  "use strict";

  const N = root.Nakama;
  if (!N) throw new Error("contract.js must be loaded before audio-demo.js");

  class DemoAudioLoop {
    constructor() {
      this.context = null;
      this.output = null;
      this.auditionFilter = null;
      this.scheduler = null;
      this.nextStepAt = 0;
      this.step = 0;
      this.running = false;
      this.auditioning = false;
      this.noiseBuffer = null;
    }

    async start() {
      if (this.running) return;
      const AudioContextClass = root.AudioContext || root.webkitAudioContext;
      if (!AudioContextClass) throw new Error("Web Audio is not available in this browser.");

      this.context = new AudioContextClass({ latencyHint: "interactive" });
      this.output = this.context.createGain();
      this.output.gain.value = 0.26;

      this.auditionFilter = this.context.createBiquadFilter();
      this.auditionFilter.type = "peaking";
      this.auditionFilter.frequency.value = 220;
      this.auditionFilter.Q.value = 1.2;
      this.auditionFilter.gain.value = this.auditioning ? -8 : 0;

      const limiter = this.context.createDynamicsCompressor();
      limiter.threshold.value = -12;
      limiter.knee.value = 6;
      limiter.ratio.value = 8;
      limiter.attack.value = 0.002;
      limiter.release.value = 0.08;

      this.output.connect(this.auditionFilter).connect(limiter).connect(this.context.destination);
      this.noiseBuffer = this.createNoiseBuffer();
      this.step = 0;
      this.nextStepAt = this.context.currentTime + 0.04;
      this.running = true;
      await this.context.resume();
      this.scheduleAhead();
      this.scheduler = root.setInterval(() => this.scheduleAhead(), 25);
    }

    async stop() {
      if (!this.context) return;
      this.running = false;
      if (this.scheduler !== null) root.clearInterval(this.scheduler);
      this.scheduler = null;
      const context = this.context;
      const now = context.currentTime;
      this.output.gain.cancelScheduledValues(now);
      this.output.gain.setValueAtTime(this.output.gain.value, now);
      this.output.gain.linearRampToValueAtTime(0, now + 0.025);
      await new Promise((resolve) => root.setTimeout(resolve, 35));
      await context.close();
      this.context = null;
      this.output = null;
      this.auditionFilter = null;
      this.noiseBuffer = null;
    }

    async toggle() {
      if (this.running) await this.stop();
      else await this.start();
      return this.running;
    }

    setAudition(active) {
      this.auditioning = Boolean(active);
      if (!this.context || !this.auditionFilter) return;
      const now = this.context.currentTime;
      const gain = this.auditionFilter.gain;
      gain.cancelScheduledValues(now);
      gain.setValueAtTime(gain.value, now);
      gain.linearRampToValueAtTime(this.auditioning ? -8 : 0, now + 0.018);
    }

    createNoiseBuffer() {
      const length = Math.floor(this.context.sampleRate * 0.08);
      const buffer = this.context.createBuffer(1, length, this.context.sampleRate);
      const data = buffer.getChannelData(0);
      let seed = 0x51f15e;
      for (let index = 0; index < length; index += 1) {
        seed = (seed * 1664525 + 1013904223) >>> 0;
        data[index] = (seed / 0xffffffff) * 2 - 1;
      }
      return buffer;
    }

    scheduleAhead() {
      if (!this.running || !this.context) return;
      const stepDuration = 60 / 112 / 4;
      while (this.nextStepAt < this.context.currentTime + 0.12) {
        this.scheduleStep(this.step, this.nextStepAt);
        this.step = (this.step + 1) % 16;
        this.nextStepAt += stepDuration;
      }
    }

    scheduleStep(step, at) {
      if ([0, 4, 8, 12].includes(step)) this.scheduleKick(at, step === 0 ? 1 : 0.78);
      if ([2, 6, 10, 14].includes(step)) this.scheduleHat(at, step % 4 === 2 ? 0.62 : 0.45);
      if ([0, 3, 6, 8, 11, 14].includes(step)) {
        const notes = [55, 65.41, 73.42, 55, 82.41, 65.41];
        const noteIndex = [0, 3, 6, 8, 11, 14].indexOf(step);
        this.scheduleBass(at, notes[noteIndex]);
      }
      if (step === 0 || step === 8) this.schedulePad(at, step === 0 ? [110, 164.81, 220] : [98, 146.83, 196]);
    }

    scheduleKick(at, accent) {
      const oscillator = this.context.createOscillator();
      const envelope = this.context.createGain();
      oscillator.type = "sine";
      oscillator.frequency.setValueAtTime(118, at);
      oscillator.frequency.exponentialRampToValueAtTime(43, at + 0.11);
      envelope.gain.setValueAtTime(0.68 * accent, at);
      envelope.gain.exponentialRampToValueAtTime(0.0001, at + 0.18);
      oscillator.connect(envelope).connect(this.output);
      oscillator.start(at);
      oscillator.stop(at + 0.2);
    }

    scheduleHat(at, accent) {
      const source = this.context.createBufferSource();
      const highpass = this.context.createBiquadFilter();
      const envelope = this.context.createGain();
      source.buffer = this.noiseBuffer;
      highpass.type = "highpass";
      highpass.frequency.value = 6200;
      envelope.gain.setValueAtTime(0.11 * accent, at);
      envelope.gain.exponentialRampToValueAtTime(0.0001, at + 0.045);
      source.connect(highpass).connect(envelope).connect(this.output);
      source.start(at);
      source.stop(at + 0.06);
    }

    scheduleBass(at, frequency) {
      const oscillator = this.context.createOscillator();
      const lowpass = this.context.createBiquadFilter();
      const envelope = this.context.createGain();
      oscillator.type = "triangle";
      oscillator.frequency.value = frequency;
      lowpass.type = "lowpass";
      lowpass.frequency.value = 620;
      lowpass.Q.value = 0.7;
      envelope.gain.setValueAtTime(0.0001, at);
      envelope.gain.exponentialRampToValueAtTime(0.23, at + 0.008);
      envelope.gain.exponentialRampToValueAtTime(0.0001, at + 0.18);
      oscillator.connect(lowpass).connect(envelope).connect(this.output);
      oscillator.start(at);
      oscillator.stop(at + 0.2);
    }

    schedulePad(at, frequencies) {
      for (const frequency of frequencies) {
        const oscillator = this.context.createOscillator();
        const envelope = this.context.createGain();
        oscillator.type = "sine";
        oscillator.frequency.value = frequency;
        envelope.gain.setValueAtTime(0.0001, at);
        envelope.gain.exponentialRampToValueAtTime(0.026, at + 0.04);
        envelope.gain.exponentialRampToValueAtTime(0.0001, at + 0.95);
        oscillator.connect(envelope).connect(this.output);
        oscillator.start(at);
        oscillator.stop(at + 1);
      }
    }
  }

  N.DemoAudioLoop = DemoAudioLoop;
})(globalThis);
