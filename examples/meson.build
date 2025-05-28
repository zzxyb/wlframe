examples = {
	'wayland_registry_test': {
		'src': ['wayland_registry_test.c'],
		'dep': [],
	},
	'image_test': {
		'src': ['image_test.c'],
		'dep': [],
	},
}

pngtest = configure_file(
    input: 'resources/pngtest.png',
    output: 'pngtest.png',
    copy: true
)

foreach example, info : examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
