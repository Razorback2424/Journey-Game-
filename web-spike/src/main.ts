import { activeUnit, attack, capture, captureChance, defend, endTurn, isCoverTile, isDifficultTile, mend, move, previewAttackDamage, setCommand, useFocus, validAttackTargets, validCaptureTargets, validMendTargets, validMoveTiles, type BattleState, type CommandType, type Tile, type Unit } from './battle';
import { advanceNavigation, completeBattle, interactFocused, interactables, moveToInteractable, restoreWorld, returnToTown, setActiveCompanion, upgradeTown, setDestination, WORLD_SAVE_KEY, type Point } from './world';
import './style.css';

declare global { interface Window { render_game_to_text: () => string; advanceTime: (ms: number) => void; } }

const world = restoreWorld(window.localStorage.getItem(WORLD_SAVE_KEY));
const app = document.querySelector<HTMLDivElement>('#app')!;
const gridUnit = 100 / 14;
// Public assets are not fingerprinted by Vite. Keep this release token in the URL so a
// previously cached Pages 404 cannot survive after an asset-only deployment.
const asset = (path: string) => `${import.meta.env.BASE_URL}assets/${path}?v=20260804e`;
document.documentElement.style.setProperty('--ui-chrome', `url("${asset('ui/painterly-ui-chrome.png')}")`);
document.documentElement.style.setProperty('--reward-background', `url("${asset('world/hearthglen-bg.png')}")`);
const animationSource: Record<string, string> = { guardian: asset('battle/battle-sprite-guardian.png'), mossling: asset('battle/battle-sprite-companion.png'), scout: asset('battle/battle-sprite-scout.png'), slime: asset('battle/battle-sprite-enemy-slime.png'), bat: asset('battle/battle-sprite-enemy-bat.png') };
type CueKind = 'impact' | 'mend' | 'moonstone' | 'discovery' | 'upgrade' | 'move' | 'victory';
interface Cue { id: number; kind: CueKind; unitId?: string; targetId?: string; }
let cue: Cue | null = null;
let cueSequence = 0;
let feedback = '';
let selectedTile: Tile | null = null;
let portraitDismissed = false;
let pendingCaptureId: string | null = null;

function showCue(kind: CueKind, unitId?: string, targetId?: string) {
  const next = { id: ++cueSequence, kind, unitId, targetId }; cue = next;
  window.setTimeout(() => { if (cue?.id === next.id) { cue = null; render(); } }, kind === 'victory' ? 1200 : 680);
}
function persistWorld() { window.localStorage.setItem(WORLD_SAVE_KEY, JSON.stringify(world)); }
function modeTitle() { return world.mode === 'town' ? 'Hearthglen' : world.mode === 'explore' ? 'Moss Hollow' : world.mode === 'rewards' ? 'Expedition complete' : 'First light at Moss Hollow'; }
function escapeHtml(value: string) { return value.replace(/[&<>'"]/g, (character) => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', "'": '&#039;', '"': '&quot;' }[character]!)); }
function unitPosition(unit: Unit) { return { left: (5 + unit.x - unit.y) * gridUnit + gridUnit, top: (unit.x + unit.y) * gridUnit + gridUnit }; }
function tileKey(tile: Tile) { return `${tile.x},${tile.y}`; }
function isPortraitBattle() { return world.mode === 'battle' && window.innerHeight > window.innerWidth && !portraitDismissed; }
function commandLabel(command: CommandType) { return command === 'none' ? 'Choose an action' : command === 'mend' ? 'Select an injured ally' : command === 'attack' ? 'Select a nearby enemy' : command === 'capture' ? 'Select a weakened enemy to capture' : 'Select a destination'; }

