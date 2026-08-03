# JourneyGame — Vision Document

## One-sentence vision

JourneyGame is a turn-based RPG about building a team, exploring a dangerous world, making strategic choices, and bringing the rewards of adventure home to grow a town or city.

## Product hierarchy

The game has one primary identity and two supporting systems:

1. **Primary: turn-based RPG and team building.** The player builds a capable, personal team and makes meaningful decisions in battle.
2. **Supporting: exploration and discovery.** The player travels to find creatures, resources, characters, secrets, and strategic advantages.
3. **Supporting: town or city growth.** The player invests the rewards of exploration into a place that becomes more useful, expressive, and alive.

The town is not a separate life-simulation game competing for equal attention. It is the persistent payoff and strategic consequence of the RPG adventure. Exploration gives the player things to bring home; town growth gives those things lasting value; the improved town helps prepare the next expedition.

## Design north star

Every major system should strengthen the feeling that the player is building a team, learning a world, and making a place better through strategic adventure.

If a feature does not improve at least one of those three feelings, it should not be part of the first version.

## What the game needs to be compelling

### A battle system with real decisions

Battles must be more than selecting the strongest attack. Players need meaningful tradeoffs involving position, turn order, target priority, abilities, risk, terrain, and team composition. Outcomes must be understandable so players can learn from both victory and failure.

### A team the player cares about

Team building needs both strategic value and emotional identity. Creatures should have distinct roles, strengths, weaknesses, growth paths, and visual personalities. The player should be able to say why a particular team member belongs in the lineup.

### Exploration that produces meaningful discoveries

The world cannot be a backdrop between battles. Each region should contain creatures, resources, characters, secrets, or situations that make exploration worthwhile. Returning to an earlier region should sometimes reveal new possibilities rather than simply repeat the same content.

### Rewards with multiple uses

Loot should create decisions. A rare material might improve a creature, unlock equipment, fund a building, or support a town project. The player should sometimes choose between immediate team strength and long-term town growth.

### A town that visibly remembers the player

Town growth must be more than an upgrade menu. The player should see new buildings, services, residents, paths, decorations, or landmarks appear as a consequence of their actions.

### Strong pacing and contrast

The game needs a satisfying rhythm between calm preparation and dangerous missions, strategic battles and expressive exploration, immediate rewards and long-term investment, and personal team growth and communal town growth.

### Readability and polish

Players should quickly understand what they can do, why a result happened, what an item is useful for, what changed in town, and what opportunity is available next. Clarity matters more than feature count.

## The experience we are making

The player begins with a small home and a few trusted companions. They travel outward into a connected world, discover creatures and resources, meet memorable people, and bring what they learn back to town. The town becomes a visible record of the player’s journey: new buildings, workshops, characters, decorations, and services unlock as the player explores and helps the community.

When exploration leads to danger, the game shifts into a deliberate tactical battle. Position, terrain, turn order, abilities, equipment, and team composition matter. Battles should feel like a natural consequence of the adventure rather than a separate minigame.

The emotional rhythm is:

> Wonder → discovery → preparation → danger → mastery → return → growth

## Inspiration blend

The three reference points define different parts of the experience:

### Creature-adventure inspiration

This provides:

- A sense of journey and discovery.
- Creature collecting and team-building.
- Distinct areas with their own ecology, encounters, and secrets.
- A readable, approachable adventure structure.

JourneyGame should not copy existing creature names, characters, maps, story, or exact progression. The target is the feeling of building a personal relationship with a world and its inhabitants.

### Tactical-RPG inspiration

This provides:

- Turn-based battles with meaningful positioning.
- Terrain, range, movement, abilities, status effects, and team roles.
- Battles that reward planning rather than reflexes.
- Strong character identity expressed through abilities and battlefield behavior.

The battle system should be deep enough to support interesting decisions, but readable enough that a new player can understand why an outcome happened.

### Town-life inspiration

This provides:

- A welcoming home base.
- Construction, decoration, services, and visible community growth.
- Characters with routines, requests, relationships, and small stories.
- A restorative contrast to exploration and combat.

