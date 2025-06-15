translator_examples = {
	'advanced_i18n_test': {
		'src': ['advanced_i18n_test.c'],
		'dep': [],
	},
	'i18n_test': {
		'src': ['i18n_test.c'],
		'dep': [],
	},
}

foreach example, info : translator_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
	)
endforeach
