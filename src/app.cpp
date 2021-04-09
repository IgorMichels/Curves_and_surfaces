#include "defs.h"
#include "curvas.h"

Curva alpha;

bool frenet = false;
bool grid = false;

void Curvas_start(int argc, char **argv)
{
}

void Curvas_update()
{
    alpha.setParameter(alpha.parameter + masterSpeed * cameraSpeed * deltaTime * (keys[']'] - keys['[']));

    if(!flyMode && alpha.iFrenet >= 0)
    {
        cameraFront = alpha.pieces[alpha.iFrenet].tangent;
        cameraLeft = -alpha.pieces[alpha.iFrenet].binormal;
        cameraUp = -alpha.pieces[alpha.iFrenet].normal;
        cameraPos = alpha.pieces[alpha.iFrenet].point + cameraUp/5.0f + cameraLeft/10.0f - cameraFront/5.0f;
        updateCamera();
    }
}

void Curvas_draw()
{
    if(grid) alpha.drawGrid();
    if(frenet) alpha.drawFrenet();
    alpha.drawCurve();
}

void Curvas_keypress(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 'g':
            grid = !grid;
            break;
        case 'f':
            frenet = !frenet;
            break;
        case '0':
            alpha = CURVA(1, 0);
            alpha.eulerBuild(-PI, PI+0.1, 0.01, {cameraFront, -cameraUp, -cameraLeft, cameraPos});
            break;
        case '1':
            alpha = CURVA(1, 0.3);
            alpha.eulerBuild(-4*PI, 4*PI, 0.01, {cameraFront, -cameraUp, -cameraLeft, cameraPos});
            break;
        case '2':
            alpha = CURVA(t, t/10);
            alpha.eulerBuild(-4*PI, 4*PI, 0.01, {cameraFront, -cameraUp, -cameraLeft, cameraPos});
            break;
        case '3':
            alpha = CURVA(1, sin(t)/9);
            alpha.eulerBuild(-8*PI, 8*PI, 0.01, {cameraFront, -cameraUp, -cameraLeft, cameraPos});
            break;
        case '4':
            alpha = CURVA(sin(t), 0);
            alpha.eulerBuild(-8*PI, 8*PI, 0.01, {cameraFront, -cameraUp, -cameraLeft, cameraPos});
            break;
    }
}

void Curvas_mousewheel(int wheel, int direction)
{
    printf("%d\n", direction);
}