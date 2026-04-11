svg_examples = {
	'svg_info_test': {
		'src': ['svg_info_test.c'],
		'dep': [],
	},
}

foreach example, info : svg_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
