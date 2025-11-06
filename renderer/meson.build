wlf_files += files(
	'wlf_renderer.c',
)

if is_macos
	subdir('metal')
elif is_linux
	subdir('vulkan')
else
endif
