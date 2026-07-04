wlf_files += files(
	'wlf_backend.c',
	'wlf_theme.c',
	'wlf_fontconfig.c',
)

if is_linux
	subdir('linux')
	subdir('wayland')
elif is_macos
	subdir('macos')
elif is_windows
	subdir('windows')
endif
