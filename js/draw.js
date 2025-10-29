export function drawFrame(canvas, sim) {
  const ctx = canvas.getContext('2d');
  const W = canvas.width, H = canvas.height;
  ctx.clearRect(0,0,W,H);
  for (const n of sim.nodes) {
    ctx.beginPath();
    ctx.arc(n.x*W, n.y*H, 2, 0, Math.PI*2);
    // keep colors simple (or use alpha like before)
    ctx.fillStyle = n.s==='S' ? '#444' : n.s==='E' ? '#f0a' : n.s==='I' ? '#c00' : '#0a0';
    ctx.fill();
  }
}