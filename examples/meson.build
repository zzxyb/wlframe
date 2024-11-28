examples = {
	'surface': {
		'src': ['surface.c'],
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
