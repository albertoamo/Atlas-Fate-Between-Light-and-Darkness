#pragma once

#include "modules/module.h"
#include "render/mesh/mesh_instanced.h"

class TInstance;
class TCompTransform;

struct TInstance {
    MAT44 world;
};

class CModuleInstancing : public IModule {

    // Static instances, for testing purposes
    // -------------------------------------------------------------------
    CRenderMeshInstanced* instances_mesh = nullptr;
    std::vector< TInstance > instances;

    // -------------------------------------------------------------------
    struct TInstanceBlood {
        MAT44 world;
        VEC4  color;
    };
    CRenderMeshInstanced* blood_instances_mesh = nullptr;
    std::vector<TInstanceBlood> blood_instances;

    // -------------------------------------------------------------------
    struct TRenderParticle {
        VEC3  pos;      // Maps to iCenter.xyz
        float angle;    // Maps to iCenter.w
        VEC4  color;
        float scale_x;
        float scale_y;
        float nframe;
    };
    CRenderMeshInstanced* particles_instances_mesh = nullptr;
    std::vector<TRenderParticle> particles_instances;

    // Dynamic global instances, to be hold in the map.
    // -------------------------------------------------------------------
    struct TInstanceCollector {
        std::vector<TInstance> _instances;
        CRenderMeshInstanced* _instances_mesh;
    };
    
    std::map<std::string, TInstanceCollector> _global_instances;

public:

    CModuleInstancing(const std::string& name) : IModule(name) {}

    bool start() override;
    void render() override;
    void update(float delta) override;

    int addInstance(const std::string & name, MAT44 w_matrix);
    void removeInstance(TInstance* instance);
    void updateInstance(const std::string& name, int index, const MAT44& w_matrix);
    void clearInstances();
};