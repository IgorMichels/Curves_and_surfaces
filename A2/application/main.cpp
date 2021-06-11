#include <stdio.h>
#include <vector>
#include "surface.h"
#include "viewer.h"

#define MAX_NUM 512

struct Surface
{
    float min_u, max_u;
    float min_v, max_v;
    int numGrid;

    float side = 1.0f;

    bool grid = false;
    bool faces = true;
    bool partials = true;
    bool showant = false;
    bool trace = false;

    uv pos, vel;
    Compiler comp;

    vector points[MAX_NUM][MAX_NUM];

    std::vector<vector> paths;
    std::vector<int> cuts;

    Camera localCam;

    void sample(int numGrid)
    {
        if(!comp.library) 
        {
            printf("No surface compiled\n");
            return;
        }

        quad q = comp.limits(0, 0);

        if(numGrid < 4 || numGrid >= MAX_NUM || q.w >= q.x || q.y >= q.z)
        {
            this->numGrid = 0;
            return;
        }

        this->numGrid = numGrid;
        this->min_u = q.w;
        this->max_u = q.x;
        this->min_v = q.y;
        this->max_v = q.z;

        for(int iu = 0; iu < numGrid; iu++)
        for(int iv = 0; iv < numGrid; iv++)
        {
            float u = min_u + iu*(max_u - min_u)/(numGrid-1);
            float v = min_v + iv*(max_v - min_v)/(numGrid-1);

            points[iu][iv] = comp.surface(u, v);
        }
    }

    void resetPartials()
    {
        this->pos.u = (min_u + max_u)/2;
        this->pos.v = (min_v + max_v)/2;
    }

    void draw()
    {
        if(!comp.library) return;

        updateCamera();

        //surface faces
        glBegin(GL_QUADS);
        if(faces)
        for(int i = 0; i < numGrid-1; i++)
        for(int j = 0; j < numGrid-1; j++)
        {
            vector a = points[i+0][j+0];
            vector b = points[i+1][j+0];
            vector c = points[i+1][j+1];
            vector d = points[i+0][j+1];
            
            glColor3f(1, (float)i/numGrid, (float)j/numGrid);
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
            glVertex3f(d.x, d.y, d.z);   
        }
        glEnd();

        //surface grid
        glLineWidth(1);
        glBegin(GL_LINES);
        glColor3f(0.5, 0.5, 0.5);
        if(grid)
        for(int i = 0; i < numGrid; i++)
        for(int j = 0; j < numGrid; j++)
        {
            vector a = points[i+0][j+0];

            if(i != numGrid-1)
            {
                vector b = points[i+1][j+0];
                glVertex3f(a.x, a.y, a.z);
                glVertex3f(b.x, b.y, b.z);
            }
            if(j != numGrid-1)
            {
                vector d = points[i+0][j+1];
                glVertex3f(a.x, a.y, a.z);
                glVertex3f(d.x, d.y, d.z);
            }
        }
        glEnd();

        vector p  = comp.surface  (pos.u, pos.v);
        vector du = comp.partial_u(pos.u, pos.v);
        vector dv = comp.partial_v(pos.u, pos.v);

        //2-vector base
        if(partials)
        {
            glDisable(GL_DEPTH_TEST);
            glBegin(GL_LINES);
            glColor3f(1, 0, 0);
            glVertex3f(p.x, p.y, p.z);
            glVertex3f(p.x + du.x, p.y + du.y, p.z + du.z);
            glColor3f(0, 1, 0);
            glVertex3f(p.x, p.y, p.z);
            glVertex3f(p.x + dv.x, p.y + dv.y, p.z + dv.z);
            glEnd();
            glEnable(GL_DEPTH_TEST);
        }

        //ant
        glDisable(GL_DEPTH_TEST);
        if(showant)
        {
            vector da = {vel.u * du.x + vel.v * dv.x, vel.u * du.y + vel.v * dv.y, vel.u * du.z + vel.v * dv.z};
            glLineWidth(2);
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(p.x, p.y, p.z);
            glVertex3f(p.x + da.x, p.y + da.y, p.z + da.z);
            glEnd();
        }
        glPointSize(8);
        glColor3f(1, 1, 0.6);
        glBegin(GL_POINTS);
        glVertex3f(p.x, p.y, p.z);
        glEnd();
        glEnable(GL_DEPTH_TEST);

        //path
        glColor3f(0, 0, 0);
        glLineWidth(4);
        glBegin(GL_LINES);

        int pt = 0;
        for(int c = 0; c < cuts.size(); c++)
        {
            for(int i = 1; i < cuts[c]; i++)
            {
                if(pt+i < paths.size())
                {
                    glVertex3f(paths[pt+i-0].x, paths[pt+i-0].y, paths[pt+i-0].z);
                    glVertex3f(paths[pt+i-1].x, paths[pt+i-1].y, paths[pt+i-1].z);
                }
            }
            pt += cuts[c];
        }
        
        glEnd();
    }

