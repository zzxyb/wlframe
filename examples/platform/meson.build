platform_examples = {
	'theme_palette_test': {
		'src': ['theme_palette_test.c'],
		'dep': [],
	},
	'fontconfig_test': {
		'src': ['fontconfig_test.c'],
		'dep': [],
	},
}

foreach example, info : platform_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
