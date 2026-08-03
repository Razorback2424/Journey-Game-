export type Team = 'player' | 'enemy';
export type CommandType = 'attack' | 'move';

export interface Unit {
  id: string;
  name: string;
  team: Team;
  hp: number;
  maxHp: number;
  attack: number;
  ability?: 'mend';
  x: number;
  y: number;
  acted: boolean;
}

export interface BattleEvent {
  message: string;
  kind: 'info' | 'damage' | 'victory';
}

export interface BattleState {
  units: Unit[];
  turnIndex: number;
  round: number;
  winner: Team | null;
  events: BattleEvent[];
  command: CommandType;
}

export function createTutorialBattle(companion: 'Scout' | 'Mossling' = 'Scout'): BattleState {
  const companionUnit: Unit = companion === 'Mossling'
    ? { id: 'mossling', name: 'Mossling', team: 'player', hp: 14, maxHp: 14, attack: 3, ability: 'mend', x: 1, y: 4, acted: false }
    : { id: 'scout', name: 'Scout', team: 'player', hp: 12, maxHp: 12, attack: 4, x: 1, y: 4, acted: false };
  return {
    units: [
      { id: 'guardian', name: 'Guardian', team: 'player', hp: 18, maxHp: 18, attack: 5, x: 1, y: 2, acted: false },
      companionUnit,
      { id: 'slime', name: 'Moss Slime', team: 'enemy', hp: 14, maxHp: 14, attack: 3, x: 6, y: 2, acted: false },
      { id: 'bat', name: 'Cave Bat', team: 'enemy', hp: 10, maxHp: 10, attack: 4, x: 6, y: 4, acted: false },
    ],
    turnIndex: 0,
    round: 1,
    winner: null,
    events: [{ message: `${companion} joins Guardian for this encounter.`, kind: 'info' }],
    command: 'attack',
  };
}

export function activeUnit(state: BattleState): Unit | undefined {
  return state.units[state.turnIndex];
}

function living(state: BattleState, team: Team): Unit[] {
  return state.units.filter((unit) => unit.team === team && unit.hp > 0);
}

function pushEvent(state: BattleState, event: BattleEvent) {
  state.events = [event, ...state.events].slice(0, 5);
}

function checkWinner(state: BattleState) {
  if (living(state, 'enemy').length === 0) {
    state.winner = 'player';
    pushEvent(state, { message: 'Victory. The path forward is clear.', kind: 'victory' });
  } else if (living(state, 'player').length === 0) {
    state.winner = 'enemy';
    pushEvent(state, { message: 'Defeat. Restart the encounter to try again.', kind: 'victory' });
  }
}

function advanceTurn(state: BattleState) {
  const start = state.turnIndex;
  for (let scan = 0; scan < state.units.length; scan += 1) {
    state.turnIndex = (state.turnIndex + 1) % state.units.length;
    const next = activeUnit(state);
    if (next && next.hp > 0) {
      if (state.turnIndex <= start) state.round += 1;
      next.acted = false;
      return;
    }
  }
}

export function endTurn(state: BattleState) {
  if (state.winner) return;
  const current = activeUnit(state);
  if (!current) return;
  current.acted = true;
  advanceTurn(state);
  const next = activeUnit(state);
  if (next?.team === 'enemy') {
    const target = living(state, 'player')[0];
    if (target) {
      target.hp = Math.max(0, target.hp - next.attack);
      next.acted = true;
      pushEvent(state, { message: `${next.name} strikes ${target.name} for ${next.attack}.`, kind: 'damage' });
      checkWinner(state);
      if (!state.winner) advanceTurn(state);
    }
  }
}

export function setCommand(state: BattleState, command: CommandType) {
  if (activeUnit(state)?.team === 'player' && !state.winner) state.command = command;
}

export function move(state: BattleState, x: number, y: number) {
  if (state.winner) return false;
  const unit = activeUnit(state);
  const destinationOccupied = state.units.some((candidate) => candidate.hp > 0 && candidate.x === x && candidate.y === y);
  if (!unit || unit.team !== 'player' || destinationOccupied || Math.abs(unit.x - x) + Math.abs(unit.y - y) !== 1) return false;
  unit.x = x;
  unit.y = y;
  pushEvent(state, { message: `${unit.name} repositions.`, kind: 'info' });
  state.command = 'attack';
  endTurn(state);
  return true;
}

export function mend(state: BattleState) {
  if (state.winner) return false;
  const healer = activeUnit(state);
  if (!healer || healer.team !== 'player' || healer.ability !== 'mend') return false;
  const target = living(state, 'player').sort((a, b) => (a.hp / a.maxHp) - (b.hp / b.maxHp))[0];
  if (!target || target.hp === target.maxHp) return false;
  const restored = Math.min(4, target.maxHp - target.hp);
  target.hp += restored;
  pushEvent(state, { message: `${healer.name} restores ${restored} health to ${target.name}.`, kind: 'info' });
  endTurn(state);
  return true;
}

export function attack(state: BattleState, targetId: string) {
  if (state.winner) return;
  const attacker = activeUnit(state);
  const target = state.units.find((unit) => unit.id === targetId);
  if (!attacker || !target || attacker.team !== 'player' || target.team !== 'enemy' || target.hp <= 0) return;
  target.hp = Math.max(0, target.hp - attacker.attack);
  attacker.acted = true;
  state.command = 'attack';
  pushEvent(state, { message: `${attacker.name} hits ${target.name} for ${attacker.attack}.`, kind: 'damage' });
  checkWinner(state);
  if (!state.winner) endTurn(state);
}
