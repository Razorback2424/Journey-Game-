import { createTutorialBattle, type BattleRewards, type BattleState, type EncounterVariant, type Loot } from './battle';

export type Mode = 'town' | 'explore' | 'battle' | 'rewards';
export type CompanionName = 'Scout' | 'Mossling' | 'Moss Slime' | 'Cave Bat';
export type SceneId = 'town' | 'explore';
export type EncounterSource = 'gate' | 'wild';
export interface Point { x: number; y: number; }
export interface WorldInteractable { id: 'home' | 'workshop' | 'trail' | 'mossling' | 'moonstone' | 'encounter'; label: string; position: Point; approach: Point; action: 'rest' | 'workshop' | 'explore' | 'discover' | 'collect' | 'encounter'; }
export interface NavigationState {
  scene: SceneId; width: number; height: number; player: Point; visualPlayer: Point; destination: Point | null;
  path: Point[]; walkable: Point[]; focused: string | null; moving: boolean; progress: number; facing: 'left' | 'right';
}
export interface RewardSummary extends BattleRewards { outcome: 'victory' | 'defeat'; remainingOrbs: number; }
export interface WorldState {
  mode: Mode; townLevel: number; materials: number; discovered: string[]; activeCompanion: CompanionName;
  hasAcceptedRequest: boolean; resourceCollected: boolean; encounterCleared: boolean; battle: BattleState | null; message: string;
  teamXp: number; capturedRoster: string[]; captureOrbs: number; lootInventory: Loot[]; battleRewardHistory: RewardSummary[]; lastRewards: RewardSummary | null;
  encounterSteps: number; encounterSeed: number; encounterCount: number; activeEncounterSource: EncounterSource | null;
  navigation: NavigationState;
}

export const WORLD_SAVE_KEY = 'journeygame-web-spike-save-v1';
export const NAVIGATION_STEP_MS = 180;
const key = (point: Point) => `${point.x},${point.y}`;
const same = (a: Point, b: Point) => a.x === b.x && a.y === b.y;
const clone = (point: Point) => ({ ...point });
const pointRange = (from: number, to: number, y: number) => Array.from({ length: to - from + 1 }, (_, index) => ({ x: from + index, y }));
const townWalkable = [
  ...pointRange(12, 15, 3), ...pointRange(9, 15, 4), ...pointRange(8, 14, 5),
  ...pointRange(8, 13, 6), ...pointRange(8, 12, 7), ...pointRange(9, 12, 8), ...pointRange(9, 11, 9),
];
const exploreWalkable = [
  ...pointRange(4, 12, 2), ...pointRange(4, 15, 3), ...pointRange(5, 15, 4), ...pointRange(5, 16, 5),
  ...pointRange(5, 15, 6), ...pointRange(5, 15, 7), ...pointRange(6, 15, 8), ...pointRange(6, 14, 9),
];
const maps: Record<SceneId, { width: number; height: number; walkable: Point[]; spawn: Point }> = {
  town: { width: 20, height: 12, walkable: townWalkable, spawn: { x: 10, y: 9 } },
  explore: { width: 20, height: 12, walkable: exploreWalkable, spawn: { x: 7, y: 9 } },
};
const townInteractables: WorldInteractable[] = [
  { id: 'home', label: 'Hearthglen home', position: { x: 8, y: 8 }, approach: { x: 8, y: 7 }, action: 'rest' },
  { id: 'workshop', label: 'Field workshop', position: { x: 12, y: 8 }, approach: { x: 12, y: 7 }, action: 'workshop' },
  { id: 'trail', label: 'Moss Hollow trail', position: { x: 15, y: 3 }, approach: { x: 15, y: 4 }, action: 'explore' },
];
const exploreInteractables: WorldInteractable[] = [
  { id: 'mossling', label: 'Mossling clearing', position: { x: 5, y: 2 }, approach: { x: 5, y: 3 }, action: 'discover' },
  { id: 'moonstone', label: 'Moonstone vein', position: { x: 12, y: 2 }, approach: { x: 12, y: 3 }, action: 'collect' },
  { id: 'encounter', label: 'Encounter gate', position: { x: 16, y: 5 }, approach: { x: 15, y: 5 }, action: 'encounter' },
];

