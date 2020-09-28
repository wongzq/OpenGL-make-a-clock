#include <gl/glew.h>
#include <gl/freeglut.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <ctime>

// openGL variables
const GLuint numOfVAO = 7;
const GLuint numOfVBO = 14;

GLuint program;
GLuint VAO[numOfVAO];	// ID for Vertex Array Objects:
						// VAO[0] is for Clock Frame
						// VAO[1] is for Clock Frame Shadow
						// VAO[2] is for Clock Body

						// VAO[3] is for Clock Sec Hands
						// VAO[4] is for Clock Min Hands
						// VAO[5] is for Clock Hour Hands

						// VAO[6] is for Clock Dials

GLuint VBO[numOfVBO];	// ID for Vertex Buffer Objects:
						// VBO[0] is for Clock Frame Vertices
						// VBO[1] is for Clock Frame Color
						// VBO[2] is for Clock Frame Shadow Vertices
						// VBO[3] is for Clock Frame Shadow Color
						// VBO[4] is for Clock Body Vertices
						// VBO[5] is for Clock Body Color

						// VBO[6] is for Clock Sec Hand Vertices
						// VBO[7] is for Clock Sec Hand Color
						// VBO[8] is for Clock Min Hand Vertices
						// VBO[9] is for Clock Min Hand Color
						// VBO[10] is for Clock Hour Hand Vertices
						// VBO[11] is for Clock Hour Hand Color

						// VBO[12] is for Clock Dials Vertices
						// VBO[13] is for Clock Dials Color

// clock parts enum
enum Clock :int {
	FRAME,			// Clock Frame		  == Outer  circle / square
	FRAME_SHADOW,	// Clock Frame Shadow == Middle circle / square
	BODY,			// Clock Body		  == Inner  circle / square
	CLOCK_LENGTH	// Length of Clock enum
};
enum Hand :int {
	SEC, MIN, HOUR, HAND_LENGTH
};

// clock options enum
enum MenuOption :int {
	CIRCLE, SQUARE,
	CYAN, MAGENTA, YELLOW,
	SMALL, MEDIUM, LARGE,
	SHOW, HIDE,
	EXIT
};
enum ClockShape :int {
	CIRCLE_SHAPE, SQUARE_SHAPE
};
enum ClockColor :int {
	CYAN_COLOR = 1, MAGENTA_COLOR = 2, YELLOW_COLOR = 3
};
enum ClockSize : int {
	SMALL_SIZE, MEDIUM_SIZE, LARGE_SIZE
};
enum ClockDigits :int {
	SHOW_DIGITS, HIDE_DIGITS
};

// structs
struct coordinate { GLfloat x, y; };
struct color { GLfloat r, g, b; };

// constants
const int window_w = 600, window_h = 600;
const float PI = 3.14159f;
const unsigned int numOfClockVertices = 100;
const int numOfColors = 4;
const int numOfHandVertices = 8;
const int numOfDialVertices = 6;
const int numOfDigits = 12;
const int numOfDials = 12;
const int interval = 100;
const int clockDialIndex = Clock::CLOCK_LENGTH + Hand::HAND_LENGTH;

// clock data
coordinate clockVertex[Clock::CLOCK_LENGTH][numOfClockVertices];
color clockColorOptions[numOfColors][numOfClockVertices];
coordinate clockHand[Hand::HAND_LENGTH][numOfHandVertices];
color clockHandColor[numOfHandVertices];
coordinate clockDial[numOfDials][numOfDialVertices];
color clockDialColor[numOfDials * numOfDialVertices];
float clockDiameter = 1.00f;

