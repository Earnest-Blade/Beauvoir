#version 400

#ifdef _VERTEX_

layout(location=0) in vec2 in_position;
layout(location=1) in vec2 in_uvs;

out V_DATA {
	vec2 uvs;
} vertex;

void main() {
	gl_Position = vec4(in_position, 0.0, 1.0);
	
	vertex.uvs = in_uvs;
}

#endif

#ifdef _FRAGMENT_

in V_DATA {
	vec2 uvs;
} vertex;

uniform sampler2D bvr_texture;

void main() {
	vec4 tex = texture(bvr_texture, vec2(vertex.uvs));
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

#endif