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

common_c_args = cc.get_supported_arguments([
	'-Wundef',
	'-Wmissing-include-dirs',
	'-Wold-style-definition',
	'-Wpointer-arith',
	'-Winit-self',
	'-Wstrict-prototypes',
	'-Wendif-labels',
	'-Wstrict-aliasing=2',
	'-Woverflow',
	'-Wmissing-prototypes',
	'-Walloca',

	'-Wno-missing-braces',
	'-Wno-missing-field-initializers',
	'-Wno-unused-parameter',
])

linux_c_args = cc.get_supported_arguments([
	'-Wlogical-op',
	'-Wimplicit-fallthrough=2',
	'-fstack-protector-strong',
	'-Wformat-security',
])

macos_c_args = cc.get_supported_arguments([
	'-Wno-unused-command-line-argument',
])

is_linux = host_machine.system() == 'linux'
is_macos = host_machine.system() == 'darwin'
if is_linux
	add_project_arguments(common_c_args + linux_c_args, language: 'c')
elif is_macos
	add_project_arguments(common_c_args + macos_c_args, language: 'c')
else
	error('Unsupported platform: ' + host_machine.system())
endif

fs = import('fs')

relative_dir = fs.relative_to(meson.current_source_dir(), meson.global_build_root()) + '/'
if cc.has_argument('-fmacro-prefix-map=/prefix/to/hide=')
	add_project_arguments(
		'-fmacro-prefix-map=@0@='.format(relative_dir),
		language: 'c',
	)
else
	add_project_arguments(
		'-D_WLF_REL_SRC_DIR="@0@"'.format(relative_dir),
		language: 'c',
	)
endif

features = {
	'wayland-backend': false,
	'x11-backend': false,
	'vulkan-render': false,
	'macos_platform': is_macos,
	'linux_platform': is_linux,
}

math = cc.find_library('m')
if is_linux
	message('Building for Linux')

	rt = cc.find_library('rt')
	wayland_project_options = ['tests=false', 'documentation=false']

	wayland_client = dependency('wayland-client',
		version: '>=1.23',
		fallback: 'wayland',
		default_options: wayland_project_options,
	)
	libpng = dependency('libpng', required: true)
	libturbojpeg = dependency('libjpeg', required: true)
	vulkan = dependency('vulkan', version: '>=1.2.182', required: true)
elif is_macos
	message('Building for macOS')
	libpng = dependency('libpng', required: true, method: 'pkg-config')
	libturbojpeg = dependency('libturbojpeg', required: true, method: 'pkg-config')
else
	error('Unsupported platform')
endif

wlf_files = []
wlf_deps = [
	math,
	libpng,
	libturbojpeg,
]

if is_macos
elif is_linux
	wlf_deps += [
		rt,
		wayland_client,
		vulkan,
	]
else
endif

subdir('include')
subdir('utils')
if is_linux
	subdir('wayland')
endif
subdir('image')
subdir('math')
subdir('platform')
subdir('renderer')
subdir('types')
subdir('window')

wlf_inc = include_directories('include')

symbols_file = 'wlframe.syms'
symbols_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbols_file)
if is_linux
    symbols_flag = '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbols_file)
    lib_wlf = library(
        versioned_name, wlf_files,
        dependencies: wlf_deps,
        include_directories: [wlf_inc],
        install: true,
        link_args: symbols_flag,
        link_depends: symbols_file,
    )
elif is_macos
    lib_wlf = library(
        versioned_name, wlf_files,
        dependencies: wlf_deps,
        include_directories: [wlf_inc],
        install: true,
    )
endif

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

subdir('examples')

if get_option('documentation').enabled()
	subdir('docs')
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