function navigationFor(scene: SceneId, player = maps[scene].spawn): NavigationState {
  const map = maps[scene];
  return { scene, width: map.width, height: map.height, player: clone(player), visualPlayer: clone(player), destination: null, path: [], walkable: map.walkable.map(clone), focused: null, moving: false, progress: 0, facing: 'right' };
}
function validPoint(navigation: NavigationState, point: Point) { return point.x >= 0 && point.y >= 0 && point.x < navigation.width && point.y < navigation.height; }
function isWalkable(navigation: NavigationState, point: Point) { return navigation.walkable.some((tile) => same(tile, point)); }
export function createWorld(): WorldState { return { mode: 'town', townLevel: 1, materials: 0, discovered: ['Guardian'], activeCompanion: 'Scout', hasAcceptedRequest: false, resourceCollected: false, encounterCleared: false, battle: null, message: 'Choose a destination in Hearthglen.', teamXp: 0, capturedRoster: [], captureOrbs: 3, lootInventory: [], battleRewardHistory: [], lastRewards: null, encounterSteps: 0, encounterSeed: 17, encounterCount: 0, activeEncounterSource: null, navigation: navigationFor('town') }; }
export function restoreWorld(saved: string | null): WorldState {
  const fresh = createWorld();
  if (!saved) return fresh;
  try {
    const parsed = JSON.parse(saved) as Partial<WorldState>;
    if (!parsed || !['town', 'explore', 'battle', 'rewards'].includes(parsed.mode ?? '')) return fresh;
    const scene: SceneId = parsed.mode === 'explore' ? 'explore' : 'town';
    const restored = navigationFor(scene, parsed.navigation?.player && isWalkable(navigationFor(scene), parsed.navigation.player) ? parsed.navigation.player : maps[scene].spawn);
    return { ...fresh, ...parsed, discovered: Array.isArray(parsed.discovered) ? parsed.discovered : fresh.discovered, capturedRoster: Array.isArray(parsed.capturedRoster) ? parsed.capturedRoster : [], lootInventory: Array.isArray(parsed.lootInventory) ? parsed.lootInventory : [], battleRewardHistory: Array.isArray(parsed.battleRewardHistory) ? parsed.battleRewardHistory : [], encounterSteps: typeof parsed.encounterSteps === 'number' ? parsed.encounterSteps : 0, encounterSeed: typeof parsed.encounterSeed === 'number' ? parsed.encounterSeed : 17, encounterCount: typeof parsed.encounterCount === 'number' ? parsed.encounterCount : 0, activeEncounterSource: parsed.activeEncounterSource === 'gate' || parsed.activeEncounterSource === 'wild' ? parsed.activeEncounterSource : null, navigation: restored };
  } catch { return fresh; }
}
export function interactables(world: WorldState) { return world.mode === 'town' ? townInteractables : world.mode === 'explore' ? exploreInteractables.filter((item) => (item.id !== 'mossling' || !world.discovered.includes('Mossling')) && (item.id !== 'moonstone' || !world.resourceCollected) && (item.id !== 'encounter' || !world.encounterCleared)) : []; }
function pathTo(navigation: NavigationState, destination: Point) {
  const queue: Point[] = [navigation.player]; const previous = new Map<string, Point | null>([[key(navigation.player), null]]);
  while (queue.length) {
    const current = queue.shift()!;
    if (same(current, destination)) { const path: Point[] = []; for (let cursor: Point | null = destination; cursor && !same(cursor, navigation.player); cursor = previous.get(key(cursor)) ?? null) path.unshift(cursor); return path; }
    for (const step of [{ x: 1, y: 0 }, { x: -1, y: 0 }, { x: 0, y: 1 }, { x: 0, y: -1 }]) { const next = { x: current.x + step.x, y: current.y + step.y }; if (validPoint(navigation, next) && isWalkable(navigation, next) && !previous.has(key(next))) { previous.set(key(next), current); queue.push(next); } }
  }
  return null;
}
export function snapToWalkable(navigation: NavigationState, requested: Point) {
  if (!validPoint(navigation, requested)) return null;
  if (isWalkable(navigation, requested)) return clone(requested);
  const candidates = navigation.walkable.map((point) => ({ point, distance: Math.abs(point.x - requested.x) + Math.abs(point.y - requested.y) }));
  const nearest = candidates.sort((a, b) => a.distance - b.distance)[0];
  return nearest && nearest.distance <= 2 ? clone(nearest.point) : null;
}
export function setDestination(world: WorldState, requested: Point) {
  if (world.mode !== 'town' && world.mode !== 'explore') return false;
  const destination = snapToWalkable(world.navigation, requested);
  if (!destination) { world.message = 'That terrain is impassable. Choose the road or a clear patch of ground.'; return false; }
  const path = pathTo(world.navigation, destination);
  if (!path) { world.message = 'No safe path reaches that ground.'; return false; }
  world.navigation.visualPlayer = clone(world.navigation.player); world.navigation.progress = 0; world.navigation.destination = destination; world.navigation.path = path; world.navigation.focused = null; world.navigation.moving = path.length > 0;
  world.message = path.length ? 'Guardian is moving across safe ground.' : 'Guardian is already there.'; focusIfNearby(world); return true;
}
export function moveToInteractable(world: WorldState, id: string) { const item = interactables(world).find((candidate) => candidate.id === id); return item ? setDestination(world, item.approach) : false; }
export function advanceNavigation(world: WorldState, elapsed = NAVIGATION_STEP_MS) {
  const navigation = world.navigation;
  if (!navigation.moving) return false;
  let remaining = elapsed; let changed = false;
  while (remaining > 0 && navigation.path.length) {
    const target = navigation.path[0]; const available = NAVIGATION_STEP_MS * (1 - navigation.progress); const consumed = Math.min(available, remaining); navigation.progress += consumed / NAVIGATION_STEP_MS; remaining -= consumed;
    const direction = target.x - navigation.player.x; if (direction) navigation.facing = direction < 0 ? 'left' : 'right';
    navigation.visualPlayer = { x: navigation.player.x + (target.x - navigation.player.x) * navigation.progress, y: navigation.player.y + (target.y - navigation.player.y) * navigation.progress }; changed = true;
    if (navigation.progress >= .999) { navigation.player = clone(target); navigation.visualPlayer = clone(target); navigation.path.shift(); navigation.progress = 0; if (world.mode === 'explore' && maybeStartWildEncounter(world)) { remaining = 0; break; } }
  }
  navigation.moving = navigation.path.length > 0;
  if (!navigation.moving) focusIfNearby(world);
  return changed;
}
function focusIfNearby(world: WorldState) { const item = interactables(world).find((candidate) => same(candidate.approach, world.navigation.player)); if (item) { world.navigation.focused = item.id; world.navigation.destination = null; world.message = `At ${item.label}. Interact when ready.`; } }
export function interactFocused(world: WorldState) { const item = interactables(world).find((candidate) => candidate.id === world.navigation.focused); if (!item || world.navigation.moving) return false; if (item.action === 'rest') world.message = 'The party rests at Hearthglen home. Everyone is ready for the next expedition.'; if (item.action === 'workshop') world.message = world.townLevel >= 2 ? 'Inside the field workshop, your expedition materials and creature roster are ready to prepare.' : world.materials >= 3 ? 'The workshop plans are ready. Build it from the field notes when you are ready.' : 'The workshop lot needs 3 Moonstone to open.'; if (item.action === 'explore') enterExplore(world); if (item.action === 'discover') discoverCompanion(world); if (item.action === 'collect') collectResource(world); if (item.action === 'encounter') startEncounter(world, 'gate'); return true; }
function resetNavigation(world: WorldState, scene: SceneId) { world.navigation = navigationFor(scene); }
export function enterExplore(world: WorldState) { world.mode = 'explore'; resetNavigation(world, 'explore'); world.message = 'Tap or click a clear route through Moss Hollow.'; }
export function returnToTown(world: WorldState) { world.mode = 'town'; resetNavigation(world, 'town'); world.message = world.encounterCleared ? 'Back home. Your new roster is ready for future expeditions.' : 'Back home. Choose where to walk next.'; }
export function collectResource(world: WorldState) { if (world.resourceCollected) return; world.resourceCollected = true; world.materials += 3; world.navigation.focused = null; world.message = 'You gathered 3 moonstone fragments. The town can put them to work.'; }
export function discoverCompanion(world: WorldState) { if (world.discovered.includes('Mossling')) return; world.discovered.push('Mossling'); world.navigation.focused = null; world.message = 'A Mossling joins your field notes. It may become a future team choice.'; }
export function setActiveCompanion(world: WorldState, companion: CompanionName) { if (companion === 'Mossling' && !world.discovered.includes('Mossling')) return; if ((companion === 'Moss Slime' || companion === 'Cave Bat') && !world.capturedRoster.includes(companion)) return; world.activeCompanion = companion; world.message = `${companion} is ready to join Guardian on the next expedition.`; }
function nextEncounterRoll(world: WorldState) { world.encounterSeed = (world.encounterSeed * 1664525 + 1013904223) >>> 0; return world.encounterSeed % 100; }
function maybeStartWildEncounter(world: WorldState) {
  world.encounterSteps += 1;
  if (world.encounterSteps < 3) return false;
  const forced = world.encounterSteps >= 7;
  if (!forced && nextEncounterRoll(world) >= 28) return false;
  startEncounter(world, 'wild');
  return true;
}
export function startEncounter(world: WorldState, source: EncounterSource = 'gate') {
  if (source === 'gate' && world.encounterCleared) { world.message = 'The hollow is quiet here, but wild creatures still roam the trails.'; return; }
  const variants: EncounterVariant[] = ['crossroads', 'slime-nest', 'bat-roost']; const variant = variants[(world.encounterSeed + world.encounterCount) % variants.length];
  world.mode = 'battle'; world.navigation.moving = false; world.navigation.path = []; world.navigation.destination = null; world.lastRewards = null; world.activeEncounterSource = source; world.encounterSteps = 0;
  world.battle = createTutorialBattle(world.activeCompanion, world.captureOrbs, variant);
  world.message = source === 'wild' ? 'A wild encounter interrupts your journey!' : 'A territorial pair blocks the path. Win the battle or capture the creatures to bring its reward home.';
}
export function upgradeTown(world: WorldState) { if (world.townLevel >= 2 || world.materials < 3) return; world.materials -= 3; world.townLevel = 2; world.message = 'The field workshop is open. Future expeditions can turn rare materials into team upgrades.'; }
export function completeBattle(world: WorldState) { const battle = world.battle; if (!battle || world.lastRewards || battle.winner !== 'player') return; if (world.activeEncounterSource === 'gate') world.encounterCleared = true; world.encounterCount += 1; world.materials += 2; world.teamXp += battle.rewards.xp; world.captureOrbs = battle.captureOrbs; for (const captured of battle.rewards.captured) if (!world.capturedRoster.includes(captured)) world.capturedRoster.push(captured); for (const loot of battle.rewards.loot) { const existing = world.lootInventory.find((entry) => entry.id === loot.id); if (existing) existing.quantity += loot.quantity; else world.lootInventory.push({ ...loot }); } const summary: RewardSummary = { ...battle.rewards, loot: battle.rewards.loot.map((loot) => ({ ...loot })), outcome: 'victory', remainingOrbs: world.captureOrbs }; world.lastRewards = summary; world.battleRewardHistory.push(summary); world.mode = 'rewards'; world.message = 'The encounter is complete. Review the expedition rewards before heading home.'; }
