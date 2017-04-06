#include "specularbrdf.h"

Color3f SpecularBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    if(!sampledType)
    {
        *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }
    //Color3f specular_f(0.0f);

    wi->x=-1.0*wo.x;
    wi->y=-1.0*wo.y;
    wi->z=wo.z;
    if(AbsCosTheta(*wi)<FLT_EPSILON)
        return Color3f(0.0f);
    *pdf=1;
    Color3f fresnel_f=fresnel->Evaluate(CosTheta(*wi))*R/AbsCosTheta(*wi);
    return fresnel_f;



}
