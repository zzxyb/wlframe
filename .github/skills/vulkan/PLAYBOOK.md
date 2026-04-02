# Vulkan Playbook

## Fast Start

1. Identify the Vulkan subsystem involved: setup, memory, synchronization, commands, pipelines, or presentation.
2. List handles, memory objects, and synchronization primitives in acquisition order.
3. Trace state transitions: image layouts, queue ownership, visibility, and submit/present flow.
4. Check `VkResult`, feature gating, and cleanup before looking at style.

## Implementation Patterns

### Add A New Vulkan Handle

1. Decide which wrapper owns it.
2. Create it after its parent object is valid.
3. Store it in zero-initialized state so destroy paths remain safe.
4. Destroy it before the parent handle in the wrapper destroy function.

### Add A New Synchronized Operation

1. Identify the precise execution and memory hazard.
2. Choose the minimal correct primitive: barrier, fence, semaphore, or timeline semaphore.
3. Keep stage masks, access masks, and queue ownership transfer explicit.
4. Re-check the submit path and teardown path together.

### Add A New Pipeline Or Descriptor Contract

1. Keep shader interface, descriptor layout, and pipeline layout consistent.
2. Treat push constants and descriptor bindings like ABI contracts.
3. Review vertex input and attachment formats together.
4. Validate that command recording matches pipeline assumptions.

### Extend Renderer Initialization

1. Keep instance, device, and renderer responsibilities separate.
2. Fail immediately on any Vulkan creation error.
3. Reuse one cleanup path instead of many ad hoc frees.
4. Make ownership transfer to renderer/device explicit.

## Review Checklist

- Every `vkCreate*` has a matching destroy path.
- Every failing Vulkan call is checked and logged.
- Cleanup order is child before parent.
- Synchronization covers the actual execution and visibility hazard.
- Layout transitions and access masks match real resource usage.
- Descriptor, pipeline, and shader contracts are consistent.
- Predicate helpers do not collapse bad input into `false`.
- Extension-dependent code is guarded and not assumed.

## Risk Hotspots

- Destroying `VkDevice` before command pools or semaphores.
- Forgetting to clean up instance when device creation fails.
- Using barriers or stage masks that do not match the hazard.
- Mapping or reusing memory without respecting coherence rules.
- Descriptors or pipelines drifting away from shader expectations.
- Returning wrappers that own half-initialized Vulkan state.
- Hiding invalid renderer casts behind tolerant helper functions.
