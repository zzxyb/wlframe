# Here be dragons
# This is a workaround for Meson's custom_target() directive, which
# currently does not support outputs pointing to a sub-directory
# XXX: try turning these into maps, so they can be indexed with picture name
dot_png = []
dot_map = []

doxygen_conf = configuration_data()
doxygen_conf.set('VERSION', meson.project_version())
doxygen_conf.set('top_builddir', meson.project_build_root())
wlframe_doxygen = configure_file(
		input: 'wlframe.doxygen.in',
		output: 'wlframe.doxygen',
	configuration: doxygen_conf,
)

src_files = files([
		'../../include/wlf/image/wlf_bmp_image.h',
		'../../include/wlf/image/wlf_image.h',
		'../../include/wlf/image/wlf_jpeg_image.h',
		'../../include/wlf/image/wlf_png_image.h',
		'../../include/wlf/image/wlf_ppm_image.h',

		'../../include/wlf/math/wlf_fpoint.h',
		'../../include/wlf/math/wlf_frect.h',
		'../../include/wlf/math/wlf_fsize.h',
		'../../include/wlf/math/wlf_matrix3x3.h',
		'../../include/wlf/math/wlf_matrix4x4.h',
		'../../include/wlf/math/wlf_point.h',
		'../../include/wlf/math/wlf_quaternion.h',
		'../../include/wlf/math/wlf_ray2.h',
		'../../include/wlf/math/wlf_ray3.h',
		'../../include/wlf/math/wlf_rect.h',
		'../../include/wlf/math/wlf_region.h',
		'../../include/wlf/math/wlf_size.h',
		'../../include/wlf/math/wlf_vector2.h',
		'../../include/wlf/math/wlf_vector3.h',
		'../../include/wlf/math/wlf_vector4.h',

		'../../include/wlf/platform/wlf_backend_builtin.h',
		'../../include/wlf/platform/wlf_backend.h',

		'../../include/wlf/renderer/wlf_renderer.h',

		'../../include/wlf/types/wlf_color.h',

		'../../include/wlf/utils/wlf_cmd_parser.h',
		'../../include/wlf/utils/wlf_env.h',
		'../../include/wlf/utils/wlf_linked_list.h',
		'../../include/wlf/utils/wlf_list.h',
		'../../include/wlf/utils/wlf_log.h',
		'../../include/wlf/utils/wlf_signal.h',
		'../../include/wlf/utils/wlf_time.h',
		'../../include/wlf/utils/wlf_utils.h',
])

if is_linux
	src_files += files([
			'../../include/wlf/platform/wayland/backend.h',

			'../../include/wlf/renderer/vulkan/device.h',
			'../../include/wlf/renderer/vulkan/instance.h',
			'../../include/wlf/renderer/vulkan/renderer.h',

			'../../include/wlf/wayland/wlf_wl_compositor.h',
			'../../include/wlf/wayland/wlf_wl_display.h',
	])
endif

gen_doxygen = find_program('gen-doxygen.py')

subdir('xml')

formats = {
	'html': {
		'wlframe': src_files,
	},
}

foreach f_name, sections: formats
	foreach s_name, s_files: sections
		t_name = '@0@-@1@-docs'.format(f_name, s_name)

		# We do not really need an output file, but Meson
		# will complain if one is not set, so we use a
		# dummy 'stamp' file
		stamp = join_paths(meson.current_build_dir(), '@0@.stamp'.format(t_name))
		custom_target(
			t_name,
			command: [
				gen_doxygen,
				# XXX pass doxygen path as argument
				'--builddir=@OUTDIR@',
				'--section=@0@'.format(s_name),
				'--output-format=@0@'.format(f_name),
				'--stamp=@0@'.format(stamp),
					wlframe_doxygen,
				'@INPUT@',
			],
			input: s_files,
			output: '@0@.stamp'.format(t_name),
			depends: [dot_png, dot_map],
			build_by_default: true,
		)
	endforeach
endforeach

man_files = src_files
stamp = join_paths(meson.current_build_dir(), 'man3.stamp')
custom_target(
	'man-pages-3',
	command: [
		gen_doxygen,
		'--builddir=@OUTDIR@',
		'--output-format=man3',
		'--stamp=@0@'.format(stamp),
			wlframe_doxygen,
		'@INPUT@',
	],
	input: man_files,
	output: 'man3',
	build_by_default: true,
	install: true,
	install_dir: join_paths(get_option('prefix'), get_option('mandir')),
)
