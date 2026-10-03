# Here be dragons
# This is a workaround for Meson's custom_target() directive, which
# currently does not support outputs pointing to a sub-directory
# XXX: try turning these into maps, so they can be indexed with picture name
dot_png = []
dot_map = []

doxygen_conf = configuration_data()
doxygen_conf.set('VERSION', meson.project_version())
doxygen_conf.set('top_builddir', meson.project_build_root())
doxygen_conf.set('top_srcdir', meson.project_source_root())
wlframe_doxygen = configure_file(
		input: 'wlframe.doxygen.in',
		output: 'wlframe.doxygen',
	configuration: doxygen_conf,
)

src_files = files([
		'../../include/wlf/allocator/wlf_allocator.h',

		'../../include/wlf/animation/wlf_animation.h',
		'../../include/wlf/animation/wlf_animation_group.h',
		'../../include/wlf/animation/wlf_parallel_animation_group.h',
		'../../include/wlf/animation/wlf_sequential_animation_group.h',

		'../../include/wlf/buffer/wlf_buffer.h',

		'../../include/wlf/curve/curve_helpers.h',
		'../../include/wlf/curve/easing_functions.h',
		'../../include/wlf/curve/wlf_curve.h',
		'../../include/wlf/curve/wlf_curve_back.h',
		'../../include/wlf/curve/wlf_curve_bounce.h',
		'../../include/wlf/curve/wlf_curve_circ.h',
		'../../include/wlf/curve/wlf_curve_cubic.h',
		'../../include/wlf/curve/wlf_curve_elastic.h',
		'../../include/wlf/curve/wlf_curve_expo.h',
		'../../include/wlf/curve/wlf_curve_linear.h',
		'../../include/wlf/curve/wlf_curve_quad.h',
		'../../include/wlf/curve/wlf_curve_quart.h',
		'../../include/wlf/curve/wlf_curve_quint.h',
		'../../include/wlf/curve/wlf_curve_sine.h',

		'../../include/wlf/effect/wlf_drop_shadow.h',
		'../../include/wlf/effect/wlf_filter.h',
		'../../include/wlf/effect/wlf_gaussian_blur.h',

		'../../include/wlf/image/wlf_bmp_image.h',
		'../../include/wlf/image/wlf_gif_image.h',
		'../../include/wlf/image/wlf_image.h',
		'../../include/wlf/image/wlf_jpeg_image.h',
		'../../include/wlf/image/wlf_png_image.h',
		'../../include/wlf/image/wlf_ppm_image.h',
		'../../include/wlf/image/wlf_webp_image.h',
		'../../include/wlf/image/wlf_xpm_image.h',

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
		'../../include/wlf/math/wlf_size.h',
		'../../include/wlf/math/wlf_vector2.h',
		'../../include/wlf/math/wlf_vector3.h',
		'../../include/wlf/math/wlf_vector4.h',

		'../../include/wlf/node/wlf_circle_node.h',
		'../../include/wlf/node/wlf_ellipse_node.h',
		'../../include/wlf/node/wlf_event_node.h',
		'../../include/wlf/node/wlf_line_node.h',
		'../../include/wlf/node/wlf_path_node.h',
		'../../include/wlf/node/wlf_poly_node.h',
		'../../include/wlf/node/wlf_rect_node.h',
		'../../include/wlf/node/wlf_rect_shape_node.h',
		'../../include/wlf/node/wlf_scene_node.h',
		'../../include/wlf/node/wlf_scene_tree.h',
		'../../include/wlf/node/wlf_svg_node.h',
		'../../include/wlf/node/wlf_text_node.h',
		'../../include/wlf/node/wlf_texture_node.h',

		'../../include/wlf/pass/wlf_circle_pass.h',
		'../../include/wlf/pass/wlf_ellipse_pass.h',
		'../../include/wlf/pass/wlf_line_pass.h',
		'../../include/wlf/pass/wlf_pass.h',
		'../../include/wlf/pass/wlf_path_pass.h',
		'../../include/wlf/pass/wlf_poly_pass.h',
		'../../include/wlf/pass/wlf_rect_pass.h',
		'../../include/wlf/pass/wlf_rect_shape_pass.h',
		'../../include/wlf/pass/wlf_render_target_info.h',
		'../../include/wlf/pass/wlf_texture_pass.h',
		'../../include/wlf/pass/wlf_vector_pass.h',

		'../../include/wlf/platform/wlf_backend.h',
		'../../include/wlf/platform/wlf_fontconfig.h',
		'../../include/wlf/platform/wlf_text.h',
		'../../include/wlf/platform/wlf_theme.h',

		'../../include/wlf/renderer/wlf_renderer.h',

		'../../include/wlf/scene/wlf_scene.h',

		'../../include/wlf/shapes/wlf_circle_shape.h',
		'../../include/wlf/shapes/wlf_ellipse_shape.h',
		'../../include/wlf/shapes/wlf_line_shape.h',
		'../../include/wlf/shapes/wlf_path_shape.h',
		'../../include/wlf/shapes/wlf_poly_shape.h',
		'../../include/wlf/shapes/wlf_rect_shape.h',
		'../../include/wlf/shapes/wlf_shape.h',
		'../../include/wlf/shapes/wlf_shape_tree.h',
		'../../include/wlf/shapes/wlf_symbol_shape.h',
		'../../include/wlf/shapes/wlf_text_shape.h',
		'../../include/wlf/shapes/wlf_use_shape.h',

		'../../include/wlf/svg/wlf_svg.h',

		'../../include/wlf/swapchain/wlf_swapchain.h',

		'../../include/wlf/texture/wlf_texture.h',

		'../../include/wlf/types/wlf_color.h',
		'../../include/wlf/types/wlf_cursor.h',
		'../../include/wlf/types/wlf_format_set.h',
		'../../include/wlf/types/wlf_gradient.h',
		'../../include/wlf/types/wlf_keyboard.h',
		'../../include/wlf/types/wlf_linear_gradient.h',
		'../../include/wlf/types/wlf_mesh_gradient.h',
		'../../include/wlf/types/wlf_output.h',
		'../../include/wlf/types/wlf_pixel_format.h',
		'../../include/wlf/types/wlf_pointer.h',
		'../../include/wlf/types/wlf_radial_gradient.h',
		'../../include/wlf/types/wlf_touch.h',

		'../../include/wlf/utils/wlf_addon.h',
		'../../include/wlf/utils/wlf_array.h',
		'../../include/wlf/utils/wlf_cmd_parser.h',
		'../../include/wlf/utils/wlf_compat.h',
		'../../include/wlf/utils/wlf_env.h',
		'../../include/wlf/utils/wlf_linked_list.h',
		'../../include/wlf/utils/wlf_list.h',
		'../../include/wlf/utils/wlf_log.h',
		'../../include/wlf/utils/wlf_signal.h',
		'../../include/wlf/utils/wlf_time.h',
		'../../include/wlf/utils/wlf_utils.h',

		'../../include/wlf/window/wlf_titlebar.h',
		'../../include/wlf/window/wlf_window.h',
])

