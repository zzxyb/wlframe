image_examples = {
	'png_image_test': {
		'src': ['png_image_test.c'],
		'dep': [],
	},
	'jpeg_image_test': {
		'src': ['jpeg_image_test.c'],
		'dep': [],
	},
	'format_conversion_demo': {
		'src': ['format_conversion_demo.c'],
		'dep': [],
	},
	'bmp_image_test': {
		'src': ['bmp_image_test.c'],
		'dep': [],
	},
	'xbm_image_test': {
		'src': ['xbm_image_test.c'],
		'dep': [],
	},
	'xpm_image_test': {
		'src': ['xpm_image_test.c'],
		'dep': [],
	},
}

foreach example, info : image_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
