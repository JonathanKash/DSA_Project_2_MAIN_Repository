export function onRouteChange(handler) {
  window.addEventListener('hashchange', () => handler(getRoute()));
}
export function getRoute() {
  return location.hash.replace(/^#/, '') || '/setup';
}
export function goto(route) {
  if (!route.startsWith('/')) route = '/' + route;
  location.hash = route;
}