function settleVictory() { if (world.battle?.winner === 'player' && !world.lastRewards) { completeBattle(world); showCue('victory'); feedback = 'Victory confirmed — return to Hearthglen for your reward.'; } }
function setFeedbackFromEvent(state: BattleState, fallback: string) { feedback = state.events[0]?.message ?? fallback; }
function resetBattleSelection() { selectedTile = activeUnit(world.battle!) ? { x: activeUnit(world.battle!)!.x, y: activeUnit(world.battle!)!.y } : null; }
let navigationTimer: number | null = null;
let playerEl: HTMLElement | null = null;
function stopNavigationTimer() { if (navigationTimer !== null) { window.clearTimeout(navigationTimer); navigationTimer = null; } }
function pointStyle(point: Point, nav = world.navigation) { return `left:${(point.x + .5) * (100 / nav.width)}%;top:${(point.y + .5) * (100 / nav.height)}%`; }
function updateAvatarPosition() {
  if (!playerEl) return;
  const nav = world.navigation;
  playerEl.setAttribute('style', pointStyle(nav.visualPlayer, nav));
  playerEl.classList.toggle('facing-left', nav.facing === 'left');
  playerEl.classList.toggle('facing-right', nav.facing === 'right');
}
function runNavigation() {
  stopNavigationTimer();
  render();
  if (!world.navigation.moving) return;
  if (window.matchMedia('(prefers-reduced-motion: reduce)').matches) { advanceNavigation(world, 60_000); render(); return; }
  const tick = () => {
    advanceNavigation(world, 32);
    if (world.navigation.moving) { updateAvatarPosition(); navigationTimer = window.setTimeout(tick, 32); }
    else { navigationTimer = null; render(); }
  };
  navigationTimer = window.setTimeout(tick, 32);
}
function chooseDestination(point: Point) { if (setDestination(world, point)) runNavigation(); else render(); }

async function requestLandscape() {
  if (world.mode !== 'battle') return;
  try { await (screen.orientation as ScreenOrientation & { lock?: (orientation: string) => Promise<void> }).lock?.('landscape'); } catch { /* portrait overlay remains available when the browser rejects a lock. */ }
}

