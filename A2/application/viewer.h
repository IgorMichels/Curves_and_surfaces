#ifndef VIEWER_H
#define VIEWER_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

using namespace glm;

struct Camera
{
    vec3 pos;
    vec2 ang;

    vec3 front, left, up;
};

extern const float pi;

extern int width;
extern int height;

extern Camera mainCamera;
extern Camera *cam;

extern float currentTime;
extern float deltaTime;
extern float masterSpeed;
extern float cameraSpeed;
extern float mouseSpeed;

extern bool lookMode;
extern bool flyMode;

extern float keys[256];
extern int nKeys;

void updateCamera();
void display();
void idle();
void reshape(int, int);
void motion(int, int);
void keyDown(unsigned char, int, int);
void keyUp(unsigned char, int, int);
int start(int, char**);

void surf_idle();
void surf_draw();
void surf_keydown(unsigned char);

#endif