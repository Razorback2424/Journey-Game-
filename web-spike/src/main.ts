import Phaser from 'phaser';
import { activeUnit, attack, endTurn, mend, move, setCommand } from './battle';
import { collectResource, completeBattle, createWorld, discoverCompanion, enterExplore, returnToTown, setActiveCompanion, startEncounter, upgradeTown, type WorldState } from './world';
import './style.css';

const world = createWorld();
const boardWidth = 800;
const boardHeight = 500;
const cell = 64;

class JourneyScene extends Phaser.Scene {
  private board!: Phaser.GameObjects.Graphics;
  private labels: Phaser.GameObjects.Text[] = [];
  private townBackdrop!: Phaser.GameObjects.Image;

  constructor() { super('journey'); }

  preload() {
    this.load.image('hearthglen-backdrop', '/assets/hearthglen-moss-hollow-concept.png');
  }

  create() {
    this.townBackdrop = this.add.image(boardWidth / 2, boardHeight / 2, 'hearthglen-backdrop').setDisplaySize(boardWidth, boardHeight).setAlpha(0.72);
    this.board = this.add.graphics();
    this.input.on('pointerdown', (pointer: Phaser.Input.Pointer) => this.handleCanvasClick(pointer.x, pointer.y));
    this.render();
  }

  update() { this.render(); }

  private handleCanvasClick(x: number, y: number) {
    if (world.mode === 'town') {
      if (x > 70 && x < 260 && y > 230 && y < 350) enterExplore(world);
    } else if (world.mode === 'explore') {
      if (!world.resourceCollected && x > 430 && x < 560 && y > 90 && y < 220) collectResource(world);
      else if (!world.discovered.includes('Mossling') && x > 100 && x < 240 && y > 90 && y < 220) discoverCompanion(world);
      else if (x > 570 && x < 760 && y > 270 && y < 430) startEncounter(world);
    } else if (world.mode === 'battle') {
      const state = world.battle;
      if (!state || state.winner) return;
      const gridX = Math.round((x - 48) / cell);
      const gridY = Math.round((y - 42) / cell);
      if (state.command === 'move' && move(state, gridX, gridY)) {
        renderHud(world);
        return;
      }
      const target = state.units.find((unit) => {
        const px = 48 + unit.x * cell;
        const py = 42 + unit.y * cell;
        return unit.team === 'enemy' && unit.hp > 0 && Math.abs(x - px) < 25 && Math.abs(y - py) < 25;
      });
      if (target) attack(state, target.id);
      if (state.winner === 'player') completeBattle(world);
    }
    renderHud(world);
  }

  private render() {
    this.board.clear();
    this.labels.forEach((label) => label.destroy());
    this.labels = [];
    this.townBackdrop.setVisible(world.mode === 'town');
    if (world.mode === 'town') this.renderTown();
    else if (world.mode === 'explore') this.renderExplore();
    else this.renderBattle();
  }

  private renderTown() {
    this.board.fillStyle(0x061316, 0.25).fillRect(0, 0, boardWidth, boardHeight);
    this.board.fillStyle(0x10292a, 0.78).fillRoundedRect(25, 378, 300, 87, 10);
    this.board.fillStyle(0x10292a, 0.78).fillRoundedRect(455, 378, 320, 87, 10);
    this.addLabel(45, 398, 'HEARTHGLEN HOME', '#f7d488');
    this.addLabel(45, 422, 'A safe return after every expedition.', '#d6ece0');
    this.addLabel(475, 398, world.townLevel >= 2 ? 'FIELD WORKSHOP OPEN' : 'WORKSHOP LOT', '#f7d488');
    this.addLabel(475, 422, world.townLevel >= 2 ? 'Materials can now strengthen your team.' : 'Bring Moonstone home to begin.', '#d6ece0');
  }

  private renderExplore() {
    this.board.fillStyle(0x1b3a43).fillRect(0, 0, boardWidth, boardHeight);
    this.board.fillStyle(0x2d5a4b).fillRect(0, 320, boardWidth, 180);
    for (let i = 0; i < 10; i += 1) {
      this.board.fillStyle(i % 2 ? 0x46745b : 0x3b684f).fillCircle(44 + i * 82, 350 + (i % 3) * 42, 26);
    }
    if (!world.discovered.includes('Mossling')) {
      this.board.fillStyle(0x78c8a4).fillCircle(164, 145, 28);
      this.addLabel(123, 190, 'Mossling?', '#b9f2c5');
    }
    if (!world.resourceCollected) {
      this.board.fillStyle(0x85d7da).fillCircle(494, 150, 30);
      this.board.fillStyle(0xdafcff).fillCircle(482, 140, 7);
      this.addLabel(437, 194, 'MOONSTONE', '#b9f2c5');
    }
    this.board.fillStyle(0x8f5c54).fillRect(625, 300, 120, 110);
    this.board.fillStyle(0xf1c37a).fillTriangle(612, 300, 758, 300, 685, 246);
    this.addLabel(624, 430, world.encounterCleared ? 'CLEARED PATH' : 'ENCOUNTER', '#f7d488');
  }

