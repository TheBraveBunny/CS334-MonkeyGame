/**
*	CS 334 - Fundamentals of Computer Graphics
*	Framework for assignment 2
*
*	Based on:
*		http://antongerdelan.net/opengl/cubemaps.html
*		https://github.com/capnramses/antons_opengl_tutorials_book/tree/master/21_cube_mapping
*
*	Instructions:
*	- Use Arrow Keys, q, w, e, a, s, d, z, x and c
*	- Press ESC to exit
*/

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <ctime>

#include "GL/glew.h"
#include "GL/glut.h"

#include "stb_image.h"
#include "math_funcs.h"
#include "obj_parser.h"
#include "gl_utils.h"

#define MESH_FILE_SUZANNE "suzanne.obj"
#define MESH_FILE "sphere.obj"
#define MONKEY_VERT_FILE "reflect_vs.glsl"
#define MONKEY_FRAG_FILE "reflect_fs.glsl"
#define MONKEY_REFRACT_VERT_FILE "refract_vs.glsl"
#define MONKEY_REFRACT_FRAG_FILE "refract_fs.glsl"

#define CUBE_VERT_FILE "cube_vs.glsl"
#define CUBE_FRAG_FILE "cube_fs.glsl"
#define FRONT "negz.jpg"
#define BACK "posz.jpg"
#define TOP "posy.jpg"
#define BOTTOM "negy.jpg"
#define LEFT "negx.jpg"
#define RIGHT "posx.jpg"

/* Window information */
float windowWidth = 800;
float windowHeight = 600;

/* Information about the texture */
const char* textureFile = "crate.jpg";
unsigned char* textureData;
GLint textureDataLocation;
int textureWidth;
int textureHeight;
int textureComp;
GLuint texture;

/* Information about the texture */
const char* whiteFile = "sphere1.bmp";
unsigned char* whiteData;
GLuint white;
GLint whiteDataLocation;
int whiteWidth;
int whiteHeight;
int whiteComp;

/* Information about the normal */
const char* normalFile = "example_normalmap.png";
unsigned char* normalData;
GLint normalDataLocation;
GLint cubeDataLocation;
int normalWidth;
int normalHeight;
int normalComp;
GLuint normal;

/* big cube. returns Vertex Array Object */
GLuint make_big_cube () {
	float points[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		 
		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		-10.0f,  10.0f, -10.0f,
		 10.0f,  10.0f, -10.0f,
		 10.0f,  10.0f,  10.0f,
		 10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		 10.0f, -10.0f, -10.0f,
		 10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		 10.0f, -10.0f,  10.0f
	};
	GLuint vbo;
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (
		GL_ARRAY_BUFFER, 3 * 36 * sizeof (GLfloat), &points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	return vao;
}

GLuint vao;
GLuint vao2;

GLuint cube_sp;
GLuint cube_map_texture;
GLuint cube_vao;

GLuint monkey_sp;
int monkey_M_location;
int monkey_V_location;
int monkey_P_location;

GLuint suzanne_sp;
int suzanne_M_location;
int suzanne_V_location;
int suzanne_P_location;

int g_point_count = 0;
int g_point_count2 = 0;

const int numOfBugs = 6;
mat4 model_mat[numOfBugs];
int clickedLocation;
bool clicked;
int frameCount[numOfBugs];
float planeAngle = -45.0f;
int score = 0;
int waitFrame = 1000;
int timeElapsed = 0;

float cam_speed = 3.0f; // 1 unit per second
float cam_heading_speed = 5.00f; // 30 degrees per second
float cam_heading = 0.0f; // y-rotation in degrees

mat4 T;
mat4 R;
versor q;

vec4 fwd;
vec4 rgt;
vec4 up;

// camera matrices. it's easier if they are global
mat4 view_mat; 
mat4 proj_mat;
vec3 cam_pos (0.0f, 0.0f, 5.0f);

int cube_V_location;
int cube_P_location;

bool cam_moved = false;
vec3 move;
float cam_yaw = 0.0f; // y-rotation in degrees
float cam_pitch = 0.0f;
float cam_roll = 0.0;

double previous_seconds;
double current_seconds;
double elapsed_seconds;

/* use stb_image to load an image file into memory, and then into one side of
a cube-map texture. */
bool load_cube_map_side (
	GLuint texture, GLenum side_target, const char* file_name
) {
	glBindTexture (GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int force_channels = 4;
	unsigned char*  image_data = stbi_load (
		file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// non-power-of-2 dimensions check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (
			stderr, "WARNING: image %s is not power-of-2 dimensions\n", file_name
		);
	}
	
	// copy image data into 'target' side of cube map
	glTexImage2D (
		side_target,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	free (image_data);
	return true;
}


/* load all 6 sides of the cube-map from images, then apply formatting to the
final texture */
void create_cube_map (
	const char* front,
	const char* back,
	const char* top,
	const char* bottom,
	const char* left,
	const char* right,
	GLuint* tex_cube
) {
	// generate a cube-map texture to hold all the sides
	glActiveTexture (GL_TEXTURE0);
	glGenTextures (1, tex_cube);
	
	// load each image and copy into a side of the cube-map texture
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front));
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back));
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top));
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom));
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left));
	assert (load_cube_map_side (*tex_cube, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right));
	// format cube map texture
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

