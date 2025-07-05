window_examples = {
	'window_test': {
		'src': ['window_test.c'],
		'dep': [],
	},
}

foreach example, info : window_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
		install: false,
	)
endforeach
