add_project_arguments('-DCL_TARGET_OPENCL_VERSION=300', language: 'c')

wlf_files += files(
	'wlf_accel.c',
)

if is_linux
	subdir('opencl')
endif
