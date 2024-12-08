wayland_args = {
	'version': '>=1.23.1',
	'fallback': 'wayland',
	'default_options': [
		'tests=false',
		'documentation=false',
	],
}

subdir('protocol')

wayland_client = dependency('wayland-client',
	kwargs: wayland_args,
)

wlf_deps += wayland_client

client_protos = [
	'linux-dmabuf-v1',
	'linux-drm-syncobj-v1',
	'pointer-gestures-unstable-v1',
	'presentation-time',
	'relative-pointer-unstable-v1',
	'tablet-v2',
	'viewporter',
	'xdg-activation-v1',
	'xdg-decoration-unstable-v1',
	'xdg-shell',
]

foreach proto : client_protos
	wlf_files += protocols_client_header[proto]
endforeach

wlf_files += files(
	'wlf_wl_backend.c',
	'wlf_wl_display.c',
	'wlf_wl_seat.c',
	'wlf_wl_pointer.c',
	'wlf_wl_keyboard.c',
	'wlf_wl_compositor.c',
)
