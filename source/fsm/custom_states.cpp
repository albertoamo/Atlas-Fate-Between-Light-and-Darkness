#include "mcv_platform.h"
#include "custom_states.h"
#include "context.h"
#include "components/player_controller/comp_player_animator.h"
#include "components/player_controller/comp_player_tempcontroller.h"

//class TCompTempPlayerController;
//class TCompPlayerAnimator;
//
//typedef void (TCompTempPlayerController::*actionhandler)(float);
//typedef void (TCompPlayerAnimator::*animatonhandler)(float);

namespace FSM
{

	bool AnimationState::load(const json& jData){

		_animationName = jData["animation"];

		return true;
	}

	void AnimationState::onStart(CContext& ctx) const {

		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ _animationName });
		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgChangeState{ (actionhandler)&TCompTempPlayerController::idleState, "pj_idle" });
	}

	void AnimationState::onFinish(CContext& ctx) const {

	}

	bool WalkState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 4.f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);

		return true;
	}

	void WalkState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "walk" });

		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgChangeState{ (actionhandler)&TCompTempPlayerController::playerMotion, "pj_walk" });
	}

	void WalkState::onFinish(CContext& ctx) const {

	}

	bool RunState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 1.f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);

		return true;
	}

	void RunState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "run" });
	}

	void RunState::onFinish(CContext& ctx) const {

	}

	bool FallState::load(const json& jData) {

		_force = jData.value("force", 1.f);
		_animationName = jData["animation"];
		return true;
	}

	void FallState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "fall" });
	}

	void FallState::onFinish(CContext& ctx) const {

	}


	bool CrouchState::load(const json& jData) {

		_animationName = jData["animation"];
		_speed = jData.value("speed", 1.f);
		_rotation_speed = jData.value("rotationSpeed", 10.f);

		return true;
	}

	void CrouchState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });
		CEntity* e = ctx.getOwner();
		e->sendMsg(TMsgChangeState{ (actionhandler)&TCompTempPlayerController::playerMotion, "pj_crouch" });
	}

	void CrouchState::onFinish(CContext& ctx) const {

	}

	bool MergeState::load(const json& jData) {

		_animationName = jData["animation"];
		return true;
	}

	void MergeState::onStart(CContext& ctx) const {

		// Send a message to the player controller
		// Send a message to the player controller
		//CEntity* e = ctx.getOwner();
		//e->sendMsg(TMsgAnimation{ "crouch" });
	}

	void MergeState::onFinish(CContext& ctx) const {

	}

	bool AttackState::load(const json& jData) {

		_animationName = jData["animation"];

		return true;
	}

	void AttackState::onStart(CContext& ctx) const {

		// ..
	}
	void AttackState::onFinish(CContext& ctx) const {

	}

}