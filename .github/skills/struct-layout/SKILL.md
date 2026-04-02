---
name: struct-layout
description: "Use this skill when designing or reviewing C struct layout, field ordering, alignment, cache locality, hot-path access, intrusive list nodes, implementation pointers, signal/listener grouping, and ABI-sensitive object layout. Best for deciding where fields like impl, link, events, and listeners should live and why."
---

# C Struct Layout Skill

Apply this skill whenever a task involves C struct design, field ordering, alignment, cache locality, intrusive nodes, or public layout stability.

## Primary Goal

Produce struct layouts that are technically justified, not accidental:
- Keep alignment and padding sensible.
- Keep hot fields in the first cache line when practical.
- Keep type-erasure or dispatch fields such as `impl` in a predictable location when the design depends on them.
- Group related signals, listeners, and intrusive nodes so lifecycle and ownership remain obvious.

## Use This Skill To

- Design new public or private structs.
- Review whether field order causes avoidable padding or poor locality.
- Decide where to place `impl`, `link`, `events`, listeners, state flags, strings, and other metadata.
- Evolve wlframe structs without making the layout harder to reason about.

## Core Principles

- Start from semantics first, then improve layout for alignment and locality.
- Optimize the first cache line for fields touched on nearly every operation.
- Keep cold, descriptive, or optional data away from the hottest control fields when practical.
- Do not churn public struct layout casually; layout is part of the contract when exposed in headers.

## Mandatory Rules

### 1) Alignment And Padding

- Order fields with an eye toward natural alignment, but do not sacrifice readability without a real benefit.
- Prefer grouping same-size pointer fields, handle fields, and machine-word-sized scalars together when that reduces padding.
- Group smaller scalars such as enums, ints, shorts, and bool-like flags thoughtfully to avoid needless holes.
- Do not micro-optimize padding at the expense of a clear lifecycle model unless the struct is measurably hot or widely instantiated.

### 2) Hot And Cold Fields

- Put fields accessed on nearly every hot-path operation near the front of the struct when practical.
- Keep dispatch fields, frequently dereferenced pointers, and intrusive nodes that participate in hot iteration in the early part of the layout.
- Move cold strings, descriptions, debug-only data, or rarely touched configuration away from the first cache line when practical.
- Re-evaluate field order when profiling or usage patterns show that a supposedly cold field is actually hot.

### 3) `impl` Placement

- If a struct uses an implementation pointer such as `impl` for type dispatch, put it first by default.
- The first-field `impl` rule exists for predictable type identity, early dispatch, consistent casts, and fast access in generic helpers.
- Deviate from first-field `impl` placement only when there is a clear technical reason and the design still keeps dispatch obvious.
- Keep the technical reason visible in review: dispatch stability, common-prefix access, or ABI/layout consistency.

### 3.1) Base And Derived Struct Pattern

- When one wlframe struct embeds a base object and extends it with subtype-specific fields, the base struct must be the first field by default.
- The first-field base rule exists so the object has a stable common prefix, predictable dispatch, and a safe `wlf_container_of()` recovery path.
- For each derived type, provide both a predicate helper such as `wlf_texture_is_pixman()` and a conversion helper such as `wlf_pixman_texture_from_texture()`.
- `*_from_*()` helpers must recover the outer object with `wlf_container_of()`, not with raw pointer casts or manual address arithmetic.
- `*_from_*()` helpers should rely on a prior `*_is_*()` classification or equivalent invariant check before calling `wlf_container_of()`.
- Do not treat embedded-base layout as an excuse to cast between unrelated struct types directly.

### 4) Intrusive Nodes Such As `link`

- Place intrusive list or tree nodes such as `link` according to actual usage frequency, not habit.
- If the object is frequently iterated through intrusive containers, keep `link` in the hot part of the struct.
- If the node is only used for ownership bookkeeping and not touched on hot paths, it may live later in the struct.
- Keep node placement consistent across similar object families unless a measurable usage difference justifies divergence.

### 5) Grouping Signals And Listeners

- If a struct has multiple `wlf_signal` members, group them inside an `events` sub-struct.
- If a struct has multiple `wlf_listener` members, group them inside a dedicated sub-struct such as `listeners` by default.
- Use `listeners` when the struct subscribes to external events; use a different name only if the role is materially different and clearer.
- Keep each group cohesive: events for emitted signals, listeners for subscribed callbacks, links for intrusive container membership.

### 6) Naming And Semantic Buckets

- Prefer layout buckets that reveal intent, such as `events`, `listeners`, `state`, `pending`, `current`, or `base`.
- Do not scatter related lifecycle members across the struct without a reason.
- If a name like `slots` is less clear than `listeners`, prefer the clearer name.
- Keep naming consistent across similar wlframe object types.

### 7) Public ABI Discipline

- In public headers, changing field order is an ABI-sensitive change unless the struct is explicitly private or opaque.
- Do not reorder public struct fields just to shave a few bytes unless the benefit is real and the compatibility impact is acceptable.
- If a public struct must remain exposed, keep layout changes minimal and well-justified.

## wlframe-Oriented Layout Guidance

- `impl` should usually be the first field in polymorphic wlframe objects.
- When a wlframe object embeds a base object, that base should usually be the first field.
- `link` should be early when iteration, container membership checks, or manager traversal are hot.
- Multiple `wlf_signal` fields should live under `events`.
- Multiple `wlf_listener` fields should live under `listeners`.
- Human-readable strings and descriptive metadata are usually colder than dispatch, links, and event state.

## Code Review Protocol

Prioritize findings in this order:
- Layout choices that hide ownership, dispatch, or lifecycle relationships.
- Missing first-field `impl` where polymorphic dispatch depends on it.
- Missing first-field embedded base where subtype recovery depends on it.
- Missing `*_is_*()` or `*_from_*()` helpers for a derived object family.
- `*_from_*()` helpers that use casts instead of `wlf_container_of()`.
- Hot-path fields pushed behind cold metadata with no reason.
- Avoidable padding or poor grouping in heavily used structs.
- Mixed signals/listeners without `events` or `listeners` grouping.

Review questions to answer:
- Is `impl` first when the type uses polymorphic dispatch?
- Is the embedded base first when the type relies on base-to-derived recovery?
- Does the derived type provide `*_is_*()` and `*_from_*()` helpers?
- Does `*_from_*()` use `wlf_container_of()` rather than raw casting?
- Are hot fields near the front for the common access pattern?
- Is `link` placed according to actual usage frequency?
- Are multiple signals grouped under `events` and multiple listeners grouped under `listeners`?
- Is any field reordering worth the ABI or churn cost?

## Good Pattern

This is a hypothetical layout example for illustration, not a copy of a public wlframe ABI struct.

```c
struct wlf_example_output {
	const struct wlf_example_output_impl *impl;
	struct wlf_linked_list link;

	struct {
		struct wlf_signal destroy;
		struct wlf_signal name_change;
		struct wlf_signal model_change;
	} events;

	char *name;
	char *model;
};
```

## Response Pattern

For review tasks:
- Explain struct-layout findings in technical terms: dispatch, locality, padding, lifecycle clarity, or ABI impact.
- Do not argue for reordering purely on taste.
- Treat unjustified hot/cold mixing as a maintainability and performance issue, not just style.

See also:
- `PLAYBOOK.md` for concrete struct design and review workflows.