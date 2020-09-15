#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

// openGL variables
GLuint VAO[2];		// ID for Vertex Array Objects

GLuint VBO[4];		// ID for Vertex Buffer Objects:
					// VBO[0] is for Clock Frame Vertices
					// VBO[1] is for Clock Frame Color
					// VBO[2] is for Clock Body Vertices
					// VBO[3] is for Clock Body Color

GLuint program;


// clock options
enum Clock :int {
	FRAME,	// Clock Frame == Outer circle
	BODY	// Clock Body == Inner circle
};

enum MenuOption :int {
	ROUND, SQUARE,
	SHOW, HIDE,
	SMALL, MEDIUM, LARGE,
	RED, GREEN, BLUE,
	EXIT
};

struct coordinate {
	GLfloat x;
	GLfloat y;
};

// constants
const double PI = 3.14159;
const unsigned int numOfCircleVertices = 100;

// clockVertex[0] is Clock Frame, clockVertex[1] is Clock Body
coordinate clockVertex[2][numOfCircleVertices];

// color[Red Option,Green Option, Blue Option][100][R value, G value, B value]
GLfloat color[3][numOfCircleVertices][3];

float clockSize = 1.0f;

// function to load shaders
GLuint loadShaders(const std::string vShaderFile, const std::string fShaderFile) {
	GLint status;	// to check compile and linking status

	// VERTEX SHADER
	// load vertex shader code from file
	std::string vShaderCodeStr;
	std::ifstream vShaderStream(vShaderFile, std::ios::in);

	if (vShaderStream.is_open()) {
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
	// initialize color array
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < numOfCircleVertices; j++) {
			color[i][j][0] = (i == 0 ? 0.5 : 0.0);	// if i == 0, color is Red
			color[i][j][1] = (i == 1 ? 0.5 : 0.0);	// if i == 1, color is Green
			color[i][j][2] = (i == 2 ? 0.5 : 0.0);	// if i == 2, color is Blue
		}
	}

	glGenVertexArrays(2, VAO);
	glGenBuffers(4, VBO);
	for (int index = 0; index < 2; index++) {
		glBindVertexArray(VAO[index]);

		// clock vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockVertex[index]), clockVertex[index], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

		// clock color
		glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * 100 * 3 * sizeof(GLfloat)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(1 * 100 * 3 * sizeof(GLfloat)));
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * 100 * 3 * sizeof(GLfloat)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
	}

	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program);
}

// index is to determine if it is clock 'FRAME or BODY'
void generateCircleVertices(GLfloat x, GLfloat y, GLfloat r, int index) {
	float theta = 0.0;
	float increment = 2 * PI / numOfCircleVertices;

	// generate 100 vertex points
	for (int i = 0; i < numOfCircleVertices; i++) {
		clockVertex[index][i] = { (cos(theta) * r / 2) + x, (sin(theta) * r / 2) + y };
		theta += increment;
	}

	glBindVertexArray(VAO[index]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockVertex[index]), clockVertex[index]);
}

void drawCircle(int index) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");

	glBindVertexArray(VAO[index]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 1]);
	glDrawArrays(GL_LINE_LOOP, 0, numOfCircleVertices);
	glFlush();
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	generateCircleVertices(0, 0, clockSize * 1.00, FRAME);
	drawCircle(FRAME);
	generateCircleVertices(0, 0, clockSize * 0.75, BODY);
	drawCircle(BODY);

	glFlush();
}

void processMenuEvents(int option) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");

	switch (static_cast<MenuOption>(option)) {
		// color options
	case MenuOption::RED:
		glUniform1i(uniformLocation, 1);
		break;
	case MenuOption::GREEN:
		glUniform1i(uniformLocation, 2);
		break;
	case MenuOption::BLUE:
		glUniform1i(uniformLocation, 3);
		break;

		// size options
	case MenuOption::SMALL:
		clockSize = 0.75;
		break;
	case MenuOption::MEDIUM:
		clockSize = 1;
		break;
	case MenuOption::LARGE:
		clockSize = 1.5;
		break;
	case MenuOption::EXIT:
		exit(0);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}


void createMenu() {
	//// > clock shape menu
	//int clockShapeMenu = glutCreateMenu(processMenuEvents);
	//glutAddMenuEntry("Round", ROUND);
	//glutAddMenuEntry("Square", SQUARE);

	// > clock color menu
	int clockColorMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Red", RED);
	glutAddMenuEntry("Green", GREEN);
	glutAddMenuEntry("Blue", BLUE);

	//// > clock digits menu
	//int clockDigitsMenu = glutCreateMenu(processMenuEvents);
	//glutAddMenuEntry("Show", SHOW);
	//glutAddMenuEntry("Hide", HIDE);

	// > clock size menu
	int clockSizeMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Small", SMALL);
	glutAddMenuEntry("Medium", MEDIUM);
	glutAddMenuEntry("Large", LARGE);

	// main menu
	//glutAddSubMenu("Shape", clockShapeMenu);
	//glutAddSubMenu("Digits", clockDigitsMenu);
	int menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Color", clockColorMenu);
	glutAddSubMenu("Size", clockSizeMenu);
	glutAddMenuEntry("Exit", EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 25);
	glutCreateWindow("Make a Clock");

	glewInit();
	init();
	createMenu();
	glutDisplayFunc(display);

	glutMainLoop();
}