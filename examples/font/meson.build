font_examples = {
	'font_example': {
		'src': ['font_example.c'],
		'dep': [],
	},
	'font_backend_example': {
		'src': ['font_backend_example.c'],
		'dep': [],
	},
}

foreach example, info : font_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