Town building is not an unrelated decoration mode. It is the persistent consequence of adventure: what the player finds and accomplishes changes the town. Its most important functions are to provide useful services, visible progress, and meaningful choices about where to invest limited rewards.

## Core pillars

### 1. Build a team worth thinking about

Creatures are not just statistical units. Each should have a distinct role, recognizable fantasy, strengths, limitations, viable growth choices, and a reason to use it. The player should feel attached to a small active team while still having reasons to collect and experiment with others.

### 2. Explore with purpose

Every region should offer a combination of:

- Creatures to discover.
- Resources or crafting materials.
- Characters and stories.
- Environmental puzzles or traversal challenges.
- Optional secrets.
- Encounters that test the player’s current team.

Exploration should create meaningful reasons to return to earlier places with new abilities, companions, or information.

### 3. Make battles tactical and legible

The battle system should make the player think about:

- Where units stand.
- Who acts next.
- Which target matters most.
- Whether to attack, defend, reposition, support, or take a risk.
- How the team’s abilities interact.

The first playable battle should be understandable without a wiki. Advanced depth should emerge from combinations, not opaque rules.

### 4. Let the town convert adventure into progress

Town growth should be visible, useful, and emotionally rewarding.

Examples:

- A rescued character opens a shop or service.
- A rare resource unlocks a workshop upgrade.
- A creature discovery changes the town’s habitat or museum.
- A completed regional quest adds a building, path, garden, or landmark.
- Decoration allows personal expression without blocking functional progression.

The town is the player’s anchor and the clearest visual proof that their actions matter. It should offer useful services, visible progress, and meaningful choices about where to invest limited rewards.

### 5. Protect warmth and wonder

The game may contain danger, failure, and serious stakes, but the overall tone should remain inviting. The player should want to return after a difficult battle because there is always a calm, rewarding place to recover, prepare, and plan.

## Core gameplay loop

1. Wake up in town and review available needs, requests, and opportunities.
2. Prepare the active creature team, equipment, items, and supplies.
3. Travel to a nearby region.
4. Explore, collect, interact, solve, and encounter creatures or threats.
5. Enter tactical battle when conflict occurs.
6. Earn experience, materials, creature discoveries, relationships, and story progress.
7. Return to town with new knowledge or resources.
8. Build, improve, decorate, trade, craft, or help residents.
9. Unlock a new route, creature, ability, building, or story thread.
10. Choose the next destination.

## The first vertical slice

The first slice should prove the complete fantasy in miniature, not attempt to represent the whole game.

It should contain:

- One compact home town.
- One nearby wilderness region.
- One player character.
- Three to five collectible creatures.
- A small active team limit.
- One friendly town character with a request.
- One resource-gathering activity.
- One simple town construction or upgrade.
- One tactical battle arena.
- Two player creatures and at least two enemy types in the battle.
- A clear victory, defeat, and return-to-town flow.

The slice is successful when a new player can understand the following without explanation:

> “I have a home, I can go explore, I can find companions and materials, danger creates tactical battles, and what I accomplish improves my town.”

## Battle direction

The battle system is a turn-based grid tactics game embedded in the adventure.

### Required first-layer mechanics

- Grid movement.
- Turn order.
- Movement range.
- Basic attack.
- At least one support or defensive action.
- Terrain or cover as a readable modifier.
- Health and defeat.
- Clear battle log or event feedback.
- Victory and retreat/defeat states.

### Later battle depth

- Creature-specific ability kits.
- Status effects.
- Reactions and opportunity attacks.
- Environmental hazards.
- Team synergies.
- Equipment and build choices.
- Multi-stage encounters.
- Boss behaviors.

The battle system should not begin with every advanced feature. Depth should be layered after the basic tactical decisions are fun and clear.

## Town direction

The town is a compact, authored space with player-directed growth rather than an unlimited simulation.

### Required first-layer mechanics

- A home or base area.
- A small number of buildable locations.
- Resource spending.
- At least one service unlocked through progress.
- Basic decoration or placement.
- NPC dialogue and one request.
- A visible before-and-after change.

### Later town depth

