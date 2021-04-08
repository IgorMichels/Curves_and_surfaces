#include "viewer.h"

const float PI = 3.141592653589793238463;

int width = 800, height = 800;

vec3 cameraPos = {-5, -1, -1};
vec2 cameraAng = {0, 0};
vec3 cameraFront;
vec3 cameraLeft;
vec3 cameraUp;

float time = 0.0f, deltaTime = 0.0f;
float masterSpeed = 1.0f;
float cameraSpeed = 1.0f;
float mouseSpeed = 1.0f;

bool mouseLook = false;
float keys[256];
int nKeys = 0;

void updateCamera()
{
    if(cameraAng.y > +PI/2) cameraAng.y = +PI/2;
    if(cameraAng.y < -PI/2) cameraAng.y = -PI/2;
    if(cameraAng.x < 0) cameraAng.x = 2*PI;
    if(cameraAng.x > 2*PI) cameraAng.x = 0;
    cameraFront = {cos(cameraAng.x)*cos(cameraAng.y), sin(cameraAng.x)*cos(cameraAng.y), sin(cameraAng.y)};
    cameraLeft = {-sin(cameraAng.x), cos(cameraAng.x), 0};
    cameraUp = cross(cameraFront, cameraLeft);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        cameraPos.x, cameraPos.y, cameraPos.z, 
        cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z,
        cameraUp.x, cameraUp.y, cameraUp.z);
}

void display()
{
    static float fps_time = 0;
    static int frames = 0;
    if(time - fps_time > 1)
    {
        fps_time = time;
        printf("FPS: %d\n", frames);
        frames = 0;
    }
    frames++;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    glutSwapBuffers();
}

void idle()
{
    float newtime = (float)glutGet(GLUT_ELAPSED_TIME)/1000;
    deltaTime = newtime - time;
    time = newtime;

    vec3 vel = (
        (keys['w']- keys['s']) * cameraFront + 
        (keys['a']- keys['d']) * cameraLeft);

    cameraPos += vel*masterSpeed*cameraSpeed*deltaTime;
    updateCamera();

    glutPostRedisplay();
}

void reshape(int w, int h)
{
    if(h == 0) h == 1;
    float aspect = (float)w/h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(120, aspect, 1, 10000);
    updateCamera();
    width = w;
    height = h;
}

void motion(int x, int y)
{
    if(!mouseLook) return;
    if(x == width/2 && y == height/2) return;

    cameraAng.x += ((float)width/2 - x) * mouseSpeed / 100.0f;
    cameraAng.y += ((float)height/2 - y) * mouseSpeed / 100.0f;

    updateCamera();

    if(x != width/2 || y != height/2) glutWarpPointer(width/2, height/2);
}

void keyDown(unsigned char key, int x, int y)
{
    keys[(int)key] = 1.0f;
}

void keyUp(unsigned char key, int x, int y)
{
    keys[(int)key] = 0.0f;

    switch(key)
    {
        case 'm':
            if(mouseLook)
            {
                mouseLook = false;
                glutSetCursor(GLUT_CURSOR_INHERIT);
            }
            else
            {
                mouseLook = true;
                glutWarpPointer(width/2, height/2);
                glutSetCursor(GLUT_CURSOR_NONE);   
            }
            break;
        case '+':
            cameraSpeed *= 2.0f;
            break;
        case '-':
            cameraSpeed /= 2.0f;
            if(cameraSpeed <= 1.0f) cameraSpeed = 1.0f;
            break;
    }
}

int main(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowPosition(500, 500);
    glutInitWindowSize(width, height);
    glutCreateWindow("Curves - curvature & torsion");

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(motion);
    glutIgnoreKeyRepeat(true);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);

    for(int i = 0; i < 256; i++) keys[i] = false;

    glutSetOption(GLUT_MULTISAMPLE, 8);
    glEnable(GL_MULTISAMPLE);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
    glEnable(GL_POINT_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    updateCamera();
    glutMainLoop();

    return 0;
}