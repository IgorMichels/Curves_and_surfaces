#include "viewer.h"

const float pi = 3.14159265358979323846;

int width = 800;
int height = 800;

Camera mainCamera = {{-5, -1, -1}, {0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
Camera *cam = &mainCamera;

float currentTime = 0.0f;
float deltaTime = 0.0f;
float masterSpeed = 1.0f;
float cameraSpeed = 1.0f;
float mouseSpeed = 1.0f;

bool lookMode = false;
bool flyMode = true;

float keys[256];
int nKeys = 0;

void updateCamera()
{
    if(cam->ang.y > +pi/2) cam->ang.y = +pi/2;
    if(cam->ang.y < -pi/2) cam->ang.y = -pi/2;
    if(cam->ang.x < 0) cam->ang.x = 2*pi;
    if(cam->ang.x > 2*pi) cam->ang.x = 0;
    cam->front = {cos(cam->ang.x)*cos(cam->ang.y), sin(cam->ang.x)*cos(cam->ang.y), sin(cam->ang.y)};
    cam->left = {-sin(cam->ang.x), cos(cam->ang.x), 0};
    cam->up = cross(cam->front, cam->left);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        cam->pos.x, cam->pos.y, cam->pos.z, 
        cam->pos.x + cam->front.x, cam->pos.y + cam->front.y, cam->pos.z + cam->front.z,
        cam->up.x, cam->up.y, cam->up.z);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    surf_draw();
    glutSwapBuffers();
}

void idle()
{
    float newtime = (float)glutGet(GLUT_ELAPSED_TIME)/1000;
    deltaTime = newtime - currentTime;
    currentTime = newtime;

    if(flyMode)
    {
        vec3 vel = 
            (keys['w']-keys['s']) * cam->front + 
            (keys['a']-keys['d']) * cam->left + 
            (keys['e']-keys['q']) * cam->up;

        cam->pos += vel*masterSpeed*cameraSpeed*deltaTime;
    }

    surf_idle();

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
    gluPerspective(90, aspect, 0.01, 2000);
    updateCamera();
    width = w;
    height = h;
}

void motion(int x, int y)
{
    if(!lookMode) return;
    if(x == width/2 && y == height/2) return;

    cam->ang.x += ((float)width/2 - x) * mouseSpeed / 100.0f;
    cam->ang.y += ((float)height/2 - y) * mouseSpeed / 100.0f;

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
    surf_keydown(key);
}

int start(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowPosition(500, 500);
    glutInitWindowSize(width, height);
    glutCreateWindow("Surfaces - Geodesic curves");

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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    glClearColor(0.2, 0.2, 0.8, 0);

    updateCamera();

    glutMainLoop();

    return 0;
}