---
name: vulkan
description: "Use this skill when working on Vulkan broadly: instance and device setup, queues, swapchains, render passes, pipelines, descriptor sets, synchronization, memory allocation, command recording, extensions, validation layers, and Vulkan portability concerns. Best for full Vulkan implementation and code review, with wlframe-specific integration as a secondary concern."
---

# Vulkan Graphics Skill

Apply this skill whenever a task involves Vulkan concepts, even if the current wlframe codebase does not yet use that Vulkan subsystem.

## Primary Goal

Produce Vulkan code that is explicit about ownership, result handling, and teardown order:
- Every Vulkan handle has a clear parent and destroy path.
- Every `VkResult` is checked where failure is possible.
- Cleanup order matches Vulkan object hierarchy.
- Validation-layer and extension assumptions stay explicit.
- Synchronization and memory visibility are treated as correctness concerns, not optional optimization details.

## Use This Skill To

- Implement or review instance, physical device, logical device, queue family, and surface/swapchain setup.
- Work on render passes, framebuffers, pipelines, descriptor sets, shader interfaces, and command buffer recording.
- Audit synchronization with fences, semaphores, pipeline barriers, queue ownership transfer, and timeline semaphores.
- Review memory allocation, image/buffer layout transitions, staging uploads, and host/device visibility rules.
- Validate wlframe's Vulkan renderer integration, wrapper casting, and ownership conventions when applicable.

## Domain Coverage

This skill should be treated as broad Vulkan guidance, including:
- Instance and device creation, layers, extensions, feature structs, and portability constraints.
- Queues, queue families, present support, and ownership transfer.
- Command pools, command buffers, submission flow, and reuse rules.
- Images, buffers, memory types, mapping, coherence, layout transitions, and staging.
- Descriptor set layouts, descriptor pools, pipeline layouts, and shader resource interfaces.
- Render passes or dynamic rendering, framebuffers, attachments, and load/store semantics.
- Synchronization: fences, binary semaphores, timeline semaphores, barriers, access masks, and stage masks.
- Swapchains, presentation, resize/out-of-date handling, and multi-frame-in-flight hazards.

## Source Of Truth

Prefer Vulkan API rules and driver-portable design first, then adapt them to wlframe.

Within this repository, relevant integration points include:
- `renderer/vulkan/instance.c`
- `renderer/vulkan/device.c`
- `renderer/vulkan/renderer.c`
- `include/wlf/renderer/vulkan/*.h`
- top-level `meson.build`

## wlframe Integration Notes

- wlframe wraps `VkInstance`, `VkPhysicalDevice`, and `VkDevice` in wlframe structs.
- The renderer owns a command pool and timeline semaphore.
- Validation is toggled through `WLF_RENDER_DEBUG` and logs mention `VK_LAYER_KHRONOS_validation`.

## Mandatory Rules

### 1) Handle Ownership

- Track parent-child ownership explicitly: instance -> device -> renderer-owned handles.
- Destroy child Vulkan handles before destroying the parent `VkDevice`.
- `*_destroy()` may accept `NULL`; helper predicates and conversions should assume valid objects unless documented otherwise.
- Avoid freeing wrapper memory before all Vulkan handles it owns are released.

### 2) Result Handling

- Check every `VkResult` that can fail.
- Convert failure into a clear cleanup path immediately.
- Log the failing Vulkan call name with the error code helper when possible.
- Do not continue initialization after a failed Vulkan creation call.
- Re-check functions whose success still depends on side conditions, such as extension support or presentation compatibility.

### 3) Initialization And Unwind

- Acquire resources step by step and unwind in reverse order.
- Keep partially initialized wrappers safe for destroy-path cleanup.
- Avoid split ownership where one helper allocates and another silently steals without documentation.
- If a helper consumes ownership on success or failure, make that obvious in naming or comments.
- Keep resize, device-lost, and out-of-date recovery paths as explicit state transitions.

### 4) Predicate And Cast Helpers

- Type-check helpers such as `wlf_renderer_is_vk()` should return the direct comparison and not special-case `NULL` unless the API says so.
- Conversion helpers like `from_renderer()` should assert or rely on valid prior classification rather than silently hide invalid call paths.
- Keep classification logic and lifetime logic separate.

### 5) Extensions And Validation

- Check optional extensions before use.
- Load extension function pointers only after the owning instance/device exists.
- Do not assume validation layers or debug utils are available on every runtime.
- Keep extension-dependent code behind explicit checks.

### 6) Synchronization And Visibility

- Be explicit about execution order and memory visibility requirements.
- Use barriers, stage masks, and access masks that match the actual data hazard.
- Do not treat semaphores, fences, and barriers as interchangeable.
- Review queue-family ownership transfer and present/acquire synchronization carefully.

### 7) Memory And Resource State

- Track image layouts and buffer/image usage transitions explicitly.
- Respect host-coherent versus non-coherent memory behavior.
- Make staging uploads and mapped-memory flushing/invalidation rules explicit.
- Treat swapchain images, transient attachments, and exported memory as distinct ownership domains.

### 8) Pipelines And Binding Contracts

- Keep descriptor layouts, shader expectations, vertex formats, and pipeline layouts consistent.
- Do not change resource binding contracts casually across pipeline and shader code.
- Review push constants, specialization constants, and descriptor indexing with ABI-like caution.

## Code Review Protocol

Prioritize findings in this order:
- Use-after-free or wrong Vulkan destroy order.
- Missing `VkResult` checks or bad error unwinding.
- Broken synchronization, layout transitions, or visibility assumptions.
- Invalid memory allocation, mapping, or coherence behavior.
- Pipeline/descriptors/shader contract mismatches.
- Misowned wrapper/device/instance relationships.
- Invalid assumptions about extensions, validation, or queue family capabilities.
- Predicate helpers masking invalid inputs.

Review questions to answer:
- Does every created Vulkan object have exactly one clear destroy path?
- Does every failure jump to a cleanup path that matches acquisition order?
- Do synchronization primitives and barriers actually cover the hazard being guarded?
- Are image layouts, access masks, and memory visibility rules coherent?
- Are wrapper destroy functions safe for partial initialization?
- Are type/predicate helpers sharp and free of defensive `NULL` handling?

## Validation Pointers

- When adding new Vulkan subsystems, review handle ownership, synchronization, and feature gating together.
- When touching rendering flow, inspect command recording, layout transitions, descriptor validity, and submit/present sequencing as one unit.
- Check `renderer/vulkan/renderer.c` when changing renderer-level ownership.
- Check `instance` and `device` wrappers together when altering setup flows.
- Reconfirm Meson Vulkan dependency wiring if new source files or features are added.

## Response Pattern

For review tasks:
- Lead with lifetime, synchronization, memory, or `VkResult` findings.
- Mention extension, feature, queue-family, or portability assumptions if they can break on real drivers.
- Keep style feedback secondary.

See also:
- `PLAYBOOK.md` for focused implementation and review steps.