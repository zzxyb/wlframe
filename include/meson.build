foreach name, have : internal_features
	internal_config.set10('HAVE_' + name.underscorify().to_upper(), have)
endforeach
wlf_files += configure_file(
	output: 'config.h',
	configuration: internal_config,
)
