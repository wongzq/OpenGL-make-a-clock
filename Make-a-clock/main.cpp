#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <ctime>

// openGL variables
GLuint program;
GLuint VAO[5];		// ID for Vertex Array Objects:
					// VAO[0] is for Clock Frame
					// VAO[1] is for Clock Body
					// VAO[2] is for Clock Sec Hands
					// VAO[3] is for Clock Min Hands
					// VAO[4] is for Clock Hour Hands

GLuint VBO[10];		// ID for Vertex Buffer Objects:
					// VBO[0] is for Clock Frame Vertices
					// VBO[1] is for Clock Frame Color
					// VBO[2] is for Clock Body Vertices
					// VBO[3] is for Clock Body Color
					// VBO[4] is for Clock Sec Hand Vertices
					// VBO[5] is for Clock Sec Hand Color
					// VBO[6] is for Clock Min Hand Vertices
					// VBO[7] is for Clock Min Hand Color
					// VBO[8] is for Clock Hour Hand Vertices
					// VBO[9] is for Clock Hour Hand Color

// clock options
enum Clock :int {
	FRAME,			// Clock Frame == Outer circle
	BODY,			// Clock Body == Inner circle
	CLOCK_LENGTH	// Length of Clock enum
};
enum MenuOption :int {
	ROUND, SQUARE,
	SHOW, HIDE,
	SMALL, MEDIUM, LARGE,
	CYAN, MAGENTA, YELLOW,
	EXIT
};
enum ColorOption :int {
	NOTHING, CYAN_COLOR, MAGENTA_COLOR, YELLOW_COLOR
};
struct coordinate { GLfloat x, y; };
struct color { GLfloat r, g, b; };

// constants
const int window_w = 600, window_h = 600;
const float PI = 3.14159f;
const unsigned int numOfCircleVertices = 100;
const int numOfColors = 4;
void* font = GLUT_BITMAP_TIMES_ROMAN_24;

// clockVertex[0] is Clock Frame, clockVertex[1] is Clock Body
coordinate clockVertex[2][numOfCircleVertices];
color colorOptions[numOfColors][numOfCircleVertices];

coordinate clockHand[3][3];
color clockHandColor[3];

float clockSize = 1.0f;
int xOffset = 10;
int yOffset = 10;
int diameter = 180;
int clockColor = ColorOption::CYAN_COLOR;

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
	for (int i = 0; i < numOfColors; i++) {
		for (int j = 0; j < numOfCircleVertices; j++) {
			if (i == 3) {
				colorOptions[i][j] = { (GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.7 };
			}
			else {
				// if i == 0, color is Magenta
				// if i == 1, color is Yellow
				// if i == 2, color is Cyan
				colorOptions[i][j].r = (GLfloat)(i == 1 || i == 2 ? 0.85 : 0.3);
				colorOptions[i][j].g = (GLfloat)(i == 2 || i == 0 ? 0.85 : 0.3);
				colorOptions[i][j].b = (GLfloat)(i == 0 || i == 1 ? 0.85 : 0.3);
			}
		}
	}

	for (int i = 0; i < 3; i++) {
		clockHandColor[i] = { (GLfloat)0.3, (GLfloat)0.3, (GLfloat)0.3 };
	}

	glGenVertexArrays(5, VAO);
	glGenBuffers(10, VBO);

	// clock frame and body
	for (int index = 0; index < Clock::CLOCK_LENGTH; index++) {
		glBindVertexArray(VAO[index]);

		// clock vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockVertex[index]), clockVertex[index], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// clock color
		glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colorOptions), colorOptions, GL_STATIC_DRAW);
		for (int i = 0; i < numOfColors; i++) {
			glVertexAttribPointer(i + 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)((unsigned long long) i * 100 * 3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(i + 1);
		}
	}

	// clock hands
	for (int index = 0; index < 3; index++) {
		glBindVertexArray(VAO[index + Clock::CLOCK_LENGTH]);
		// clock hand vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) * 2 + 0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockHand[index]), clockHand[index], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//clock hand color
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) * 2 + 1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockHandColor), clockHandColor, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
	}

	// program
	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
	glUseProgram(program);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

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

	switch (index) {
	case BODY:
		glUniform1i(uniformLocation, 4);
		break;
	case FRAME:
		glUniform1i(uniformLocation, clockColor);
		break;
	}

	glDrawArrays(GL_POLYGON, 0, numOfCircleVertices);
}

void renderBitmapCharacter(int x, int y, void* font, char* string) {
	switch (clockColor) {
	case ColorOption::CYAN_COLOR:
		glColor3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)1.0);
		break;
	case ColorOption::MAGENTA_COLOR:
		glColor3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)1.0);
		break;
	case ColorOption::YELLOW_COLOR:
		glColor3f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.0);
		break;
	}
	glRasterPos2d(x, y);
	glutBitmapString(font, (const unsigned char*)string);
}

