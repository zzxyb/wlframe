cairo = dependency('cairo', required: false, disabler: true)
wayland_client = dependency('wayland-client', required: false, disabler: true)
wayland_egl = dependency('wayland-egl', required: false, disabler: true)
egl = dependency('egl', version: '>= 1.5', required: false, disabler: true)
glesv2 = dependency('glesv2', required: false, disabler: true)

examples = {
	'surface': {
		'src': ['surface.c'],
		'dep': [wayland_client, wayland_egl, egl, glesv2],
	},
	'simple_server': {
		'src': ['simple_server.c'],
		'dep': [wayland_client, wayland_server, wayland_egl, egl, glesv2],
	},
}

foreach example, info : examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
