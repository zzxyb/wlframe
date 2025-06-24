translator_examples = {
	'wlf_i18n_example': {
		'src': ['wlf_i18n_example.c'],
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