if is_linux
	src_files += files([
			'../../include/wlf/allocator/egl/allocator.h',
			'../../include/wlf/allocator/shm/allocator.h',

			'../../include/wlf/buffer/egl/buffer.h',
			'../../include/wlf/buffer/pixman/buffer.h',
			'../../include/wlf/buffer/shm/buffer.h',

			'../../include/wlf/dmabuf/wlf_dmabuf.h',

			'../../include/wlf/pass/gles/rect_pass.h',
			'../../include/wlf/pass/gles/render_target_info.h',
			'../../include/wlf/pass/gles/texture_pass.h',
			'../../include/wlf/pass/gles/vector_pass.h',
			'../../include/wlf/pass/pixman/rect_pass.h',
			'../../include/wlf/pass/pixman/render_target_info.h',
			'../../include/wlf/pass/pixman/texture_pass.h',
			'../../include/wlf/pass/pixman/vector_pass.h',
			'../../include/wlf/pass/vulkan/vulkan.h',

			'../../include/wlf/platform/linux/text.h',
			'../../include/wlf/platform/linux/theme.h',
			'../../include/wlf/platform/wayland/backend.h',

			'../../include/wlf/renderer/gles/egl.h',
			'../../include/wlf/renderer/gles/renderer.h',
			'../../include/wlf/renderer/pixman/renderer.h',
			'../../include/wlf/renderer/vulkan/device.h',
			'../../include/wlf/renderer/vulkan/instance.h',
			'../../include/wlf/renderer/vulkan/renderer.h',

			'../../include/wlf/swapchain/egl/swapchain.h',
			'../../include/wlf/swapchain/shm/swapchain.h',
			'../../include/wlf/swapchain/vulkan/swapchain.h',

			'../../include/wlf/texture/gles/texture.h',
			'../../include/wlf/texture/pixman/texture.h',

			'../../include/wlf/wayland/wlf_wl_compositor.h',
			'../../include/wlf/wayland/wlf_wl_cursor.h',
			'../../include/wlf/wayland/wlf_wl_interface.h',
			'../../include/wlf/wayland/wlf_wl_keyboard.h',
			'../../include/wlf/wayland/wlf_wl_output.h',
			'../../include/wlf/wayland/wlf_wl_pointer.h',
			'../../include/wlf/wayland/wlf_wl_seat.h',
			'../../include/wlf/wayland/wlf_wl_subcompositor.h',
			'../../include/wlf/wayland/wlf_wl_subsurface.h',
			'../../include/wlf/wayland/wlf_wl_surface.h',
			'../../include/wlf/wayland/wlf_wl_touch.h',
			'../../include/wlf/wayland/wlf_wp_alpha_modifier_v1.h',
			'../../include/wlf/wayland/wlf_wp_content_type_v1.h',
			'../../include/wlf/wayland/wlf_wp_fractional_scale_v1.h',
			'../../include/wlf/wayland/wlf_wp_viewporter.h',
			'../../include/wlf/wayland/wlf_xdg_wm_base.h',
			'../../include/wlf/wayland/wlf_xdg_wm_dialog_v1.h',
			'../../include/wlf/wayland/wlf_zwlr_layer_shell_v1.h',
			'../../include/wlf/wayland/wlf_zxdg_decoration_v1.h',

			'../../include/wlf/window/wayland/wlr_layer_window.h',
			'../../include/wlf/window/wayland/xdg_dialog_window.h',
			'../../include/wlf/window/wayland/xdg_popup_window.h',
			'../../include/wlf/window/wayland/xdg_toplevel_window.h',
	])
elif is_macos
	src_files += files([
			'../../include/wlf/platform/macos/backend.h',
			'../../include/wlf/platform/macos/fontconfig.h',
			'../../include/wlf/platform/macos/theme.h',

			'../../include/wlf/renderer/metal/device.h',
			'../../include/wlf/renderer/metal/renderer.h',
	])
elif is_windows
	src_files += files([
			'../../include/wlf/platform/windows/backend.h',
			'../../include/wlf/platform/windows/fontconfig.h',
			'../../include/wlf/platform/windows/theme.h',

			'../../include/wlf/renderer/directx12/renderer.h',
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
