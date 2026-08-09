attribute vec2 pos;
attribute vec2 texcoord;
uniform vec2 viewport;
varying vec2 v_texcoord;

void main() {
	vec2 ndc = pos / viewport * 2.0 - 1.0;
	gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
	v_texcoord = texcoord;
}
