#include "squareplane.h"

float SquarePlane::Area() const
{
    //TODO
//    Point3f p0=glm::vec3(transform.T()*glm::vec4(0.5f,0.5f,0,1));
//    Point3f p1=glm::vec3(transform.T()*glm::vec4(0.5f,-0.5f,0,1));
//    Point3f p2=glm::vec3(transform.T()*glm::vec4(-0.5f,0.5f,0,1));
//    return glm::length(glm::cross(p1-p0,p2-p0));
    glm::vec3 scale=transform.getScale();
    return 1.0f*scale[0]*scale[1];

    //return 1.0f;
    //return 0.f;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    //TODO: Compute tangent and bitangent
    Vector3f tangent=Vector3f(1.0,0,0);
    Vector3f bitangent=Vector3f(0,1.0,0);
    //CoordinateSystem(Normal3f(0,0,1),&tangent,&bitangent);
//    *tan=glm::normalize(Vector3f(transform.T()* glm::vec4(tangent,0.0f)));
//    *bit=glm::normalize(Vector3f(transform.T()* glm::vec4(bitangent,0.0f)));
    *tan=glm::normalize(transform.T3()*tangent);
    *bit=glm::normalize(transform.T3()*bitangent);
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}
Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const
{
    //return Intersection();
    Point2f pd(xi.x-0.5,xi.y-0.5);
    Point3f pObj(pd.x,pd.y,0.0f);
    Intersection it;
    it.normalGeometric=glm::normalize(transform.invTransT()*Normal3f(0,0,1));
    it.point=Point3f(transform.T()*glm::vec4(pObj,1.0f));
    *pdf=1.0f/Area();
    return it;
}
Bounds3f SquarePlane::WorldBound() const
{
    //return Bounds3f();
    Bounds3f local_bounds(Point3f(-0.5,-0.5,0),Point3f(0.5,0.5,0));
    return local_bounds.Apply(transform);
}
