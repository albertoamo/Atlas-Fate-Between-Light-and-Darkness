#pragma once

#include "components/comp_base.h"
#include "geometry/transform.h"
#include "entity/common_msgs.h"

class TCompGlitchController : public TCompBase {

    void onGlitchDeploy(const TMsgGlitchController & msg);

    CTimer time_controller;

    DECL_SIBLING_ACCESS();
public:

    float fade_time;
    float fade_multiplier;

    void debugInMenu();
    void load(const json& j, TEntityParseContext& ctx);
    void update(float dt);

    static void registerMsgs();
};