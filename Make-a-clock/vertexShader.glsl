#version 330 core
layout (location = 0) in vec2 vertices;
//layout (location = 1) in vec3 color1;
//layout (location = 2) in vec3 color2;
//layout (location = 3) in vec3 color3;
//layout (location = 4) in vec3 color4;
//uniform int colorChoice = 1;

//out vec3 vertexColor;
void main() {
	gl_Position = vec4(vertices, 0.0, 1.0);

//	switch(colorChoice) {
//	case 1:
//		vertexColor = color1;
//		break;
//	case 2:
//		vertexColor = color2;
//		break;
//	case 3:
//		vertexColor = color3;
//		break;
//	case 4:
//		vertexColor = color4;
//		break;
//	case 5:
//		vertexColor = vec3(0.2, 0.2, 0.2);
//		break;
//	}
}