#include "microfacetbtdf.h"

Color3f MicrofacetBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    if (SameHemisphere(wo, wi)) return Color3f(0.0f);

        Float cosThetaO = CosTheta(wo);
        Float cosThetaI = CosTheta(wi);
        if (cosThetaI == 0.0f || cosThetaO == 0.0f) return Color3f(0.0f);

        // Compute wh for microfacet transmission
        Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
        Vector3f wh = glm::normalize(wo + wi * eta);
        if (wh.z < 0) wh = -wh;

        //Color3f F = fresnel.Evaluate(Dot(wo, wh));
        Color3f F = fresnel->Evaluate(glm::dot(wo, wh));  //why not wi

        Float sqrtD = glm::dot(wo, wh) + eta * glm::dot(wi, wh);
        if(fabs(sqrtD)<FLT_EPSILON) return Color3f(0.0f);
        return (Color3f(1.0f) - F) * T *
               std::abs(distribution->D(wh) * distribution->G(wo, wi) * eta * eta *
                        AbsDot(wi, wh) * AbsDot(wo, wh) /
                        (cosThetaI * cosThetaO * sqrtD * sqrtD));
}

Color3f MicrofacetBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    if (wo.z == 0.0f) return Color3f(0.0f);
    Vector3f wh = distribution->Sample_wh(wo, xi);
    Float eta = CosTheta(wo) > 0 ? (etaA / etaB) : (etaB / etaA);
    //Normal3f norm=CosTheta(wo)>0.0f? wh:-wh;
    Normal3f norm=wh;
    bool refract=false;
    Float cosThetaI = glm::dot(norm, wo);
    Float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
    Float sin2ThetaT = eta * eta * sin2ThetaI;
    if (sin2ThetaT >= 1) refract=false;
    else{
    Float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wi = eta * -wo + (eta * cosThetaI - cosThetaT) * Vector3f(norm);
    refract=true;
    }
    if (!refract)
    return Color3f(0.0f);
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
    //return Color3f(0.0f);
}

FLOAT MicrofacetBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //return 0.0f;
    if (SameHemisphere(wo, wi)) return 0;
    //Compute wh from wo and wi for microfacet transmission
    Float eta = CosTheta(wo) > 0 ? (etaB / etaA) : (etaA / etaB);
    Vector3f wh = glm::normalize(wo + wi * eta);
    //Compute change of variables dwh_dwi for microfacet transmission
    Float sqrtD = glm::dot(wo, wh) + eta * glm::dot(wi, wh);
    if(fabs(sqrtD)<FLT_EPSILON) return 0.0f;
    Float dwh_dwi =
        std::abs((eta * eta * glm::dot(wi, wh)) / (sqrtD * sqrtD));
    return distribution->Pdf(wo, wh) * dwh_dwi;

}