- Resident schedules.
- Friendship and relationship arcs.
- Seasonal changes.
- More advanced decoration.
- Production chains.
- Visitor events.
- Community projects.

Town systems should remain focused on meaningful choices and visible change. We are not trying to simulate every household activity.

## World structure

The world should be a collection of connected, memorable regions rather than a single empty open world.

Each region needs:

- A strong visual identity.
- A creature ecology.
- A resource profile.
- A traversal or interaction hook.
- A reason to revisit.
- A relationship to the town or story.

The initial architecture should favor compact maps with authored encounters and secrets. We can expand horizontally once the loop is proven.

## Progression

Progression should operate across four connected tracks:

- Creature growth: levels, abilities, traits, and team composition.
- Player capability: traversal, interaction, tools, and knowledge.
- Town growth: buildings, services, decoration, and residents.
- Story/world growth: new regions, factions, mysteries, and consequences.

These tracks should reinforce one another. A new town service should help exploration; exploration should provide materials for town growth; creature growth should open battle and traversal options.

## Visual and audio direction

The target is a highly readable, polished stylized 2D game with a strong sense of place. The vision does not require 3D. A 2D game can deliver the desired quality through excellent art direction, animation, composition, effects, sound, UI, and encounter design.

Priorities:

- Distinct silhouettes.
- Expressive creature poses and states.
- Clear grid readability during battle.
- Warm, inviting town colors.
- More varied and dangerous palettes in wilderness regions.
- Small environmental details that reward attention.
- UI that feels like part of the same world without obscuring play.

The exact art style is intentionally undecided. The reference to original Pokémon is about proving that strong adventure, collection, and RPG design do not depend on 3D—not a requirement to imitate its visual style.

The visual style should be chosen for consistency and production sustainability. A smaller number of excellent, reusable assets is better than a large collection of inconsistent placeholders.

## Product and platform direction

The first goal is a polished playable slice that can be shared easily and iterated quickly.

The initial implementation should prioritize:

- Desktop browser play.
- Keyboard and mouse support.
- Deterministic, testable simulation.
- Fast content iteration.
- Clear separation between simulation, rendering, UI, and data.

Mobile support can follow once the interaction model is proven. Native desktop or console packaging should remain possible, but it is not the first constraint unless the project’s release goal changes.

## Scope boundaries

We are not initially building:

- An MMO or always-online game.
- Multiplayer combat.
- A procedurally infinite world.
- A full life simulator.
- Hundreds of creatures before the core loop is proven.
- A cinematic AAA RPG.
- A complex economy or live-service system.
- A large branching narrative before the first region is fun.

## Design principles

- Make the player curious, then reward the curiosity.
- Prefer visible consequences over abstract rewards.
- Make rules explainable through play.
- Give every creature a reason to exist.
- Keep the town restorative, useful, and personal.
- Let combat create stories rather than interrupt them.
- Build small complete loops before adding breadth.
- Protect readability over feature count.

## Technical direction implied by the vision

The game should be designed around an engine-neutral simulation model:

- Simulation owns units, turns, encounters, progression, resources, and saveable state.
- Rendering owns sprites, animation, camera, particles, and effects.
- UI owns menus, HUD, dialogue, construction controls, and accessibility-sensitive text.
- Data files define creatures, abilities, encounters, maps, buildings, items, and residents.

This architecture supports a browser-first Phaser implementation while keeping the gameplay model portable if the project later moves to Unity or another runtime.

## Decision rule for the next phase

Do not expand the game until the first vertical slice proves the complete loop:

> Town → explore → discover/collect → tactical battle → return → improve town

The next implementation milestone should therefore be a complete, small vertical slice in the chosen runtime—not more isolated systems, more engine comparison, or a large asset library.

## Definition of success

JourneyGame is succeeding when players can describe a session in personal terms:

- “I found this creature and built my team around it.”
- “That battle was difficult because I positioned badly.”
- “I brought back what I needed to build something new.”
- “The town changed because of what I did.”
- “I want to go back out and see what else is there.”

That feeling—not the number of systems, maps, or assets—is the product we are building.
