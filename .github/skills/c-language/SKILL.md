---
name: c
description: "Use this skill when writing or reviewing C code for simple, readable, cross-platform systems libraries. Best for allocation failure handling, ownership, cleanup paths, Linux-kernel-inspired C clarity, low-macro design, avoiding typedef struct aliases, and maintainable API-oriented C code."
---

# C Systems Style Skill

Apply this skill whenever a task involves C API design, struct design, allocation, ownership transfer, error handling, or multi-step resource initialization.

## Primary Goal

Produce C code that is easy to read, easy to review, and easy to maintain over time:
- Prefer straightforward control flow over clever abstraction.
- Keep APIs explicit and cross-platform friendly.
- Minimize preprocessor magic.
- Avoid `typedef struct foo { ... } foo;`-style aliasing unless there is a compelling API reason.
- Prefer ordinary declarations over macro-generated declarations.
- Prefer `enum`, `static const`, and normal helper functions over `#define` when they express the intent more clearly.
- If memory allocation may fail, the call site must check it immediately.
- Allocation failure in wlframe-style code must log with `wlf_log_errno()` before returning or unwinding.
- Partial initialization must unwind in reverse order without leaking resources.
- Ownership transfer and nullable-return contracts must remain obvious.

## Design Goal

Use this skill to push the codebase toward a simple, readable cross-platform UI library:
- Public APIs should read like plain C, not macro DSLs.
- Type names, ownership, and lifecycle should be visible directly in the code.
- Common cases should be obvious from the first read.
- Platform-specific details should stay behind narrow interfaces.
- Header files should stay boring in the good sense: clear declarations, small contracts, little preprocessor noise.
- The reader should not need to mentally expand layers of macros or aliases to understand a call site or struct definition.

## Use This Skill To

- Design or review plain-C APIs for maintainability and clarity.
- Implement or review code using `malloc()`, `calloc()`, `realloc()`, `strdup()`, `strndup()`, `aligned_alloc()`, and similar allocation APIs.
- Audit construction paths that allocate several resources in sequence.
- Review `NULL` handling after factory, wrapper, bind, or lookup APIs.
- Tighten error paths in C code to be maintainer-friendly and reviewable.
- Reduce unnecessary macro and typedef indirection.

## Core Style

Borrow the strongest parts of Linux-kernel-style C judgment without copying kernel-specific conventions blindly:
- Prefer explicit data flow and obvious invariants.
- Use small helpers instead of macro metaprogramming.
- Keep naming concrete and subsystem-oriented.
- Separate hot logic, slow paths, and cleanup paths clearly.
- Make invalid states hard to express and easy to spot in review.
- Prefer direct struct access and explicit ownership over abstract handle indirection unless the abstraction buys something real.
- Make the normal path read top-to-bottom with minimal hidden machinery.

## Mandatory Rules

### 1) Plain C Over Clever C

- Prefer ordinary functions, structs, and enums over macro-heavy abstractions.
- Avoid creating mini-frameworks with `#define` when a normal helper function is clearer.
- Use macros primarily for compile-time constants, narrow utility wrappers, or unavoidable language gaps.
- If a macro hides control flow, allocation, ownership, or side effects, it is usually the wrong tool.

### 1.1) Prefer `enum` And `static const` Over `#define`

- Use `enum` for related integral constants when the values belong to one conceptual set.
- Use `static const` objects for typed constants when a macro would erase type information.
- Do not use `#define` for values that should participate in the type system.
- Keep macros for header guards, feature toggles, compile-time configuration, and truly unavoidable language-level needs.

### 2) Avoid `typedef struct` Aliasing By Default

- Prefer `struct wlf_output`, `struct wlf_renderer`, and similar explicit type names instead of aliasing them away with `typedef`.
- Do not introduce `typedef struct foo foo;` just to save a few characters.
- Keep pointer-ness visible in declarations; do not use typedefs that hide whether something is a pointer.
- Only use typedefs when wrapping function-pointer signatures, scalar aliases with real domain meaning, or other cases where they materially improve the API.

### 2.1) Keep Headers Easy To Read

- A public header should usually be understandable without jumping through macro layers.
- Prefer explicit forward declarations and explicit `struct` names over alias networks.
- Avoid declaration styles that make it hard to tell which identifiers are types, pointers, callbacks, or ownership-bearing objects.

### 3) Cross-Platform Boundary Discipline

- Keep public APIs platform-neutral unless platform specificity is the point of that API.
- Do not leak platform headers, platform types, or platform assumptions into generic interfaces without need.
- Isolate platform-specific behavior behind clearly named helpers, backends, or implementation structs.
- Keep cross-platform contracts simple even when per-platform implementations differ.