float step0[numOfBugs];
float step1[numOfBugs];
float xPos[numOfBugs];
float yPos[numOfBugs];
float planeRotation = 0;
float speedMultiplier = 1;

void updateXPos(int index) {
	xPos[index] += step0[index]*speedMultiplier;
	model_mat[index].m[12] = cos(planeRotation)*xPos[index]-sin(planeRotation)*yPos[index];
}

void updateYPos(int index) {
	yPos[index] += step1[index]*speedMultiplier;
	model_mat[index].m[13] = sin(planeRotation)*xPos[index] + cos(planeRotation)*yPos[index] + 1;
}

void updateZPos(int index) {
	model_mat[index].m[14] = (model_mat[index].m[13] + 3) / tan(planeAngle) - 3;
}

void moveItems() {
	planeRotation += .0005;
	for (int i = 0; i < numOfBugs; i++) {
		updateXPos(i);
		updateYPos(i);
		updateZPos(i);
	}
}

void clearDrawingSurface() {
	glClearStencil(0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void renderSkyBox() {
	// render a sky-box using the cube-map texture
	glStencilFunc(GL_ALWAYS, 0.0, 0.0);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glDepthMask (GL_FALSE);
	glUseProgram (cube_sp);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_CUBE_MAP, cube_map_texture);
	glBindVertexArray (cube_vao);
	glDrawArrays (GL_TRIANGLES, 0, 36);
	glDepthMask (GL_TRUE);

	for (int i = 0; i < numOfBugs; i++) {
		glStencilFunc(GL_ALWAYS, i+1, 0.0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		if (i % 3 == 0) {
			glUseProgram(suzanne_sp);
			glBindVertexArray(vao2);
		}
		else {
			glUseProgram(monkey_sp);
			glBindVertexArray(vao);
		}
		glUniformMatrix4fv(monkey_M_location, 1, GL_FALSE, model_mat[i].m);
		if (++frameCount[i] > waitFrame) {
			glUniform1i(clickedLocation, 0);
		} else if (frameCount[i] == waitFrame) {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(textureDataLocation, 2);
			glUniform1i(clickedLocation, 1);
			if ((i % 3) == 0) {
				speedMultiplier = speedMultiplier / 8;
			}
		} else {
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, texture);
			glUniform1i(textureDataLocation, 2);
			glUniform1i(clickedLocation, 1);
		}
		glDrawArrays(GL_TRIANGLES, 0, g_point_count);
	}
}

void checkXBoundary(int index) {
	if (xPos[index] > 3) {
		step0[index] = -step0[index];
	}
	else if (xPos[index] < -3) {
		step0[index] = -step0[index];
	}
}

void checkYBoundary(int index) {
	if (yPos[index] > 5) {
		step1[index] = -step1[index];
	}
	else if (yPos[index] < -3) {
		step1[index] = -step1[index];
	}
}

void checkBoundaries() {
	for (int i = 0; i < numOfBugs; i++) {
		checkXBoundary(i);
		checkYBoundary(i);
	}
}

void updateViewMatrix() {
	// update view matrix
	//if (cam_moved) {
	cam_heading += cam_yaw;

	// re-calculate local axes so can move fwd in dir cam is pointing
	R = quat_to_mat4(q);
	fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
	rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
	up = R * vec4(0.0, 1.0, 0.0, 0.0);

	cam_pos = cam_pos + vec3(fwd) * -move.v[2];
	cam_pos = cam_pos + vec3(up) * move.v[1];
	cam_pos = cam_pos + vec3(rgt) * move.v[0];

	checkBoundaries();

	mat4 T = translate(identity_mat4(), vec3(cam_pos));

	view_mat = inverse(R) * inverse(T);
	glUseProgram(monkey_sp);
	glUniformMatrix4fv(monkey_V_location, 1, GL_FALSE, view_mat.m);
	glUseProgram(suzanne_sp);
	glUniformMatrix4fv(monkey_V_location, 1, GL_FALSE, view_mat.m);

	// cube-map view matrix has rotation, but not translation
	glUseProgram(cube_sp);
	glUniformMatrix4fv(cube_V_location, 1, GL_FALSE, inverse(R).m);

	move = vec3(0.0, 0.0, 0.0);
	cam_yaw = 0.0f;
	cam_pitch = 0.0f;
	cam_roll = 0.0;
	cam_moved = false;
	//}
}

void drawText(const char *text, int length, int x, int y) {
	glMatrixMode(GL_PROJECTION);
	double *matrix = new double[16];
	glGetDoublev(GL_PROJECTION_MATRIX, matrix);
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -5, 5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glLoadIdentity();
	glColor3f(0, 1, 0);
	glRasterPos2i(x, y);
	for (int i = 0; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)text[i]);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(matrix);
	glMatrixMode(GL_MODELVIEW);
}

