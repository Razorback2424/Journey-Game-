# JourneyGame - Stage 1 Overworld Setup (UE 5.7, Paper2D, Grid Movement)

This guide is written for basic Unreal Engine familiarity and follows the project decisions we already made:

- UE 5.7
- Pixel-perfect visuals
- Kenney roguelike transparent sheet
- 16x16 art, shown larger in-game
- Grid-based movement (one tile step at a time)
- Blueprint-only for Stage 1

## What has already been prepped in this project

Folders created:

- `Content/Art/Tiles/KenneyRoguelike`
- `Content/Data/Reference/KenneyRoguelike`
- `Content/Overworld/TileSets`
- `Content/Overworld/TileMaps`
- `Content/Overworld/Blueprints/Player`
- `Content/Overworld/Blueprints/Interactables`
- `Content/Overworld/Blueprints/Encounters`
- `Content/Overworld/Levels`

Kenney files copied in:

- `Content/Art/Tiles/KenneyRoguelike/roguelikeSheet_transparent.png`
- `Content/Art/Tiles/KenneyRoguelike/roguelikeSheet_magenta.png`
- `Content/Art/Tiles/KenneyRoguelike/spritesheetInfo.txt`

Kenney sheet slicing values:

- Tile Width: `16`
- Tile Height: `16`
- Margin: `1`
- Spacing: `1`

## Stage 1 plan (what you are building)

1. Enable Paper2D.
2. Import/setup the Kenney texture for pixel art.
3. Create a Tile Set with correct slicing.
4. Create a Tile Map and paint a small test overworld.
5. Enable tile collision.
6. Create a grid-based player Blueprint.
7. Switch to an orthographic camera.
8. Add one-step movement with collision checks.
9. Add simple interaction input.
10. Add an encounter test trigger volume.

## Recommended values (starting defaults)

- Target playtest resolution: `1280x720`
- Camera type: `Orthographic`
- TileSizeUU (one gameplay tile in Unreal units): `32`
- StepDuration: `0.14`

## Important guardrails (keep it stable)

- No gameplay movement logic on Tick.
- Move one tile at a time with a Timeline.
- Snap to exact grid after each step.
- Keep tilemap actor scale at `1,1,1`.