function renderNavigationLayer() {
  const nav = world.navigation;
  const player = `<div class="world-player ${nav.moving ? 'walking' : 'idle'} facing-${nav.facing}" style="${pointStyle(nav.visualPlayer, nav)}" aria-label="Guardian party avatar, ${nav.moving ? `walking ${nav.facing}` : 'standing'}"><i class="world-player-shadow"></i><span class="world-player-direction"><span class="world-player-sprite" style="--idle-sprite:url('${asset('battle/battle-sprite-guardian.png')}')" aria-hidden="true"></span></span></div>`;
  const destination = nav.destination ? `<div class="destination-marker" style="${pointStyle(nav.destination, nav)}" aria-label="Destination"></div>` : '';
  return `<div class="world-navigation"><button class="world-map-input" data-world-map aria-label="Choose a safe place for Guardian to walk">Move Guardian</button>${destination}${player}</div>`;
}
function renderTownStage() {
  const workshopOpen = world.townLevel >= 2;
  return `<div class="town-stage"><img class="world-bg" src="${asset('world/hearthglen-bg.png')}" alt="Hearthglen village"><img class="town-building home-building" src="${asset('world/hearthglen-home.png')}" alt="Hearthglen home"><img class="town-building workshop-building ${cue?.kind === 'upgrade' ? 'upgrade-reveal' : ''}" src="${asset(`world/${workshopOpen ? 'workshop-open' : 'workshop-closed'}.png`)}" alt="${workshopOpen ? 'Open field workshop' : 'Closed field workshop'}">${renderNavigationLayer()}<button class="world-interactable town-trail ${world.navigation.focused === 'trail' ? 'focused' : ''}" data-interactable="trail"><span>TO MOSS HOLLOW</span></button>${cue?.kind === 'upgrade' ? `<div class="world-cue workshop-cue"><img src="${asset('effects/discovery.png')}" alt=""></div>` : ''}<div class="place-copy home-copy"><strong>HEARTHGLEN HOME</strong><span>A safe return after every expedition.</span></div><div class="place-copy workshop-copy"><strong>${workshopOpen ? 'FIELD WORKSHOP OPEN' : 'WORKSHOP LOT'}</strong><span>${workshopOpen ? 'Materials can now strengthen your team.' : 'Bring Moonstone home to begin.'}</span></div></div>`;
}
function renderExploreStage() {
  const items = interactables(world);
  return `<div class="explore-stage"><img class="world-bg" src="${asset('world/moss-hollow-bg.png')}" alt="Moss Hollow forest">${renderNavigationLayer()}${items.some((item) => item.id === 'mossling') ? `<button class="world-node mossling-node ${world.navigation.focused === 'mossling' ? 'focused' : ''}" data-interactable="mossling"><img src="${asset('world/mossling-node.png')}" alt="Mossling clearing"><span>MOSSLING?</span></button>` : ''}${items.some((item) => item.id === 'moonstone') ? `<button class="world-node moonstone-node ${world.navigation.focused === 'moonstone' ? 'focused' : ''}" data-interactable="moonstone"><img src="${asset('world/moonstone-node.png')}" alt="Moonstone vein"><span>MOONSTONE</span></button>` : ''}${items.some((item) => item.id === 'encounter') ? `<button class="encounter-gate ${world.navigation.focused === 'encounter' ? 'focused' : ''}" data-interactable="encounter"><img src="${asset('world/encounter-gate.png')}" alt="Encounter gate"><span>ENCOUNTER</span></button>` : ''}${cue?.kind === 'discovery' ? `<div class="world-cue discovery-cue"><img src="${asset('effects/discovery.png')}" alt=""></div>` : ''}${cue?.kind === 'moonstone' ? `<div class="world-cue moonstone-cue"><img src="${asset('effects/moonstone.png')}" alt=""></div>` : ''}</div>`;
}
function renderRewardsStage() {
  const reward = world.lastRewards!;
  const loot = reward.loot.length ? reward.loot.map((item) => `<li>${escapeHtml(item.name)} ×${item.quantity}</li>`).join('') : '<li>No items recovered</li>';
  const captured = reward.captured.length ? reward.captured.map(escapeHtml).join(', ') : 'None';
  return `<div class="rewards-stage"><div class="rewards-card"><span class="kicker">FIRST EXPEDITION</span><h2>${reward.outcome === 'victory' ? 'Victory secured' : 'Expedition ended'}</h2><p>The road home carries new stories, materials, and possible companions.</p><div class="reward-grid"><div><strong>TEAM XP</strong><b>+${reward.xp}</b></div><div><strong>CAPTURE ORBS</strong><b>${reward.remainingOrbs}</b></div><div><strong>CAPTURED</strong><span>${captured}</span></div><div><strong>LOOT</strong><ul>${loot}</ul></div></div><button class="gold" data-action="return">Continue to Hearthglen</button></div></div>`;
}

