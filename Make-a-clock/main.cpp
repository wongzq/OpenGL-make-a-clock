#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

const float PI = 3.14159;
const unsigned int numOfCircleVertices = 100;

GLuint VBO;		// ID for Vertex Buffer Objects
GLuint VAO;		// ID for Vertex Array Objects
GLuint program;

struct coordinate {
	GLfloat x;
	GLfloat y;
};

// clock options
GLfloat center_x = 0;
GLfloat center_y = 0;
GLfloat circle_w = 1;
GLfloat circle_h = 1;
coordinate vertex[numOfCircleVertices + 4];

// function to load shaders
GLuint loadShaders(const std::string vShaderFile, const std::string fShaderFile) {
	GLint status;	// to check compile and linking status

	// VERTEX SHADER
	// load vertex shader code from file
	std::string vShaderCodeStr;		
	std::ifstream vShaderStream(vShaderFile, std::ios::in);

	if(vShaderStream.is_open()) {
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(vShaderStream, line))
			vShaderCodeStr += line + "\n";
		vShaderStream.close();
	}
	else {
		// output error message and exit
		std::cout << "Failed to open vertex shader file - " << vShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// FRAGMENT SHADER
	// load fragment shader code from file
	std::string fShaderCodeStr;
	std::ifstream fShaderStream(fShaderFile, std::ios::in);

	if (fShaderStream.is_open()) {
		// read from stream line by line and append it to shader code
		std::string line = "";
		while (std::getline(fShaderStream, line))
			fShaderCodeStr += line + "\n";
		fShaderStream.close();
	}
	else {
		// output error message and exit
		std::cout << "Failed to open fragment shader file - " << fShaderFile << std::endl;
		exit(EXIT_FAILURE);
	}

	// create shader objects
	GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// provide source code for shaders
	const GLchar* vShaderCode = vShaderCodeStr.c_str();
	const GLchar* fShaderCode = fShaderCodeStr.c_str();
	glShaderSource(vShaderID, 1, &vShaderCode, NULL);
	glShaderSource(fShaderID, 1, &fShaderCode, NULL);

	// compile vertex shader
	status = GL_FALSE;
	glCompileShader(vShaderID);
	glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		std::cout << "Failed to compile vertex shader - " << vShaderFile << std::endl;

		int infoLogLength;
		glGetShaderiv(fShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(vShaderID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;

		exit(EXIT_FAILURE);
	}

	// compile fragment shader
	status = GL_FALSE;
	glCompileShader(fShaderID);
	glGetShaderiv(fShaderID, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		std::cout << "Failed to compile fragment shader - " << fShaderFile << std::endl;
		int infoLogLength;
		glGetShaderiv(fShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(fShaderID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;

		exit(EXIT_FAILURE);
	}

	// create program
	GLuint programID = glCreateProgram();

	// attach shaders to program object
	glAttachShader(programID, vShaderID);
	glAttachShader(programID, fShaderID);

	// link program object
	glLinkProgram(programID);

	// check link status
	status = GL_FALSE;
	glGetProgramiv(programID, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		std::cout << "Failed to link program object." << std::endl;
		int infoLogLength;
		glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(programID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;

		exit(EXIT_FAILURE);
	}

	return programID;
}

void init(void) {
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");

	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// activate shader program
	glUseProgram(program);
	// if the VAO already exists, make that buffer the current active one
	glBindBuffer(GL_ARRAY_BUFFER, VAO);
}

void generateCircleVertices() {
	float theta = 0.0;
	float increment = 2 * PI / numOfCircleVertices;
	for (int i = 0; i < numOfCircleVertices; i++) {
		vertex[i] = { (cos(theta) * circle_w / 2) + center_x, (sin(theta) * circle_h / 2) + center_y };
		theta += increment;
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), vertex);
}

void drawCircle() {
	int uniformLocation;
	uniformLocation = glGetUniformLocation(program, "color");
	glUniform4f(uniformLocation, 1.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_LINE_LOOP, 0, numOfCircleVertices);
	glFlush();
}

void drawFrame() {
	int uniformLocation;
	uniformLocation = glGetUniformLocation(program, "color");
	glUniform4f(uniformLocation, 0.5, 0.5, 0.5, 1.0);
	// enable draw lines with a different stipple
	vertex[100] = { -0.5, -0.5 };
	vertex[101] = {  0.5, -0.5 };
	vertex[102] = {  0.5,  0.5 };
	vertex[103] = { -0.5,  0.5 };
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), vertex);

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0f0f);
	glDrawArrays(GL_LINE_LOOP, numOfCircleVertices, 4);
	glDisable(GL_LINE_STIPPLE);
	glFlush();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	drawCircle();
	drawFrame();
	glFlush();
}

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 25);
	glutCreateWindow("Make a Clock");

	glewInit();
	init();
	generateCircleVertices();
	glutDisplayFunc(display);

	glutMainLoop();
}