#include <stdio.h>
#include <vector>
#include "surface.h"
#include "viewer.h"

#define MAX_NUM 512

struct Surface
{
    Compiler comp;
    uv uv_pos, uv_vel, uv_accel;

    vector pos;
    vector du, dv;
    vector vel;

    float E, F, G;

    quad limits;
    int numGrid;
    vector grid[MAX_NUM][MAX_NUM];

    std::vector<vector> vec_paths;
    std::vector<uv> uv_paths;
    std::vector<int> cuts;

    Camera localCam;

    float side = 1.0f;

    bool vecView = true;
    bool showGrid = false;
    bool showFaces = true;
    bool showPartials = true;
    bool showVel = false;
    bool tracePath = false;

    void sample(int numGrid)
    {
        if(!comp.library) 
        {
            printf("No surface compiled\n");
            return;
        }

        if(numGrid < 4) numGrid = 4;
        if(numGrid > MAX_NUM) numGrid = MAX_NUM;

        this->numGrid = numGrid;

        limits = comp.limits(0, 0);

        if(limits.w >= limits.x)
        {
            float tmp = limits.x;
            limits.x = limits.w;
            limits.w = tmp;
        }

        if(limits.y >= limits.z)
        {
            float tmp = limits.z;
            limits.z = limits.y;
            limits.y = tmp;
        }

        for(int iu = 0; iu < numGrid; iu++)
        for(int iv = 0; iv < numGrid; iv++)
        {
            float u = limits.w + iu*(limits.x - limits.w)/(numGrid-1);
            float v = limits.y + iv*(limits.z - limits.y)/(numGrid-1);

            grid[iu][iv] = comp.surface(u, v);
        }
    }

    void draw()
    {
        if(vecView) drawVec();
        else drawUv();
    }

