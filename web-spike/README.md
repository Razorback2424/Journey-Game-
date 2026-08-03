# JourneyGame web battle spike

This is a deliberately bounded vertical slice. It ports the battle concepts from the Unreal implementation into a TypeScript simulation and connects them to a small town, one exploration region, discovery, resource collection, and a visible town upgrade.

It is not a production migration and intentionally omits final art, save data, audio, broad progression, NPC schedules, and the full Unreal ruleset.

## Run

```sh
npm install
npm test
npm run dev
```

## Spike decision gate

Commit to the web route only if this sandbox remains easy to extend after adding the full command surface, the majority of battle simulation tests port cleanly, and the Unreal asset/editor work still looks materially larger. Otherwise, return to Unreal and finish the current vertical slice.

## Current playable loop

`Town → Moss Hollow → discover Mossling → collect Moonstone → tactical encounter → victory reward → return → build Field Workshop`
