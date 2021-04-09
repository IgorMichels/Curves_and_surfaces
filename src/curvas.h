#ifndef CURVAS_H
#define CURVAS_H

#include <vector>

#define CURVA(x, y) Curva([](float t) { return (float)(x);}, [](float t) { return (float)(y);})

typedef float (*Func)(float);

struct Frenet
{
    vec3 tangent, normal, binormal;

    vec3 point;
    float parameter;
};

struct Curva
{
    Func curvature, torsion;
    std::vector<Frenet> pieces;

    float a, b, h;

    float parameter;
    int iFrenet = -1;

    bool setParameter(float t)
    {
        if(pieces.size() < 2) return false;
        
        int f = (int)((t-a)/h);

        if(f < 0 || f >= pieces.size()) return false;

        parameter = t;
        iFrenet = f;
        return true;
    }

    Curva()
    {
        curvature = nullptr;
        torsion = nullptr;
        iFrenet = -1;
    }

    Curva(Func c, Func t)
    {
        curvature = c;
        torsion = t;
        iFrenet = -1;
    }

    Curva(Func c, Func t, float a, float b, float h)
    {
        curvature = c;
        torsion = t;
        eulerBuild(a, b, h);
    }

    bool eulerBuild(float a, float b, float h)
    {
        Frenet cur = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0, 0, 0}, a};
        eulerBuild(a, b, h, cur);
    }

    bool eulerBuild(float a, float b, float h, Frenet cur)
    {
        if(!curvature || !torsion) return false;
        this->a = a;
        this->b = b;
        this->h = h;
        if(h <= 0.0) return false;
        if(b <= a) return false;

        iFrenet = 0;

        pieces.clear();
        cur.parameter = a;
        pieces.push_back(cur);

        for(float t = a+h; t <= b; t += h)
        {
            Frenet next;
            next.tangent = normalize(cur.tangent + curvature(cur.parameter) * cur.normal * h);
            next.normal = normalize(cur.normal + 
            (torsion(cur.parameter) * cur.binormal - curvature(cur.parameter) * cur.tangent) * h);
            next.binormal = cross(next.tangent, next.normal);
            next.point = cur.point + cur.tangent * h;
            next.parameter = t;
            pieces.push_back(next);
            cur = next;
        }

        return setParameter(a);
    }

    void drawCurve()
    {
        if(pieces.size() < 2) return;
        
        glLineWidth(4);
        glBegin(GL_LINES);
        float c = 1;
        for(int i = 0; i < pieces.size()-1; i++)
        {
            if(pieces[i].point.z <= 0) c = 0.5; else c = 1; 
            glColor4f((pieces[i].tangent.x+1)/2, (pieces[i].tangent.y+1)/2, (pieces[i].tangent.z+1)/2, c);
            glVertex3f(pieces[i].point.x, pieces[i].point.y, pieces[i].point.z);
            glColor4f((pieces[i+1].tangent.x+1)/2, (pieces[i+1].tangent.y+1)/2, (pieces[i+1].tangent.z+1)/2, c);
            glVertex3f(pieces[i+1].point.x, pieces[i+1].point.y, pieces[i+1].point.z);
        }
        glEnd();
    }

    void drawFrenet()
    {
        int f = iFrenet;
        if(f < 0) return;

        glLineWidth(3);
        glBegin(GL_LINES);


        float s1 = 1-(sin(currentTime*masterSpeed*20)+1)/4;

        //TANGENT
        glColor4f(1, 0, 0, s1);
        glVertex3f(pieces[f].point.x, pieces[f].point.y, pieces[f].point.z);
        glVertex3f(
            pieces[f].point.x+pieces[f].tangent.x, 
            pieces[f].point.y+pieces[f].tangent.y,
            pieces[f].point.z+pieces[f].tangent.z);

        //NORMAL
        glColor4f(0, 1, 0, s1);
        glVertex3f(pieces[f].point.x, pieces[f].point.y, pieces[f].point.z);
        glVertex3f(
            pieces[f].point.x+pieces[f].normal.x, 
            pieces[f].point.y+pieces[f].normal.y,
            pieces[f].point.z+pieces[f].normal.z);

        //BINORMAL
        glColor4f(0, 0, 1, s1);
        glVertex3f(pieces[f].point.x, pieces[f].point.y, pieces[f].point.z);
        glVertex3f(
            pieces[f].point.x+pieces[f].binormal.x, 
            pieces[f].point.y+pieces[f].binormal.y,
            pieces[f].point.z+pieces[f].binormal.z);
        glEnd();
    }

    void drawGrid()
    {
        float N = 20;
        float S = 1;

        glLineWidth(2);
        glBegin(GL_LINES);
        for(float k = -N; k <= +N; k++)
        {
            float c = 1-abs(k/(N+1));
            
            glColor4f(c, c, c, 0.5);
            glVertex3f(+k*S, +N*S, 0);
            glVertex3f(+k*S, -N*S, 0);
            glVertex3f(+N*S, +k*S, 0);
            glVertex3f(-N*S, +k*S, 0);
        }

        glEnd();
    }
};

#endif