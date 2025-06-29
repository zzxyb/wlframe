platform_examples = {
	'backend_demo': {
		'src': ['backend_demo.c'],
		'dep': [],
	},
}

# Build examples if enabled
foreach example, info : platform_examples
	executable(
		example,
		info.get('src', []),
		dependencies: [wlframe] + info.get('dep', []),
		install: false,
	)
endforeach

# Plugin example as a shared library
plugin_example = shared_library(
	'wlf_backend_plugin_example',
	'wlf_backend_plugin_example.c',
	dependencies: [wlframe],
	install: false,
	name_prefix: '',  # Remove 'lib' prefix
)