function renderBattleStage() {
  const state = world.battle!; const current = activeUnit(state);
  const moves = new Set(validMoveTiles(state).map(tileKey));
  const attacks = new Set(validAttackTargets(state).map((unit) => tileKey(unit)));
  const heals = new Set(validMendTargets(state).map((unit) => tileKey(unit)));
  const captures = new Set(validCaptureTargets(state).map((unit) => tileKey(unit)));
  const tiles = Array.from({ length: state.grid.width * state.grid.height }, (_, index) => {
    const x = index % state.grid.width; const y = Math.floor(index / state.grid.width); const point = { x, y }; const key = tileKey(point);
    const classes = [moves.has(key) && state.command === 'move' ? 'reachable' : '', attacks.has(key) && state.command === 'attack' ? 'attackable' : '', heals.has(key) && state.command === 'mend' ? 'mendable' : '', captures.has(key) && state.command === 'capture' ? 'capturable' : '', isCoverTile(state, x, y) ? 'cover' : '', isDifficultTile(state, x, y) ? 'difficult' : '', selectedTile?.x === x && selectedTile?.y === y ? 'selected' : ''].filter(Boolean).join(' ');
    return `<button class="battle-tile ${classes}" data-action="tile" data-tile="${key}" style="left:${(5 + x - y) * gridUnit}%;top:${(x + y) * gridUnit}%;width:${2 * gridUnit}%;height:${2 * gridUnit}%" aria-label="Tile ${x + 1}, ${y + 1}${isCoverTile(state, x, y) ? ', cover' : ''}${isDifficultTile(state, x, y) ? ', difficult terrain' : ''}"></button>`;
  }).join('');
  const units = state.units.filter((unit) => unit.hp > 0).map((unit) => {
    const point = unitPosition(unit); const hp = Math.round((unit.hp / unit.maxHp) * 100); const cueClass = cue?.targetId === unit.id && cue.kind === 'impact' ? 'hit' : cue?.unitId === unit.id && cue.kind === 'move' ? 'move-arrive' : '';
    const stateText = `${unit.moved ? 'Move ✓' : `Move ${unit.moveRange}`} · ${unit.actionUsed ? 'Action ✓' : unit.defending ? 'Defending' : 'Action ready'} · ${unit.bonusUsed ? 'Bonus ✓' : unit.bonusAbility ? 'Bonus ready' : 'No bonus'}`;
    const captureText = unit.team === 'enemy' ? `<em class="capture-readout ${captureChance(unit) ? 'ready' : ''}">${captureChance(unit) ? `Capture ${captureChance(unit)}%` : `${Math.ceil(unit.maxHp * .35)} HP to capture`}</em>` : '';
    const attackText = unit.team === 'enemy' && current?.team === 'player' && validAttackTargets(state).some((target) => target.id === unit.id) ? `<em class="attack-readout">Hit ${previewAttackDamage(state, unit)}</em>` : '';
    const sprite = animationSource[unit.id] ?? (unit.name === 'Moss Slime' ? animationSource.slime : unit.name === 'Cave Bat' ? animationSource.bat : animationSource.scout);
    return `<button class="battle-unit ${unit.team} ${current?.id === unit.id ? 'active' : ''} ${cueClass}" data-unit="${unit.id}" style="left:${point.left}%;top:${point.top}%" aria-label="${escapeHtml(unit.name)}, ${unit.hp} of ${unit.maxHp} health, ${stateText}"><i class="turn-diamond"></i><span class="sprite-animated" style="--sprite:url('${sprite}')" aria-hidden="true"></span><div class="health"><i style="width:${hp}%"></i></div><span>${escapeHtml(unit.name)}</span><small>${stateText}</small>${attackText}${captureText}</button>`;
  }).join('');
  const target = cue?.targetId ? state.units.find((unit) => unit.id === cue?.targetId) : undefined; const cuePoint = target ? unitPosition(target) : undefined;
  const battleCue = cue?.kind === 'impact' && cuePoint ? `<img class="battle-cue impact-cue" src="${asset('effects/impact.png')}" alt="" style="left:${cuePoint.left}%;top:${cuePoint.top}%">` : cue?.kind === 'mend' && cuePoint ? `<img class="battle-cue mend-cue" src="${asset('effects/mend.png')}" alt="" style="left:${cuePoint.left}%;top:${cuePoint.top}%">` : cue?.kind === 'victory' ? '<div class="victory-glow"></div>' : '';
  return `<div class="battle-stage"><div class="turn-strip"><strong>${escapeHtml(current?.name ?? '—')}</strong><span>Initiative ${state.turnIndex + 1}/${state.units.length}</span><span>Round ${state.round}</span><span>${current?.moved ? 'Move used' : `Move ${current?.moveRange ?? 0}`}</span><span>${current?.actionUsed ? 'Action used' : current?.defending ? 'Defending' : 'Action ready'}</span><span>${current?.bonusUsed ? 'Bonus used' : current?.bonusAbility ? 'Bonus ready' : 'Reaction ready'}</span><span class="orb-count">◈ ${state.captureOrbs} Orbs</span></div><div class="battlefield"><img class="battlefield-bg" src="${asset('battle/battle-arena-bg.png')}" alt=""><img class="battle-prop left" src="${asset('battle/battle-prop-left.png')}" alt=""><img class="battle-prop right" src="${asset('battle/battle-prop-right.png')}" alt="">${tiles}${units}${battleCue}</div>${isPortraitBattle() ? '<div class="rotate-overlay" role="dialog" aria-label="Rotate for battle"><div><strong>Battle plays best in landscape</strong><span>Rotate your device for a wider tactical view.</span><button data-action="portrait-dismiss">Continue in portrait</button></div></div>' : ''}</div>`;
}

