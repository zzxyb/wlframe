utils_examples = {
	'utils_cmd_parser_test': {
		'src': ['utils_cmd_parser_test.c'],
		'dep': [],
	},
	'standard_paths_test': {
		'src': ['standard_paths_test.c'],
		'dep': [],
	},
}

foreach example, info : utils_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
