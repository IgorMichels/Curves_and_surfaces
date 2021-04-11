#include "defs.h"
#include "curvas.h"

Curva alpha;

bool frenet = false;
bool grid = false;
bool localMode = false;
bool customPos = false;
bool lineDepth = false;
float stepSize = 0.1;

void Curvas_start(int argc, char **argv)
{

}

void Curvas_update()
{
    alpha.setParameter(alpha.parameter + masterSpeed * cameraSpeed * deltaTime * (keys[']'] - keys['[']));

    if(!flyMode && localMode && alpha.iFrenet >= 0)
    {
        cameraFront = alpha.pieces[alpha.iFrenet].tangent;
        cameraLeft = -alpha.pieces[alpha.iFrenet].binormal;
        cameraUp = -alpha.pieces[alpha.iFrenet].normal;
        cameraPos = alpha.pieces[alpha.iFrenet].point + cameraUp/5.0f + cameraLeft/10.0f - cameraFront/5.0f;
        updateCamera();
    }
}

void drawGrid(bool lineDepth)
{
    float N = 20;
    float S = 1;

    
    for(float k = -N; k <= +N; k++)
    {
        if(lineDepth)
            glLineWidth(2 / length((vec2){cameraPos.y - k*S, cameraPos.z}));
        else 
            glLineWidth(2);

        glBegin(GL_LINES);  
        glColor3f(0.5, 0, 0);
        glVertex3f(+N*S, +k*S, 0);
        glVertex3f(-N*S, +k*S, 0);
        glEnd();

        if(lineDepth)
            glLineWidth(2 / length((vec2){cameraPos.x - k*S, cameraPos.z}));
        else
            glLineWidth(2);

        glBegin(GL_LINES);
        glColor3f(0, 0.5, 0);
        glVertex3f(+k*S, +N*S, 0);
        glVertex3f(+k*S, -N*S, 0);
        glEnd();
    }

    if(lineDepth)
        glLineWidth(2 / length((vec2){cameraPos.x, cameraPos.z}));
    else 
        glLineWidth(2);

    glBegin(GL_LINES);
    glColor3f(0, 0, 0.5);
    glVertex3f(0, 0, +N*S);
    glVertex3f(0, 0, -N*S);
    glEnd();

    glEnd();
}

void Curvas_draw()
{
    if(grid) drawGrid(lineDepth);
    if(frenet) alpha.drawFrenet(lineDepth);
    alpha.drawCurve(lineDepth);
}

void Curvas_keypress(unsigned char key, int x, int y)
{
    Frenet cur = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    if(customPos) cur = {cameraFront, -cameraUp, cameraLeft, cameraPos};
    switch(key)
    {
        case 'g':
            grid = !grid;
            break;
        case 'f':
            frenet = !frenet;
            break;
        case 'l':
            localMode = !localMode;
            break;
        case 'c':
            customPos = !customPos;
            break;
        case 'z':
            stepSize /= 2;
            if(stepSize < 0.0001) stepSize = 0.0001;
            break;
        case 'x':
            stepSize *= 2;
            break;
        case 'b':
            lineDepth = !lineDepth;
            break;
        case '0':
            alpha = CURVA(1, 0);
            alpha.eulerBuild(-PI, PI+0.1, stepSize, cur);
            break;
        case '1':
            alpha = CURVA(1, 0.3);
            alpha.eulerBuild(-4*PI, 4*PI, stepSize, cur);
            break;
        case '2':
            alpha = CURVA(t, t/10);
            alpha.eulerBuild(-4*PI, 4*PI, stepSize, cur);
            break;
        case '3':
            alpha = CURVA(1, sin(t)/9);
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '4':
            alpha = CURVA(sin(t), 0);
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '5':
            alpha = CURVA(1, t/abs(t+1));
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '6':
            alpha = CURVA(3*exp(-t*t/9)/2, 0.1);
            alpha.eulerBuild(-4, 4, stepSize, cur);
            break;
        case '7':
            alpha = CURVA(0, 0);
            alpha.eulerBuild(-4, 4, stepSize, cur);
            break;
        case '8':
            alpha = CURVA(0, 1);
            alpha.eulerBuild(-4, 4, stepSize, cur);
            break;
    }
}

void Curvas_mousewheel(int wheel, int direction)
{
    printf("%d\n", direction);
}