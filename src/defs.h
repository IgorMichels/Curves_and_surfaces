#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

using namespace glm;

const float PI = 3.141592653589793238463;

void Curvas_keypress(unsigned char key, int x, int y);
void Curvas_draw();
void Curvas_update();
void Curvas_start(int, char **);

#define CURVA(x, y) Curva([](float t) { return (float)(x);}, [](float t) { return (float)(y);})

typedef float (*Func)(float);

struct Curva
{
    Func curvature, torsion;


    float parameter;
    vec3 tangent, normal, binormal, point;

    void reset()
    {
        parameter = 0;
        tangent = {1, 0, 0};
        normal = {0, 1, 0};
        binormal = {0, 0, 1};
        point = {0, 0, 0};
    }

    Curva(Func c, Func t)
    {
        curvature = c;
        torsion = t;
        reset();
    }

    void eulerStep(float h)
    {
        float K = curvature(parameter);
        float T = torsion(parameter);

        vec3 nTangent = normalize(tangent + K*normal*h);
        vec3 nNormal = normalize(normal + (T*binormal - K*tangent)*h);
        vec3 nBinormal = cross(nTangent, nNormal);

        parameter += h;
        point += tangent*h;

        tangent = nTangent;
        normal = nNormal;
        binormal = nBinormal;
    }
};


#endif