    void drawVec()
    {
        if(&localCam == cam) updateCamera();

        //surface faces
        if(showFaces)
        {
            glBegin(GL_TRIANGLES);
            for(int i = 0; i < numGrid-1; i++)
            for(int j = 0; j < numGrid-1; j++)
            {
                vector a = grid[i+0][j+0];
                vector b = grid[i+1][j+0];
                vector c = grid[i+1][j+1];
                vector d = grid[i+0][j+1];
                
                glColor3f(1, (float)i/numGrid, (float)j/numGrid);
                glVertex3f(a.x, a.y, a.z);
                glVertex3f(b.x, b.y, b.z);
                glVertex3f(c.x, c.y, c.z);
                
                glVertex3f(c.x, c.y, c.z);  
                glVertex3f(d.x, d.y, d.z);
                glVertex3f(a.x, a.y, a.z);
            }
            glEnd();
        }

        //surface grid
        if(showGrid)
        {
            glLineWidth(1);
            glBegin(GL_LINES);
            glColor3f(0.5, 0.5, 0.5);
            for(int i = 0; i < numGrid; i++)
            for(int j = 0; j < numGrid; j++)
            {
                vector a = grid[i+0][j+0];

                if(i != numGrid-1)
                {
                    vector b = grid[i+1][j+0];
                    glVertex3f(a.x, a.y, a.z);
                    glVertex3f(b.x, b.y, b.z);
                }
                if(j != numGrid-1)
                {
                    vector d = grid[i+0][j+1];
                    glVertex3f(a.x, a.y, a.z);
                    glVertex3f(d.x, d.y, d.z);
                }
            }
            glEnd();
        }

        //2-vector base
        if(showPartials)
        {
            glLineWidth(2);
            glDisable(GL_DEPTH_TEST);
            glBegin(GL_LINES);
            glColor3f(1, 0, 0);
            glVertex3f(pos.x, pos.y, pos.z);
            glVertex3f(pos.x + du.x, pos.y + du.y, pos.z + du.z);
            glColor3f(0, 1, 0);
            glVertex3f(pos.x, pos.y, pos.z);
            glVertex3f(pos.x + dv.x, pos.y + dv.y, pos.z + dv.z);
            glEnd();
            glEnable(GL_DEPTH_TEST);
        }

        //vel
        glDisable(GL_DEPTH_TEST);
        if(showVel)
        {
            glLineWidth(2);
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(pos.x, pos.y, pos.z);
            glVertex3f(pos.x + vel.x, pos.y + vel.y, pos.z + vel.z);
            glEnd();
        }
        glPointSize(8);
        glColor3f(1, 1, 0.6);
        glBegin(GL_POINTS);
        glVertex3f(pos.x, pos.y, pos.z);
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
                if(pt+i < vec_paths.size())
                {
                    glVertex3f(vec_paths[pt+i-0].x, vec_paths[pt+i-0].y, vec_paths[pt+i-0].z);
                    glVertex3f(vec_paths[pt+i-1].x, vec_paths[pt+i-1].y, vec_paths[pt+i-1].z);
                }
            }
            pt += cuts[c];
        }
        
        glEnd();
    }

    void drawUv()
    {
        float su = (limits.x - limits.w)/(numGrid-1);
        float sv = (limits.z - limits.y)/(numGrid-1);

        //surface faces
        if(showFaces)
        {
            glBegin(GL_TRIANGLES);
            for(int i = 0; i < numGrid-1; i++)
            for(int j = 0; j < numGrid-1; j++)
            {
                uv a = { limits.w + su*(i+0), limits.y + sv*(j+0) };
                uv b = { limits.w + su*(i+1), limits.y + sv*(j+0) };
                uv c = { limits.w + su*(i+1), limits.y + sv*(j+1) };
                uv d = { limits.w + su*(i+0), limits.y + sv*(j+1) };
                
                glColor3f(1, (float)i/numGrid, (float)j/numGrid);
                glVertex3f(a.u, a.v, 0);
                glVertex3f(b.u, b.v, 0);
                glVertex3f(c.u, c.v, 0);
                
                glVertex3f(c.u, c.v, 0);  
                glVertex3f(d.u, d.v, 0);
                glVertex3f(a.u, a.v, 0);
            }
            glEnd();
        }

        //surface grid
        if(showGrid)
        {
            glLineWidth(1);
            glBegin(GL_LINES);
            glColor3f(0.5, 0.5, 0.5);
            for(int i = 0; i < numGrid; i++)
            for(int j = 0; j < numGrid; j++)
            {
                uv a = { limits.w + su*(i+0), limits.y + sv*(j+0) };

                if(i != numGrid-1)
                {
                    uv b = { limits.w + su*(i+1), limits.y + sv*(j+0) };
                    glVertex3f(a.u, a.v, 0);
                    glVertex3f(b.u, b.v, 0);
                }
                if(j != numGrid-1)
                {
                    uv d = { limits.w + su*(i+0), limits.y + sv*(j+1) };
                    glVertex3f(a.u, a.v, 0);
                    glVertex3f(d.u, d.v, 0);
                }
            }
            glEnd();
        }

        //2-vector base
        if(showPartials)
        {
            glLineWidth(2);
            glDisable(GL_DEPTH_TEST);
            glBegin(GL_LINES);
            glColor3f(1, 0, 0);
            glVertex3f(uv_pos.u+0, uv_pos.v+0, 0.1);
            glVertex3f(uv_pos.u+1, uv_pos.v+0, 0.1);
            glColor3f(0, 1, 0);
            glVertex3f(uv_pos.u+0, uv_pos.v+0, 0.1);
            glVertex3f(uv_pos.u+0, uv_pos.v+1, 0.1);
            glEnd();
            glEnable(GL_DEPTH_TEST);
        }

        //vel
        glDisable(GL_DEPTH_TEST);
        if(showVel)
        {
            glLineWidth(2);
            glBegin(GL_LINES);
            glColor3f(1, 1, 1);
            glVertex3f(uv_pos.u, uv_pos.v, 0.1);
            glVertex3f(uv_pos.u + uv_vel.u, uv_pos.v + uv_vel.v, 0.1);
            glEnd();
        }
        glPointSize(8);
        glColor3f(1, 1, 0.6);
        glBegin(GL_POINTS);
        glVertex3f(uv_pos.u, uv_pos.v, 0.1);
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
                if(pt+i < vec_paths.size())
                {
                    glVertex3f(uv_paths[pt+i-0].u, uv_paths[pt+i-0].v, 0.1);
                    glVertex3f(uv_paths[pt+i-1].u, uv_paths[pt+i-1].v, 0.1);
                }
            }
            pt += cuts[c];
        }
        
        glEnd();
    }

    void step(float dt)
    {
        if(dt == 0) return;

        uv_pos.u += dt * uv_vel.u;
        uv_pos.v += dt * uv_vel.v;
        uv_vel.u += dt * uv_accel.u;
        uv_vel.v += dt * uv_accel.v;

        float len = sqrt(E*uv_vel.u*uv_vel.u + 2*F*uv_vel.u*uv_vel.v + G*uv_vel.v*uv_vel.v);

        uv_vel = {uv_vel.u / len, uv_vel.v / len};
        calculatePos();
        calculateVel();
    }

    void turn(float dt)
    {
        if(dt == 0) return;
        dt *= side;

        float R = sqrt(E*G - F*F);

        uv_vel = 
        {
            uv_vel.u*cos(dt) - (uv_vel.u*F + uv_vel.v*G)/R*sin(dt),
            (uv_vel.u*E + uv_vel.v*F)/R*sin(dt) + uv_vel.v*cos(dt)
        };

        calculateVel();
    }

    void walk(float dup, float dleft)
    {
        static float dist = 0;
        dist += abs(dup) + abs(dleft);
        if(dist > 0.001)
        {
            if(tracePath) 
            {
                vec_paths.push_back(pos);
                uv_paths.push_back(uv_pos);
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
        localCam.front = {vel.x, vel.y, vel.z};
        uv tmp_vel = uv_vel;
        uv tmp_ac = uv_accel;
        turn(pi/2);
        localCam.left = side * (vec3){vel.x, vel.y, vel.z};
        uv_vel = tmp_vel;
        uv_accel = tmp_ac;
        localCam.up = cross(localCam.front, localCam.left) * side;
        localCam.pos =
        { 
            pos.x + height*localCam.up.x - 0.03*localCam.front.x,
            pos.y + height*localCam.up.y - 0.03*localCam.front.y,
            pos.z + height*localCam.up.z - 0.03*localCam.front.z
        };
        calculateVel();
    }

    void calculateVel()
    {
        vel = { du.x * uv_vel.u + dv.x * uv_vel.v, du.y * uv_vel.u + dv.y * uv_vel.v, du.z * uv_vel.u + dv.z * uv_vel.v };
        uv_accel = comp.accel(uv_pos, uv_vel);
    }

    void calculatePos()
    {
        pos = comp.surface(uv_pos.u, uv_pos.v);
        du = comp.partial_u(uv_pos.u, uv_pos.v);
        dv = comp.partial_v(uv_pos.u, uv_pos.v);
        E = comp.metric_e(uv_pos.u, uv_pos.v);
        F = comp.metric_f(uv_pos.u, uv_pos.v);
        G = comp.metric_g(uv_pos.u, uv_pos.v);
    }

    void setPos(uv x)
    {
        if(x.u == uv_pos.u && x.v == uv_pos.v) return;
        uv_pos = x;
        calculatePos();
        calculateVel();
    }

    void reset()
    {
        uv_pos = { (limits.w + limits.x)/2, (limits.y + limits.z)/2 };
        uv_vel = { 0, 1 };
        calculatePos();
        calculateVel();
    }

    void resetPaths()
    {
        vec_paths.clear();
        uv_paths.clear();
        cuts.clear();
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
    surf.setPos({surf.uv_pos.u + (keys['t']-keys['g'])*sp, surf.uv_pos.v + (keys['f']-keys['h'])*sp});

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
            surf.showGrid = !surf.showGrid;
            break;
        case 'x':
            surf.showFaces = !surf.showFaces;
            break;
        case 'c':
            surf.showPartials = !surf.showPartials;
            break;
        case 'v':
                surf.sample(surf.numGrid/2);
            break;
        case 'b':
                surf.sample(surf.numGrid*2);
            break;
        case 'n':
                surf.sample(surf.numGrid-1);
            break;
        case 'm':
                surf.sample(surf.numGrid+1);
            break;
        case '0':
            surf.resetPaths();
            break;
        case '1':
            surf.showVel = !surf.showVel;
            break;
        case '2':
            surf.tracePath = !surf.tracePath;
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
        case '5':
            if(surf.vecView)
            {
                surf.vecView = false;
                cam = &mainCamera;
            }
            else surf.vecView = true;
            break;
    }
}

int main(int argc, char ** argv)
{
    char *buffer = nullptr;
    size_t size = 0;

    printf("Surface >> ");
    int k = getline(&buffer, &size, stdin);

    Parser p;
    p.init(buffer);
    Expr *exp_surf = p.surface();
    if(!exp_surf) return 2;

    surf.comp.compile(exp_surf);
    if(!surf.comp.library) return 3;

    surf.sample(256);
    surf.reset();

    return start(argc, argv);
}