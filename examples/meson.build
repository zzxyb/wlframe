examples = {
	'wayland_registry_test': {
		'src': ['wayland_registry_test.c'],
		'dep': [],
	},
	'png_image_test': {
		'src': ['png_image_test.c'],
		'dep': [],
	},
}

foreach example, info : examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
