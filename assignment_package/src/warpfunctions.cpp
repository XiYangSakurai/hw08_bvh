#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented uniform disk warping!");
    float r=sqrt(sample[0]);
    float angle=sample[1]*2.0f*M_PI;
    Point3f uniDisk;
    uniDisk[0]=r*cos(angle);
    uniDisk[1]=r*sin(angle);
    uniDisk[2]=0.0f;
    return uniDisk;
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented concentric disk warping!");
    float phi, r, u, v;
    float a = 2*sample.x-1; // (a,b) is now on [-1,1]ˆ2
    float b = 2*sample.y-1;
    if (a > -b) // region 1 or 2
            {
                if (a > b) // region 1, also |a| > |b|
                {
                    r=a;
                    phi = (M_PI/4 ) * (b/a);
                }
                else // region 2, also |b| > |a|
                {
                    r = b;
                    phi = (M_PI/4) * (2 - (a/b));
                }
            }
     else // region 3 or 4
            {
                if (a < b) // region 3, also |a| >= |b|, a != 0
                {
                    r = -a;
                    phi = (M_PI/4) * (4 + (b/a));
                }

                    else // region 4, |b| >= |a|, but a==0 and b==0 could occur.
                {    r = -b;
                    if (b != 0)
                        phi = (M_PI/4) * (6 - (a/b));
                    else
                        phi = 0;
                }
            }
        u=r* cos( phi );
        v=r* sin( phi );
        return Point3f( u, v ,0.0f);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    //TODO
    //return 0;
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented uniform sphere warping!");
    float z=1-2*sample[0];
    float delta=1-pow(z,2);
    float x=0;
    float y=0;
    if(delta>=0.0f)
            {
                x=cos(2*M_PI*sample[1])*sqrt(1-pow(z,2));
                y=sin(2*M_PI*sample[1])*sqrt(1-pow(z,2));
            }
    return Point3f(x,y,z);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    //TODO
    //return 0;
    return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented sphere cap warping!");
    float z=1-2*sample[0]*(180-thetaMin)/180;
    float delta=1-pow(z,2);
    float x=0;
    float y=0;
    if(delta>=0.0f)
            {
                x=cos(2*M_PI*sample[1])*sqrt(1-pow(z,2));
                y=sin(2*M_PI*sample[1])*sqrt(1-pow(z,2));
            }

    return Point3f(x,y,z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{
    //TODO
    //return 0;
    return Inv2Pi/(1-cos(glm::radians(180-thetaMin)));
}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented uniform hemisphere warping!");
    float z=sample[0];
    float delta=1-pow(z,2);
    float x=0;
    float y=0;
    if(delta>=0.0f)
            {
                x=cos(2*M_PI*sample[1])*sqrt(1-pow(z,2));
                y=sin(2*M_PI*sample[1])*sqrt(1-pow(z,2));
            }
    return Point3f(x,y,z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    //TODO
    //return 0;
    return Inv2Pi;
}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    //TODO
    //throw std::runtime_error("You haven't yet implemented cosine-weighted hemisphere warping!");
    Point3f concentricdisk=squareToDiskConcentric(sample);
    concentricdisk[2]=sqrt(1-pow(concentricdisk[0],2)-pow(concentricdisk[1],2));
    return concentricdisk;
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
    //TODO
    //return 0;
    return InvPi*sample[2];
}
