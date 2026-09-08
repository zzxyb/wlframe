wlf_files += files(
	'wlf_allocator.c',
)

if is_linux
	subdir('shm')
	subdir('egl')
elif is_macos
	subdir('metal')
endif
