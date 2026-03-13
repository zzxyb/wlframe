---
name: wayland
description: "Use this skill when working on Wayland protocols, client or server integration, wl_display lifecycle, wl_registry binding, globals, surfaces, buffers, seats, outputs, xdg-shell, layer-shell, buffer-sharing extensions, shared memory, event loops, and protocol version negotiation. Best for broad Wayland implementation and code review, with wlframe-specific integration as a secondary concern."
---

# Wayland Protocol Skill

Apply this skill whenever a task involves Wayland concepts, even if the current wlframe codebase does not yet use that exact protocol or object family.

## Primary Goal

Produce protocol-correct, lifetime-safe Wayland code and reviews:
- Model Wayland objects by protocol contract, not by incidental current usage.
- Keep object ownership, listener lifetime, and event ordering explicit.
- Respect the async nature of Wayland requests, events, roundtrips, frame callbacks, and configure/ack cycles.
- Treat wlframe integration as one application of broader Wayland rules, not the source of those rules.

## Use This Skill To

- Implement or review Wayland client code using `wl_display`, `wl_registry`, `wl_compositor`, `wl_surface`, `wl_shm`, `wl_buffer`, `wl_seat`, `wl_pointer`, `wl_keyboard`, `wl_output`, and related objects.
- Work on shell protocols such as `xdg_wm_base`, `xdg_surface`, `xdg_toplevel`, popup flows, and output-management style extensions.
- Review buffer-sharing interop, buffer release semantics, frame callbacks, and presentation timing.
- Analyze event loops, dispatch/flush behavior, reconnect or teardown logic, and protocol version negotiation.
- Adapt wlframe to new Wayland protocols or capabilities in the future.

## Domain Coverage

This skill should be treated as broad Wayland guidance, including:
- Core protocol model: objects, requests, events, globals, interfaces, versions, and generated protocol bindings.
- Client lifecycle: connect, registry bind, initial sync, event dispatch, flush, teardown.
- Surface lifecycle: create, role assignment, attach, damage, commit, frame callbacks, enter/leave, release.
- Input and seats: capability discovery, pointer/keyboard/touch object lifetime, serial handling, focus transitions.
- Outputs and shell roles: output discovery, scale/transform, xdg-shell configure/ack sequences, popup grabs.
- Buffer transport: shm, shared or external buffer exchange, release events, explicit or implicit sync assumptions.
- Custom protocols: XML protocol generation, unstable protocol handling, version negotiation, extension fallbacks.

## Source Of Truth

Prefer authoritative Wayland protocol rules first, then adapt them to wlframe.

Within this repository, relevant integration points include:
- `wayland/wlf_wl_display.c`
- `wayland/wlf_wl_compositor.c`
- `wayland/wlf_wl_output.c`
- `platform/wayland/backend.c`
- `examples/wayland/**`
- top-level `meson.build` and `protocols/meson.build`

## wlframe Integration Notes

- wlframe uses generated Wayland protocol bindings and wrapper objects to track globals and protocol resources.
- Registry globals are tracked in a linked list of `wlf_wl_interface` objects.
- Backend startup discovers globals first, then creates compositor and output-manager wrappers.
- Output discovery currently bridges protocol globals into wlframe output objects and emits output-added signals.

## Mandatory Rules

### 1) Registry And Binding

- Always treat `wl_registry` discovery as dynamic; required interfaces may be missing.
- Bind protocol objects only after confirming the interface exists in the registry.
- Cap bind versions to the minimum of client-supported and compositor-advertised versions.
- If version fallback occurs, log it clearly when user-visible behavior may change.
- Do not assume unstable protocol availability across compositors or environments.

### 2) Object Lifetime

- Destroy Wayland wrapper objects in reverse order of acquisition.
- Remove listeners before destroying the object they target or depend on.
- A wrapper destroy function may accept `NULL`; normal protocol operations should not silently accept invalid objects.
- Do not leave stale `wl_listener` or wlframe signal links attached after stop/destroy.
- Respect protocol-owned lifetime transitions such as buffer release, surface role constraints, and seat capability removal.

### 3) Event And Dispatch Discipline

- Be explicit about when a roundtrip is required to populate registry state.
- Do not hide blocking `wl_display_roundtrip()` calls in places that should remain lightweight.
- Keep dispatch, flush, and roundtrip semantics separate in code and review comments.
- Avoid creating protocol objects before the registry state they depend on has been established.
- Respect configure/ack ordering, serial requirements, and frame callback usage for interactive surfaces.

### 4) Interface Matching

- Compare interface names by string content, not pointer identity, unless the API guarantees the same storage.
- Keep interface-specific wrappers narrow: compositor logic belongs in compositor helpers, output logic belongs in output helpers.
- Do not blur raw Wayland objects and wlframe wrappers in ownership decisions.
- Keep generated protocol bindings, handwritten wrappers, and higher-level state machines clearly separated.

### 5) Error Handling

- Missing required globals should produce a clear failure path.
- If protocol setup fails midway, unwind all previously created wrappers and listeners.
- Do not partially mark a backend as started before all required protocol objects are ready.
- Do not treat protocol errors as ordinary recoverable states unless the surrounding design explicitly supports recovery.

### 6) Buffering And Presentation

- Be explicit about who owns each `wl_buffer`, shm pool, external buffer handle, or backing storage block.
- Never assume attach implies immediate consumption; respect release events and compositor-driven lifetime.
- Keep damage, commit, scale, transform, and frame callback usage coherent with the surface update model.
- When using buffer-sharing or explicit-sync related protocols, make synchronization assumptions explicit in code and review.

### 7) Input And Shell Protocols

- Track seat capabilities dynamically; pointer, keyboard, and touch objects may appear or disappear at runtime.
- Preserve serial correctness for input-driven requests and configure acknowledgements.
- Do not commit role-specific state before the required configure/ack handshake is satisfied.
- Review popup, grab, and focus flows as state-machine problems, not just object creation problems.

## Code Review Protocol

Prioritize findings in this order:
- Wrong protocol lifetime or destroyed-object access.
- Version negotiation bugs and invalid registry binding.
- Broken configure/ack, serial, or frame-callback sequencing.
- Buffer release, shm, or external-buffer lifetime errors.
- Seat, input, or shell state-machine mistakes.
- Listener leaks, duplicate listeners, or forgotten unlinking.
- Blocking or misplaced roundtrip/dispatch behavior.
- Wrapper layering mistakes between `wayland/` and `platform/wayland/`.

Review questions to answer:
- Is every required global checked before use?
- Are bind versions safe for both client and compositor?
- Are configure/ack and serial-dependent requests ordered correctly?
- Does buffer lifetime respect release and backing-store ownership?
- Are listeners detached before dependent objects disappear?
- Does startup either complete fully or unwind cleanly?

## Validation Pointers

- When adding new protocol support, review generated bindings, XML protocol version, and feature fallback behavior together.
- When changing surface or buffer behavior, inspect commit ordering, release handling, and frame callback flow.
- Review Wayland dependency wiring and generated protocol integration together.
- Check `examples/wayland/wayland_registry_test.c` when changing registry behavior.
- When behavior changes, confirm wrapper headers and backend call sites still agree.

## Response Pattern

For review tasks:
- Lead with concrete protocol, lifetime, or sequencing findings.
- Mention registry, versioning, buffer lifetime, and shell/input state-machine issues when relevant.
- Keep summaries brief and secondary.

See also:
- `PLAYBOOK.md` for implementation and review checklists.