  private renderBattle() {
    this.board.fillStyle(0x101a24).fillRect(0, 0, boardWidth, boardHeight);
    for (let y = 0; y < 6; y += 1) for (let x = 0; x < 8; x += 1) {
      this.board.lineStyle(1, 0x33465a, 0.8).strokeRect(48 + x * cell, 42 + y * cell, cell, cell);
    }
    const state = world.battle;
    if (!state) return;
    const current = activeUnit(state);
    if (state.command === 'move' && current?.team === 'player') {
      [[current.x + 1, current.y], [current.x - 1, current.y], [current.x, current.y + 1], [current.x, current.y - 1]].forEach(([x, y]) => {
        const occupied = state.units.some((unit) => unit.hp > 0 && unit.x === x && unit.y === y);
        if (!occupied && x >= 0 && x < 8 && y >= 0 && y < 6) this.board.fillStyle(0x55c2a3, 0.22).fillRect(48 + x * cell + 3, 42 + y * cell + 3, cell - 6, cell - 6);
      });
    }
    state.units.forEach((unit) => {
      if (unit.hp <= 0) return;
      const px = 48 + unit.x * cell;
      const py = 42 + unit.y * cell;
      const color = unit.team === 'player' ? 0x55c2a3 : 0xe57777;
      this.board.fillStyle(color, 0.92).fillCircle(px, py, 22);
      if (activeUnit(state)?.id === unit.id) this.board.lineStyle(3, 0xf7d488).strokeCircle(px, py, 28);
      this.board.fillStyle(0x15202b).fillRect(px - 22, py + 29, 44, 5);
      this.board.fillStyle(0xf7d488).fillRect(px - 22, py + 29, 44 * unit.hp / unit.maxHp, 5);
      this.addLabel(px - 16, py - 8, unit.name.slice(0, 2), '#071117');
    });
  }

  private addLabel(x: number, y: number, text: string, color: string) {
    this.labels.push(this.add.text(x, y, text, { fontFamily: 'monospace', fontSize: '13px', color, fontStyle: 'bold' }));
  }
}

function renderHud(current: WorldState) {
  const state = current.battle;
  const title = current.mode === 'town' ? 'Hearthglen' : current.mode === 'explore' ? 'Moss Hollow' : 'First light at Moss Hollow';
  document.querySelector<HTMLHeadingElement>('#title')!.textContent = title;
  document.querySelector<HTMLDivElement>('#mode')!.textContent = current.mode === 'battle' ? `${activeUnit(state!)?.name ?? '—'} · ROUND ${state?.round ?? 1}` : current.mode.toUpperCase();
  document.querySelector<HTMLDivElement>('#materials')!.textContent = `${current.materials} MOONSTONE`;
  document.querySelector<HTMLDivElement>('#message')!.textContent = current.message;
  document.querySelector<HTMLDivElement>('#discovered')!.textContent = current.discovered.join(' · ');
  const actions = document.querySelector<HTMLDivElement>('#actions')!;
  if (current.mode === 'town') actions.innerHTML = `<button id="explore">Set out for Moss Hollow</button>${current.discovered.includes('Mossling') ? `<button id="companion">Use ${current.activeCompanion === 'Mossling' ? 'Scout' : 'Mossling'} next</button>` : ''}<button id="upgrade" ${current.materials < 3 || current.townLevel >= 2 ? 'disabled' : ''}>${current.townLevel >= 2 ? 'Workshop open' : 'Build field workshop · 3 moonstone'}</button>`;
  else if (current.mode === 'explore') actions.innerHTML = `<button id="return">Return to town</button><span class="hint">Click the Mossling, Moonstone, and encounter gate.</span>`;
  else actions.innerHTML = `<button id="attack" ${state?.winner ? 'disabled' : ''}>Attack</button><button id="move" ${state?.winner ? 'disabled' : ''}>Move</button>${activeUnit(state!)?.ability === 'mend' ? `<button id="mend" ${state?.winner ? 'disabled' : ''}>Mend ally</button>` : ''}<button id="end-turn" ${state?.winner ? 'disabled' : ''}>End turn</button><button id="return" ${state?.winner ? '' : 'disabled'}>Return to town</button><span class="hint">${state?.command === 'move' ? 'Click a highlighted adjacent tile.' : 'Click an enemy on the board to attack.'}</span>`;
  actions.querySelector('#explore')?.addEventListener('click', () => { enterExplore(current); renderHud(current); });
  actions.querySelector('#companion')?.addEventListener('click', () => { setActiveCompanion(current, current.activeCompanion === 'Mossling' ? 'Scout' : 'Mossling'); renderHud(current); });
  actions.querySelector('#upgrade')?.addEventListener('click', () => { upgradeTown(current); renderHud(current); });
  actions.querySelector('#attack')?.addEventListener('click', () => { if (state) setCommand(state, 'attack'); renderHud(current); });
  actions.querySelector('#move')?.addEventListener('click', () => { if (state) setCommand(state, 'move'); renderHud(current); });
  actions.querySelector('#mend')?.addEventListener('click', () => { if (state) mend(state); renderHud(current); });
  actions.querySelector('#return')?.addEventListener('click', () => { if (state?.winner === 'player') current.battle = null; returnToTown(current); renderHud(current); });
  actions.querySelector('#end-turn')?.addEventListener('click', () => { if (state) { endTurn(state); if (state.winner === 'player') completeBattle(current); } renderHud(current); });
}

document.querySelector<HTMLDivElement>('#app')!.innerHTML = `<section class="shell"><header><div><span class="kicker">JOURNEYGAME · VERTICAL SLICE</span><h1 id="title">Hearthglen</h1><p id="message"></p></div><div class="status"><div id="mode"></div><div id="materials"></div></div></header><div class="layout"><div id="game"></div><aside><div class="panel"><h2>Field notes</h2><div class="creatures" id="discovered"></div></div><div class="panel"><h2>What matters now</h2><div id="actions"></div></div></aside></div></section>`;

new Phaser.Game({ type: Phaser.AUTO, width: boardWidth, height: boardHeight, parent: 'game', backgroundColor: '#101a24', scene: JourneyScene, scale: { mode: Phaser.Scale.FIT, autoCenter: Phaser.Scale.CENTER_BOTH } });
renderHud(world);
