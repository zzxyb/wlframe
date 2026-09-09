svg_info_test = executable(
	'svg_info_test',
	'svg_info_test.c',
	dependencies: [wlframe],
)

test(
	'svg_effects',
	svg_info_test,
	args: [
		'--input', meson.current_source_dir() / 'resources/effects.svg',
		'--output', meson.current_build_dir() / 'effects-roundtrip.svg',
		'--expect-effects',
	],
)
