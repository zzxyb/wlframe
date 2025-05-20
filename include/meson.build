subdir('wlf')

exclude_files = ['meson.build', 'config.h.in', 'version.h.in']

if not features.get('x11-backend')
	exclude_files += 'backend/x11.h'
endif
if not features.get('gles2-renderer')
	exclude_files += ['render/egl.h', 'render/gles2.h']
endif
if not features.get('vulkan-renderer')
	exclude_files += 'render/vulkan.h'
endif

install_subdir('wlf',
	install_dir: get_option('includedir') / versioned_name,
	exclude_files: exclude_files,
)

foreach name, have : internal_features
	internal_config.set10('HAVE_' + name.underscorify().to_upper(), have)
endforeach
wlf_files += configure_file(
	output: 'config.h',
	configuration: internal_config,
)