    void step(float dt)
    {
        if(dt == 0) return;

        uv ac = comp.accel(pos, vel);

        pos.u += dt * vel.u;
        pos.v += dt * vel.v;
        vel.u += dt * ac.u;
        vel.v += dt * ac.v;

        float E = comp.metric_e(pos.u, pos.v);
        float F = comp.metric_f(pos.u, pos.v);
        float G = comp.metric_g(pos.u, pos.v);
        float len = sqrt(E*vel.u*vel.u + 2*F*vel.u*vel.v + G*vel.v*vel.v);

        vel = {vel.u / len, vel.v / len};
    }

    void turn(float dt)
    {
        dt *= side;
        float E = comp.metric_e(pos.u, pos.v);
        float F = comp.metric_f(pos.u, pos.v);
        float G = comp.metric_g(pos.u, pos.v);

        float R = sqrt(G - F*F/E);
        float Re = sqrt(E);

        float a_ = (vel.u + vel.v*F/E)*Re;
        float b_ = vel.v*R;

        float A = a_*cos(dt) - b_*sin(dt);
        float B = a_*sin(dt) + b_*cos(dt);

        vel = {A/Re - B*F/E/R, B/R};
    }

    void walk(float dup, float dleft)
    {
        static float dist = 0;
        dist += abs(dup) + abs(dleft);
        if(dist > 0.001)
        {
            if(trace) 
            {
                paths.push_back(comp.surface(pos.u, pos.v));
                if(cuts.size() == 0) cuts.push_back(1);
                else cuts[cuts.size()-1]++;
            }
            dist = 0;
        }
        step(dup);
        turn(pi/2);
        step(dleft);
        turn(-pi/2);
    }

    void updateCamera()
    {
        float height = 0.02;
        vector s = comp.surface(pos.u, pos.v);
        vector du = comp.partial_u(pos.u, pos.v);
        vector dv = comp.partial_v(pos.u, pos.v);
        localCam.front = { vel.u*du.x + vel.v*dv.x, vel.u*du.y + vel.v*dv.y, vel.u*du.z + vel.v*dv.z };
        uv tmp = vel;
        turn(pi/2);
        localCam.left = side * (vec3){ vel.u*du.x + vel.v*dv.x, vel.u*du.y + vel.v*dv.y, vel.u*du.z + vel.v*dv.z };
        vel = tmp;
        localCam.up = cross(localCam.front, localCam.left) * side;
        localCam.pos =
        { 
            s.x + height*localCam.up.x - 0.03*localCam.front.x,
            s.y + height*localCam.up.y - 0.03*localCam.front.y,
            s.z + height*localCam.up.z - 0.03*localCam.front.z
        };
    }
};

Surface surf;

void surf_draw()
{
    surf.draw();
}

void surf_idle()
{
    float sp = masterSpeed*cameraSpeed*deltaTime;
    surf.pos.u += (keys['t']-keys['g'])*sp;
    surf.pos.v += (keys['f']-keys['h'])*sp;

    surf.turn((keys['u'] - keys['o'])*sp*4);
    surf.walk((keys['i'] - keys['k'])*sp, (keys['j'] - keys['l'])*sp);
}

void surf_keydown(unsigned char key)
{
    switch(key)
    {
        case '*':
            lookMode = !lookMode;
            if(lookMode) glutWarpPointer(width/2, height/2);
            break;
        case '+':
            cameraSpeed *= 2;
            break;
        case '-':
            if(cameraSpeed > 0.01) cameraSpeed /= 2;
            break;
        case 'z':
            surf.grid = !surf.grid;
            break;
        case 'x':
            surf.faces = !surf.faces;
            break;
        case 'c':
            surf.partials = !surf.partials;
            break;
        case 'v':
            if(surf.numGrid/2 >= 4)
                surf.sample(surf.numGrid/2);
            break;
        case 'b':
            if(surf.numGrid*2 < MAX_NUM)
                surf.sample(surf.numGrid*2);
            break;
        case 'n':
            if(surf.numGrid-1 >= 4)
                surf.sample(surf.numGrid-1);
            break;
        case 'm':
            if(surf.numGrid+1 < MAX_NUM)
                surf.sample(surf.numGrid+1);
            break;
        case '0':
            surf.paths.clear();
            surf.cuts.clear();
            break;
        case '1':
            surf.showant = !surf.showant;
            break;
        case '2':
            surf.trace = !surf.trace;
            surf.cuts.push_back(0);
            break;
        case '3':
            if(cam == &mainCamera) 
            {
                lookMode = false;
                flyMode = false;
                cam = &surf.localCam;
            }
            else
            {
                lookMode = true;
                flyMode = true;
                cam = &mainCamera;
            }
            break;
        case '4':
            surf.side *= -1.0f;
            break;
    }
}

int main(int argc, char ** argv)
{
    char *buffer = nullptr;
    size_t size = 0;
    int num_points = 0;

    printf("Surface >> ");
    int k = getline(&buffer, &size, stdin);

    printf("Points per axis >> ");
    k = scanf("%d", &num_points);

    Parser p;
    p.init(buffer);
    Expr *exp_surf = p.surface();
    if(!exp_surf) return 2;

    surf.comp.compile(exp_surf);
    if(!surf.comp.library) return 3;

    surf.sample(num_points);
    surf.resetPartials();

    //surf.comp.profile();
    surf.vel = {1, 0};

    return start(argc, argv);
}