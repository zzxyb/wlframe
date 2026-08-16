wlf_files += files(
	'wlf_buffer.c',
)

if is_linux or is_windows
	subdir('pixman')
endif

if is_linux
	subdir('shm')
endif