// clock current option
int clockColor = ClockColor::CYAN_COLOR;
int clockShape = ClockShape::CIRCLE_SHAPE;
int clockSize = ClockSize::MEDIUM_SIZE;
int clockDigits = ClockDigits::SHOW_DIGITS;
void* font = GLUT_BITMAP_TIMES_ROMAN_24;

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

	// compile vertex shader
	GLuint vShaderID = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vShaderCode = vShaderCodeStr.c_str();
	glShaderSource(vShaderID, 1, &vShaderCode, NULL);

	status = GL_FALSE;
	glCompileShader(vShaderID);
	glGetShaderiv(vShaderID, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE) {
		std::cout << "Failed to compile vertex shader - " << vShaderFile << std::endl;
		int infoLogLength;
		glGetShaderiv(vShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* errorMsg = new char[static_cast<__int64>(infoLogLength) + 1];
		glGetShaderInfoLog(vShaderID, infoLogLength, NULL, errorMsg);
		std::cout << errorMsg << std::endl;
		delete[] errorMsg;
		exit(EXIT_FAILURE);
	}

	// compile fragment shader
	GLuint fShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fShaderCode = fShaderCodeStr.c_str();
	glShaderSource(fShaderID, 1, &fShaderCode, NULL);

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
	// color frame & body color
	for (int i = 0; i < numOfColors; i++) {
		for (int j = 0; j < numOfClockVertices; j++) {
			if (i == 3) {
				clockColorOptions[i][j] = { (GLfloat)0.7, (GLfloat)0.7, (GLfloat)0.7 };
			}
			else {
				// if i == 0, color is Cyan
				// if i == 1, color is Magenta
				// if i == 2, color is Yellow
				clockColorOptions[i][j].r = (GLfloat)(i == 1 || i == 2 ? 0.85 : 0.3);
				clockColorOptions[i][j].g = (GLfloat)(i == 2 || i == 0 ? 0.85 : 0.3);
				clockColorOptions[i][j].b = (GLfloat)(i == 0 || i == 1 ? 0.85 : 0.3);
			}
		}
	}

	// clock hand color
	for (int i = 0; i < numOfHandVertices; i++) {
		// create gradient color
		GLfloat handColor = (GLfloat)(i == (numOfHandVertices - 1) ? 0.6 : 0.2);
		clockHandColor[i] = { handColor, handColor, handColor };
	}

	// clock dial color
	for (int i = 0; i < numOfDials * numOfDialVertices; i++) {
		clockDialColor[i] = { 0.4f, 0.4f, 0.4f };
	}

	glGenVertexArrays(numOfVAO, VAO);
	glGenBuffers(numOfVBO, VBO);

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockColorOptions), clockColorOptions, GL_STATIC_DRAW);
		for (int i = 0; i < numOfColors; i++) {
			glVertexAttribPointer(i + 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)((unsigned long long) i * numOfClockVertices * 3 * sizeof(GLfloat)));
			glEnableVertexAttribArray(i + 1);
		}
	}

	// clock sec, min and hour hands
	for (int index = 0; index < Hand::HAND_LENGTH; index++) {
		glBindVertexArray(VAO[index + Clock::CLOCK_LENGTH]);
		// clock hand vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) * 2 + 0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockHand[index]), clockHand[index], GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//clock hand color
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) * 2 + 1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(clockHandColor), clockHandColor, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
	}

	// clock dial without numbers
	// clock dial vertices
	glBindVertexArray(VAO[clockDialIndex]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[clockDialIndex * 2 + 0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(clockDial), clockDial, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// clock dial color
	glBindBuffer(GL_ARRAY_BUFFER, VBO[clockDialIndex * 2 + 1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(clockDialColor), clockDialColor, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	// program
	program = loadShaders("vertexShader.glsl", "fragmentShader.glsl");
	glUseProgram(program);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

// paint clock FRAME and BODY
void generateClockVertices(GLfloat x, GLfloat y, GLfloat d, int index) {
	const float increment = 2 * PI / numOfClockVertices;
	float theta = 0.0;

	switch (clockShape) {
	case ClockShape::CIRCLE_SHAPE:
		// generate circle vertex points
		for (int i = 0; i < numOfClockVertices; i++) {
			clockVertex[index][i].x = (cos(theta) * d / 2) + x;
			clockVertex[index][i].y = (sin(theta) * d / 2) + y;
			theta += increment;
		}
		break;

	case ClockShape::SQUARE_SHAPE:
		const float turningCenter = d / 2 * 0.8f;
		const float turningVertex = d / 2 * 0.2f;
		const int quad1 = numOfClockVertices / 4 * 1;
		const int quad2 = numOfClockVertices / 4 * 2;
		const int quad3 = numOfClockVertices / 4 * 3;
		const int quad4 = numOfClockVertices / 4 * 4;

		// generate circle vertex points
		for (int i = 0; i < numOfClockVertices; i++) {
			const GLfloat xOffset =
				// x is +ve in 1st & 4th quadrant
				// x is -ve in 2nd & 3rd quadrant
				i >= 0 && i < quad1 || i >= quad3 && i < quad4
				? x + turningCenter
				: x - turningCenter;

			const GLfloat yOffset =
				// y is +ve in 1st & 2nd quadrant
				// y is -ve in 3rd & 4th quadrant
				i >= 0 && i < quad2
				? y + turningCenter
				: y - turningCenter;

			clockVertex[index][i].x = (cos(theta) * turningVertex) + xOffset;
			clockVertex[index][i].y = (sin(theta) * turningVertex) + yOffset;
			theta += increment;
		}
		break;
	}

	glBindVertexArray(VAO[index]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockVertex[index]), clockVertex[index]);
}

void drawClock(int index) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");
	glBindVertexArray(VAO[index]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[index * 2 + 1]);

	switch (index) {
	case Clock::FRAME:
		glUniform1i(uniformLocation, clockColor);
		break;
	case Clock::FRAME_SHADOW:
		glUniform1i(uniformLocation, clockColor + 4);
		break;
	case Clock::BODY:
		glUniform1i(uniformLocation, 4);
		break;
	}

	glDrawArrays(GL_POLYGON, 0, numOfClockVertices);
}

// paint digits
void renderBitmapCharacter(int x, int y, void* font, char* string) {
	switch (clockColor) {
	case ClockColor::CYAN_COLOR:
		glColor3f((GLfloat)0.0, (GLfloat)1.0, (GLfloat)1.0);
		break;
	case ClockColor::MAGENTA_COLOR:
		glColor3f((GLfloat)1.0, (GLfloat)0.0, (GLfloat)1.0);
		break;
	case ClockColor::YELLOW_COLOR:
		glColor3f((GLfloat)1.0, (GLfloat)1.0, (GLfloat)0.0);
		break;
	}
	glRasterPos2d(x, y);
	glutBitmapString(font, (const unsigned char*)string);
}

coordinate rotate(coordinate coord, GLfloat theta) {
	return {
		coord.x * cos(theta) - coord.y * sin(theta),
		coord.x * sin(theta) + coord.y * cos(theta)
	};
};

void drawDigits() {
	int digitRadius = 0;
	coordinate digitOffset = { 0, 0 };

	switch (clockSize) {
	case ClockSize::SMALL_SIZE:
		digitRadius = 65;
		digitOffset = { 5, 5 };
		font = GLUT_BITMAP_TIMES_ROMAN_10;
		break;
	case ClockSize::MEDIUM_SIZE:
		digitRadius = 90;
		digitOffset = { 10, 10 };
		font = GLUT_BITMAP_TIMES_ROMAN_24;
		break;
	case ClockSize::LARGE_SIZE:
		digitRadius = 135;
		digitOffset = { 10, 10 };
		font = GLUT_BITMAP_TIMES_ROMAN_24;
		break;
	}

	switch (clockDigits) {
	case ClockDigits::SHOW_DIGITS: {
		// use OpenGL fixed pipeline functions
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-window_w / 2, window_w / 2, -window_h / 2, window_h / 2);

		const float decrement = (float)(2.0f * PI / numOfDigits);
		float theta = PI / 2.0;

		for (int i = 0; i < numOfDigits; i++) {
			int x = (int)(cos(theta) * digitRadius - digitOffset.x);
			int y = (int)(sin(theta) * digitRadius - digitOffset.y);
			theta -= decrement;	// decrement instead of increment for clockwise direction
			renderBitmapCharacter(x, y, (void*)font, (char*)std::to_string(i == 0 ? 12 : i).c_str());
		}

		glUseProgram(program);
		break;
	}

	case ClockDigits::HIDE_DIGITS: {
		// render clock dials
		glBindVertexArray(VAO[clockDialIndex]);

		// clock dial vertices
		float theta = PI / 2;
		const float decrement = PI * 2 / numOfDials;

		for (int i = 0; i < numOfDials; i++) {
			// inner part of dial
			const coordinate corner1 = { -0.01f, clockDiameter * 0.29f };
			const coordinate corner2 = { +0.01f, clockDiameter * 0.29f };
			const coordinate corner3 = { +0.00f, clockDiameter * (i % 3 == 0 ? 0.20f : 0.25f) };

			// outer part of dial
			const coordinate corner4 = { -0.01f, clockDiameter * 0.30f };
			const coordinate corner5 = { +0.01f, clockDiameter * 0.30f };
			const coordinate corner6 = { +0.00f, clockDiameter * (i % 3 == 0 ? 0.36f : 0.32f) };

			clockDial[i][0] = rotate(corner1, theta);
			clockDial[i][1] = rotate(corner2, theta);
			clockDial[i][2] = rotate(corner3, theta);
			clockDial[i][3] = rotate(corner4, theta);
			clockDial[i][4] = rotate(corner5, theta);
			clockDial[i][5] = rotate(corner6, theta);

			theta -= decrement;
		}
		glBindBuffer(GL_ARRAY_BUFFER, VBO[clockDialIndex * 2 + 0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockDial), clockDial);

		// clock dial color
		int uniformLocation = glGetUniformLocation(program, "colorChoice");
		glBindBuffer(GL_ARRAY_BUFFER, VBO[clockDialIndex * 2 + 1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockDialColor), clockDialColor);
		glUniform1i(uniformLocation, 1);
		glDrawArrays(GL_TRIANGLES, 0, numOfDials * numOfDialVertices);
		break;
	}
	}
}

// paint clock hands
void generateHandVertices(int _) {
	time_t curTime = time(0);
	tm* localTime = new tm;
	localtime_s(localTime, &curTime);

	for (int index = 0; index < 3; index++) {
		double theta;
		switch (index) {
		case Hand::SEC:
			theta = PI / 2 - (localTime->tm_sec / 60.0) * 2.0 * PI;
			break;
		case Hand::MIN:
			theta = PI / 2 - (localTime->tm_min / 60.0) * 2.0 * PI;
			theta -= (localTime->tm_sec / 60.0) * (1.0 / 60.0) * 2.0 * PI;	// accurate minute hand, based on current second
			break;
		case Hand::HOUR:
			theta = PI / 2 - (localTime->tm_hour / 12.0) * 2.0 * PI;
			theta -= (localTime->tm_min / 60.0) * (1.0 / 12.0) * 2.0 * PI;	// accurate hour hand, based on current minute
			break;
		}

		double handLength =
			(clockDiameter / 2) * (
				index == Hand::SEC ? 0.70 :
				index == Hand::MIN ? 0.60 :
				index == Hand::HOUR ? 0.50 : 0);

		// inner part of hand
		clockHand[index][0].x = (GLfloat)(cos(theta + 0.25) * clockDiameter * 0.08);
		clockHand[index][0].y = (GLfloat)(sin(theta + 0.25) * clockDiameter * 0.08);
		clockHand[index][1].x = (GLfloat)(cos(theta) * clockDiameter * 0.06);
		clockHand[index][1].y = (GLfloat)(sin(theta) * clockDiameter * 0.06);
		clockHand[index][2].x = (GLfloat)(cos(theta - 0.25) * clockDiameter * 0.08);
		clockHand[index][2].y = (GLfloat)(sin(theta - 0.25) * clockDiameter * 0.08);
		clockHand[index][3].x = 0;
		clockHand[index][3].y = 0;

		// outer part of hand
		clockHand[index][4].x = (GLfloat)(cos(theta + 0.25) * clockDiameter * 0.08);
		clockHand[index][4].y = (GLfloat)(sin(theta + 0.25) * clockDiameter * 0.08);
		clockHand[index][5].x = (GLfloat)(cos(theta) * clockDiameter * 0.1);
		clockHand[index][5].y = (GLfloat)(sin(theta) * clockDiameter * 0.1);
		clockHand[index][6].x = (GLfloat)(cos(theta - 0.25) * clockDiameter * 0.08);
		clockHand[index][6].y = (GLfloat)(sin(theta - 0.25) * clockDiameter * 0.08);
		clockHand[index][7].x = (GLfloat)(cos(theta) * handLength);
		clockHand[index][7].y = (GLfloat)(sin(theta) * handLength);

		glBindVertexArray(VAO[index + Clock::CLOCK_LENGTH]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) * 2]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(clockHand[index]), clockHand[index]);
	}

	glFlush();
	glutTimerFunc(interval, generateHandVertices, 0);
}

