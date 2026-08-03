import { createTutorialBattle, type BattleState } from './battle';

export type Mode = 'town' | 'explore' | 'battle';

export interface WorldState {
  mode: Mode;
  townLevel: number;
  materials: number;
  discovered: string[];
  activeCompanion: 'Scout' | 'Mossling';
  hasAcceptedRequest: boolean;
  resourceCollected: boolean;
  encounterCleared: boolean;
  battle: BattleState | null;
  message: string;
}

export function createWorld(): WorldState {
  return {
    mode: 'town',
    townLevel: 1,
    materials: 0,
    discovered: ['Guardian'],
    activeCompanion: 'Scout',
    hasAcceptedRequest: false,
    resourceCollected: false,
    encounterCleared: false,
    battle: null,
    message: 'The town needs materials before its first workshop can open.',
  };
}

export function enterExplore(world: WorldState) {
  world.mode = 'explore';
  world.message = 'Moss Hollow is close enough for a short expedition. Look for resources and new companions.';
}

export function returnToTown(world: WorldState) {
  world.mode = 'town';
  world.message = world.encounterCleared
    ? 'The town feels different after your first victory. Spend the materials on a lasting improvement.'
    : 'Back home. Prepare your team, then choose what to pursue next.';
}

export function collectResource(world: WorldState) {
  if (world.resourceCollected) return;
  world.resourceCollected = true;
  world.materials += 3;
  world.message = 'You gathered 3 moonstone fragments. The town can put them to work.';
}

export function discoverCompanion(world: WorldState) {
  if (world.discovered.includes('Mossling')) return;
  world.discovered.push('Mossling');
  world.message = 'A Mossling joins your field notes. It may become a future team choice.';
}

export function setActiveCompanion(world: WorldState, companion: 'Scout' | 'Mossling') {
  if (companion === 'Mossling' && !world.discovered.includes('Mossling')) return;
  world.activeCompanion = companion;
  world.message = `${companion} is ready to join Guardian on the next expedition.`;
}

export function startEncounter(world: WorldState) {
  if (world.encounterCleared) {
    world.message = 'The hollow is quiet now. More regions will open in a larger slice.';
    return;
  }
  world.mode = 'battle';
  world.battle = createTutorialBattle(world.activeCompanion);
  world.message = 'A territorial pair blocks the path. Win the battle to bring its reward home.';
}

export function upgradeTown(world: WorldState) {
  if (world.townLevel >= 2 || world.materials < 3) return;
  world.materials -= 3;
  world.townLevel = 2;
  world.message = 'The field workshop is open. Future expeditions can turn rare materials into team upgrades.';
}

export function completeBattle(world: WorldState) {
  world.encounterCleared = true;
  world.materials += 2;
  world.message = 'Victory brought home 2 more materials. The town can now grow.';
}
