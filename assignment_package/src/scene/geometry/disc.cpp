#include "disc.h"
#include "warpfunctions.h"
float Disc::Area() const
{
    //TODO
//    Point3f p0=glm::vec3(transform.T()*glm::vec4(0.5f,0.5f,0,1));
//    float r=glm::length(p0);
//    return Pi*pow(r,2);
    glm::vec3 scale=transform.getScale();
    return Pi*scale[0]*scale[1];
    //return 0.f;
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    //TODO: Compute tangent and bitangent
    Vector3f tangent;
    Vector3f bitangent;
    CoordinateSystem(Normal3f(0,0,1),&tangent,&bitangent);
//    *tan=glm::normalize(Vector3f(transform.T()* glm::vec4(tangent,1.0f)));
//    *bit=glm::normalize(Vector3f(transform.T()* glm::vec4(bitangent,1.0f)));
    *tan=glm::normalize(transform.T3()*tangent);
    *bit=glm::normalize(transform.T3()*bitangent);
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}
Intersection Disc::Sample(const Point2f &xi, Float *pdf) const
{
//    Point2f pd =WarpFunctions::squareToDiskConcentric(xi);
//    Point3f pObj(pd.x,pd.y,0.0f);
    Point3f pObj=WarpFunctions::squareToDiskConcentric(xi);
    Intersection it;
    it.normalGeometric=glm::normalize(transform.invTransT()*Normal3f(0,0,1));
    //it.point=transform.T3()*pObj;
    it.point=Point3f(transform.T()*glm::vec4(pObj,1.0f));
    *pdf=1.0f/Area();
    return it;
    //return Intersection();
}
Bounds3f Disc::WorldBound() const
{
    //return Bounds3f();
    Bounds3f local_bounds(Point3f(-1,-1,0),Point3f(1,1,0));
    return local_bounds.Apply(transform);
}