void drawHand(int index) {
	int uniformLocation = glGetUniformLocation(program, "colorChoice");
	glBindVertexArray(VAO[index + Clock::CLOCK_LENGTH]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[(index + Clock::CLOCK_LENGTH) + 1]);
	glBufferSubData(GL_ARRAY_BUFFER, 1, sizeof(clockHandColor), clockHandColor);
	glUniform1i(uniformLocation, 1);

	glDrawArrays(GL_QUADS, 0, numOfClockVertices);
}

// display method
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// clock frame
	generateClockVertices(0, 0, GLfloat(clockDiameter * 1.00), Clock::FRAME);
	drawClock(Clock::FRAME);

	// clock frame shadow
	generateClockVertices(0, 0, GLfloat(clockDiameter * 0.80), Clock::FRAME_SHADOW);
	drawClock(Clock::FRAME_SHADOW);

	// clock body
	generateClockVertices(0, 0, GLfloat(clockDiameter * 0.75), Clock::BODY);
	drawClock(Clock::BODY);

	// clock digits
	drawDigits();

	// clock hands
	drawHand(Hand::SEC);	// second hand
	drawHand(Hand::MIN);	// minute hand
	drawHand(Hand::HOUR);	// hour hand

	glFlush();
}

// menu
void processMenuEvents(int option) {
	switch (static_cast<MenuOption>(option)) {
		// shape options:
	case MenuOption::CIRCLE:
		clockShape = ClockShape::CIRCLE_SHAPE;
		break;
	case MenuOption::SQUARE:
		clockShape = ClockShape::SQUARE_SHAPE;
		break;

		// color options
	case MenuOption::CYAN:
		clockColor = ClockColor::CYAN_COLOR;
		break;
	case MenuOption::MAGENTA:
		clockColor = ClockColor::MAGENTA_COLOR;
		break;
	case MenuOption::YELLOW:
		clockColor = ClockColor::YELLOW_COLOR;
		break;

		// size options
	case MenuOption::SMALL:
		clockSize = ClockSize::SMALL_SIZE;
		clockDiameter = 0.75f;
		break;
	case MenuOption::MEDIUM:
		clockSize = ClockSize::MEDIUM_SIZE;
		clockDiameter = 1.00f;
		break;
	case MenuOption::LARGE:
		clockSize = ClockSize::LARGE_SIZE;
		clockDiameter = 1.50f;
		break;

		// digits options
	case MenuOption::SHOW:
		clockDigits = ClockDigits::SHOW_DIGITS;
		break;
	case MenuOption::HIDE:
		clockDigits = ClockDigits::HIDE_DIGITS;
		break;

		// exit
	case MenuOption::EXIT:
		exit(0);
		break;
	}

	glutPostRedisplay();
}

