# C Playbook

## Fast Start

1. Identify whether the task is about API shape, struct design, allocation, or cleanup.
2. Mark every call that can return `NULL`.
3. Check whether macros or typedefs are hiding type meaning or control flow.
4. Check whether `#define` should really be an `enum`, `static const`, or helper function.
5. Verify failure paths log and unwind correctly.

## Implementation Patterns

### Add A New Public C API

1. Use plain `struct foo *` style types rather than alias-heavy declarations.
2. Keep parameter order predictable and ownership explicit.
3. Avoid macro wrappers when a normal function is clearer.
4. Keep platform-specific details behind the implementation boundary.

### Add A New Constant Or Small Helper

1. Prefer `enum` for grouped integral constants.
2. Prefer `static const` for typed constants.
3. Use `#define` only when the language gives no cleaner option.
4. Prefer a normal helper function over a macro if there is any control flow or side effect.

### Add A New Heap Allocation

1. Allocate into a named pointer.
2. Check for `NULL` immediately.
3. Log with `wlf_log_errno(WLF_ERROR, ...)` if wlframe logging is available.
4. Return failure or jump to cleanup before touching dependent state.

### Extend A Multi-Step Constructor

1. Keep resources listed in acquisition order.
2. After each nullable call, check and branch to cleanup on failure.
3. Log allocator failures at the exact failing step.
4. Unwind in reverse order.

### Use `realloc()` Safely

1. Store the result in a temporary pointer.
2. Check the temporary pointer for `NULL`.
3. Preserve the original allocation on failure.
4. Only replace the original pointer after success.

### Remove Unnecessary Macro Or Typedef Indirection

1. Replace macro-generated control flow with small helpers.
2. Replace `typedef struct` aliases with explicit `struct` usage when readability improves.
3. Keep function-pointer typedefs only when they genuinely clarify signatures.
4. Re-check header readability after the simplification.

### Simplify A Cross-Platform Boundary

1. Move platform-specific branches behind an implementation boundary.
2. Keep generic headers free of avoidable platform headers and macros.
3. Make the common API read the same across backends.
4. Re-check that the preprocessor footprint shrank rather than spread.

## Review Checklist

- APIs use plain C constructs and visible ownership.
- Macros do not hide allocation, control flow, or side effects.
- `#define` is not used where `enum`, `static const`, or a function would be clearer.
- `typedef struct` is not being used just to shorten names.
- Platform details do not leak into generic interfaces.
- Every allocation result is checked immediately.
- Allocator failures use `wlf_log_errno()` where applicable.
- Failure logs name the object or resource that failed.
- Cleanup paths unwind in reverse order.
- `realloc()` does not clobber the original pointer on failure.

## Risk Hotspots

- Macro layers that turn readable C into pseudo-language.
- Constants encoded as macros when typed C constructs would be clearer.
- Typedefs that hide whether a declaration is a struct or a pointer.
- Platform types leaking into supposedly generic headers.
- Calling into follow-up initialization before checking allocation results.
- Returning `NULL` without any errno-aware log on allocation failure.
- Leaking early allocations when a later step fails.
- Overwriting the only pointer with `realloc()` before checking the result.
