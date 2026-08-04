export type Team = 'player' | 'enemy';
export type CommandType = 'attack' | 'move' | 'mend' | 'capture' | 'defend' | 'focus' | 'none';
export interface Tile { x: number; y: number; }
export interface Loot { id: string; name: string; quantity: number; }
export interface BattleRewards { xp: number; loot: Loot[]; captured: string[]; defeated: string[]; }

export interface Unit {
  id: string; name: string; team: Team; hp: number; maxHp: number; attack: number; x: number; y: number;
  moveRange: number; attackRange: number; ability?: 'mend'; bonusAbility?: 'focus';
  moved: boolean; actionUsed: boolean; bonusUsed: boolean; reactionReady: boolean; defending: boolean; focused: boolean;
  xpValue?: number; loot?: Loot; captureIdentity?: string; captureReward?: Loot; rewarded: boolean;
}
export interface BattleEvent { message: string; kind: 'info' | 'damage' | 'heal' | 'defense' | 'terrain' | 'capture' | 'reward' | 'victory' | 'defeat'; }
export interface BattleState {
  units: Unit[]; turnIndex: number; round: number; winner: Team | null; events: BattleEvent[]; command: CommandType;
  grid: { width: number; height: number }; coverTiles: Tile[]; difficultTiles: Tile[];
  captureOrbs: number; captureAttempts: number; rewards: BattleRewards;
}
const distance = (a: Tile, b: Tile) => Math.abs(a.x - b.x) + Math.abs(a.y - b.y);
const key = (x: number, y: number) => `${x},${y}`;
const makeUnit = (unit: Omit<Unit, 'moved' | 'actionUsed' | 'bonusUsed' | 'reactionReady' | 'defending' | 'focused' | 'rewarded'>): Unit => ({ ...unit, moved: false, actionUsed: false, bonusUsed: false, reactionReady: true, defending: false, focused: false, rewarded: false });

export function createTutorialBattle(companion: 'Scout' | 'Mossling' = 'Scout', captureOrbs = 3): BattleState {
  const companionUnit = companion === 'Mossling'
    ? makeUnit({ id: 'mossling', name: 'Mossling', team: 'player', hp: 14, maxHp: 14, attack: 3, ability: 'mend', x: 1, y: 4, moveRange: 2, attackRange: 1 })
    : makeUnit({ id: 'scout', name: 'Scout', team: 'player', hp: 12, maxHp: 12, attack: 4, bonusAbility: 'focus', x: 1, y: 4, moveRange: 2, attackRange: 2 });
  return { units: [
    makeUnit({ id: 'guardian', name: 'Guardian', team: 'player', hp: 18, maxHp: 18, attack: 5, x: 1, y: 2, moveRange: 2, attackRange: 1 }), companionUnit,
    makeUnit({ id: 'slime', name: 'Moss Slime', team: 'enemy', hp: 14, maxHp: 14, attack: 3, x: 6, y: 2, moveRange: 2, attackRange: 1, xpValue: 20, loot: { id: 'moss-gel', name: 'Moss Gel', quantity: 1 }, captureIdentity: 'Moss Slime', captureReward: { id: 'slime-essence', name: 'Slime Essence', quantity: 1 } }),
    makeUnit({ id: 'bat', name: 'Cave Bat', team: 'enemy', hp: 10, maxHp: 10, attack: 4, x: 6, y: 4, moveRange: 3, attackRange: 1, xpValue: 16, loot: { id: 'bat-wing', name: 'Cave Wing', quantity: 1 }, captureIdentity: 'Cave Bat' }),
  ], turnIndex: 0, round: 1, winner: null, events: [{ message: `${companion} joins Guardian for this encounter.`, kind: 'info' }], command: 'none', grid: { width: 8, height: 6 }, coverTiles: [{ x: 3, y: 2 }, { x: 4, y: 3 }, { x: 3, y: 4 }], difficultTiles: [{ x: 2, y: 3 }, { x: 5, y: 3 }], captureOrbs, captureAttempts: 0, rewards: { xp: 0, loot: [], captured: [], defeated: [] } };
}

