---
name: wlframe
description: "Use this skill when implementing, refactoring, or reviewing wlframe C code, headers, Meson files, or documentation. Best for code review, bug fixing, API changes, memory-safety work, rigorous systems-style C review standards, and review-ready patches."
---

# wlframe Core Development Skill

Apply this skill for any task that changes wlframe code under `curve/`, `buffer/`, `dmabuf/`, `image/`, `math/`, `platform/`, `renderer/`, `texture/`, `types/`, `utils/`, `wayland/`, `window/`, `include/wlf/`, examples, docs, and top-level `meson.build` files.

## Primary Goal

Produce minimal, correct, review-friendly patches that match wlframe's existing architecture, module boundaries, and rigorous systems-level review expectations:
- C11 + POSIX compatible code.
- K&R braces on the same line.
- Tabs for indentation in C sources and headers.
- Always use braces for all `if`, `for`, `while`, and `switch` blocks, matching `CONTRIBUTING.md`.
- Global symbols use `wlf_<submodule>_` prefix.
- Construction and destruction follow `init/create` and `finish/destroy` patterns.
- Prefer plain C over macro-heavy or typedef-heavy abstractions.
- Review output prioritizes correctness, memory ownership, lifetime, ABI, and build integration over style nits.

## Use This Skill To

- Implement new C functions or small subsystems in wlframe.
- Review patches for correctness, regressions, memory leaks, null handling, and API drift.
- Change public headers, exported symbols, Meson wiring, or platform guards.
- Refactor code to better match wlframe's existing module structure without changing behavior.
- Audit lifecycle code such as `init`, `create`, `finish`, `destroy`, attach, detach, map, unmap, or backend setup paths.

## Related Dependency Skills

- Use the `c` skill for plain-C API design, low-macro style, avoiding unnecessary `typedef struct`, memory allocation failure handling, `NULL` checks after nullable APIs, `wlf_log_errno()` usage, and cleanup-path discipline.
- Use the `struct-layout` skill for field ordering, alignment, hot-path locality, first-field `impl`, intrusive `link` placement, and `events`/`listeners` grouping.
- Use the `wayland` skill for `wl_display`, `wl_registry`, protocol versioning, listeners, and backend registry/output wiring.
- Use the `vulkan` skill for instance/device/renderer ownership, `VkResult` handling, extension checks, and Vulkan teardown order.
- Use the `pixman` skill for software rendering, format mapping, endianness, `pixman_image_t` lifetime, and buffer access rules.

## Core Mindset

Work like a strict C maintainer:
- Fix the root cause, not just the observed symptom.
- Prefer simple control flow and explicit cleanup over clever abstractions.
- Treat every ownership transition as review-critical.
- Treat public headers as contracts and examples as compatibility checks.
- For review tasks, list findings first, ordered by severity, with concrete file references.

## Source Of Truth

Follow these project files first:
- `CONTRIBUTING.md` for style, naming, lifecycle, commit guidance.
- `.clang-format` for formatting behavior.
- `meson.build` and subdir `meson.build` files for module dependencies and platform guards.
- Public headers in `include/wlf/**` for API contracts.

If instructions conflict, prefer explicit repository conventions over generic defaults.

Also borrow judgment from:
- wlframe's subsystem boundaries and explicit ownership.
- strict maintainer review habits: small diffs, clear invariants, obvious failure paths, and skepticism toward hidden side effects.

## Repository Map

- `include/wlf/**`: public API headers, module entry points, ABI-facing declarations.
- `curve/`: easing/curve implementations.
- `buffer/`: buffer abstraction and pixman backend integrations.
- `dmabuf/`: external buffer helpers and lifecycle management.
- `image/`: image loading and format-specific decoding.
- `math/`: vectors, matrices, geometry, and numerical helpers.
- `platform/`: backend abstraction and built-in backend wiring.
- `renderer/`: rendering abstraction plus backend-specific implementations.
- `texture/`: texture abstraction and backend-specific texture handling.
- `types/`: core value types such as color/output.
- `utils/`: logging, env, lists, parser, signal, time helpers.
- `wayland/`: wl_* wrappers and protocol-side integration.
- `window/`: window lifecycle and higher-level surface coordination.
- `examples/`: usage references; keep behavior aligned with API changes.

