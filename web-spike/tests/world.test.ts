import assert from 'node:assert/strict';
import test from 'node:test';
import { createTutorialBattle } from '../src/battle.ts';
import { advanceNavigation, collectResource, completeBattle, createWorld, discoverCompanion, enterExplore, interactables, moveToInteractable, restoreWorld, setActiveCompanion, setDestination, snapToWalkable, startEncounter, upgradeTown } from '../src/world.ts';

test('the vertical slice connects town, exploration, discovery, and resources', () => {
  const world = createWorld();
  enterExplore(world);
  collectResource(world);
  discoverCompanion(world);
  assert.equal(world.mode, 'explore');
  assert.equal(world.materials, 3);
  assert.deepEqual(world.discovered, ['Guardian', 'Mossling']);
});

test('encounter completion rewards the town and unlocks a visible upgrade', () => {
  const world = createWorld();
  enterExplore(world);
  collectResource(world);
  startEncounter(world);
  assert.equal(world.mode, 'battle');
  world.battle!.winner = 'player';
  world.battle!.rewards.xp = 20;
  world.battle!.rewards.captured.push('Moss Slime');
  world.battle!.rewards.loot.push({ id: 'moss-gel', name: 'Moss Gel', quantity: 1 });
  world.battle!.captureOrbs = 2;
  completeBattle(world);
  assert.equal(world.encounterCleared, true);
  assert.equal(world.materials, 5);
  assert.equal(world.teamXp, 20);
  assert.deepEqual(world.capturedRoster, ['Moss Slime']);
  assert.equal(world.captureOrbs, 2);
  assert.equal(world.mode, 'rewards');
  upgradeTown(world);
  assert.equal(world.townLevel, 2);
  assert.equal(world.materials, 2);
});

test('town upgrade cannot be purchased without enough materials', () => {
  const world = createWorld();
  upgradeTown(world);
  assert.equal(world.townLevel, 1);
  assert.equal(world.materials, 0);
});

test('a discovered companion can be selected for the next encounter', () => {
  const world = createWorld();
  discoverCompanion(world);
  setActiveCompanion(world, 'Mossling');
  enterExplore(world);
  startEncounter(world);
  assert.equal(world.battle?.units.some((unit) => unit.id === 'mossling'), true);
  assert.equal(world.battle?.units.some((unit) => unit.id === 'scout'), false);
});

test('saved progression restores after a browser refresh and malformed saves fall back safely', () => {
  const world = createWorld();
  collectResource(world);
  discoverCompanion(world);
  world.teamXp = 12;
  const restored = restoreWorld(JSON.stringify(world));
  assert.equal(restored.materials, 3);
  assert.deepEqual(restored.discovered, ['Guardian', 'Mossling']);
  assert.equal(restored.teamXp, 12);
  assert.equal(restoreWorld('{bad json').mode, 'town');
});

test('scene-aware navigation snaps a nearby click and rejects terrain behind buildings', () => {
  const world = createWorld();
  const snapped = snapToWalkable(world.navigation, { x: 14, y: 2 });
  assert.deepEqual(snapped, { x: 14, y: 3 });
  assert.equal(setDestination(world, { x: 5, y: 10 }), false);
  assert.match(world.message, /impassable/);
});

test('a new navigation target replaces the active route and reaches authored approaches', () => {
  const world = createWorld();
  assert.equal(moveToInteractable(world, 'trail'), true);
  const firstDestination = world.navigation.destination;
  assert.ok(firstDestination);
  assert.equal(setDestination(world, { x: 12, y: 5 }), true);
  assert.deepEqual(world.navigation.destination, { x: 12, y: 5 });
  assert.notDeepEqual(world.navigation.destination, firstDestination);
  for (let index = 0; index < 40; index += 1) advanceNavigation(world, 180);
  assert.deepEqual(world.navigation.player, { x: 12, y: 5 });
  enterExplore(world);
  for (const item of interactables(world)) {
    assert.equal(moveToInteractable(world, item.id), true, `${item.id} should be reachable`);
    for (let index = 0; index < 60; index += 1) advanceNavigation(world, 180);
    assert.deepEqual(world.navigation.player, item.approach, `${item.id} should end at its approach point`);
  }
});

test('exploration can trigger repeatable wild encounters after movement', () => {
  const world = createWorld();
  enterExplore(world);
  assert.equal(setDestination(world, { x: 15, y: 5 }), true);
  for (let index = 0; index < 20 && world.mode === 'explore'; index += 1) advanceNavigation(world, 180);
  assert.equal(world.mode, 'battle');
  assert.equal(world.activeEncounterSource, 'wild');
  world.battle!.winner = 'player';
  completeBattle(world);
  assert.equal(world.encounterCount, 1);
  assert.equal(world.mode, 'rewards');
});

test('captured creature companions have distinct tactical roles', () => {
  const slime = createTutorialBattle('Moss Slime').units.find((unit) => unit.id === 'moss-slime')!;
  const bat = createTutorialBattle('Cave Bat').units.find((unit) => unit.id === 'cave-bat')!;
  assert.equal(slime.maxHp, 22);
  assert.equal(slime.ability, 'mend');
  assert.equal(bat.moveRange, 3);
  assert.equal(bat.attackRange, 3);
});
