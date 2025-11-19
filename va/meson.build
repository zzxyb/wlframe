wlf_files += files(
	'wlf_va_display.c',
##	'wlf_video_decoder.c',
##	'wlf_video_encoder.c',
##	'wlf_video_buffer.c',
##	'wlf_video_session.c',
##	'wlf_hwdec.c',
##	'wlf_hwdec_software.c',
##	'wlf_sw_video_buffer.c',
)

if is_linux
	subdir('jpeg')
	subdir('wayland')
endif

## Vulkan backend
#if vulkan.found()
#	wlf_files += files(
#		'wlf_hwdec_vulkan.c',
#		'wlf_vk_video_buffer.c',
#	)
#endif

## VA-API backend
#if libva.found()
#	wlf_files += files(
#		'wlf_hwdec_vaapi.c',
#		'wlf_va_video_buffer.c',
#		'wlf_va_jpeg.c',
#	)
#endif