void printText() {
	glUseProgram(0);
	std::string text;
	char buf[5];
	_itoa(score, buf, 10);
	text = "Score: ";
	text.append(buf);
	drawText(text.data(), text.size(), 50, 50);

	glUseProgram(0);
	std::string timeText;
	char timeBuf[5];
	timeElapsed = glutGet(GLUT_ELAPSED_TIME)/1000 - 3;
	if (timeElapsed > 60) {
		timeElapsed = 60;
	}
	_itoa(timeElapsed, timeBuf, 10);
	timeText = "Time: ";
	timeText.append(timeBuf);
	drawText(timeText.data(), timeText.size(), 50, 550);
}

/**
*    Function invoked for drawing using OpenGL
*/
void display()
{
	elapsed_seconds = 1;
	clearDrawingSurface();
	moveItems();
	if (timeElapsed < 60) {
		renderSkyBox();
		updateViewMatrix();
	}
	printText();
	
	/* Force execution of OpenGL commands */
	glFlush();

	/* Swap buffers for animation */
	glutSwapBuffers();
}

/**
*    Function invoked when window system events are not being received
*/
void idle()
{
	/* Redraw the window */
	glutPostRedisplay();
}

int checkForHit(int x, int y) {
	GLuint index;
	glReadPixels(x, windowHeight - y - 1, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &index);
	return index;
}

/**
*    Function invoked when an event on a regular keys occur
*/
void keyboard(unsigned char k, int x, int y)
{
	/*if(k == 'a') {
		move.v[0] -= cam_speed * elapsed_seconds;
	} else if(k == 'd') {
		move.v[0] += cam_speed * elapsed_seconds;
	} else if(k == 'q') {
		move.v[1] += cam_speed * elapsed_seconds;
	} else if(k == 'e') {
		move.v[1] -= cam_speed * elapsed_seconds;
	} else if(k == 'w') {
		move.v[2] -= cam_speed * elapsed_seconds;
	} else if(k == 's') {
		move.v[2] += cam_speed * elapsed_seconds;
	} else if(k == 'z') {
		cam_roll -= cam_heading_speed * elapsed_seconds;
		versor q_roll = quat_from_axis_deg (
			cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]
		);
		q = q_roll * q;
	} else if(k == 'c') {
		cam_roll += cam_heading_speed * elapsed_seconds;
		versor q_roll = quat_from_axis_deg (
			cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]
		);
		q = q_roll * q;
	} else */if (k == 27) {
        /* Close application if ESC is pressed */
        exit(0);
    }
}

void special(int key, int x, int y) 
{
	/*if(key == GLUT_KEY_LEFT) {
		cam_yaw += cam_heading_speed * elapsed_seconds;
		cam_moved = true;
		versor q_yaw = quat_from_axis_deg (
			cam_yaw, up.v[0], up.v[1], up.v[2]
		);
		q = q_yaw * q;
	} else if(key == GLUT_KEY_RIGHT) {
		cam_yaw -= cam_heading_speed * elapsed_seconds;
		cam_moved = true;
		versor q_yaw = quat_from_axis_deg (
			cam_yaw, up.v[0], up.v[1], up.v[2]
		);
		q = q_yaw * q;
	} else if(key == GLUT_KEY_UP) {
		cam_pitch += cam_heading_speed * elapsed_seconds;
		cam_moved = true;
		versor q_pitch = quat_from_axis_deg (
			cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]
		);
		q = q_pitch * q;
	} else if(key == GLUT_KEY_DOWN) {
		cam_pitch -= cam_heading_speed * elapsed_seconds;
		cam_moved = true;
		versor q_pitch = quat_from_axis_deg (
			cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]
		);
		q = q_pitch * q;
	}*/
}

