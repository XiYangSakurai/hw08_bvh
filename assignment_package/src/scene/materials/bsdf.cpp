#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//TODO: Properly set worldToTangent and tangentToWorld
    : worldToTangent(glm::transpose(Matrix3x3(isect.tangent,isect.bitangent,isect.normalGeometric))),
      tangentToWorld(Matrix3x3(isect.tangent,isect.bitangent,isect.normalGeometric)),
//      worldToTangent(Matrix3x3(isect.tangent,isect.bitangent,isect.normalGeometric)),
//            tangentToWorld(glm::transpose(worldToTangent)),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{}
BSDF::~BSDF()
{
    for(int i=0;i<numBxDFs;i++)
    {
        delete bxdfs[i];
    }
}

void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //TODO: Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
//    worldToTangent=Matrix3x3(t,b,n);
//    tangentToWorld=glm::transpose(Matrix3x3(t,b,n));
    worldToTangent=glm::transpose(Matrix3x3(t,b,n));
    tangentToWorld=Matrix3x3(t,b,n);
}


//
Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    //TODO
    //return Color3f(0.f);


    //compute all BSDF
    Color3f f(0.0f);
    Vector3f wo=worldToTangent*woW;
    Vector3f wi=worldToTangent*wiW;
    bool is_reflect= glm::dot(wiW, normal)*glm::dot(woW,normal)>0;
    f=Color3f(0.0f);
    for(int i=0;i<numBxDFs;i++)
        {
            if(bxdfs[i]->MatchesFlags(flags) &&
                 ((is_reflect && (bxdfs[i]->type & BSDF_REFLECTION))||
                  (!is_reflect && (bxdfs[i]->type &BSDF_TRANSMISSION))))
            {
                f += bxdfs[i]->f(wo,wi);
            }
        }

    return f;

}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //TODO
    //choose BxDF
    int matchtype=BxDFsMatchingFlags(type);
    if(matchtype==0)
    {
        *pdf=0.0f;
        if(sampledType)
            *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }
    //generate random BxDF type using xi
    int bxdf_type = std::min((int)std::floor(xi[0]*matchtype),matchtype-1);
    BxDF* bxdf = nullptr;
    int count=bxdf_type;
    for(int i=0;i<numBxDFs;i++)
    {
        if(bxdfs[i]->MatchesFlags(type) && count--==0)
        {
            bxdf=bxdfs[i];
            break;
        }
    }
    //rewrite xi
    Point2f xi_new=Point2f(std::min(xi[0]*matchtype-bxdf_type,OneMinusEpsilon),xi[1]);

    //sample chosen BxDF
    Vector3f wi;
    Vector3f wo=worldToTangent*woW;
    //wo=glm::normalize(wo);
    if(fabs(wo.z)<FLT_EPSILON)
        return Color3f(0.0f);
    *pdf=0.0f;
    if(sampledType)
        *sampledType=bxdf->type;

    //compute f of BxDF
    Color3f f=bxdf->Sample_f(wo,&wi,xi_new,pdf,sampledType);
    if(fabs(*pdf)<FLT_EPSILON)
    {
        if(sampledType)
            *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }
    *wiW=tangentToWorld*wi;
    //wiW=glm::normalize(*wiW);
    //compute all pdf
    if(!(bxdf->type & BSDF_SPECULAR)&& matchtype>1)
    {
        for(int i=0;i<numBxDFs;++i)
        {
            if(bxdfs[i]!=bxdf && bxdfs[i]->MatchesFlags(type))
            {
                *pdf+=bxdfs[i]->Pdf(wo,wi);
            }
        }
    }
    if(matchtype>1)
        *pdf/=matchtype;

    //compute all BSDF
    if(!(bxdf->type &BSDF_SPECULAR) && matchtype>1)
    {
        bool is_reflect= glm::dot(*wiW, normal)*glm::dot(woW,normal)>0;
        f=Color3f(0.0f);
        for(int i=0;i<numBxDFs;i++)
        {
            if(bxdfs[i]->MatchesFlags(type) &&
                 ((is_reflect && (bxdfs[i]->type & BSDF_REFLECTION))||
                  (!is_reflect && (bxdfs[i]->type &BSDF_TRANSMISSION))))
            {
                f += bxdfs[i]->f(wo,wi);
            }
        }
    }
//    if(!(bxdf->type & BSDF_SPECULAR)&& matchtype>1)
//    {
//        *pdf+=this->Pdf(woW,*wiW,type);
//    }
//    if(!(bxdf->type &BSDF_SPECULAR) && matchtype>1)
//    {
//        f += this->f(woW,*wiW,type);
//    }

    return f;
//    //return Color3f(0.f);
     // Choose which BxDF to sample
