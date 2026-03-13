# Struct Layout Playbook

## Fast Start

1. Mark which fields are hot, cold, emitted, subscribed, intrusive, or descriptive.
2. Identify whether the struct is polymorphic and whether `impl` drives dispatch.
3. Check whether the layout is public and therefore ABI-sensitive.
4. Reorder only when the technical reason is clear.

## Implementation Patterns

### Design A New Polymorphic Struct

1. Put `impl` first if dispatch depends on it.
2. Keep hot traversal or dispatch fields early.
3. Group signals under `events`.
4. Group subscribed listeners under `listeners`.

### Add A New Field To An Existing Struct

1. Decide whether the field is hot, cold, or lifecycle-related.
2. Place it near related fields first, then check alignment and padding.
3. Avoid pushing hot fields behind cold strings or debug metadata.
4. Re-check ABI impact if the struct is public.

### Place An Intrusive `link`

1. Decide whether iteration or membership checks are hot.
2. Keep `link` early for hot traversal-heavy objects.
3. Move it later only if it is mostly bookkeeping and the front of the struct is hotter.
4. Keep similar object families consistent.

## Review Checklist

- `impl` is first when polymorphic dispatch depends on it.
- Hot fields are not buried behind cold metadata.
- `link` placement matches real usage frequency.
- Multiple signals are grouped under `events`.
- Multiple listeners are grouped under `listeners`.
- Public layout changes are justified and minimal.

## Risk Hotspots

- Putting strings and descriptive metadata ahead of hot dispatch state.
- Mixing emitted signals and subscribed listeners in the same flat field list.
- Reordering public structs casually.
- Treating alignment-only packing as more important than lifecycle clarity.
