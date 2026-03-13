# wlframe Module Architecture Notes

## Dependency Shape (High Level)

- Public API roots in `include/wlf/**`.
- Utility and math modules provide foundational helpers.
- Platform and wayland modules integrate OS/protocol specifics.
- Renderer, buffer, and dmabuf build graphics/data path layers.
- Examples consume exported interfaces and act as usage validation.

## Practical Boundaries

- Keep `utils/` generic and low-level.
- Keep `math/` deterministic and side-effect minimal.
- Keep `types/` focused on data contracts.
- Keep `platform/` and `wayland/` explicit about backend assumptions.
- Keep `renderer/` backend-specific logic separated by subdir.

## API Hygiene

- Avoid exposing private structs when opaque handles suffice.
- Prefer stable parameter ordering and explicit ownership docs.
- Keep include dependencies minimal to reduce transitive coupling.

## Build And Platform Facts

- Top-level project enables strict warnings and `werror=true`.
- Different build targets may enable different dependency sets and backend integrations.
- Platform-specific dependency wiring must stay isolated from shared interfaces and generic modules.

## Review Priorities

1. Correctness and memory safety.
2. API consistency with existing module naming/lifecycle.
3. Build integration and platform guard correctness.
4. Readability and maintainability under wlroots-like style.
