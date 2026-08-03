import assert from 'node:assert/strict';
import test from 'node:test';
import { activeUnit, attack, createTutorialBattle, endTurn, mend, move, setCommand } from '../src/battle.ts';

test('creates the tutorial encounter with two teams and a deterministic first turn', () => {
  const state = createTutorialBattle();
  assert.equal(state.units.length, 4);
  assert.equal(activeUnit(state)?.id, 'guardian');
  assert.equal(state.units.filter((unit) => unit.team === 'player').length, 2);
  assert.equal(state.units.filter((unit) => unit.team === 'enemy').length, 2);
});

test('player attack resolves damage and advances through the enemy response', () => {
  const state = createTutorialBattle();
  attack(state, 'slime');
  assert.equal(state.units.find((unit) => unit.id === 'slime')?.hp, 9);
  assert.equal(state.units.find((unit) => unit.id === 'guardian')?.hp, 18);
  assert.equal(activeUnit(state)?.id, 'scout');
  assert.match(state.events[0].message, /hits/);
});

test('end turn resolves the enemy turn and returns control to the next player', () => {
  const state = createTutorialBattle();
  endTurn(state);
  assert.equal(state.units.find((unit) => unit.id === 'guardian')?.hp, 18);
  assert.equal(activeUnit(state)?.id, 'scout');
});

test('defeated enemies cannot be targeted and victory is recorded', () => {
  const state = createTutorialBattle();
  state.units.find((unit) => unit.id === 'slime')!.hp = 5;
  attack(state, 'slime');
  assert.equal(state.units.find((unit) => unit.id === 'slime')?.hp, 0);
  assert.equal(state.winner, null);
  const batBefore = state.units.find((unit) => unit.id === 'bat')!.hp;
  attack(state, 'slime');
  assert.equal(state.units.find((unit) => unit.id === 'bat')?.hp, batBefore);
});

test('movement accepts one empty adjacent tile and consumes the player turn', () => {
  const state = createTutorialBattle();
  setCommand(state, 'move');
  assert.equal(move(state, 2, 2), true);
  assert.deepEqual([state.units[0].x, state.units[0].y], [2, 2]);
  assert.equal(activeUnit(state)?.id, 'scout');
  assert.equal(move(state, 4, 2), false);
});

test('Mossling replaces Scout and can restore an injured ally', () => {
  const state = createTutorialBattle('Mossling');
  const guardian = state.units.find((unit) => unit.id === 'guardian')!;
  guardian.hp = 10;
  assert.equal(activeUnit(state)?.id, 'guardian');
  endTurn(state);
  assert.equal(activeUnit(state)?.id, 'mossling');
  assert.equal(mend(state), true);
  assert.equal(guardian.hp, 11);
});
