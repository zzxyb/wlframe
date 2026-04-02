# Wayland Playbook

## Fast Start

1. Identify the Wayland object family involved: globals, surfaces, buffers, seats, outputs, or shell roles.
2. Write down the protocol state machine before changing code.
3. Trace object creation, event callbacks, and destroy order.
4. Confirm which requests require serials, configure acknowledgements, roundtrips, or frame callbacks.

## Implementation Patterns

### Add A New Registry-Backed Wrapper

1. Add a wrapper type and destroy helper under `wayland/`.
2. Discover the interface through `wlf_wl_display_find_interface()`.
3. Bind using the safe negotiated version.
4. Hook destroy/listener behavior before exposing the wrapper to higher layers.

### Add A New Protocol Or Extension

1. Decide whether the protocol is mandatory, optional, or opportunistic.
2. Negotiate version conservatively.
3. Keep generated bindings, wrapper code, and high-level state handling separate.
4. Define fallback behavior when the protocol is absent.

### Add Surface Or Shell Behavior

1. Identify the exact role and configure sequence.
2. Track serials and ack rules explicitly.
3. Keep attach, damage, commit, and frame callbacks ordered correctly.
4. Treat popup, grab, and focus logic as state-machine code.

### Extend Wayland Backend Startup

1. Keep required-global checks explicit.
2. Create protocol wrappers before advertising the backend as started.
3. Register listeners immediately after successful object creation.
4. If any step fails, unwind in reverse order.

## Review Checklist

- Registry lookup does not assume globals always exist.
- Version negotiation and optional-protocol fallback are explicit.
- Configure/ack and serial-dependent requests are ordered correctly.
- Buffer release and backing-store lifetime are coherent.
- Interface comparison uses content equality where needed.
- Listener links are removed during stop/destroy.
- No roundtrip or dispatch call is hidden in an unexpected helper.
- Wayland wrappers and backend logic remain cleanly separated.

## Risk Hotspots

- Comparing interface strings by pointer identity.
- Using compositor-advertised versions without negotiation.
- Violating configure/ack or input serial requirements.
- Reusing buffers before compositor release.
- Destroying display or registry while listeners still point into them.
- Emitting output-added signals for partially initialized outputs.