function actionButton(action: string, label: string, disabled: boolean, extra = '') { return `<button class="battle-action ${action}" data-action="${action}" ${disabled ? 'disabled' : ''}>${label}${extra ? `<small>${extra}</small>` : ''}</button>`; }
function renderActions() {
  if (world.mode === 'town' || world.mode === 'explore') { const canUpgrade = world.materials >= 3 && world.townLevel < 2; const focused = interactables(world).find((item) => item.id === world.navigation.focused); const roster = ['Scout', ...(world.discovered.includes('Mossling') ? ['Mossling'] : []), ...world.capturedRoster].map((name) => `<button data-action="choose-companion" data-companion="${name}" ${world.activeCompanion === name ? 'disabled' : ''}>Field ${escapeHtml(name)}${world.activeCompanion === name ? ' · active' : ''}</button>`).join(''); const navigation = `<p class="hint">${world.navigation.moving ? 'Walking… tap/click another tile to change destination.' : focused ? `At ${escapeHtml(focused.label)}.` : 'Click or tap a destination to walk there.'}</p>${focused ? `<button class="gold" data-action="interact">Interact · ${escapeHtml(focused.label)}</button>` : ''}`; if (world.mode === 'explore') return `${navigation}<button data-action="return">Return to town</button>`; return `${navigation}<div class="roster-actions">${roster}</div><button data-action="upgrade" ${canUpgrade ? '' : 'disabled'}>${world.townLevel >= 2 ? 'Workshop open' : 'Build field workshop · 3 moonstone'}</button>`; }
  const state = world.battle!; const current = activeUnit(state); const hasAttack = validAttackTargets(state).length > 0; const hasMend = validMendTargets(state).length > 0; const hasCapture = validCaptureTargets(state).length > 0;
  const actionDisabled = Boolean(state.winner || current?.actionUsed); const moveDisabled = Boolean(state.winner || current?.moved);
  const reason = state.winner ? (state.winner === 'player' ? 'Battle won' : 'Battle lost') : feedback || commandLabel(state.command);
  const capturePreview = hasCapture ? validCaptureTargets(state).map((unit) => `${unit.name} ${captureChance(unit)}%`).join(' · ') : state.captureOrbs ? 'Weaken an enemy below 35% HP.' : 'No Capture Orbs remaining.';
  return `<div class="action-tray" aria-label="Battle actions">${actionButton('attack', '1 Attack', actionDisabled || !hasAttack, !hasAttack && !actionDisabled ? 'No target in range' : '')}${actionButton('move', '2 Move', moveDisabled, moveDisabled ? 'Already moved' : '')}${current?.ability === 'mend' ? actionButton('mend', '3 Mend', actionDisabled || !hasMend, !hasMend && !actionDisabled ? 'No ally to heal' : '') : ''}${actionButton('capture', '5 Capture', actionDisabled || !hasCapture, capturePreview)}${actionButton('defend', '4 Defend', actionDisabled, 'Reduce next hit')}${current?.bonusAbility === 'focus' ? actionButton('focus', 'B Focus', Boolean(current.bonusUsed), 'Bonus: +1 next hit') : ''}${actionButton('end', 'E End turn', Boolean(state.winner))}${state.command !== 'none' ? '<button class="battle-action cancel" data-action="cancel">Esc Cancel</button>' : ''}</div><p class="hint battle-hint" aria-live="polite">${escapeHtml(reason)}</p><button class="return-battle" data-action="return" ${state.winner ? '' : 'disabled'}>Return to town</button>`;
}
function panel(content: string, className = '') { return `<section class="panel ${className}"><img class="panel-frame" src="${asset('ui/painterly-ui-chrome.png')}" alt="">${content}</section>`; }
function renderSidebar() {
  if (world.mode === 'rewards') return `<aside>${panel(`<h2>Expedition record</h2><div class="notes"><span>${world.teamXp} team XP</span><span>${world.capturedRoster.length} captured</span><span>${world.lootInventory.length} loot types</span></div>`, 'field-notes')}${panel(`<h2>Next step</h2><p class="hint">Captured creatures are now visible in your field roster at Hearthglen.</p>`, 'actions')}</aside>`;
  if (world.mode !== 'battle') return `<aside>${panel(`<h2>Field notes</h2><div class="notes">${world.discovered.map((name) => `<span>${escapeHtml(name)}</span>`).join('')}</div>`, 'field-notes')}${panel(`<h2>What matters now</h2>${renderActions()}`, 'actions')}</aside>`;
  const events = world.battle!.events.map((event) => `<p class="event ${event.kind}">${escapeHtml(event.message)}</p>`).join('');
  return `<aside class="battle-sidebar">${panel(`<h2>Tactics</h2>${renderActions()}`, 'tactics')}${panel(`<h2>Battle log</h2>${events}`, 'battle-log')}</aside>`;
}
function render() {
  persistWorld();
  const state = world.battle; const mode = world.mode === 'battle' ? `${activeUnit(state!)?.name ?? '—'} · ROUND ${state?.round ?? 1}` : world.mode.toUpperCase(); const stage = world.mode === 'town' ? renderTownStage() : world.mode === 'explore' ? renderExploreStage() : world.mode === 'rewards' ? renderRewardsStage() : renderBattleStage();
  app.innerHTML = `<section class="shell ${world.mode === 'battle' ? 'battle-shell' : ''}"><header><div><span class="kicker">JOURNEYGAME · VERTICAL SLICE</span><h1>${modeTitle()}</h1><p>${escapeHtml(world.message)}</p></div><div class="status"><strong>${mode}</strong><span><i></i>${world.materials} MOONSTONE</span></div></header><div class="layout"><main class="stage ${world.mode}">${stage}</main>${renderSidebar()}</div></section>`;
  playerEl = app.querySelector('.world-player');
  bindInteractions();
}

