#ifndef MICROFACETMATERIAL_H
#define MICROFACETMATERIAL_H
#include "scene/materials/material.h"

class MicrofacetMaterial : public Material
{
public:
    MicrofacetMaterial(const Color3f &Kr, const Color3f &Kt,
                       float roughness, float indexOfRefraction,
                       const std::shared_ptr<QImage> &roughnessMap,
                       const std::shared_ptr<QImage> &textureMapRefl,
                       const std::shared_ptr<QImage> &textureMapTransmit,
                       const std::shared_ptr<QImage> &normalMap)
             : Kr(Kr), Kt(Kt),roughness(roughness),indexOfRefraction(indexOfRefraction),
               roughnessMap(roughnessMap), textureMapRefl(textureMapRefl),textureMapTransmit(textureMapTransmit),
               normalMap(normalMap)
         {}

         void ProduceBSDF(Intersection *isect) const;


     private:
         Color3f Kr;

         Color3f Kt;

         float roughness;               // The overall roughness of the material. Will be multiplied
                                        // with the roughness map's values.

         float indexOfRefraction;
         std::shared_ptr<QImage> roughnessMap;
         std::shared_ptr<QImage> textureMapRefl;
         std::shared_ptr<QImage> textureMapTransmit;
         std::shared_ptr<QImage> normalMap;
};

#endif // MICROFACETMATERIAL_H
