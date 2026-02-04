wlf_files += files(
	'wlf_renderer.c',
)

if is_linux
	subdir('vulkan')
	subdir('pixman')
elif is_macos
	subdir('metal')
endif
