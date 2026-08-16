wlf_files += files(
	'wlf_swapchain.c',
)

if is_linux
	subdir('shm')
	subdir('egl')
	subdir('vulkan')
elif is_windows
	subdir('directx12')
endif
