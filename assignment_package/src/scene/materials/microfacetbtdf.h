#ifndef MICROFACETBTDF_H
#define MICROFACETBTDF_H

#include "bsdf.h"
#include "fresnel.h"
#include "microfacet.h"

class MicrofacetBTDF : public BxDF
{
public:
    MicrofacetBTDF(const Color3f &T,MicrofacetDistribution *distribution, Float etaA, Float etaB,Fresnel* fresnel)
                    : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), T(T), distribution(distribution),etaA(etaA),etaB(etaB),fresnel(fresnel) {}
      ~MicrofacetBTDF(){delete fresnel;}

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                                                     const Point2f &xi, Float *pdf,
                                                     BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;

private:
    const Color3f T;
    const MicrofacetDistribution* distribution;
    float etaA, etaB;
    const Fresnel* fresnel;
};

#endif // MICROFACETBTDF_H