export function activeUnit(state: BattleState) { return state.units[state.turnIndex]; }
export function living(state: BattleState, team: Team) { return state.units.filter((unit) => unit.team === team && unit.hp > 0); }
export function isCoverTile(state: BattleState, x: number, y: number) { return state.coverTiles.some((tile) => tile.x === x && tile.y === y); }
export function isDifficultTile(state: BattleState, x: number, y: number) { return state.difficultTiles.some((tile) => tile.x === x && tile.y === y); }
export function isInsideGrid(state: BattleState, x: number, y: number) { return x >= 0 && x < state.grid.width && y >= 0 && y < state.grid.height; }
export function captureChance(unit: Unit) { const percent = (unit.hp / unit.maxHp) * 100; return percent <= 10 ? 100 : percent <= 25 ? 70 : percent <= 35 ? 45 : 0; }
function pushEvent(state: BattleState, event: BattleEvent) { state.events = [event, ...state.events].slice(0, 8); }
function occupied(state: BattleState, x: number, y: number, ignore?: Unit) { return state.units.some((unit) => unit !== ignore && unit.hp > 0 && unit.x === x && unit.y === y); }

function rewardEnemy(state: BattleState, enemy: Unit, captured: boolean) {
  if (enemy.rewarded) return; enemy.rewarded = true;
  const xp = Math.floor((enemy.xpValue ?? 0) * (captured ? 0.5 : 1)); state.rewards.xp += xp;
  if (captured && enemy.captureIdentity) state.rewards.captured.push(enemy.captureIdentity); else state.rewards.defeated.push(enemy.name);
  const addLoot = (loot?: Loot) => { if (!loot) return; const existing = state.rewards.loot.find((entry) => entry.id === loot.id); if (existing) existing.quantity += loot.quantity; else state.rewards.loot.push({ ...loot }); };
  addLoot(enemy.loot); if (captured) addLoot(enemy.captureReward);
  pushEvent(state, { message: `${captured ? 'Captured' : 'Defeated'} ${enemy.name}: +${xp} XP${enemy.loot ? `, ${enemy.loot.name}` : ''}.`, kind: 'reward' });
}
function checkWinner(state: BattleState) {
  if (state.winner) return;
  if (!living(state, 'enemy').length) { state.winner = 'player'; state.command = 'none'; pushEvent(state, { message: 'Victory. The path forward is clear.', kind: 'victory' }); }
  else if (!living(state, 'player').length) { state.winner = 'enemy'; state.command = 'none'; pushEvent(state, { message: 'Defeat. Restart the encounter to try again.', kind: 'defeat' }); }
}
function beginTurn(unit: Unit) { unit.moved = false; unit.actionUsed = false; unit.bonusUsed = false; unit.reactionReady = true; unit.focused = false; unit.defending = false; }
function advanceTurn(state: BattleState) {
  const start = state.turnIndex;
  for (let scan = 0; scan < state.units.length; scan += 1) {
    state.turnIndex = (state.turnIndex + 1) % state.units.length; const next = activeUnit(state);
    if (next?.hp > 0) { if (state.turnIndex <= start) state.round += 1; beginTurn(next); state.command = 'none'; pushEvent(state, { message: `${next.name}'s turn.`, kind: 'info' }); return; }
  }
}
function hasLineOfEffect(state: BattleState, from: Unit, to: Unit) {
  if (from.x !== to.x && from.y !== to.y) return true;
  const dx = Math.sign(to.x - from.x); const dy = Math.sign(to.y - from.y);
  for (let x = from.x + dx, y = from.y + dy; x !== to.x || y !== to.y; x += dx, y += dy) if (occupied(state, x, y, from)) return false;
  return true;
}
function tileCost(state: BattleState, x: number, y: number) { return isDifficultTile(state, x, y) ? 2 : 1; }
export function movementCost(state: BattleState, unit: Unit, destination: Tile) {
  const frontier: Array<{ x: number; y: number; cost: number }> = [{ x: unit.x, y: unit.y, cost: 0 }]; const costs = new Map([[key(unit.x, unit.y), 0]]);
  while (frontier.length) { frontier.sort((a, b) => a.cost - b.cost); const current = frontier.shift()!; if (current.x === destination.x && current.y === destination.y) return current.cost;
    for (const step of [{ x: 1, y: 0 }, { x: -1, y: 0 }, { x: 0, y: 1 }, { x: 0, y: -1 }]) { const x = current.x + step.x; const y = current.y + step.y; if (!isInsideGrid(state, x, y) || occupied(state, x, y, unit)) continue; const next = current.cost + tileCost(state, x, y); if (next < (costs.get(key(x, y)) ?? Infinity)) { costs.set(key(x, y), next); frontier.push({ x, y, cost: next }); } }
  } return Infinity;
}
export function validMoveTiles(state: BattleState, unit = activeUnit(state)): Tile[] { if (!unit || unit.moved || unit.hp <= 0 || state.winner) return []; const tiles: Tile[] = []; for (let x = 0; x < state.grid.width; x += 1) for (let y = 0; y < state.grid.height; y += 1) { const cost = movementCost(state, unit, { x, y }); if (cost > 0 && cost <= unit.moveRange) tiles.push({ x, y }); } return tiles; }
export function validAttackTargets(state: BattleState, unit = activeUnit(state)) { if (!unit || unit.actionUsed || state.winner) return []; return living(state, unit.team === 'player' ? 'enemy' : 'player').filter((target) => distance(unit, target) <= unit.attackRange && hasLineOfEffect(state, unit, target)); }
export function validMendTargets(state: BattleState, unit = activeUnit(state)) { if (!unit || unit.ability !== 'mend' || unit.actionUsed || state.winner) return []; return living(state, unit.team).filter((target) => target.hp < target.maxHp && distance(unit, target) <= 2 && hasLineOfEffect(state, unit, target)); }
export function validCaptureTargets(state: BattleState, unit = activeUnit(state)) { if (!unit || unit.team !== 'player' || unit.actionUsed || state.captureOrbs <= 0 || state.winner) return []; return living(state, 'enemy').filter((target) => distance(unit, target) <= 2 && hasLineOfEffect(state, unit, target) && captureChance(target) > 0); }
export function setCommand(state: BattleState, command: CommandType) { const current = activeUnit(state); if (state.winner || current?.team !== 'player') return false; if (command === 'move' && current.moved) return false; if (command === 'attack' && !validAttackTargets(state, current).length) return false; if (command === 'mend' && !validMendTargets(state, current).length) return false; if (command === 'capture' && !validCaptureTargets(state, current).length) return false; if (command === 'defend' && current.actionUsed) return false; if (command === 'focus' && (current.bonusUsed || current.bonusAbility !== 'focus')) return false; state.command = command; return true; }
export function move(state: BattleState, x: number, y: number) { const unit = activeUnit(state); if (!unit || state.winner || unit.team !== 'player' || unit.moved || !validMoveTiles(state, unit).some((tile) => tile.x === x && tile.y === y)) return false; unit.x = x; unit.y = y; unit.moved = true; state.command = 'none'; pushEvent(state, { message: `${unit.name} moves${isCoverTile(state, x, y) ? ' into cover' : ''}${isDifficultTile(state, x, y) ? ' through difficult ground' : ''}.`, kind: isDifficultTile(state, x, y) ? 'terrain' : 'info' }); return true; }
function applyDamage(state: BattleState, attacker: Unit, target: Unit) { const defended = target.defending ? 2 : 0; const cover = isCoverTile(state, target.x, target.y) ? 1 : 0; const focused = attacker.focused ? 1 : 0; const damage = Math.max(0, attacker.attack + focused - defended - cover); target.hp = Math.max(0, target.hp - damage); target.defending = false; attacker.focused = false; const modifiers = [focused ? 'Focus +1' : '', defended ? 'Defend −2' : '', cover ? 'cover −1' : ''].filter(Boolean).join(', '); pushEvent(state, { message: `${attacker.name} hits ${target.name} for ${damage}${modifiers ? ` (${modifiers})` : ''}.`, kind: 'damage' }); if (target.hp === 0) { pushEvent(state, { message: `${target.name} is defeated.`, kind: 'defeat' }); if (target.team === 'enemy') rewardEnemy(state, target, false); } checkWinner(state); }
export function previewAttackDamage(state: BattleState, target: Unit, attacker = activeUnit(state)) { if (!attacker) return 0; return Math.max(0, attacker.attack + (attacker.focused ? 1 : 0) - (target.defending ? 2 : 0) - (isCoverTile(state, target.x, target.y) ? 1 : 0)); }
export function attack(state: BattleState, targetId: string) { const attacker = activeUnit(state); const target = state.units.find((unit) => unit.id === targetId); if (!attacker || !target || attacker.team !== 'player' || !validAttackTargets(state, attacker).some((candidate) => candidate.id === targetId)) return false; attacker.actionUsed = true; state.command = 'none'; applyDamage(state, attacker, target); return true; }
export function mend(state: BattleState, targetId: string) { const healer = activeUnit(state); const target = state.units.find((unit) => unit.id === targetId); if (!healer || !target || healer.team !== 'player' || !validMendTargets(state, healer).some((candidate) => candidate.id === targetId)) return false; const restored = Math.min(4, target.maxHp - target.hp); target.hp += restored; healer.actionUsed = true; state.command = 'none'; pushEvent(state, { message: `${healer.name} restores ${restored} health to ${target.name}.`, kind: 'heal' }); return true; }
export function defend(state: BattleState) { const unit = activeUnit(state); if (!unit || state.winner || unit.team !== 'player' || unit.actionUsed) return false; unit.actionUsed = true; unit.defending = true; state.command = 'none'; pushEvent(state, { message: `${unit.name} braces for the next hit.`, kind: 'defense' }); return true; }
export function useFocus(state: BattleState) { const unit = activeUnit(state); if (!unit || state.winner || unit.team !== 'player' || unit.bonusUsed || unit.bonusAbility !== 'focus') return false; unit.bonusUsed = true; unit.focused = true; state.command = 'none'; pushEvent(state, { message: `${unit.name} focuses: next attack gains +1 damage.`, kind: 'info' }); return true; }
export function capture(state: BattleState, targetId: string) { const actor = activeUnit(state); const target = state.units.find((unit) => unit.id === targetId); if (!actor || !target || !validCaptureTargets(state, actor).some((candidate) => candidate.id === targetId)) return { ok: false, success: false, chance: 0, reason: 'Capture is not available.' }; const chance = captureChance(target); state.captureOrbs -= 1; state.captureAttempts += 1; actor.actionUsed = true; state.command = 'none'; const roll = (state.captureAttempts * 37 + [...target.id].reduce((sum, letter) => sum + letter.charCodeAt(0), 0)) % 100; const success = roll < chance; if (success) { target.hp = 0; rewardEnemy(state, target, true); pushEvent(state, { message: `Capture orb binds ${target.name}! (${chance}% chance)`, kind: 'capture' }); checkWinner(state); return { ok: true, success: true, chance, reason: 'Captured.' }; } pushEvent(state, { message: `Capture orb failed on ${target.name} (${chance}% chance, roll ${roll}).`, kind: 'capture' }); return { ok: true, success: false, chance, reason: 'The creature broke free.' }; }
function runEnemyTurn(state: BattleState) { const enemy = activeUnit(state); if (!enemy || enemy.team !== 'enemy' || state.winner) return; const players = living(state, 'player'); const target = players.sort((a, b) => a.hp / a.maxHp - b.hp / b.maxHp || distance(enemy, a) - distance(enemy, b) || a.id.localeCompare(b.id))[0]; if (!target) return; if (!validAttackTargets(state, enemy).length) { const moves = validMoveTiles(state, enemy); const next = moves.sort((a, b) => distance(a, target) - distance(b, target) || movementCost(state, enemy, a) - movementCost(state, enemy, b))[0]; if (next) { enemy.x = next.x; enemy.y = next.y; enemy.moved = true; pushEvent(state, { message: `${enemy.name} closes in.`, kind: 'info' }); } } const victim = validAttackTargets(state, enemy).sort((a, b) => a.hp / a.maxHp - b.hp / b.maxHp || a.id.localeCompare(b.id))[0]; if (victim) { enemy.actionUsed = true; applyDamage(state, enemy, victim); } }
export function endTurn(state: BattleState) { const current = activeUnit(state); if (state.winner || !current || current.team !== 'player') return false; pushEvent(state, { message: `${current.name} ends their turn.`, kind: 'info' }); advanceTurn(state); while (!state.winner && activeUnit(state)?.team === 'enemy') { runEnemyTurn(state); if (!state.winner) advanceTurn(state); } return true; }
