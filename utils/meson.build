wlf_files += files(
	'wlf_linked_list.c',
	'wlf_time.c',
	'wlf_log.c',
	'wlf_signal.c',
	'wlf_utils.c',
	'wlf_list.c',
	'wlf_env.c',
	'wlf_cmd_parser.c',
	'wlf_standard_paths.c',
)

if is_linux
	wlf_files += files('wlf_standard_paths_linux.c')
elif is_macos
	wlf_files += files('wlf_standard_paths_macos.m')
endif