void createMenu() {
	// > clock shape menu
	int clockShapeMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Circle", MenuOption::CIRCLE);
	glutAddMenuEntry("Square", MenuOption::SQUARE);

	// > clock color menu
	int clockColorMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Cyan", MenuOption::CYAN);
	glutAddMenuEntry("Magenta", MenuOption::MAGENTA);
	glutAddMenuEntry("Yellow", MenuOption::YELLOW);

	// > clock size menu
	int clockSizeMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Small", MenuOption::SMALL);
	glutAddMenuEntry("Medium", MenuOption::MEDIUM);
	glutAddMenuEntry("Large", MenuOption::LARGE);

	// > clock digits menu
	int clockDigitsMenu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Show", MenuOption::SHOW);
	glutAddMenuEntry("Hide", MenuOption::HIDE);

	// main menu
	int menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Shape", clockShapeMenu);
	glutAddSubMenu("Color", clockColorMenu);
	glutAddSubMenu("Size", clockSizeMenu);
	glutAddSubMenu("Digits", clockDigitsMenu);
	glutAddMenuEntry("Exit", MenuOption::EXIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// main
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	glutInitWindowSize(window_w, window_h);
	glutInitWindowPosition(50, 25);
	glutCreateWindow("Make a Clock");

	glewInit();
	init();
	createMenu();

	glutTimerFunc(interval, generateHandVertices, 0);
	glutDisplayFunc(display);
	glutIdleFunc(display);

	glutMainLoop();
}