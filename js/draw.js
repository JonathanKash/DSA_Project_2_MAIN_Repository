export function drawFrame(canvas, sim) {
  const ctx = canvas.getContext('2d');
  const { W, H, cells } = sim;
  const CW = canvas.width / W;
  const CH = canvas.height / H;

  ctx.clearRect(0, 0, canvas.width, canvas.height);

  for (let y = 0; y < H; y++) {
    for (let x = 0; x < W; x++) {
      const i = y * W + x;
      const s = cells[i];

      // Colors per state (tweak to taste)
      // S: neutral, E: magenta, I: red, R: greenish
      let color = '#555';
      if (s === 1) color = '#d36';      // E
      else if (s === 2) color = '#c00'; // I
      else if (s === 3) color = '#0a5'; // R

      ctx.fillStyle = color;
      ctx.fillRect(Math.floor(x * CW), Math.floor(y * CH), Math.ceil(CW), Math.ceil(CH));
    }
  }

  // Optional: subtle grid lines
  ctx.strokeStyle = 'rgba(0,0,0,0.08)';
  for (let x = 1; x < W; x++) {
    const X = Math.floor(x * CW);
    ctx.beginPath(); ctx.moveTo(X, 0); ctx.lineTo(X, canvas.height); ctx.stroke();
  }
  for (let y = 1; y < H; y++) {
    const Y = Math.floor(y * CH);
    ctx.beginPath(); ctx.moveTo(0, Y); ctx.lineTo(canvas.width, Y); ctx.stroke();
  }
}