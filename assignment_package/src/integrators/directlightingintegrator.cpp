#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth,Color3f beta) const
{
    //TODO
    //compute intersection and Le
    Point2f xi=sampler->Get2D();
    Intersection isect;
    bool is_intersect=scene.Intersect(ray,&isect);
    if(is_intersect==false)
        return Color3f(0.0f);
    Color3f L_e=isect.Le(-1.0f*ray.direction);

    if(!isect.objectHit->GetMaterial())
        return L_e;


    Vector3f wiW;
    float pdf;
    Color3f Li(0.0f);
    Color3f fi;

    //sample one light
    int nLights=scene.lights.size();
    if(nLights==0)
        return L_e;
    int lightNum=std::min((int)(sampler->Get1D()*nLights),nLights-1);
    Li=scene.lights[lightNum]->Sample_Li(isect,xi,&wiW,&pdf);
    wiW=glm::normalize(wiW);
    pdf/=nLights;
    Float cs=AbsDot(isect.normalGeometric,wiW);
    if(fabs(pdf)<FLT_EPSILON)
        return L_e;

    //visibility
    Ray backRay=isect.SpawnRay(wiW);
    Intersection backIsect;
    bool back_inter=scene.Intersect(backRay,&backIsect);
    if(!back_inter)
        return L_e;
    const std::shared_ptr<Light> &a=scene.lights[lightNum];
    const std::shared_ptr<Light> &b=backIsect.objectHit->areaLight;
    if(!( a.get()==b.get()))
    Li=Color3f(0.0f);

    //compute fi
    fi=isect.bsdf->f(-1.0f*ray.direction,wiW);
    //LTE
    Color3f final=L_e+fi*Li*cs/pdf;
    //final=glm::clamp(final,0.0f,1.0f);
    return final;
}
