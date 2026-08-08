attribute vec2 pos;
attribute float coverage;
uniform vec2 viewport;
varying mediump float v_coverage;

void main() {
	vec2 ndc = pos / viewport * 2.0 - 1.0;
	gl_Position = vec4(ndc.x, -ndc.y, 0.0, 1.0);
	v_coverage = coverage;
}
