#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    //TODO

    cosThetaI=glm::clamp(cosThetaI,-1.0f,1.0f);
    float f=0.0f;
    //potentially swap indices
    bool entering=cosThetaI>0.0f;
    float ei=etaI;
    float et=etaT;
    if(!entering)
    {
        std::swap(ei,et);
        cosThetaI=std::fabs(cosThetaI);
    }
    //compute costhetaT using Snell's law
    float sinThetaI=std::sqrt(std::max((float)0,1-cosThetaI*cosThetaI));
    float sinThetaT=ei/et*sinThetaI;
    if(sinThetaT>=1.0f)  //internal reflection
        f=1.0f;
    float cosThetaT=std::sqrt(std::max((float)0,1-sinThetaT*sinThetaT));

    float Rparl=((et*cosThetaI)-(ei*cosThetaT))/((et*cosThetaI)+(ei*cosThetaT));
    float Rperp=((ei*cosThetaI)-(et*cosThetaT))/((ei*cosThetaI)+(et*cosThetaT));
    f = (Rparl*Rparl + Rperp*Rperp) / 2.0f;
    return f*Color3f(1.0f);
}
Color3f FresnelConductor::Evaluate(float cosThetaI) const
{
    return Color3f(0.0f);
}