void myMouseFunc(int button, int state, int x, int y)
{
	if(button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		//if(checkForHit(x /(float) windowWidth, y /(float) windowHeight)) {
		int index = checkForHit(x, y);
		if ((index > 0) && (frameCount[index - 1] > waitFrame)) {
			frameCount[index - 1] = 0;
			if ((index - 1) % 3 == 0) {
				score += 5;
				speedMultiplier = speedMultiplier * 8;
			} else {
				score += 1;
			}
		}
	}
}

void setNormalMap() {
	/* Set the normal map of the model */
	normalData = stbi_load(normalFile, &normalWidth, &normalHeight, &normalComp, STBI_rgb);
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_2D, normal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normalWidth, normalHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, normalData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void setWoodenTexture() {
	/* Set the texture of the model */
	textureData = stbi_load(textureFile, &textureWidth, &textureHeight, &textureComp, STBI_rgb);
	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void setBlankTexture() {
	/* Set the texture of the model */
	glActiveTexture(GL_TEXTURE3);
	whiteData = stbi_load(whiteFile, &whiteWidth, &whiteHeight, &whiteComp, STBI_rgb);
	glGenTextures(1, &white);
	glBindTexture(GL_TEXTURE_2D, white);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, whiteWidth, whiteHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, whiteData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

void setTextures() {
	setNormalMap();
	setWoodenTexture();
	setBlankTexture();	
}

void setMonkeyShader() {
	// shaders for "Suzanne" mesh
	monkey_sp = create_programme_from_files (
		MONKEY_VERT_FILE, MONKEY_FRAG_FILE
	);
	monkey_M_location = glGetUniformLocation (monkey_sp, "M");
	monkey_V_location = glGetUniformLocation (monkey_sp, "V");
	monkey_P_location = glGetUniformLocation (monkey_sp, "P");
	normalDataLocation = glGetUniformLocation(monkey_sp, "normalData");
	textureDataLocation = glGetUniformLocation(monkey_sp, "textureData");
	cubeDataLocation = glGetUniformLocation(monkey_sp, "cube_texture");
	clickedLocation = glGetUniformLocation(monkey_sp, "clicked");

	suzanne_sp = create_programme_from_files(
		MONKEY_REFRACT_VERT_FILE, MONKEY_REFRACT_FRAG_FILE
		);
	suzanne_M_location = glGetUniformLocation(suzanne_sp, "M");
	suzanne_V_location = glGetUniformLocation(suzanne_sp, "V");
	suzanne_P_location = glGetUniformLocation(suzanne_sp, "P");
	normalDataLocation = glGetUniformLocation(suzanne_sp, "normalData");
	textureDataLocation = glGetUniformLocation(suzanne_sp, "textureData");
	cubeDataLocation = glGetUniformLocation(suzanne_sp, "cube_texture");
	clickedLocation = glGetUniformLocation(suzanne_sp, "clicked");
}

void setCubeShader() {
	// cube-map shaders
	cube_sp = create_programme_from_files (
		CUBE_VERT_FILE, CUBE_FRAG_FILE
	);
	// note that this view matrix should NOT contain camera translation.
	cube_V_location = glGetUniformLocation (cube_sp, "V");
	cube_P_location = glGetUniformLocation (cube_sp, "P");
}

void setShaders() {
	setMonkeyShader();
	setCubeShader();	
}

void createCamera() {
	/*-------------------------------CREATE CAMERA--------------------------------*/
	#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444

	// input variables
	float znear = 0.1f; // clipping plane
	float zfar = 100.0f; // clipping plane
	float fovy = 67.0f; // 67 degreesglActiveTexture
	float aspect = windowWidth / windowHeight; // aspect ratio
	proj_mat = perspective (fovy, aspect, znear, zfar);
		
	
	T = translate (
		identity_mat4 (), vec3 (-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2])
	);
	R = rotate_y_deg (identity_mat4 (), -cam_heading);
	q = quat_from_axis_deg (-cam_heading, 0.0f, 1.0f, 0.0f);
	view_mat = R * T;
	// keep track of some useful vectors that can be used for keyboard movement
	fwd = vec4(0.0f, 0.0f, -1.0f, 0.0f);
	rgt = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
}

void setRenderingDefaults() {
	/*---------------------------SET RENDERING DEFAULTS---------------------------*/
	glUseProgram (monkey_sp);
	glUniformMatrix4fv (monkey_V_location, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv (monkey_P_location, 1, GL_FALSE, proj_mat.m);
	glUniform1i(cubeDataLocation, 0);
	glUniform1i(normalDataLocation, 1);
	glUniform1i(textureDataLocation, 2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal);

	glUseProgram(suzanne_sp);
	glUniformMatrix4fv(suzanne_V_location, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(suzanne_P_location, 1, GL_FALSE, proj_mat.m);
	glUniform1i(cubeDataLocation, 0);
	glUniform1i(normalDataLocation, 1);
	glUniform1i(textureDataLocation, 2);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, normal);

	glUseProgram (cube_sp);
	glUniformMatrix4fv (cube_V_location, 1, GL_FALSE, R.m);
	glUniformMatrix4fv (cube_P_location, 1, GL_FALSE, proj_mat.m);
	// unique model matrix for each sphere
	for (int i = 0; i < numOfBugs; i++) {
		model_mat[i] = identity_mat4();
	}
	
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // set counter-clock-wise vertex order to mean the front
	glClearColor (0.2, 0.2, 0.2, 1.0); // grey background to help spot mistakes
}

void setPos(int index) {
	xPos[index] = (rand() % (int)(windowWidth))/700.0f;
	yPos[index] = (rand() % (int)(windowHeight))/700.0f;
}

void setStep(int index) {
	step0[index] = (rand() / 3000000.0f);
	step1[index] = (rand() / 3000000.0f);
}

void setInitialModels() {
	srand(time(NULL));

	for (int i = 0; i < numOfBugs; i++) {
		setPos(i);
		setStep(i);
		frameCount[i] = 10000;
	}
}

void initializeCube() {
	cube_vao = make_big_cube();

	create_cube_map(FRONT, BACK, TOP, BOTTOM, LEFT, RIGHT, &cube_map_texture);
}

void loadModels() {
	GLfloat* vp = NULL; // array of vertex points
	GLfloat* vn = NULL; // array of vertex normals
	GLfloat* vt = NULL; // array of texture coordinates

	assert(load_obj_file(MESH_FILE, vp, vt, vn, g_point_count));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint points_vbo, normals_vbo;
	if (NULL != vp) {
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, 3 * g_point_count * sizeof(GLfloat), vp, GL_STATIC_DRAW
			);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}
	if (NULL != vn) {
		glGenBuffers(1, &normals_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, 3 * g_point_count * sizeof(GLfloat), vn, GL_STATIC_DRAW
			);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);
	}

	vp = NULL; // array of vertex points
	vn = NULL; // array of vertex normals
	vt = NULL; // array of texture coordinates

	assert(load_obj_file(MESH_FILE_SUZANNE, vp, vt, vn, g_point_count2));

	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);

	GLuint points_vbo2, normals_vbo2;
	if (NULL != vp) {
		glGenBuffers(1, &points_vbo2);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo2);
		glBufferData(
			GL_ARRAY_BUFFER, 3 * g_point_count2 * sizeof(GLfloat), vp, GL_STATIC_DRAW
			);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);
	}
	if (NULL != vn) {
		glGenBuffers(1, &normals_vbo2);
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo2);
		glBufferData(
			GL_ARRAY_BUFFER, 3 * g_point_count2 * sizeof(GLfloat), vn, GL_STATIC_DRAW
			);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(1);
	}
}


/**
*    Set OpenGL initial state
*/
void init()
{
	initializeCube();
	loadModels();
	setTextures();
	setShaders();
	createCamera();
	setRenderingDefaults();
	setInitialModels();
	glEnable(GL_STENCIL_TEST);
}

void initializeGlutWindow(int argc, char **argv) {
	/* Initialize the GLUT window */
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(30, 30);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("OpenGL/FreeGLUT - Assignment 2 Framework");
}

void initializeGlew() {
	/* Init GLEW */
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}
	std::cout << "GLEW version: " << glewGetString(GLEW_VERSION) << std::endl;
}


/**
*    Main function
*/
int main(int argc, char **argv)
{
    initializeGlutWindow(argc, argv);
	initializeGlew();	

    /* Set OpenGL initial state */
    init();

    /* Callback functions */
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(myMouseFunc);

    /* Start the main GLUT loop */
    /* NOTE: No code runs after this */
    glutMainLoop();
}


//ex cred ideas
//1. crosshairs instead of cursor