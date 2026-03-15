# wlframe Editing Playbook

## Fast Start

1. Find related API in `include/wlf/**`.
2. Locate implementation in matching module directory.
3. Trace call sites with ripgrep before changing signatures.
4. Make minimal edits with clear ownership and error paths.
5. Build with Meson + Ninja and report verification scope.

## Review Fast Start

1. Read the public header before the implementation when exported behavior is involved.
2. Check object lifetime, cleanup order, and partially initialized failure paths.
3. Compare source changes against Meson source lists, platform guards, and obvious callers.
4. Report only real findings first; leave style nits for last.

## Common Task Patterns

### Add A New Public Function

1. Add declaration and docs to the corresponding header.
2. Implement in source with `wlf_<submodule>_` prefix.
3. Validate naming, nullability, and ownership semantics.
4. Update examples if behavior is user-visible.
5. Ensure symbol export expectations remain intact.

### Review A Public API Change

1. Compare old and new ownership, nullability, and argument ordering.
2. Check whether examples and internal users still match the contract.
3. Verify ABI-sensitive public structs or enums were not changed casually.
4. Call out missing documentation or missing symbol/export follow-up.

### Extend A Struct

1. Add field in public header only if ABI/public API needs it.
2. Initialize field in `*_init` and `*_create` paths.
3. Handle field teardown in `*_finish`/`*_destroy`.
4. Audit call sites for default behavior changes.
5. Re-check field order for hot-path locality, alignment, and grouping under `events` or `listeners` when relevant.

### Add A Derived Object Type

1. Embed the base object as the first field unless there is a stronger technical reason not to.
2. Add a `*_is_*()` helper for dynamic-type classification.
3. Add a `*_from_*()` helper that recovers the outer object with `wlf_container_of()`.
4. Do not use raw pointer casts for base-to-derived conversion.
5. Keep the helper naming consistent across the subsystem.

### Add Or Extend An Event Interface

1. Group owned signals under an `events` sub-struct.
2. Initialize each signal with `wlf_signal_init()` in the owning object's init/create path.
3. Set `listener.notify` before calling `wlf_signal_add()`.
4. Use `wlf_signal_emit_mutable()` for destroy or mutation-prone lifecycle events.
5. Remove externally attached listeners during teardown before freeing the owner.

### Add Or Extend Intrusive List Usage

1. Initialize every list head and intrusive node with `wlf_linked_list_init()` before use.
2. Insert nodes only after their ownership and teardown path are clear.
3. Remove linked nodes with `wlf_linked_list_remove()` before freeing the owning object.
4. Use `_safe` iteration macros when removal may happen during traversal.
5. Reinitialize a removed node before any later reuse.

### Add A New Source File

1. Place file in the owning module directory.
2. Add file to that module `meson.build` source list.
3. Export declarations in proper header if API is public.
4. Build to catch missing prototypes and link errors.

### Modify Platform-Dependent Code

1. Follow existing build-guard and dependency-boundary patterns.
2. Do not weaken dependency requirements silently.
3. Keep behavior parity where a cross-platform contract exists.

### Review A Failure Path

1. Enumerate resources in acquisition order.
2. Verify unwind happens in reverse order.
3. Check that cleanup helpers tolerate partially initialized state.
4. Confirm return value and logging behavior still tell the caller what failed.

### Review NULL Handling

1. Check whether `NULL` tolerance is part of the API contract or just defensive habit.
2. Allow silent `NULL` returns by default only in `*_destroy()`-style cleanup entry points.
3. For non-destroy functions, prefer explicit error reporting or a strict non-NULL contract.
4. Flag inconsistent lifecycle behavior, such as `destroy(NULL)` being allowed while sibling operations silently ignore programmer errors.

### Review Predicate Helpers

1. For `is_*()`, `has_*()`, and type-classification helpers, check whether they are supposed to require a valid object.
2. If the surrounding API expects a valid object, do not accept defensive `NULL` checks that collapse invalid usage into `false`.
3. Prefer direct return expressions such as `renderer->impl == &vk_renderer_impl` or `backend->type == WLF_BACKEND_WAYLAND`.
4. Flag helpers that mix classification logic with argument validation unless the API contract explicitly says nullable input is valid.

### Review Signal Usage

1. Check that every owned signal is initialized before first add or emit.
2. Check that listeners are attached only after `notify` is set.
3. Check whether callbacks can mutate listener state during emission.
4. Require `wlf_signal_emit_mutable()` for destroy and other mutation-prone emissions.
5. Check that teardown removes externally attached listener links before free.

### Review Linked-List Usage

1. Check that every list head and intrusive node is initialized before use.
2. Check that teardown removes linked nodes before object free.
3. Check that removal during iteration uses `wlf_linked_list_for_each_safe()` or the reverse-safe variant.
4. Check that removed nodes are not reused without `wlf_linked_list_init()`.
5. Check that no node is inserted into multiple lists.

### Review A Meson Change

1. Check new sources are added to the correct target.
2. Verify dependency-bound files are not pulled into the wrong build.
3. Confirm dependencies and feature guards still match the code.
4. Watch for build-only regressions such as missing headers or unresolved symbols.

## C Style Quick Rules

- Braces always required.
- Tabs for indentation.
- Keep helpers `static` unless exported.
- Prefer explicit cleanup labels for multi-step init failures.
- Use `bool` for success/failure in non-value-returning operations.
- Prefer obvious invariants over compact expressions.
- Avoid hiding ownership transitions behind helper macros.
- Only `*_destroy()` should be `NULL`-tolerant by default.
- Predicate/type-check helpers should usually assume non-NULL input and return the direct comparison.
- If a called API may return `NULL`, check it immediately before any dereference or dependent operation.
- If an allocation API returns `NULL`, log it with `wlf_log_errno()` before returning or entering cleanup.
- In polymorphic structs, put `impl` first by default.
- In derived structs, put the embedded base object first by default.
- Group multiple `wlf_signal` members under `events` and multiple `wlf_listener` members under `listeners`.
- Use `*_is_*()` plus `*_from_*()` helpers for base/derived conversions, and make `*_from_*()` use `wlf_container_of()`.
- Initialize owned signals with `wlf_signal_init()`, attach listeners with `wlf_signal_add()` after setting `notify`, and use `wlf_signal_emit_mutable()` when callbacks may mutate listener state.
- Initialize intrusive lists with `wlf_linked_list_init()`, remove nodes with `wlf_linked_list_remove()` before free, and use safe iteration when removing during traversal.

## Risk Hotspots

- Partial initialization leaks.
- Ownership mismatch between header docs and implementation.
- Silent API behavior changes without example updates.
- Build breakage from missing meson source registration.
- Symbol visibility or naming drift in public functions.
- Missing `NULL` checks after nullable API calls.
- Missing `wlf_log_errno()` after allocation failure.
- Struct layouts that bury hot fields behind cold metadata or flatten signals/listeners without grouping.
- Base-to-derived conversions implemented with raw casts instead of `wlf_container_of()`.
- Signals emitted before initialization, listeners added before `notify` is set, or mutation-prone emissions using `wlf_signal_emit()`.
- Linked-list nodes freed while still linked, removed during non-safe iteration, or reused without reinitialization.

## Review Output Template

For code review tasks, structure the response like this:

1. Findings with severity and file references.
2. Short note on assumptions or missing validation.
3. Brief summary only after findings.

Good finding shape:

- What changed.
- Why it is risky or incorrect.
- What breaks or may regress.
- Smallest reasonable fix.
