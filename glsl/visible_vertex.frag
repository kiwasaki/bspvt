#version 410

layout( location = 0 ) out int color;

flat in int face_index;

void main()
{
	color = face_index;
}