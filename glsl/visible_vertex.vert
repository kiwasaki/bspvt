#version 410

// Input vertex data, different for all executions of this shader.
layout( location = 0 ) in vec3    vertex_position;
layout( location = 1 ) in int     vertex_face_index;

flat out int face_index;

uniform mat4 mvp;

void main(){
	face_index = vertex_face_index;
    gl_Position = mvp * vec4( vertex_position, 1);
}

