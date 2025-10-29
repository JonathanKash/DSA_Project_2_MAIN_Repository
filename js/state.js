export const state = {
  config: { population: 2000, beta:0.3, sigma:0.2, gamma:0.1, simType:'grid' },
  sim: null,
  paused: false,
};

export function saveConfigFromInputs() {
  state.config = {
    population: +document.getElementById('pop').value,
    beta: +document.getElementById('beta').value,
    sigma: +document.getElementById('sigma').value,
    gamma: +document.getElementById('gamma').value,
    simType: document.getElementById('simType').value,
  };
  localStorage.setItem('cfg', JSON.stringify(state.config));
}

export function loadConfigToInputs() {
  const saved = localStorage.getItem('cfg');
  if (saved) state.config = JSON.parse(saved);
  document.getElementById('pop').value   = state.config.population;
  document.getElementById('beta').value  = state.config.beta;
  document.getElementById('sigma').value = state.config.sigma;
  document.getElementById('gamma').value = state.config.gamma;
  document.getElementById('simType').value = state.config.simType || 'grid';
}