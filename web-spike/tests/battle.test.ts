import assert from 'node:assert/strict';
import test from 'node:test';
import { activeUnit, attack, capture, captureChance, createTutorialBattle, defend, endTurn, mend, move, setCommand, useFocus, validMoveTiles } from '../src/battle.ts';

test('creates deterministic teams, terrain, and a Guardian first turn', () => {
  const state = createTutorialBattle();
  assert.equal(activeUnit(state)?.id, 'guardian');
  assert.equal(state.coverTiles.length, 3);
  assert.equal(state.units.filter((unit) => unit.team === 'player').length, 2);
});

test('movement supports range two and preserves one action for the same turn', () => {
  const state = createTutorialBattle();
  assert.equal(setCommand(state, 'move'), true);
  assert.equal(move(state, 3, 2), true);
  assert.equal(activeUnit(state)?.id, 'guardian');
  assert.equal(activeUnit(state)?.moved, true);
  assert.equal(activeUnit(state)?.actionUsed, false);
  assert.equal(validMoveTiles(state).length, 0);
});

test('movement rejects blocked, occupied, and out-of-range destinations', () => {
  const state = createTutorialBattle();
  assert.equal(move(state, 6, 2), false);
  assert.equal(move(state, 1, 4), false);
  assert.equal(move(state, -1, 2), false);
});

test('attacks require range, reject invalid targets, and consume only the action', () => {
  const state = createTutorialBattle();
  assert.equal(attack(state, 'slime'), false);
  move(state, 3, 2);
  assert.equal(attack(state, 'slime'), false);
  endTurn(state);
  endTurn(state);
  assert.equal(activeUnit(state)?.id, 'guardian');
  assert.equal(attack(state, 'slime'), true);
  assert.equal(activeUnit(state)?.actionUsed, true);
  assert.equal(activeUnit(state)?.moved, false);
  assert.equal(attack(state, 'bat'), false);
});

test('Defend reduces exactly the next incoming hit and then expires', () => {
  const state = createTutorialBattle();
  state.units[1].hp = 0;
  state.units[3].hp = 0;
  state.units[2].x = 2; state.units[2].y = 2;
  assert.equal(defend(state), true);
  assert.equal(state.units[0].defending, true);
  endTurn(state);
  const guardian = state.units[0];
  assert.equal(guardian.defending, false);
  assert.equal(guardian.hp, 17);
});

test('cover reduces incoming damage by one', () => {
  const state = createTutorialBattle();
  state.units[1].hp = 0;
  state.units[3].hp = 0;
  const guardian = state.units[0];
  guardian.x = 4; guardian.y = 3;
  const slime = state.units[2];
  slime.x = 5; slime.y = 3;
  endTurn(state);
  assert.equal(guardian.hp, 16);
  assert.match(state.events.map((event) => event.message).join(' '), /cover/);
});

test('Mend validates selected targets, healing limits, and action consumption', () => {
  const state = createTutorialBattle('Mossling');
  state.units[0].hp = 10;
  endTurn(state);
  assert.equal(activeUnit(state)?.id, 'mossling');
  assert.equal(mend(state, 'guardian'), true);
  assert.equal(state.units[0].hp, 14);
  assert.equal(mend(state, 'guardian'), false);
  assert.equal(state.units[1].actionUsed, true);
});

test('difficult terrain costs extra movement while bonus Focus remains separate from the action', () => {
  const state = createTutorialBattle();
  assert.equal(move(state, 2, 3), false);
  assert.equal(useFocus(state), false);
  endTurn(state);
  assert.equal(activeUnit(state)?.id, 'scout');
  assert.equal(useFocus(state), true);
  assert.equal(activeUnit(state)?.bonusUsed, true);
  assert.equal(activeUnit(state)?.actionUsed, false);
});

test('capture uses health threshold, consumes an orb, and awards reduced XP on success', () => {
  const state = createTutorialBattle();
  const slime = state.units[2];
  slime.x = 2; slime.y = 2; slime.hp = 1;
  assert.equal(captureChance(slime), 100);
  const result = capture(state, 'slime');
  assert.equal(result.success, true);
  assert.equal(state.captureOrbs, 2);
  assert.deepEqual(state.rewards.captured, ['Moss Slime']);
  assert.equal(state.rewards.xp, 10);
});

test('capture fails deterministically when its success roll exceeds a weakened target chance', () => {
  const state = createTutorialBattle();
  const slime = state.units[2];
  slime.x = 2; slime.y = 2; slime.hp = 4;
  state.captureAttempts = 2;
  const result = capture(state, 'slime');
  assert.equal(result.ok, true);
  assert.equal(result.success, false);
  assert.equal(state.captureOrbs, 2);
  assert.equal(slime.hp, 4);
  assert.equal(activeUnit(state)?.actionUsed, true);
});

test('enemy moves toward nearest player, attacks, and defeated units are skipped', () => {
  const state = createTutorialBattle();
  endTurn(state); endTurn(state);
  assert.equal(activeUnit(state)?.id, 'guardian');
  assert.ok(state.units[2].x < 6 || state.units[3].x < 6);
  state.units[1].hp = 0;
  endTurn(state);
  assert.equal(activeUnit(state)?.id, 'guardian');
});

test('victory and defeat stop repeated commands', () => {
  const state = createTutorialBattle();
  state.units[2].hp = 0; state.units[3].hp = 0;
  state.units[2].x = 2; state.units[2].y = 2;
  state.units[3].x = 2; state.units[3].y = 2;
  assert.equal(attack(state, 'slime'), false);
  state.units[2].hp = 1; state.units[2].x = 2; state.units[2].y = 2;
  assert.equal(attack(state, 'slime'), true);
  assert.equal(state.winner, 'player');
  assert.equal(endTurn(state), false);
  assert.equal(move(state, 2, 3), false);
});
