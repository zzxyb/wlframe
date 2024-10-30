# Here be dragons
# This is a workaround for Meson's custom_target() directive, which
# currently does not support outputs pointing to a sub-directory
# XXX: try turning these into maps, so they can be indexed with picture name
dot_png = []
dot_map = []

doxygen_conf = configuration_data()
doxygen_conf.set('VERSION', meson.project_version())
doxygen_conf.set('top_builddir', meson.project_build_root())
wlf_doxygen = configure_file(
		input: 'wlf.doxygen.in',
		output: 'wlf.doxygen',
	configuration: doxygen_conf,
)

src_files = ([
	'../../include/wlf/types/wlf_backend.h',
	'../../include/wlf/types/wlf_muti_backend.h',
	'../../include/wlf/types/wlf_buffer.h',
	'../../include/wlf/types/wlf_cursor.h',
	'../../include/wlf/types/wlf_damage.h',
	'../../include/wlf/types/wlf_input_device.h',
	'../../include/wlf/types/wlf_keyboard.h',
	'../../include/wlf/types/wlf_output.h',
	'../../include/wlf/types/wlf_pointer.h',
	'../../include/wlf/types/wlf_surface.h',
	'../../include/wlf/types/wlf_texture.h',
	'../../include/wlf/types/wlf_touch.h',
	'../../include/wlf/math/wlf_matrix3x3.h',
	'../../include/wlf/math/wlf_matrix4x4.h',
	'../../include/wlf/math/wlf_point.h',
	'../../include/wlf/math/wlf_quaternion.h',
	'../../include/wlf/math/wlf_ray.h',
	'../../include/wlf/math/wlf_rect.h',
	'../../include/wlf/math/wlf_size.h',
	'../../include/wlf/math/wlf_vector2.h',
	'../../include/wlf/math/wlf_vector3.h',
	'../../include/wlf/math/wlf_vector4.h',
	'../../include/wlf/render/wlf_allocator.h',
	'../../include/wlf/render/wlf_dmabuf.h',
	'../../include/wlf/render/wlf_drm_format_set.h',
	'../../include/wlf/render/wlf_egl.h',
	'../../include/wlf/render/wlf_gles.h',
	'../../include/wlf/render/wlf_gl.h',
	'../../include/wlf/render/wlf_vulkan.h',
	'../../include/wlf/render/wlf_pass.h',
	'../../include/wlf/render/wlf_pixman.h',
	'../../include/wlf/render/wlf_renderer.h',
	'../../include/wlf/render/wlf_swapchain.h',
	'../../include/wlf/util/wlf_addon.h',
	'../../include/wlf/util/wlf_env.h',
	'../../include/wlf/util/wlf_log.h',
	'../../include/wlf/util/wlf_time.h',
	'../../include/wlf/util/wlf_utils.h',
	'../../include/wlf/wayland/wlf_wl_backend.h',
	'../../include/wlf/wayland/wlf_wl_data_device.h',
	'../../include/wlf/wayland/wlf_wl_data_offer.h',
	'../../include/wlf/wayland/wlf_wl_fractional_scale.h',
	'../../include/wlf/wayland/wlf_wl_layer_shell.h',
	'../../include/wlf/wayland/wlf_wl_pointer.h',
	'../../include/wlf/wayland/wlf_wl_shm.h',
	'../../include/wlf/wayland/wlf_wl_surface.h',
	'../../include/wlf/wayland/wlf_wl_xdg_shell.h',
	'../../include/wlf/wayland/wlf_wl_buffer.h',
	'../../include/wlf/wayland/wlf_wl_data_device_manager.h',
	'../../include/wlf/wayland/wlf_wl_data_source.h',
	'../../include/wlf/wayland/wlf_wl_keyboard.h',
	'../../include/wlf/wayland/wlf_wl_output.h',
	'../../include/wlf/wayland/wlf_wl_seat.h',
	'../../include/wlf/wayland/wlf_wl_subsurface.h',
	'../../include/wlf/wayland/wlf_wl_touch.h',
])

gen_doxygen = find_program('gen-doxygen.py')

subdir('xml')

formats = {
	'html': {
		'wlf': src_files,
	},
}

foreach f_name, sections: formats
	foreach s_name, s_files: sections
		t_name = '@0@-@1@-doc'.format(f_name, s_name)

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
					 wlf_doxygen,
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
			wlf_doxygen,
		'@INPUT@',
	],
	input: man_files,
	output: 'man3',
	build_by_default: true,
	install: true,
	install_dir: join_paths(get_option('prefix'), get_option('mandir')),
)
