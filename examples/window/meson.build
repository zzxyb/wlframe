window_examples = {
	'window_test': {
		'src': ['window_test.c'],
		'dep': [],
	},
	'scene_damage_test': {
		'src': ['scene_damage_test.c'],
		'dep': [],
	},
	'svg_node_test': {
		'src': ['svg_node_test.c'],
		'dep': [],
	},
}

if is_linux or is_windows
	executable(
		'native_window_test',
		'native_window_test.c',
		dependencies: [wlframe],
		install: false,
	)
endif

if is_linux or is_windows
	foreach example, info : window_examples
		executable(
			example,
			info.get('src', []),
			dependencies: [wlframe] + info.get('dep', []),
			c_args: [
				'-DWLF_WINDOW_TEST_IMAGE="' + meson.project_source_root() /
					'examples/image/resources/png/pngtest.png' + '"',
				'-DWLF_SVG_NODE_TEST_IMAGE="' + meson.project_source_root() /
					'examples/svg/resources/svg_node_test.svg' + '"',
			],
			install: false,
		)
	endforeach
endif
