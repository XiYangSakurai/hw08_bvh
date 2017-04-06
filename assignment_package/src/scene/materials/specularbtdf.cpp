#include "specularbTdf.h"
#include <iostream>
Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    if(!sampledType)
    {
        *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }

    //determine incident and which is transmitted
    bool entering=CosTheta(wo)>0.0f;
    float ei=entering? etaA:etaB;
    float et=entering? etaB:etaA;
    //compute ray direction for specular transmission
    Normal3f norm=entering? Normal3f(0,0,1):Normal3f(0,0,-1);
    bool refract=false;
    float eta=ei/et;
    float cosThetaI=glm::dot(norm,wo);
    float sin2ThetaI=std::max(0.0f,1.0f-cosThetaI*cosThetaI);
    float sin2ThetaT=eta*eta*sin2ThetaI;
    if(sin2ThetaT>=1.0f)
        refract=false;
    else{
    float cosThetaT=std::sqrt(1-sin2ThetaT);
    *wi=eta*(-1.0f*wo)+(eta*cosThetaI-cosThetaT)*norm;
    refract=true;
    }
    if(!refract)
        return Color3f(0.0f);
    if(AbsCosTheta(*wi)<FLT_EPSILON) return Color3f(0.0f);
    *pdf=1.0f;
    Color3f ft=T-T*fresnel->Evaluate(CosTheta(*wi));
    return ft/AbsCosTheta(*wi);


}
