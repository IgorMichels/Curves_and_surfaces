#include "defs.h"
#include "curvas.h"

Curva alpha;

bool frenet = false;
bool grid = false;

void Curvas_start(int argc, char **argv)
{
    alpha = CURVA(1, 1);
    alpha.eulerBuild(-20, 20, 0.01);
}

void Curvas_update()
{
    alpha.setParameter(alpha.parameter + masterSpeed * deltaTime * (keys[']'] - keys['[']));

    if(!flyMode)
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
        case 'r':
            Frenet cur = {cameraFront, -cameraUp, -cameraLeft, cameraPos};
            alpha.eulerBuild(-20, 20, 0.01, cur);
            break;
    }
}

void Curvas_mousewheel(int wheel, int direction)
{
    printf("%d\n", direction);
}