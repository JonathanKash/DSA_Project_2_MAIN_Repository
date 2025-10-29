import { state, saveConfigFromInputs, loadConfigToInputs } from './state.js';
import { goto } from './router.js';

export function initSetup() {
  loadConfigToInputs();

  document.getElementById('start').onclick = () => {
    saveConfigFromInputs();
    goto('/viz');
  };

  document.getElementById('file').onchange = async (e) => {
    const f = e.target.files?.[0]; if (!f) return;
    const json = JSON.parse(await f.text());
    // Expect same shape as state.config; adjust as needed
    state.config = { ...state.config, ...json };
    goto('/viz');
  };
}