### 3.1) Keep Preprocessor Scope Narrow

- Keep `#if`, `#ifdef`, and feature guards as local as possible.
- Prefer isolating platform divergence in source files or backend-specific modules instead of sprinkling preprocessor branches through generic headers.
- Do not let compile-time branches fragment the normal reading flow of core library code unless there is no cleaner boundary.

### 4) Allocation Failure Checks

- Check the result of `malloc()`, `calloc()`, `realloc()`, `strdup()`, and similar nullable-returning allocation APIs immediately.
- Do not dereference, assign dependent fields, register listeners, or continue initialization before the `NULL` check.
- In wlframe code, allocation failure must call `wlf_log_errno(WLF_ERROR, ...)` with a specific message before returning `NULL` or unwinding.
- Make the failure message name the object or resource that failed to allocate.

### 5) `realloc()` Discipline

- Never overwrite the original pointer with the result of `realloc()` before checking for `NULL`.
- Use a temporary pointer for `realloc()` results.
- Preserve the original allocation on failure and either unwind or propagate the error explicitly.

### 6) Constructor And Factory Paths

- For functions that allocate and initialize objects, fail fast after the first allocation error.
- If later steps fail, release everything already acquired in reverse order.
- Keep partially initialized objects valid enough for a destroy helper or explicit cleanup path.
- Do not leave hidden ownership transfers undocumented.

### 7) Nullable API Results

- Treat any API that may return `NULL` as a failure point that must be checked at the call site.
- If the failure is allocator-backed or errno-bearing, log with `wlf_log_errno()` where wlframe logging is available.
- If the API contract does not use errno, log with the normal error logger and keep the reason explicit.
- Do not collapse nullable failure into silent continuation.

### 8) Logging Rules

- Use `wlf_log_errno()` for allocation or resource-acquisition failures that rely on errno-backed APIs.
- Prefer messages such as `failed to allocate wlf_curve_back` over vague messages such as `allocation failed`.
- Log once at the failure point; do not spam repeated logs for the same failing allocation unless multiple layers need separate context.

### 9) Control Flow And Readability

- Prefer early validation and straightforward exits over deep nesting.
- Keep each function focused on one clear responsibility.
- Split large functions when they mix discovery, mutation, cleanup, and logging in one body.
- Use comments sparingly; let naming and structure carry most of the explanation.

### 10) Simplicity Over Frameworkization

- Do not invent local macro frameworks, callback registries, or generic object systems unless the codebase demonstrably needs them.
- Prefer a few explicit subsystem patterns reused consistently over one highly abstract "universal" pattern.
- If a helper pattern saves a little typing but makes debugging or review harder, prefer the explicit version.

### 11) API Shape

- Keep parameter ordering stable and unsurprising.
- Make ownership transfer explicit in naming, documentation, or both.
- Prefer clear lifecycle pairs such as `init/finish` and `create/destroy`.
- Avoid APIs that require hidden ambient state when explicit parameters are practical.

## Code Review Protocol

Prioritize findings in this order:
- Macro or typedef indirection that obscures control flow, ownership, or type meaning.
- `#define` use where `enum`, `static const`, or a normal helper would be clearer.
- Platform leakage into generic APIs.
- Missing `NULL` checks after allocation or nullable API calls.
- Missing `wlf_log_errno()` on allocator failure in wlframe-style code.
- Leaks or double-frees in partial initialization paths.
- `realloc()` misuse that can lose the original pointer.
- Vague or non-actionable failure logs.

Review questions to answer:
- Is the code using plain C constructs where they are sufficient?
- Are macros and typedefs justified, or just hiding detail?
- Are constants and simple helpers expressed with the least surprising language construct?
- Does the API stay platform-neutral at the right boundary?
- Is every nullable allocation result checked immediately?
- Does allocator failure log with `wlf_log_errno()` before returning or unwinding?
- Does cleanup release previously acquired resources in reverse order?
- Is `realloc()` handled with a temporary pointer?

## Good Pattern

```c
struct wlf_curve_back *curve = malloc(sizeof(*curve));
if (curve == NULL) {
	wlf_log_errno(WLF_ERROR, "failed to allocate wlf_curve_back");
	return NULL;
}
```

## Response Pattern

For review tasks:
- Lead with hidden complexity, bad API shape, platform leakage, missing checks, or broken unwind paths.
- Treat unchecked allocator returns as correctness bugs, not style issues.
- Keep summary brief and secondary.

See also:
- `PLAYBOOK.md` for focused allocation and cleanup workflows.
