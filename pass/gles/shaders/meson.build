sh = find_program('sh', native: true)
embed = files('embed.sh')

shaders = [
	'rect.vert',
	'rect.frag',
]

foreach name : shaders
	output = name.underscorify() + '_src.h'
	var = name.underscorify() + '_src'
	wlf_files += custom_target(
		output,
		command: [sh, embed, var],
		input: name,
		output: output,
		feed: true,
		capture: true,
	)
endforeach
