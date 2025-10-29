import { onRouteChange, getRoute } from './router.js';
import { initSetup } from './setup.js';
import { initViz } from './viz.js';

const screens = {
  '/setup': document.getElementById('screen-setup'),
  '/viz':   document.getElementById('screen-viz'),
};
const hdr = document.getElementById('hdr');

function render(route) {
  Object.values(screens).forEach(el => el.style.display = 'none');
  const el = screens[route] || screens['/setup'];
  el.style.display = 'block';
  hdr.textContent = ` — ${route === '/viz' ? 'Visualization' : 'Setup'}`;
  if (route === '/setup') initSetup();
  if (route === '/viz')   initViz();
}

onRouteChange(render);
render(getRoute());