#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());


void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    //TODO
    //intersect sample ray with area light
    Intersection isect=Sample(xi,pdf);
    Vector3f wi=glm::normalize(isect.point-ref.point);
    //convert light sample weight to solid angle measure
    float cosine=AbsDot(isect.normalGeometric,-1.0f*wi);
    if(fabs(cosine)<FLT_EPSILON)
        *pdf=0.0f;
    else
//        *pdf=glm::length2(ref.point-isect.point)/(cosine*Area());
        *pdf*=glm::length2(ref.point-isect.point)/cosine;
    return isect;
}
