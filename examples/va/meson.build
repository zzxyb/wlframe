va_examples = {
	'va_wayland_test': {
		'src': ['va_wayland_test.c'],
		'dep': [],
	},
}

foreach va, info : va_examples
	executable(
		va,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
		install: false,
	)
endforeach
