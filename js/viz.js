import { state } from './state.js';
import { drawFrame } from './draw.js';
let raf = 0, started = false;

function seed(N) {
  const nodes = new Array(N).fill(0).map((_,i)=>({
    id:i, x:Math.random(), y:Math.random(), vx:0, vy:0, s:i<5?'I':'S'
  }));
  return { nodes, counts:{S:N-5,E:0,I:5,R:0}, t:0 };
}
function step(sim, cfg) {
  const { nodes, counts } = sim;
  counts.S = counts.E = counts.I = counts.R = 0;
  for (const n of nodes) {
    n.vx += (Math.random()-0.5)*0.01; n.vy += (Math.random()-0.5)*0.01;
    n.x = (n.x+n.vx+1)%1; n.y = (n.y+n.vy+1)%1;
    if (n.s==='S' && Math.random() < cfg.beta*0.01) n.s='E';
    else if (n.s==='E' && Math.random() < cfg.sigma*0.1) n.s='I';
    else if (n.s==='I' && Math.random() < cfg.gamma*0.1) n.s='R';
    counts[n.s]++;
  }
  sim.t++;
}

export function initViz() {
  if (started) return; started = true;
  const canvas = document.getElementById('canvas');
  const stats = document.getElementById('stats');
  const pauseBtn = document.getElementById('pause');
  const backBtn  = document.getElementById('back');

  // size for HiDPI
  const dpr = devicePixelRatio || 1;
  const resize = () => {
    const r = canvas.getBoundingClientRect();
    canvas.width = Math.floor(r.width*dpr); canvas.height = Math.floor(r.height*dpr);
  };
  addEventListener('resize', resize); resize();

  state.sim = seed(state.config.population);

  pauseBtn.onclick = () => { state.paused = !state.paused; pauseBtn.textContent = state.paused?'Resume':'Pause'; };
  backBtn.onclick  = () => { cancelAnimationFrame(raf); started=false; history.back(); };

  const loop = () => {
    if (!state.paused) {
      step(state.sim, state.config);
      drawFrame(canvas, state.sim);
      const c = state.sim.counts;
      stats.textContent = `t=${state.sim.t}  S:${c.S} E:${c.E} I:${c.I} R:${c.R}`;
    }
    raf = requestAnimationFrame(loop);
  };
  loop();
}