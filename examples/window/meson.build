window_examples = {
	'window_test': {
		'src': ['window_test.c'],
		'dep': [],
	},
}

if is_linux
	# The current window example uses Wayland, wl_shm, and Pixman APIs.
	foreach example, info : window_examples
		executable(
			example,
			info.get('src', []),
			dependencies: [wlframe] + info.get('dep', []),
			install: false,
		)
	endforeach
endif