## Mandatory Coding Rules

### 1) Formatting

- Use tabs, not spaces, for indentation.
- Keep braces attached:
  - `if (...) {`
  - `for (...) {`
  - `void fn(void) {`
- Do not use single-line control statements without braces.
- Keep includes in intentional order; do not auto-sort unless the file already expects it.
- Keep lines readable; split long conditions instead of compressing them.

### 2) Naming And Symbol Scope

- Exported functions and types: `wlf_<submodule>_<name>`.
- Local static helpers: concise names, no `wlf_` prefix required.
- Prefer `static` for file-local functions and internal constants.
- New public API must be declared in the matching header under `include/wlf/`.
- Avoid introducing generic names that obscure subsystem ownership.

### 3) Object Lifecycle

- `*_init(obj)` initializes caller-owned memory and must start from zeroed state.
- `*_create(...)` allocates with `calloc`, initializes, and returns owned pointer.
- `*_finish(obj)` deinitializes but does not free caller-owned object.
- `*_destroy(obj)` accepts `NULL`, performs finish logic, then frees memory.
- Only `*_destroy(obj)` should silently accept `NULL` by default.
- Do not add `NULL`-tolerant early returns to ordinary operations such as `start`, `stop`, `update`, `render`, `commit`, or getters unless the existing API contract explicitly requires it.
- For non-destroy entry points, treat `NULL` as a contract violation: either validate and report failure, or require a non-NULL caller contract consistent with the surrounding API.
- On failure, unwind in reverse order of acquisition.
- Keep partially initialized objects valid enough for cleanup helpers.

### 4) Error Handling

- For functions without meaningful return values, use `bool` success/failure.
- Validate pointers and input ranges early.
- Prefer clear fast-fail checks over deep nesting.
- Log actionable errors using project logging helpers where appropriate.
- Do not silently swallow allocator, I/O, or backend initialization failures.
- Avoid defensive `if (ptr == NULL) { return; }` patterns in non-destroy functions when that would hide bugs or broken call paths.
- If an API call can return `NULL`, check the result immediately at the call site before dereferencing it, storing dependent state, or continuing initialization.
- Do not assume constructor, lookup, cast, bind, or wrapper-returning APIs succeed unless their contract guarantees a non-`NULL` result.
- On a possible-`NULL` API result, either fail fast with a clear cleanup path or propagate the failure explicitly.
- If `malloc()`, `calloc()`, `realloc()`, `strdup()`, or a similar allocation API returns `NULL`, log the failure with `wlf_log_errno(WLF_ERROR, ...)` before returning or unwinding.
- Allocation failure logs must identify the object or resource that failed to allocate.

### 4.1) Predicate And Type-Check Helpers

- Predicate helpers such as `is_*()`, `has_*()`, `can_*()`, or backend/type classifiers should usually require valid input and not special-case `NULL`.
- As a preferred direction, helpers such as `wlf_renderer_is_pixman(renderer)` should return the direct classification expression, such as `renderer->impl == &pixman_renderer_impl`, instead of adding defensive `NULL` checks.
- Do not turn programmer errors into a normal `false` result when that masks an invalid call path.
- Only add `NULL` handling to these helpers if the public API explicitly documents nullable input.

### 4.2) Base/Derived Helper Pattern

- When a base wlframe object is embedded inside a derived object type, provide both a `*_is_*()` predicate and a `*_from_*()` conversion helper.
- `*_is_*()` should validate the dynamic type through the normal dispatch/type mechanism.
- `*_from_*()` must use `wlf_container_of()` to recover the outer derived object from the embedded base member.
- Do not use raw pointer casts to convert a base pointer into a derived pointer.
- If the derived type depends on embedded-base recovery, keep the base object as the first field by default.

### 4.3) Signal And Listener Pattern

