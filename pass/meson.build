wlf_files += files(
	'wlf_render_target_info.c',
	'wlf_rect_pass.c',
	'wlf_texture_pass.c',
	'wlf_vector_pass.c',
	'wlf_shape_geometry.c',
	'wlf_rect_shape_pass.c',
	'wlf_circle_pass.c',
	'wlf_ellipse_pass.c',
	'wlf_line_pass.c',
	'wlf_poly_pass.c',
	'wlf_path_pass.c',
)

if is_linux
	subdir('gles')
	subdir('vulkan')
endif

if is_linux
	subdir('pixman')
endif

if is_windows
	subdir('directx12')
endif
