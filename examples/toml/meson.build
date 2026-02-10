toml_examples = {
	'toml_example': {
		'src': ['toml_example.c'],
		'dep': [],
	},
}

foreach example, info : toml_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