- Initialize every `wlf_signal` with `wlf_signal_init()` during object `init` or `create` before the signal can be used.
- Set `listener.notify` or `listener->notify` before registering the listener with `wlf_signal_add()`.
- Treat a `wlf_listener` as single-attachment state: one listener instance may listen to only one signal at a time.
- Remove listener links during teardown before freeing the owning object when those listeners are still attached to external signals.
- Use `wlf_signal_emit()` when callbacks are not expected to mutate the listener list during iteration.
- Use `wlf_signal_emit_mutable()` when callbacks may add/remove listeners, destroy the emitting object, or otherwise mutate signal/listener state during emission.
- Prefer `wlf_signal_emit_mutable()` for destroy-style events and other lifecycle edges where listener mutation is realistic.
- Group owned signals under an `events` sub-struct and subscribed listeners under a `listeners` sub-struct.

### 4.4) Linked-List Pattern

- Initialize every list head and every intrusive list node with `wlf_linked_list_init()` before first insertion or iteration.
- Treat intrusive `struct wlf_linked_list` members as owned lifecycle state, not as uninitialized padding.
- Remove a node with `wlf_linked_list_remove()` before freeing the object that owns it if that node may still be linked.
- After `wlf_linked_list_remove()`, treat the node as invalid until it is explicitly reinitialized with `wlf_linked_list_init()` before reuse.
- When removing elements while iterating, use the `_safe` iteration macros.
- Do not insert a node that is already linked into another list.
- If an object owns multiple list heads or intrusive nodes, initialize each one explicitly in the owning object's init/create path.

### 5) Memory And Ownership

- Define ownership transfer explicitly in API comments and call sites.
- Avoid leaking partially initialized objects on failure paths.
- Keep cleanup paths linear and obvious.
- Do not hide allocations in macros.
- Prefer a single cleanup path when several resources are acquired.
- Do not free caller-owned memory from `*_finish` helpers.

### 6) Macros

- Minimize macro usage when a function can express the logic.
- If macros are necessary, keep them local to the file and `#undef` when practical.
- Do not introduce opaque control-flow macros for normal runtime logic.
- Do not use macros to avoid writing ordinary readable C.

### 6.1) Typedef Use

- Do not introduce `typedef struct ...` aliases by default.
- Prefer explicit `struct foo` names in APIs and implementation code.
- Reserve typedefs for cases that materially improve clarity, such as callback signatures or domain-specific scalar aliases.

### 7) Documentation

- Use `/** ... */` for API declaration comments.
- Use `foo()` for function cross-references.
- For nullable fields, mark with `// may be NULL` near declaration when relevant.
- If a function has ownership, lifetime, thread, or backend assumptions, document them where declared.

### 8) Public API And ABI Discipline

- Do not change public struct layout or enum values casually.
- Preserve function names, argument order, and ownership semantics unless the task explicitly changes API.
- When API changes are required, update declaration, implementation, docs, examples, and any exported symbol lists together.
- Be conservative with inline helpers in public headers.
- Keep helper semantics sharp: a type-check helper should answer a type question, not double as a `NULL` guard.

### 8.1) Struct Layout Discipline

- Order struct fields based on semantics first, then alignment and hot-path locality.
- In polymorphic wlframe objects, put `impl` first by default unless there is a stronger technical reason not to.
- In derived wlframe objects that embed a base object, put the base object first by default.
- Put hot intrusive fields such as `link` early when traversal or membership is common.
- If a struct has multiple `wlf_signal` members, group them under `events`.
- If a struct has multiple `wlf_listener` members, group them under `listeners` by default.
- Keep cold descriptive strings and optional metadata away from the hottest control fields when practical.

### 9) Meson And Build Discipline

- Register new files in the owning module `meson.build`.
- Keep feature guards consistent with platform-specific dependencies.
- Do not introduce platform-bound headers or calls into shared code without guard review.
- Assume `werror=true`; warning-free changes are required.

## API Change Protocol

When changing public behavior:
- Update declaration and docs in `include/wlf/**` first.
- Update implementation in module source.
- Check all direct users in `examples/` and relevant submodules.
- Preserve ABI expectations and exported symbol naming.
- Avoid silently changing ownership semantics.

## Code Review Protocol

When asked to review code, default to a maintainer-style review:

1. Check correctness before style.
2. Check lifetime and ownership before local cleanliness.
3. Check public API and ABI impact before internal refactors.
4. Check Meson integration and platform guards before approving.
5. Mention missing tests or missing build validation if relevant.

