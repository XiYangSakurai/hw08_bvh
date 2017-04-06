#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth, Color3f beta) const
{
    //TODO
    //return Color3f(0.f);

    Point2f xi=sampler->Get2D();
    Intersection isect;
    bool is_intersect=scene.Intersect(ray,&isect);
    if(is_intersect==false)
        return Color3f(0.0f);

    Color3f f_i(0.0f);
    Color3f L_e=isect.Le(-1.0f*ray.direction);

    if(!isect.objectHit->GetMaterial())
        return L_e;

    Vector3f wiW;
    float pdf;
    BxDFType type;
    f_i=isect.bsdf->Sample_f(-1.0f*ray.direction,&wiW,xi,&pdf,BSDF_ALL,&type);
    if(depth==0)
    {
        if(type==BSDF_SPECULAR)
            depth++;
        else
            return L_e;
    }
    wiW=glm::normalize(wiW);
    Float cs=AbsDot(isect.normalGeometric,wiW);
    if(fabs(pdf)<FLT_EPSILON)
        return L_e;

    Color3f L_r(0.0f);
    if(depth>0)
    {
        Ray nextRay=isect.SpawnRay(wiW);
        L_r=f_i*cs*Li(nextRay,scene,sampler,--depth,beta)/pdf;
    }

    Color3f final=L_e+L_r;
    //final=glm::clamp(final,0.0f,1.0f);
    return final;

}
