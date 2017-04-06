#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    //TODO
    if(twoSided==false)
    {
        if(glm::dot(w,isect.normalGeometric)<=0.0f)
            return Color3f(0.0f);
    }

    return emittedLight;
}
Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi, Vector3f *wi, Float *pdf) const
{
    Intersection isect=shape->Sample(ref,xi,pdf);
    *wi=glm::normalize(isect.point-ref.point);
    if(fabs(*pdf)<FLT_EPSILON)
        return Color3f(0.0f);
    if(glm::length(isect.point-ref.point)<ShadowEpsilon)
        return Color3f(0.0f);
    return L(isect,-1.0f*(*wi));

    //return Color3f(0.0f);
}
float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    //Intersect sample ray with area light geometry
    Ray ray = ref.SpawnRay(wi);
    Intersection isectLight;
    bool ifIntersect=shape->Intersect(ray,&isectLight);
    if (!ifIntersect)
        return 0.0f;
    //Convert light sample weight to solid angle measure
    if(AbsDot(isectLight.normalGeometric, -1.0f*wi)<FLT_EPSILON)
        return 0.0f;
    Float pdf = glm::length2(ref.point-isectLight.point) /
    (AbsDot(isectLight.normalGeometric, -1.0f*wi) * shape->Area());
    return pdf;
}
