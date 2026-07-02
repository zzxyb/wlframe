wlf_files += files(
	'wlf_renderer.c',
)

if is_linux
	subdir('vulkan')
	subdir('pixman')
	subdir('gles')
elif is_macos
	subdir('metal')
elif is_windows
	subdir('directx12')
endif
