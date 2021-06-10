#include "defs.h"
#include "curvas.h"

#include <vector>

Curva alpha;

bool frenet = true;
bool grid = true;
bool localMode = false;
bool customPos = false;
bool lineDepth = true;
bool drawStars = true;
float stepSize = 0.001;
const char *curveName = "None";

std::vector<vec3> stars;

void Curvas_start(int argc, char **argv)
{
    for(int i = 0; i < 1000; i++)
    {
        vec3 star = {(float)rand()/RAND_MAX*2 - 1, (float)rand()/RAND_MAX*2 - 1, (float)rand()/RAND_MAX*2 - 1};
        star = normalize(star) * 1000.0f;
        stars.push_back(star);
    }
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
        glLineWidth(2 / length((vec2){cameraPos.x, cameraPos.y}));
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
    static float fps_time = 0;
    static int frames = 0;
    if(currentTime - fps_time > 0.5)
    {
        fps_time = currentTime;
        int i = system("clear");
        printf("FPS: %d\n", frames);
        if(localMode && !flyMode) printf("Local View\n");
        else printf("Global view\n");
        printf("Frenet: %d\n", frenet);
        printf("Grid: %d\n", grid);
        printf("Line depth: %d\n", lineDepth);
        printf("Custom position: %d\n", customPos);
        printf("Curve: %s\n", curveName);
        printf("Step size: %g\n", stepSize);

        frames = 0;
    }
    frames++;

    if(drawStars)
    {
        glPointSize(0.1);
        float c = 0.3;
        glColor3f(c, c, c);
        glBegin(GL_POINTS);
        for(vec3 s : stars)
        {
            glVertex3f(s.x, s.y, s.z);
        }
        glEnd();
    }


    if(grid) drawGrid(lineDepth);
    if(frenet) alpha.drawFrenet(lineDepth);
    alpha.drawCurve(lineDepth);
}

void Curvas_keypress(unsigned char key, int x, int y)
{
    Frenet cur = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    if(customPos) cur = {cameraFront, cameraLeft, cameraUp, cameraPos};
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
            if(stepSize < 0.00001) stepSize = 0.00001;
            break;
        case 'x':
            stepSize *= 2;
            break;
        case 'b':
            lineDepth = !lineDepth;
            break;
        case 'p':
            drawStars = !drawStars;
            break;
        case '0':
            curveName = "Circle (1, 0)";
            alpha = CURVA(1, 0);
            alpha.eulerBuild(-PI, PI+0.1, stepSize, cur);
            break;
        case '1':
            curveName = "Helix (1, 0.3)";
            alpha = CURVA(1, 0.3);
            alpha.eulerBuild(-4*PI, 4*PI, stepSize, cur);
            break;
        case '2':
            curveName = "(t, t/10)";
            alpha = CURVA(t, t/10);
            alpha.eulerBuild(-4*PI, 4*PI, stepSize, cur);
            break;
        case '3':
            curveName = "Ball (1, sin(t)/9)";
            alpha = CURVA(1, sin(t)/9);
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '4':
            curveName = "Sine (sin(t), 0)";
            alpha = CURVA(sin(t), 0);
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '5':
            curveName = "Bent helix (1, t/|t+1|)";
            alpha = CURVA(1, t/abs(t+1));
            alpha.eulerBuild(-8*PI, 8*PI, stepSize, cur);
            break;
        case '6':
            curveName = "Strip (2.4*sin(t), 0.2)";
            alpha = CURVA(2.4*cos(t), t*sin(t));
            alpha.eulerBuild(-20, 20, stepSize, cur);
            break;
        case '7':
            curveName = "Line (0, 0)";
            alpha = CURVA(0, 0);
            alpha.eulerBuild(-4, 4, stepSize, cur);
            break;
        case '8':
            curveName = "Line (0, 1)";
            alpha = CURVA(0, 1);
            alpha.eulerBuild(-4, 4, stepSize, cur);
            break;
        case '9':
            curveName = "Curve (t*sin(t), 0.5)";
            alpha = CURVA(t*sin(t), 0.5);
            alpha.eulerBuild(-10, 10, stepSize, cur);
            break;
    }
}

void Curvas_mousewheel(int wheel, int direction)
{
    printf("%d\n", direction);
}