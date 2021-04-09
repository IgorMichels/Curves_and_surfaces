#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

using namespace glm;

const float PI = 3.14159265358979323846;
const float E = 2.71828182845904523536;

void Curvas_keypress(unsigned char, int, int);
void Curvas_mousewheel(int, int);
void Curvas_draw();
void Curvas_update();
void Curvas_start(int, char **);
void updateCamera();

extern int width, height;

extern vec3 cameraPos;
extern vec2 cameraAng;
extern vec3 cameraFront;
extern vec3 cameraLeft;
extern vec3 cameraUp;

extern float currentTime; 
extern float deltaTime;
extern float masterSpeed;
extern float cameraSpeed;
extern float mouseSpeed;

extern bool flyMode;
extern float keys[256];
extern int nKeys;

#endif