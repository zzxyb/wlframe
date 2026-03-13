# Pixman Playbook

## Fast Start

1. Identify whether the task is about formats, compositing, regions, transforms, glyphs, or raw buffer-backed images.
2. Check the pixel format, alpha model, endian assumptions, and stride.
3. Trace image lifetime, raw memory access windows, and clipping/damage flow.
4. Review both manual destroy and signal-driven destroy paths when external buffers are involved.

## Implementation Patterns

### Add A New Format Mapping

1. Add both endian-specific mappings if required.
2. Keep the format table grouped and readable.
3. Fail clearly when no pixman equivalent exists.
4. Review reverse mapping if both directions are supported.

### Add A New Compositing Operation

1. Choose the operator intentionally.
2. Verify source, mask, and destination semantics.
3. Confirm clip regions and transforms are applied in the right coordinate space.
4. Re-check premultiplied alpha assumptions.

### Add Region Or Damage Tracking

1. Define ownership of the region object.
2. Keep accumulated damage and consumed damage distinct.
3. Clip redraw work to the actual visible region.
4. Re-check correctness before chasing repaint performance.

### Add A New Pixman Render Buffer Field

1. Initialize it before exposing the buffer object.
2. Clean it up in both explicit and signal-driven destroy paths.
3. Keep listener links balanced.
4. Re-check image unref order.

## Review Checklist

- Begin/end access helpers are balanced.
- Data pointer lifetime matches pixman image usage.
- Format mapping is endian-correct.
- Operator, clip, and alpha semantics match the intended output.
- Region and damage logic redraw the right pixels.
- Destroy paths remove links and unref exactly once.
- Unsupported formats return a clear failure.

## Risk Hotspots

- Creating a pixman image from data that is no longer valid.
- Updating only one endian branch of a format mapping.
- Mixing premultiplied and non-premultiplied assumptions.
- Applying transforms or clips in the wrong coordinate space.
- Tracking damage too broadly or too narrowly.
- Maintaining two destroy paths that drift apart.
- Returning `0` or invalid formats without clear upstream handling.
