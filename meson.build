project(
	'wlframe',
	'c',
	version: '0.0.1-dev',
	license: 'MIT',
	meson_version: '>=1.3',
	default_options: [
		'c_std=' + (meson.version().version_compare('>=1.4.0') ? 'c23,c11' : 'c11'),
		'warning_level=2',
		'werror=true',
	],
)

version = meson.project_version().split('-')[0]
version_major = version.split('.')[0]
version_minor = version.split('.')[1]
versioned_name = '@0@-@1@.@2@'.format(meson.project_name(), version_major, version_minor)

little_endian = target_machine.endian() == 'little'
big_endian = target_machine.endian() == 'big'

add_project_arguments([
	'-D_POSIX_C_SOURCE=200809L',
], language: 'c')

cc = meson.get_compiler('c')

add_project_arguments(cc.get_supported_arguments([
	'-Wundef',
	'-Wlogical-op',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wimplicit-fallthrough=2',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wmissing-prototypes',
	'-Walloca',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
]), language: 'c')

fs = import('fs')

relative_dir = fs.relative_to(meson.current_source_dir(), meson.global_build_root()) + '/'
if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
	add_project_arguments(
		'-fmacro-prefix-map=@0@='.format(relative_dir),
		language: 'c',
	)
else
	add_project_arguments(
		'-D_WLR_REL_SRC_DIR="@0@"'.format(relative_dir),
		language: 'c',
	)
endif

features = {
	'wayland-backend': false,
	'gles2-renderer': false,
	'vulkan-renderer': false,
}
internal_features = {
	'egl': false,
}
internal_config = configuration_data()

wayland_project_options = ['tests=false', 'documentation=false']
wayland_server = dependency('wayland-server',
	version: '>=1.23',
	fallback: 'wayland',
	default_options: wayland_project_options,
)

wayland_client = dependency('wayland-client',
	version: '>=1.23',
	fallback: 'wayland',
	default_options: wayland_project_options,
)

xkbcommon = dependency(
	'xkbcommon',
	fallback: 'libxkbcommon',
	default_options: [
		'enable-tools=false',
		'enable-x11=false',
		'enable-docs=false',
		'enable-xkbregistry=false',
	],
)
pixman = dependency('pixman-1',
	version: '>=0.42.0',
	fallback: 'pixman',
	default_options: ['werror=false'],
)
math = cc.find_library('m')
rt = cc.find_library('rt')

wlf_files = []
wlf_deps = [
	wayland_server,
	wayland_client,
	xkbcommon,
	pixman,
	math,
	rt,
]

subdir('include')
#subdir('doc')
subdir('math')
subdir('util')
subdir('render')
subdir('platforms')
subdir('animator')
subdir('effects')
subdir('layouts')
subdir('backend')

wlf_inc = include_directories('include')

symbols_file = 'wlframe.syms'
symbols_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbols_file)
lib_wlf = library(
	versioned_name, wlf_files,
	dependencies: wlf_deps,
	include_directories: [wlf_inc],
	install: true,
	link_args: symbols_flag,
	link_depends: symbols_file,
)

wlf_vars = {}
foreach name, have : features
	wlf_vars += { 'have_' + name.underscorify(): have.to_string() }
endforeach

wlframe = declare_dependency(
	link_with: lib_wlf,
	dependencies: wlf_deps,
	include_directories: wlf_inc,
	variables: wlf_vars,
)

meson.override_dependency(versioned_name, wlframe)

summary(features + internal_features, bool_yn: true)

if get_option('examples')
	subdir('examples')
endif

if get_option('documentation').enabled()
	subdir('doc')
endif

pkgconfig = import('pkgconfig')
pkgconfig.generate(
	lib_wlf,
	name: versioned_name,
	description: 'UI framework library for building Wayland applications',
	subdirs: versioned_name,
	url: 'https://github.com/zzxyb/wlframe',
	variables: wlf_vars,
)
