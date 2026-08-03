# JourneyGame C++ and Blueprint Boundary

Use this rule for new gameplay code:

- C++ owns gameplay rules, state transitions, validation, reusable mechanics, and subsystem orchestration.
- Blueprints own visuals, audio, animation sequencing, one-off scripted moments, and per-instance tuning.

Practical rules:

- Do not put authoritative gameplay state machines in Blueprint graphs.
- Do not duplicate gameplay decision logic across multiple Blueprints.
- Use `BlueprintImplementableEvent` or `BlueprintNativeEvent` for visual reactions.
- Expose tunable values with `UPROPERTY`, keep runtime state internal or `VisibleInstanceOnly`.
- Keep data-driven content in Data Assets.

Safe migration checklist for a Blueprint:

1. Inspect the existing Blueprint graph and list its gameplay behaviors.
2. Add or extend a C++ parent class with equivalent tunables and events.
3. Compile before reparenting the Blueprint.
4. Reparent the Blueprint to the new C++ class.
5. Move only gameplay logic into C++ and keep visuals in Blueprint events.
6. Validate inherited defaults, collision settings, and instance behavior.
7. Run automation tests and a focused in-editor smoke test.
