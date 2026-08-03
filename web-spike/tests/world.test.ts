import assert from 'node:assert/strict';
import test from 'node:test';
import { collectResource, completeBattle, createWorld, discoverCompanion, enterExplore, setActiveCompanion, startEncounter, upgradeTown } from '../src/world.ts';

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
  completeBattle(world);
  assert.equal(world.encounterCleared, true);
  assert.equal(world.materials, 5);
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
