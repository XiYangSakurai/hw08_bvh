#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return InvPi*R;
    //return Color3f(0.f);
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //TODO
    if(!sampledType)
    {
        *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }

    *wi=WarpFunctions::squareToHemisphereCosine(u);
    if(wo.z<0.0f)
        wi->z*=-1.0f;
    Color3f s_f=f(wo,*wi);
    *pdf=Pdf(wo,*wi);
    return s_f;
    //return BxDF::Sample_f(wo, wi, u, pdf, sampledType);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    //return InvPi*wi.z;
//    Point3f sample=wi;
//    if(sample.z<0)
//        sample.z*=-1.0f;
    return SameHemisphere(wo, wi) ? InvPi*AbsCosTheta(wi) : 0;
    //return BxDF::Pdf(wo, wi);
}
