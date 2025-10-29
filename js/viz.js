// /js/viz.js — GRID-BASED SEIR with 4-neighbor spread (ripple)

import { state } from './state.js';
import { drawFrame } from './draw.js';

let raf = 0, started = false;

// Discrete state codes for speed
const S = 0, E = 1, I = 2, R = 3;

function dimsFromPopulation(N) {
  // Choose a near-square grid (W*H >= N)
  const W = Math.max(10, Math.round(Math.sqrt(N)));
  const H = Math.ceil(N / W);
  return { W, H, total: W * H };
}

function seedGrid(cfg) {
  const { W, H, total } = dimsFromPopulation(cfg.population);
  const cells = new Uint8Array(total);     // S/E/I/R per cell
  const timer = new Int16Array(total);     // counts down incubation/infectious

  // Start with all S; infect a center cross to make the ripple obvious
  const center = Math.floor(H / 2) * W + Math.floor(W / 2);
  cells[center] = I;
  timer[center] = cfg.infectiousSteps;

  // Optionally infect a few more around center
  for (const off of [-1, +1, -W, +W]) {
    const j = center + off;
    if (j >= 0 && j < total && (off === -1 ? center % W !== 0 : true) && (off === +1 ? (center % W) !== W-1 : true)) {
      cells[j] = E;
      timer[j] = cfg.incubationSteps;
    }
  }

  // Precompute 4-neighbor indices (no wraparound)
  const nbr = new Int32Array(total * 4).fill(-1); // [i*4 + 0..3] => up, right, down, left
  for (let y = 0; y < H; y++) {
    for (let x = 0; x < W; x++) {
      const i = y * W + x;
      const base = i * 4;
      if (y > 0)     nbr[base + 0] = (y - 1) * W + x;     // up
      if (x < W - 1) nbr[base + 1] = y * W + (x + 1);     // right
      if (y < H - 1) nbr[base + 2] = (y + 1) * W + x;     // down
      if (x > 0)     nbr[base + 3] = y * W + (x - 1);     // left
    }
  }

  return {
    W, H, total,
    cells,
    timer,
    nbr,
    counts: { S: 0, E: 0, I: 1, R: 0 },
    t: 0
  };
}

// One step of the cellular-automaton–style SEIR
function step(sim, cfg) {
  const { total, cells, timer, nbr } = sim;
  const next = new Uint8Array(total);
  const nextT = new Int16Array(total);

  let cS = 0, cE = 0, cI = 0, cR = 0;

  for (let i = 0; i < total; i++) {
    const s = cells[i];

    if (s === S) {
      // Count infectious neighbors (4-neighborhood)
      let k = 0;
      const base = i * 4;
      for (let d = 0; d < 4; d++) {
        const j = nbr[base + d];
        if (j !== -1 && cells[j] === I) k++;
      }
      // Infection probability from k infectious neighbors:
      // p = 1 - (1 - beta)^k
      const p = (k === 0) ? 0 : 1 - Math.pow(1 - cfg.beta, k);
      if (Math.random() < p) {
        next[i] = E;
        nextT[i] = cfg.incubationSteps;
        cE++;
      } else {
        next[i] = S; cS++;
      }

    } else if (s === E) {
      const t = timer[i] - 1;
      if (t <= 0) {
        next[i] = I; nextT[i] = cfg.infectiousSteps; cI++;
      } else {
        next[i] = E; nextT[i] = t; cE++;
      }

    } else if (s === I) {
      const t = timer[i] - 1;
      if (t <= 0) {
        next[i] = R; cR++;
      } else {
        next[i] = I; nextT[i] = t; cI++;
      }

    } else {
      // R stays R
      next[i] = R; cR++;
    }
  }

  sim.cells = next;
  sim.timer = nextT;
  sim.counts = { S: cS, E: cE, I: cI, R: cR };
  sim.t++;
}

export function initViz() {
  if (started) return; started = true;

  // Add grid-specific defaults (keeps your Setup UI the same)
  const cfg = {
    population: state.config.population ?? 2000,
    beta: state.config.beta ?? 0.30,               // per-step per-neighbor infection rate
    incubationSteps: Math.round((1 / (state.config.sigma ?? 0.20)) * 2), // rough mapping to steps
    infectiousSteps: Math.round((1 / (state.config.gamma ?? 0.10)) * 3),
  };

  // Create canvas & controls
  const canvas = document.getElementById('canvas');
  const stats  = document.getElementById('stats');
  const pauseBtn = document.getElementById('pause');
  const backBtn  = document.getElementById('back');

  // Size for HiDPI
  const dpr = devicePixelRatio || 1;
  const resize = () => {
    const r = canvas.getBoundingClientRect();
    canvas.width  = Math.floor(r.width  * dpr);
    canvas.height = Math.floor(r.height * dpr);
  };
  addEventListener('resize', resize);
  resize();

  // Seed grid simulation
  state.sim = seedGrid(cfg);

  pauseBtn.onclick = () => {
    state.paused = !state.paused;
    pauseBtn.textContent = state.paused ? 'Resume' : 'Pause';
  };
  backBtn.onclick = () => { cancelAnimationFrame(raf); started = false; history.back(); };

  const loop = () => {
    if (!state.paused) {
      step(state.sim, cfg);
      drawFrame(canvas, state.sim);
      const c = state.sim.counts;
      stats.textContent = `t=${state.sim.t}  S:${c.S} E:${c.E} I:${c.I} R:${c.R}`;
    }
    raf = requestAnimationFrame(loop);
  };
  loop();
}