void drawDigits() {
	// disable the shader program
	glUseProgram(0);

	// use OpenGL fixed pipeline functions
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-window_w / 2, window_w / 2, -window_h / 2, window_h / 2);

	const int numOfDigits = 12;
	float theta = PI / 2;
	float increment = 2 * PI / numOfDigits;

	for (int i = 0; i < numOfDigits; i++) {
		int x = (int)(cos(theta) * diameter / 2) - xOffset;
		int y = (int)(sin(theta) * diameter / 2) - yOffset;
		theta -= increment;
		renderBitmapCharacter(x, y, (void*)font, (char*)std::to_string(i == 0 ? 12 : i).c_str());
	}

	// enable the shader program
	glUseProgram(program);
}

void drawClockHand(int index) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");
	glBindVertexArray(VAO[index + Clock::CLOCK_LENGTH]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) + 1]);
	glBufferSubData(GL_ARRAY_BUFFER, 1, sizeof(clockHandColor), clockHandColor);
	glUniform1i(uniformLocation, 1);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void updateTime(int _) {
	time_t curTime = time(0);
	tm* localTime = new tm;
	localtime_s(localTime, &curTime);

	for (int i = 0; i < 3; i++) {
		double theta = 0;
		double handLength = 0;
		switch (i) {
		case 0:
			// second hand
			theta = -(localTime->tm_sec / 60.0 * 2.0 * PI) + PI / 2;
			handLength = clockSize * 0.75;
			break;
		case 1:
			// minute hand
			theta = -(localTime->tm_min / 60.0 * 2.0 * PI) + PI / 2;
			handLength = clockSize * 0.6;
			break;
		case 2:
			// hour hand
			theta = -(localTime->tm_hour / 12.0 * 2.0 * PI) + PI / 2;
			handLength = clockSize * 0.5;
			break;
		}

		clockHand[i][0].x = (GLfloat)(cos(theta - 0.5) * (clockSize * 0.05) / 2.0);
		clockHand[i][0].y = (GLfloat)(sin(theta - 0.5) * (clockSize * 0.05) / 2.0);
		clockHand[i][1].x = (GLfloat)(cos(theta + 0.5) * (clockSize * 0.05) / 2.0);
		clockHand[i][1].y = (GLfloat)(sin(theta + 0.5) * (clockSize * 0.05) / 2.0);
		clockHand[i][2].x = (GLfloat)(cos(theta) * handLength / 2.0);
		clockHand[i][2].y = (GLfloat)(sin(theta) * handLength / 2.0);

		glBindVertexArray(VAO[i + 2]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(i + 2) * 2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockHand[i]), clockHand[i]);
	}

	glFlush();
	glutTimerFunc(1000, updateTime, 0);
}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT);

	// clock frame
	generateCircleVertices(0, 0, GLfloat(clockSize * 1.00), FRAME);
	drawCircle(FRAME);

	// clock body
	generateCircleVertices(0, 0, GLfloat(clockSize * 0.75), BODY);
	drawCircle(BODY);

	// clock digits
	drawDigits();

	// clock hands
	drawClockHand(0);	// second hand
	drawClockHand(1);	// minute hand
	drawClockHand(2);	// hour hand

	glFlush();
}

void processMenuEvents(int option) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");

	switch (static_cast<MenuOption>(option)) {
		// color options
	case MenuOption::CYAN:
		clockColor = ColorOption::CYAN_COLOR;
		break;
	case MenuOption::MAGENTA:
		clockColor = ColorOption::MAGENTA_COLOR;
		break;
	case MenuOption::YELLOW:
		clockColor = ColorOption::YELLOW_COLOR;
		break;

		// size options
	case MenuOption::SMALL:
		clockSize = 0.75;
		xOffset = 5;
		yOffset = 5;
		diameter = 135;
		font = GLUT_BITMAP_TIMES_ROMAN_10;
		break;
	case MenuOption::MEDIUM:
		clockSize = 1;
		xOffset = 10;
		yOffset = 10;
		diameter = 180;
		font = GLUT_BITMAP_TIMES_ROMAN_24;
		break;
	case MenuOption::LARGE:
		clockSize = 1.5;
		xOffset = 10;
		yOffset = 10;
		diameter = 270;
		font = GLUT_BITMAP_TIMES_ROMAN_24;
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
	glutAddMenuEntry("Cyan", CYAN);
	glutAddMenuEntry("Magenta", MAGENTA);
	glutAddMenuEntry("Yellow", YELLOW);

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
	int menu = glutCreateMenu(processMenuEvents);
	//glutAddSubMenu("Shape", clockShapeMenu);
	glutAddSubMenu("Color", clockColorMenu);
	//glutAddSubMenu("Digits", clockDigitsMenu);
	glutAddSubMenu("Size", clockSizeMenu);
	glutAddMenuEntry("Exit", EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(window_w, window_h);
	glutInitWindowPosition(50, 25);
	glutCreateWindow("Make a Clock");

	glewInit();
	init();
	createMenu();
	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutTimerFunc(1000, updateTime, 0);

	glutMainLoop();
}