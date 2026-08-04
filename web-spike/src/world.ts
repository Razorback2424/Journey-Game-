import { createTutorialBattle, type BattleRewards, type BattleState, type Loot } from './battle';

export type Mode = 'town' | 'explore' | 'battle' | 'rewards';
export type CompanionName = 'Scout' | 'Mossling' | 'Moss Slime' | 'Cave Bat';
export interface Point { x: number; y: number; }
export interface WorldInteractable { id: 'trail' | 'mossling' | 'moonstone' | 'encounter'; label: string; position: Point; action: 'explore' | 'discover' | 'collect' | 'encounter'; }
export interface NavigationState { width: number; height: number; player: Point; destination: Point | null; path: Point[]; blocked: Point[]; focused: string | null; moving: boolean; }
export interface RewardSummary extends BattleRewards { outcome: 'victory' | 'defeat'; remainingOrbs: number; }
export interface WorldState {
  mode: Mode; townLevel: number; materials: number; discovered: string[]; activeCompanion: CompanionName;
  hasAcceptedRequest: boolean; resourceCollected: boolean; encounterCleared: boolean; battle: BattleState | null; message: string;
  teamXp: number; capturedRoster: string[]; captureOrbs: number; lootInventory: Loot[]; battleRewardHistory: RewardSummary[]; lastRewards: RewardSummary | null;
  navigation: NavigationState;
}
export const WORLD_SAVE_KEY = 'journeygame-web-spike-save-v1';
const townInteractables: WorldInteractable[] = [{ id: 'trail', label: 'Moss Hollow trail', position: { x: 6, y: 2 }, action: 'explore' }];
const exploreInteractables: WorldInteractable[] = [{ id: 'mossling', label: 'Mossling clearing', position: { x: 1, y: 1 }, action: 'discover' }, { id: 'moonstone', label: 'Moonstone vein', position: { x: 4, y: 1 }, action: 'collect' }, { id: 'encounter', label: 'Encounter gate', position: { x: 6, y: 3 }, action: 'encounter' }];
const blocked = [{ x: 0, y: 5 }, { x: 7, y: 0 }];
const key = (point: Point) => `${point.x},${point.y}`;
const same = (a: Point, b: Point) => a.x === b.x && a.y === b.y;
const inside = (navigation: NavigationState, point: Point) => point.x >= 0 && point.y >= 0 && point.x < navigation.width && point.y < navigation.height;
export function createWorld(): WorldState { return { mode: 'town', townLevel: 1, materials: 0, discovered: ['Guardian'], activeCompanion: 'Scout', hasAcceptedRequest: false, resourceCollected: false, encounterCleared: false, battle: null, message: 'Choose a destination in Hearthglen.', teamXp: 0, capturedRoster: [], captureOrbs: 3, lootInventory: [], battleRewardHistory: [], lastRewards: null, navigation: { width: 8, height: 6, player: { x: 2, y: 4 }, destination: null, path: [], blocked, focused: null, moving: false } }; }
export function restoreWorld(saved: string | null): WorldState {
  const fresh = createWorld();
  if (!saved) return fresh;
  try {
    const parsed = JSON.parse(saved) as Partial<WorldState>;
    if (!parsed || !['town', 'explore', 'battle', 'rewards'].includes(parsed.mode ?? '')) return fresh;
    return {
      ...fresh,
      ...parsed,
      discovered: Array.isArray(parsed.discovered) ? parsed.discovered : fresh.discovered,
      capturedRoster: Array.isArray(parsed.capturedRoster) ? parsed.capturedRoster : [],
      lootInventory: Array.isArray(parsed.lootInventory) ? parsed.lootInventory : [],
      battleRewardHistory: Array.isArray(parsed.battleRewardHistory) ? parsed.battleRewardHistory : [],
      navigation: { ...fresh.navigation, ...parsed.navigation, blocked: Array.isArray(parsed.navigation?.blocked) ? parsed.navigation.blocked : fresh.navigation.blocked },
    };
  } catch { return fresh; }
}
export function interactables(world: WorldState) { return world.mode === 'town' ? townInteractables : world.mode === 'explore' ? exploreInteractables.filter((item) => (item.id !== 'mossling' || !world.discovered.includes('Mossling')) && (item.id !== 'moonstone' || !world.resourceCollected) && (item.id !== 'encounter' || !world.encounterCleared)) : []; }
function pathTo(navigation: NavigationState, destination: Point) {
  const queue: Point[] = [navigation.player]; const previous = new Map<string, Point | null>([[key(navigation.player), null]]); const blockedSet = new Set(navigation.blocked.map(key));
  while (queue.length) { const current = queue.shift()!; if (same(current, destination)) { const path: Point[] = []; for (let cursor: Point | null = destination; cursor && !same(cursor, navigation.player); cursor = previous.get(key(cursor)) ?? null) path.unshift(cursor); return path; }
    for (const step of [{ x: 1, y: 0 }, { x: -1, y: 0 }, { x: 0, y: 1 }, { x: 0, y: -1 }]) { const next = { x: current.x + step.x, y: current.y + step.y }; if (inside(navigation, next) && !blockedSet.has(key(next)) && !previous.has(key(next))) { previous.set(key(next), current); queue.push(next); } }
  } return null;
}
export function setDestination(world: WorldState, destination: Point) {
  if (world.mode !== 'town' && world.mode !== 'explore') return false;
  if (!inside(world.navigation, destination) || world.navigation.blocked.some((tile) => same(tile, destination))) { world.message = 'That spot is blocked. Choose another destination.'; return false; }
  const path = pathTo(world.navigation, destination); if (!path) { world.message = 'No safe path reaches that location.'; return false; }
  world.navigation.destination = destination; world.navigation.path = path; world.navigation.focused = null; world.navigation.moving = path.length > 0; world.message = path.length ? `Walking to ${destination.x + 1}, ${destination.y + 1}.` : 'You are already there.'; focusIfNearby(world); return true;
}
export function moveToInteractable(world: WorldState, id: string) { const item = interactables(world).find((candidate) => candidate.id === id); if (!item) return false; return setDestination(world, item.position); }
export function advanceNavigation(world: WorldState) { if (!world.navigation.moving) return false; const next = world.navigation.path.shift(); if (!next) { world.navigation.moving = false; focusIfNearby(world); return false; } world.navigation.player = next; world.navigation.moving = world.navigation.path.length > 0; if (!world.navigation.moving) focusIfNearby(world); return true; }
function focusIfNearby(world: WorldState) { const item = interactables(world).find((candidate) => same(candidate.position, world.navigation.player)); if (item) { world.navigation.focused = item.id; world.message = `At ${item.label}. Interact when ready.`; } }
export function interactFocused(world: WorldState) { const item = interactables(world).find((candidate) => candidate.id === world.navigation.focused); if (!item || world.navigation.moving) return false; if (item.action === 'explore') enterExplore(world); if (item.action === 'discover') discoverCompanion(world); if (item.action === 'collect') collectResource(world); if (item.action === 'encounter') startEncounter(world); return true; }
function resetNavigation(world: WorldState, player: Point) { world.navigation = { ...world.navigation, player, destination: null, path: [], focused: null, moving: false }; }
export function enterExplore(world: WorldState) { world.mode = 'explore'; resetNavigation(world, { x: 2, y: 4 }); world.message = 'Tap or click a destination in Moss Hollow.'; }
export function returnToTown(world: WorldState) { world.mode = 'town'; resetNavigation(world, { x: 2, y: 4 }); world.message = world.encounterCleared ? 'Back home. Your new roster is ready for future expeditions.' : 'Back home. Choose where to walk next.'; }
export function collectResource(world: WorldState) { if (world.resourceCollected) return; world.resourceCollected = true; world.materials += 3; world.navigation.focused = null; world.message = 'You gathered 3 moonstone fragments. The town can put them to work.'; }
export function discoverCompanion(world: WorldState) { if (world.discovered.includes('Mossling')) return; world.discovered.push('Mossling'); world.navigation.focused = null; world.message = 'A Mossling joins your field notes. It may become a future team choice.'; }
export function setActiveCompanion(world: WorldState, companion: CompanionName) { if (companion === 'Mossling' && !world.discovered.includes('Mossling')) return; if ((companion === 'Moss Slime' || companion === 'Cave Bat') && !world.capturedRoster.includes(companion)) return; world.activeCompanion = companion; world.message = `${companion} is ready to join Guardian on the next expedition.`; }
export function startEncounter(world: WorldState) { if (world.encounterCleared) { world.message = 'The hollow is quiet now. More regions will open in a larger slice.'; return; } world.mode = 'battle'; world.navigation.moving = false; world.battle = createTutorialBattle(world.activeCompanion === 'Mossling' ? 'Mossling' : 'Scout', world.captureOrbs); world.message = 'A territorial pair blocks the path. Win the battle or capture the creatures to bring its reward home.'; }
export function upgradeTown(world: WorldState) { if (world.townLevel >= 2 || world.materials < 3) return; world.materials -= 3; world.townLevel = 2; world.message = 'The field workshop is open. Future expeditions can turn rare materials into team upgrades.'; }
export function completeBattle(world: WorldState) { const battle = world.battle; if (!battle || world.lastRewards || battle.winner !== 'player') return; world.encounterCleared = true; world.materials += 2; world.teamXp += battle.rewards.xp; world.captureOrbs = battle.captureOrbs; for (const captured of battle.rewards.captured) if (!world.capturedRoster.includes(captured)) world.capturedRoster.push(captured); for (const loot of battle.rewards.loot) { const existing = world.lootInventory.find((entry) => entry.id === loot.id); if (existing) existing.quantity += loot.quantity; else world.lootInventory.push({ ...loot }); } const summary: RewardSummary = { ...battle.rewards, loot: battle.rewards.loot.map((loot) => ({ ...loot })), outcome: 'victory', remainingOrbs: world.captureOrbs }; world.lastRewards = summary; world.battleRewardHistory.push(summary); world.mode = 'rewards'; world.message = 'The encounter is complete. Review the expedition rewards before heading home.'; }
