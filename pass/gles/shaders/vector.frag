#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

uniform vec4 color;
varying mediump float v_coverage;

void main() {
	gl_FragColor = color * clamp(v_coverage, 0.0, 1.0);
}
