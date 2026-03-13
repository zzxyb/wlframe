---
name: pixman
description: "Use this skill when working on pixman broadly: image creation, compositing operators, regions, transforms, trapezoids, glyphs, gradients, clipping, damage tracking, pixel formats, endianness, software rasterization, and raw buffer integration. Best for full pixman implementation and code review, with wlframe-specific integration as a secondary concern."
---

# Pixman Rasterization Skill

Apply this skill whenever a task involves pixman concepts, even if the current wlframe codebase does not yet use that pixman feature set.

## Primary Goal

Produce pixman integration code that is correct about pixel formats, data access windows, and buffer ownership:
- Format mapping stays consistent with the chosen external pixel format model and endianness.
- `pixman_image_t` lifetime follows the backing buffer lifetime.
- Buffer listeners are linked and unlinked exactly once.
- Software rendering code remains explicit about raw memory access.
- Pixman operations, regions, transforms, and clipping are treated as semantics-sensitive rendering logic, not mere helper calls.

## Use This Skill To

- Implement or review pixman image creation, compositing, clipping, transforms, source patterns, gradients, glyph rendering, and region math.
- Audit software render buffers, stride/pixel layout assumptions, and raw data access windows.
- Review pixman image creation, destroy, and listener hookup.
- Validate external pixel format to pixman format conversions when pixman is used as a display or rendering backend.
- Extend wlframe to more advanced pixman usage without losing correctness in buffer lifetime or pixel semantics.

## Domain Coverage

This skill should be treated as broad pixman guidance, including:
- `pixman_image_t` creation and ownership.
- Format codes, alpha handling, premultiplication assumptions, and endianness.
- Compositing operators, source/mask/destination semantics, and clipping.
- Regions, boxes, damage tracking, and incremental redraw logic.
- Transforms, scaling, filtering, repeated sources, solid fills, gradients, trapezoids, and glyph paths.
- Raw memory-backed images, stride calculation, alignment, and external buffer lifetime.
- Performance-sensitive software rendering tradeoffs such as avoiding redundant conversions or full-surface repaints.

## Source Of Truth

Prefer pixman semantics and pixel-correctness first, then adapt them to wlframe.

Within this repository, relevant integration points include:
- `buffer/pixman/render_buffer.c`
- `renderer/pixman/**`
- relevant public headers under `include/wlf/**`
- top-level `meson.build`

## wlframe Integration Notes

- wlframe maps external buffer formats to pixman formats with endianness-specific conditionals.
- Pixman render buffers subscribe to wlframe buffer destroy events.
- Pixman image creation uses raw buffer data and stride from wlframe buffer access helpers.

## Mandatory Rules

### 1) Format Mapping

- Treat external format and pixman format mapping as correctness-critical.
- Review both little-endian and big-endian branches when changing format tables.
- Unsupported format handling must fail clearly.
- Do not add silent fallback formats that may corrupt channel ordering.
- Be explicit about premultiplied alpha assumptions when introducing new formats or operators.

### 2) Buffer Data Access

- Use begin/end access helpers as a strict pair around raw memory access windows.
- Do not keep raw data assumptions alive longer than the buffer access contract allows.
- If pixman image lifetime depends on the backing storage remaining valid, make that ownership and access contract explicit.
- Avoid creating pixman images from data pointers whose validity ends before the image use ends.
- Review stride, row padding, and alignment assumptions whenever pixel layout changes.

### 3) Listener And Buffer Lifetime

- Link buffer-destroy listeners exactly once.
- Unlink listeners before freeing the pixman render buffer.
- A pixman render buffer destroy helper may accept `NULL`; ordinary helpers should not silently hide invalid objects.
- Keep listener teardown consistent between manual destroy and signal-driven destroy.

### 4) Compositing And Pixel Semantics

- Choose pixman operators intentionally; alpha, mask, and blending semantics matter.
- Keep source, mask, and destination roles clear in compositing code.
- Do not mix premultiplied and non-premultiplied assumptions implicitly.
- Treat color-channel ordering and endian-sensitive format interpretation as review-critical.

### 5) Regions, Clipping, And Damage

- Use regions and clipping to model visible and damaged areas explicitly.
- Prefer incremental redraw logic that matches the actual damaged region instead of repainting blindly.
- Keep region ownership and mutation boundaries clear when regions are shared across helpers.

### 6) Software Rendering Discipline

- Prefer direct, explicit memory and stride calculations over hidden macros.
- Keep pixman wrapper responsibilities narrow: format lookup, image creation, listener wiring, and teardown.
- Do not let software-rendering helpers mutate unrelated wlframe buffer state unexpectedly.
- When performance matters, optimize after correctness of format, stride, clip, and operator semantics is clear.

## Code Review Protocol

Prioritize findings in this order:
- Wrong format mapping or endian mismatch.
- Invalid data pointer lifetime or mismatched begin/end access.
- Wrong operator, alpha, clip, or transform semantics.
- Damage or region handling that repaints too much or too little.
- Listener leaks, double unlink, or double destroy.
- Pixman image lifetime not matching backing buffer lifetime.
- Missing Meson or dependency follow-up for pixman code changes.

Review questions to answer:
- Is the format mapping correct for both endian configurations?
- Is the data pointer valid for as long as the pixman image needs it?
- Are operator, alpha, source, and destination semantics correct?
- Do clipping and damage regions match the intended redraw behavior?
- Do destroy paths remove listeners and unref the image exactly once?
- Does unsupported-format behavior fail loudly instead of corrupting pixels?

## Validation Pointers

- When adding new pixman use sites, inspect format assumptions, operator semantics, clipping, and image lifetime together.
- When changing raster flow, verify raw memory access, region math, and blend semantics as one unit.
- Check format table edits against all related external pixel-format entries nearby.
- Review both manual destroy and signal-triggered destroy flows together.
- Reconfirm pixman dependency wiring if files or features move.

## Response Pattern

For review tasks:
- Lead with format correctness, pixel semantics, region behavior, or raw-memory lifetime issues.
- Call out begin/end access mismatches, operator mistakes, and damage/clip bugs explicitly.
- Keep style comments secondary.

See also:
- `PLAYBOOK.md` for implementation and review checklists.