wayland_examples = {
	'wayland_registry_test': {
		'src': ['wayland_registry_test.c'],
		'dep': [],
	},
}

foreach example, info : wayland_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
