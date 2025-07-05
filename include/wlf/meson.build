version_base = meson.project_version().split('-')[0]
version_array = version_base.split('.')
version_data = configuration_data()
version_data.set_quoted('WLF_VERSION_STR', meson.project_version())
version_data.set('WLF_VERSION_MAJOR', version_array[0])
version_data.set('WLF_VERSION_MINOR', version_array[1])
version_data.set('WLF_VERSION_MICRO', version_array[2])

conf_data = configuration_data()
foreach name, have : features
	conf_data.set10('WLF_HAS_' + name.underscorify().to_upper(), have)
endforeach

conf_h = configure_file(
	input: 'config.h.in',
	output: 'config.h',
	configuration: conf_data,
)
ver_h = configure_file(
	input: 'version.h.in',
	output: 'version.h',
	configuration: version_data,
)

install_headers(conf_h, ver_h, subdir: versioned_name / 'wlf')

subdir('types')
