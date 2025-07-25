image_examples = {
	'png_image_test': {
		'src': ['png_image_test.c'],
		'dep': [],
	},
	'jpeg_image_test': {
		'src': ['jpeg_image_test.c'],
		'dep': [],
	},
	'bmp_image_test': {
		'src': ['bmp_image_test.c'],
		'dep': [],
	},
	'ppm_image_test': {
		'src': ['ppm_image_test.c'],
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