Prioritize findings in this order:
- Memory safety, lifetime, double-free, leak, use-after-free, uninitialized state.
- API contract drift between headers, source, and call sites.
- Null handling, bounds checks, integer truncation, and invalid state transitions.
- Platform/build regressions, missing source registration, missing symbol export updates.
- Readability or maintainability issues that could hide future bugs.

Review comments should be concrete:
- Identify the risky behavior.
- Explain why it is wrong or brittle.
- State the likely consequence.
- Suggest the narrowest valid fix when obvious.

Pay special attention to `NULL` policy during review:
- `*_destroy()` may be `NULL`-tolerant for cleanup ergonomics.
- Other functions should not quietly return on `NULL` unless the API documents that behavior.
- Flag inconsistent `NULL` handling across the same lifecycle family as a review issue.
- Flag predicate helpers that return `false` on `NULL` when the surrounding API otherwise expects valid objects.
- Flag call sites that dereference or rely on API results without checking whether a nullable-returning API produced `NULL`.

Pay special attention to base/derived helper patterns during review:
- If a type extends a base object, check for both `*_is_*()` and `*_from_*()` helpers.
- Check that `*_from_*()` uses `wlf_container_of()` rather than raw casts.
- Check that embedded-base recovery relies on a stable first-field layout when the design expects it.

Pay special attention to signal usage during review:
- Check that every owned signal is initialized with `wlf_signal_init()` before first use.
- Check that listener callbacks are assigned before `wlf_signal_add()`.
- Check that teardown removes externally attached listeners before freeing the owner.
- Check that `wlf_signal_emit_mutable()` is used when callbacks may mutate listener state during emission.
- Check that stable, non-mutating notifications do not unnecessarily use a stronger emission mode without reason.

Pay special attention to linked-list usage during review:
- Check that list heads and intrusive nodes are initialized with `wlf_linked_list_init()` before use.
- Check that linked nodes are removed with `wlf_linked_list_remove()` before the owning object is freed.
- Check that removal during iteration uses the `_safe` traversal macros.
- Check that removed nodes are not reused without reinitialization.
- Check that a node is not inserted into more than one list at the same time.

## Change Workflow

1. Read the target module header and source before editing.
2. Implement the smallest viable patch.
3. Verify formatting and style conformance.
4. Build and validate affected targets.
5. Summarize behavior impact and risk in PR/commit text.

For non-trivial edits, also:
6. Trace direct call sites before changing signatures or ownership.
7. Re-check failure unwinding after the final code shape is in place.

## Validation Commands

Run from repository root:

```sh
meson setup build --prefix=/usr --buildtype=debug
ninja -C build
```

For existing build dir refreshes:

```sh
meson setup build --reconfigure
ninja -C build
```

If docs were touched and docs are enabled:

```sh
ninja -C build docs
```

If only a build refresh is needed, prefer the existing build directory instead of recreating it.

## Patch Review Checklist

Before finalizing a change, confirm:
- New code follows tabs + braces + naming rules.
- Public API edits are mirrored in headers and implementation.
- All failure paths release resources correctly.
- No GNU-only extension introduced accidentally.
- Build succeeds for the impacted configuration.
- Commit message explains what changed and why.
- Meson source lists and platform guards still match the code.
- Examples or obvious callers still reflect the updated behavior.

## Commit Message Pattern

Use English and keep subject concise:

```text
subsystem: short imperative summary

Explain why the change is needed and key design choices.

Signed-off-by: Your Name <your@email>
```

For issue-linked changes, include:

```text
issues: https://github.com/zzxyb/wlframe/issues/<id>
```

## Response Pattern For Coding Tasks

When using this skill for implementation tasks:
- Start with touched module(s) and behavior change.
- Mention API/ABI impact explicitly.
- Include exact build/test commands used.
- Call out any unverified paths or follow-up checks.

## Response Pattern For Review Tasks

When using this skill for review tasks:
- List findings first, highest severity first.
- Use file references for each finding.
- Keep summary short and secondary.
- If no findings are discovered, say so explicitly and note residual risk or missing validation.

See also:
- `ARCHITECTURE.md` for module boundaries and review priorities.
- `PLAYBOOK.md` for concrete implementation and review workflows.