//     int matchingComps = BxDFsMatchingFlags(type);
//     if (matchingComps == 0)
//     {
//     *pdf = 0;
//     if (sampledType) *sampledType = BxDFType(0);
//     return Color3f(0.f);
//     }

//     // Choose a BxDF at random from the set of candidates:

//     // Get an index that falls within the set of matching BxDFs using one of our
//     // pair of uniform random variables
//     //int comp = std::min((int)std::floor(xi[0] * matchingComps), matchingComps ‐ 1);
//     int comp=std::min((int)std::floor(xi[0]*matchingComps),matchingComps-1);
//     BxDF* bxdf = nullptr;
//     int count = comp;
//     for (int i = 0; i < numBxDFs; ++i)
//     {
//     if(bxdfs[i]->MatchesFlags(type) &&count-- ==0)
//     {
//         bxdf=bxdfs[i];
//         break;
//     }
//     }

//     // Remap BxDF sample xi to [0,1)^2
//     // If we don't do this, then we bias the _wi_ sample
//     // we'll get from BxDF::Sample_f, e.g. if we have two
//     // BxDFs each with a probability of 0.5 of being chosen, then
//     // when we sample the first BxDF (xi[0] = [0, 0.5)) we'll ALWAYS
//     // use a value between 0 and 0.5 to generate our wi for that BxDF.
//     //Point2f xiRemapped(std::min(xi[0] * matchingComps ‐ comp,OneMinusEpsilon),xi[1]);
//     Point2f xiRemapped(std::min(xi[0]*matchingComps-comp,OneMinusEpsilon),xi[1]);
//     // Sample chosen BxDF
//     Vector3f wi;
//     Vector3f wo = worldToTangent * woW; // Remember to convert woW to tangent space!
//     if (wo.z == 0) return Color3f(0.f); // The tangent‐space wo is perpendicular to the normal,
//     // so Lambert's law reduces its contribution to 0.
//     *pdf = 0;
//     if (sampledType) {*sampledType = bxdf->type;}

//     // Sample a tangent‐space wi based on the BxDF's PDF and then
//     // compute the BxDF::f of the chosen BxDF based on wi.
//      Color3f f = bxdf->Sample_f(wo, &wi, xiRemapped, pdf, sampledType);
//      if (*pdf == 0)
//      {
//      if (sampledType) {*sampledType = BxDFType(0);}
//      return Color3f(0.f);
//      }
//      *wiW = tangentToWorld * wi;

//      // Compute overall PDF with all matching BxDFs
//      // We treat specular BxDFs differently because their PDFs
//      // are delta distributions (0 in all cases but the *one* case
//      // where wi = reflect(wo, N), and 1 in that case)
//      if(!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
//      {
//      for (int i = 0; i < numBxDFs; ++i)
//      {
//      if (bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
//      {
//      *pdf += bxdfs[i]->Pdf(wo, wi);
//      }
//      }
//      }
//      if (matchingComps > 1) *pdf /= matchingComps;

//      // Compute the overall value of this BSDF for sampled direction wiW
//      // This means looking at the rest of the BxDFs that match _type_
//      // and invoking their implementations of BxDF::f.
//      // Again, we're going to skip this if the randomly chosen
//      // BxDF is specular because its PDF is a delta distribution.
//      if (!(bxdf->type & BSDF_SPECULAR) && matchingComps > 1)
//      {
//      bool reflect = glm::dot(*wiW, normal) * glm::dot(woW, normal) > 0;
//      f = Color3f(0.f);
//      for (int i = 0; i < numBxDFs; ++i)
//      {
//      if (bxdfs[i]->MatchesFlags(type) &&
//      ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
//      (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
//      {
//      f += bxdfs[i]->f(wo, wi);
//      }
//      }
//      }
//      return f;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    //TODO
    if(numBxDFs ==0)
        return 0.0f;
    Vector3f wo=worldToTangent*woW;
    Vector3f wi=worldToTangent*wiW;
    if(wo.z==0.0f)
        return 0.0f;
    Float pdf=0.0f;
    int matchNum=0;
    for(int i=0;i<numBxDFs;i++)
    {
        if(bxdfs[i]->MatchesFlags(flags))
        {
            ++matchNum;
            pdf+=bxdfs[i]->Pdf(wo,wi);
        }

    }
    float final_pdf=0.0f;
    if(matchNum>0)
        final_pdf=pdf/matchNum;
    return final_pdf;
    //return 0.f;
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //TODO
    if(!sampledType)
    {
        *sampledType=BxDFType(0);
        return Color3f(0.0f);
    }

    *wi=WarpFunctions::squareToHemisphereUniform(xi);
    if(wo.z<0.0f)
        wi->z*=-1.0f;
    Color3f s_f=f(wo,*wi);
    *pdf=Pdf(wo,*wi);
    return s_f;
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}
