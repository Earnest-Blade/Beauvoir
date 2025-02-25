#version 400

#ifdef _VERTEX_

layout(location=0) in vec3 in_position;
layout(location=1) in vec2 in_uvs;

uniform mat4 bvr_transform;

layout(std140) uniform bvr_camera {
	mat4 bvr_projection;
	mat4 bvr_view;
};

out V_DATA {
	vec2 uvs;
} vertex;

void main() {
	gl_Position = bvr_projection * bvr_view * vec4(in_position, 1.0);
	
	vertex.uvs = in_uvs;
}

#endif

#ifdef _FRAGMENT_

in V_DATA {
	vec2 uvs;
} vertex;

uniform sampler2DArray bvr_texture;
uniform int bvr_texture_layer;

void main() {
	vec4 tex = texture(bvr_texture, vec3(vertex.uvs, bvr_texture_layer));

	gl_FragColor = vec4(tex);
}

#endif