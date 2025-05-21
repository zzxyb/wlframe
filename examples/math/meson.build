math_examples = {
	'wlf_point_test': {
		'src': ['wlf_point_test.c'],
		'dep': [],
	},
	'wlf_fpoint_test': {
		'src': ['wlf_fpoint_test.c'],
		'dep': [],
	},
	'wlf_size_test': {
		'src': ['wlf_size_test.c'],
		'dep': [],
	},
	'wlf_fsize_test': {
		'src': ['wlf_fsize_test.c'],
		'dep': [],
	},
	'wlf_rect_test': {
		'src': ['wlf_rect_test.c'],
		'dep': [],
	},
	'wlf_frect_test': {
		'src': ['wlf_frect_test.c'],
		'dep': [],
	},
	'wlf_vector2_test': {
		'src': ['wlf_vector2_test.c'],
		'dep': [],
	},
	'wlf_vector3_test': {
		'src': ['wlf_vector3_test.c'],
		'dep': [],
	},
	'wlf_vector4_test': {
		'src': ['wlf_vector4_test.c'],
		'dep': [],
	},
	'wlf_ray2_test': {
		'src': ['wlf_ray2_test.c'],
		'dep': [],
	},
	'wlf_ray3_test': {
		'src': ['wlf_ray3_test.c'],
		'dep': [],
	},
	'wlf_quaternion_test': {
		'src': ['wlf_quaternion_test.c'],
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
