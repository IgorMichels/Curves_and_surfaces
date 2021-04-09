#include "defs.h"

float delta = 0.1;

void Curvas_start(int argc, char **argv)
{
}

void Curvas_update()
{

}

void Curvas_draw()
{

    glLineWidth(2);
    glBegin(GL_LINES);

    //X axis
    glColor4f(1, 0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glColor4f(0.5, 0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(-1, 0, 0);

    //Y axis
    glColor4f(0, 1, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glColor4f(0, 0.5, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, -1, 0);

    //Z axis
    glColor4f(0, 0, 1, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glColor4f(0, 0, 0.5, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, -1);

    glEnd();

    static Curva c1 = CURVA(sin(t), 0);
    c1.reset();
    glLineWidth(2);
    glBegin(GL_LINES);
    while(c1.parameter < 100)
    {
        glColor4f((c1.tangent.x+1)/2, (c1.tangent.y+1)/2, (c1.tangent.z+1)/2, 1);
        glVertex3f(c1.point.x, c1.point.y, c1.point.z);
        c1.eulerStep(delta);
        glColor4f((c1.tangent.x+1)/2, (c1.tangent.y+1)/2, (c1.tangent.z+1)/2, 1);
        glVertex3f(c1.point.x, c1.point.y, c1.point.z);
    }
    glEnd();
}

void Curvas_keypress(unsigned char key, int x, int y)
{
    switch(key)
    {
        case '1':
            delta /= 2;
            if(delta < 0.001) delta = 0.001;
            break;
        case '2':
            delta *= 2;
            break;
    }
}