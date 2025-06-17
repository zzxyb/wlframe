math_examples = {
	'wlf_region_test': {
		'src': ['wlf_region_test.c'],
		'dep': [],
	},
	'wlf_point_test': {
		'src': ['wlf_point_test.c'],
		'dep': [],
	},
	'wlf_fpoint_test': {
		'src': ['wlf_fpoint_test.c'],
		'dep': [],
	},
}

foreach example, info : math_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