function onTileClick(x: number, y: number) {
  const state = world.battle; if (!state || state.winner) return;
  selectedTile = { x, y }; const mode = state.command; const occupant = state.units.find((unit) => unit.hp > 0 && unit.x === x && unit.y === y); let success = false;
  if (mode === 'move' || (!occupant && validMoveTiles(state).some((tile) => tile.x === x && tile.y === y))) { const unit = activeUnit(state); success = move(state, x, y); if (success) showCue('move', unit?.id); }
  if (mode === 'attack' && occupant) { success = attack(state, occupant.id); if (success) showCue('impact', undefined, occupant.id); }
  if (mode === 'mend' && occupant) { const healer = activeUnit(state); success = mend(state, occupant.id); if (success) showCue('mend', healer?.id, occupant.id); }
  if (mode === 'capture' && occupant) {
    if (pendingCaptureId !== occupant.id) { pendingCaptureId = occupant.id; feedback = `Confirm Capture: ${occupant.name} has a ${captureChance(occupant)}% chance. Tap/click again to spend 1 Capture Orb.`; render(); return; }
    const result = capture(state, occupant.id); pendingCaptureId = null; success = result.ok; feedback = result.reason; if (success) showCue('impact', undefined, occupant.id);
  }
  feedback = success ? (state.events[0]?.message ?? 'Action complete.') : mode === 'move' ? 'That destination is not reachable.' : 'That tile is not a valid target.'; settleVictory(); render();
}
function chooseMode(command: CommandType) { const state = world.battle; if (!state) return; pendingCaptureId = null; if (setCommand(state, command)) { resetBattleSelection(); feedback = commandLabel(command); } else feedback = command === 'attack' ? 'No enemy is in attack range.' : command === 'mend' ? 'No injured ally is in Mend range.' : command === 'capture' ? 'No weakened enemy is in capture range, or no orbs remain.' : 'That action is not available.'; render(); }
function handleBattleAction(action: string) {
  const state = world.battle!; if (action === 'attack' || action === 'move' || action === 'mend' || action === 'capture') { chooseMode(action); return; }
  if (action === 'defend') { if (defend(state)) { showCue('mend', activeUnit(state)?.id, activeUnit(state)?.id); setFeedbackFromEvent(state, 'Defending.'); } else feedback = 'Action already used.'; }
  if (action === 'focus') { if (useFocus(state)) setFeedbackFromEvent(state, 'Focused.'); else feedback = 'Bonus action already used.'; }
  if (action === 'end') { endTurn(state); setFeedbackFromEvent(state, 'Turn ended.'); }
  if (action === 'cancel') { state.command = 'none'; pendingCaptureId = null; feedback = 'Action cancelled.'; }
  settleVictory(); render();
}
function bindInteractions() {
  app.querySelectorAll<HTMLButtonElement>('[data-action]').forEach((button) => button.addEventListener('click', (event) => {
    const action = button.dataset.action!; requestLandscape();
    switch (action) {
      case 'tile': { event.stopPropagation(); const [x, y] = button.dataset.tile!.split(',').map(Number); onTileClick(x, y); return; }
      case 'companion': setActiveCompanion(world, world.activeCompanion === 'Mossling' ? 'Scout' : 'Mossling'); break;
      case 'choose-companion': setActiveCompanion(world, button.dataset.companion as import('./world').CompanionName); break;
      case 'upgrade': { const level = world.townLevel; upgradeTown(world); if (world.townLevel > level) showCue('upgrade'); break; }
      case 'return': if (world.battle?.winner === 'player') world.battle = null; returnToTown(world); break;
      case 'interact': { const before = world.mode; const focus = world.navigation.focused; interactFocused(world); if (focus === 'mossling') showCue('discovery'); if (focus === 'moonstone') showCue('moonstone'); if (world.mode === 'battle' && before !== 'battle') { resetBattleSelection(); portraitDismissed = false; } break; }
      case 'portrait-dismiss': portraitDismissed = true; break;
      default: if (world.mode === 'battle') { handleBattleAction(action); return; }
    }
    settleVictory(); render();
  }));
}
function onKeyDown(event: KeyboardEvent) {
  const state = world.battle; if (!state || state.winner || event.metaKey || event.ctrlKey || event.altKey) return;
  const actionByKey: Record<string, string> = { '1': 'attack', '2': 'move', '3': 'mend', '4': 'defend', '5': 'capture', b: 'focus', e: 'end', Escape: 'cancel' };
  if (actionByKey[event.key]) { event.preventDefault(); handleBattleAction(actionByKey[event.key]); return; }
  const direction: Record<string, Tile> = { ArrowUp: { x: 0, y: -1 }, w: { x: 0, y: -1 }, ArrowDown: { x: 0, y: 1 }, s: { x: 0, y: 1 }, ArrowLeft: { x: -1, y: 0 }, a: { x: -1, y: 0 }, ArrowRight: { x: 1, y: 0 }, d: { x: 1, y: 0 } };
  if (direction[event.key]) { event.preventDefault(); if (state.command !== 'move') { if (!setCommand(state, 'move')) { feedback = 'Movement is not available.'; render(); return; } resetBattleSelection(); } const delta = direction[event.key]; const base = selectedTile ?? { x: activeUnit(state)!.x, y: activeUnit(state)!.y }; selectedTile = { x: Math.max(0, Math.min(state.grid.width - 1, base.x + delta.x)), y: Math.max(0, Math.min(state.grid.height - 1, base.y + delta.y)) }; feedback = `Destination ${selectedTile.x + 1}, ${selectedTile.y + 1}. Press Enter to move.`; render(); return; }
  if (event.key === 'Enter' && state.command === 'move' && selectedTile) { event.preventDefault(); onTileClick(selectedTile.x, selectedTile.y); }
}
app.addEventListener('click', (event) => {
  const target = event.target as HTMLElement;
  const battleTile = target.closest<HTMLElement>('[data-tile]');
  if (battleTile) { const [x, y] = battleTile.dataset.tile!.split(',').map(Number); requestLandscape(); onTileClick(x, y); return; }
  const unit = target.closest<HTMLElement>('[data-unit]');
  if (unit) { const battleTarget = world.battle?.units.find((candidate) => candidate.id === unit.dataset.unit); if (!battleTarget || !world.battle) return; requestLandscape(); if (battleTarget.team === 'player') { if (battleTarget.id === activeUnit(world.battle)?.id) chooseMode('move'); else { feedback = `It is not ${battleTarget.name}'s turn.`; render(); } } else onTileClick(battleTarget.x, battleTarget.y); return; }
  const worldMap = target.closest<HTMLElement>('[data-world-map]');
  if (worldMap) {
    const bounds = worldMap.getBoundingClientRect();
    const x = Math.max(0, Math.min(world.navigation.width - 1, Math.floor(((event as MouseEvent).clientX - bounds.left) / bounds.width * world.navigation.width)));
    const y = Math.max(0, Math.min(world.navigation.height - 1, Math.floor(((event as MouseEvent).clientY - bounds.top) / bounds.height * world.navigation.height)));
    chooseDestination({ x, y }); return;
  }
  const interactable = target.closest<HTMLElement>('[data-interactable]');
  if (interactable) { if (moveToInteractable(world, interactable.dataset.interactable!)) runNavigation(); else render(); }
});
document.addEventListener('keydown', onKeyDown); window.addEventListener('resize', () => { if (world.mode === 'battle') render(); });
render();
window.render_game_to_text = () => JSON.stringify({ mode: world.mode, materials: world.materials, teamXp: world.teamXp, captureOrbs: world.captureOrbs, capturedRoster: world.capturedRoster, loot: world.lootInventory, rewards: world.lastRewards, companion: world.activeCompanion, encounters: { count: world.encounterCount, steps: world.encounterSteps, source: world.activeEncounterSource }, feedback, navigation: world.mode === 'town' || world.mode === 'explore' ? { scene: world.navigation.scene, player: world.navigation.player, visualPlayer: world.navigation.visualPlayer, destination: world.navigation.destination, path: world.navigation.path, focused: world.navigation.focused, moving: world.navigation.moving, walkablePoints: world.navigation.walkable.length } : null, portraitOverlay: isPortraitBattle(), battle: world.battle ? { active: activeUnit(world.battle)?.id, winner: world.battle.winner, round: world.battle.round, command: world.battle.command, selectedTile, cover: world.battle.coverTiles, difficult: world.battle.difficultTiles, captureOrbs: world.battle.captureOrbs, rewards: world.battle.rewards, units: world.battle.units.filter((unit) => unit.hp > 0).map((unit) => ({ id: unit.id, team: unit.team, hp: unit.hp, x: unit.x, y: unit.y, moved: unit.moved, actionUsed: unit.actionUsed, bonusUsed: unit.bonusUsed, reactionReady: unit.reactionReady, defending: unit.defending, captureChance: unit.team === 'enemy' ? captureChance(unit) : 0 })), valid: { move: validMoveTiles(world.battle).map(tileKey), attack: validAttackTargets(world.battle).map((unit) => unit.id), mend: validMendTargets(world.battle).map((unit) => unit.id), capture: validCaptureTargets(world.battle).map((unit) => unit.id) } } : null, coordinateSystem: 'World navigation uses authored hidden walkable regions; battle uses upper-left grid coordinates.' });
window.advanceTime = (ms: number) => { for (let elapsed = 0; elapsed < ms; elapsed += 32) advanceNavigation(world, Math.min(32, ms - elapsed)); render(); };
