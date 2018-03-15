#include "mcv_platform.h"
#include "module_physics.h"
#include "entity/entity.h"
#include "components/comp_transform.h"
#include "render/mesh/mesh.h"
#include "render/mesh/mesh_loader.h"

#pragma comment(lib,"PhysX3_x64.lib")
#pragma comment(lib,"PhysX3Common_x64.lib")
#pragma comment(lib,"PhysX3Extensions.lib")
#pragma comment(lib,"PxFoundation_x64.lib")
#pragma comment(lib,"PxPvdSDK_x64.lib")
#pragma comment(lib,"PhysX3CharacterKinematic_x64.lib")
#pragma comment(lib,"PhysX3Cooking_x64.lib")

using namespace physx;

const VEC3 CModulePhysics::gravity(0, -1, 0);
physx::PxQueryFilterData CModulePhysics::defaultFilter;

/* REFACTOR THIS IN THE FUTURE, IT'S A BIG MESS */
void CModulePhysics::createActor(TCompCollider& comp_collider)
{
	const TCompCollider::TConfig & config = comp_collider.config;
	CHandle h_comp_collider(&comp_collider);
	CEntity* e = h_comp_collider.getOwner();
	TCompTransform * compTransform = e->get<TCompTransform>();
	VEC3 pos = compTransform->getPosition();
	QUAT quat = compTransform->getRotation();

	PxTransform initialTrans(PxVec3(pos.x, pos.y, pos.z), PxQuat(quat.x, quat.y, quat.z, quat.w));
	PxRigidActor* actor = nullptr;

	if (config.shapeType == physx::PxGeometryType::ePLANE)
	{
		PxRigidStatic* plane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
		actor = plane;
		setupFiltering(actor, config.group, config.mask);
		gScene->addActor(*actor);
	}
	else if (config.shapeType == physx::PxGeometryType::eCAPSULE && config.is_controller)
	{
		PxControllerDesc* cDesc;
		PxCapsuleControllerDesc capsuleDesc;

		PX_ASSERT(desc.mType == PxControllerShapeType::eCAPSULE);
		capsuleDesc.height = config.height;
		capsuleDesc.radius = config.radius;
		capsuleDesc.climbingMode = PxCapsuleClimbingMode::eCONSTRAINED;
		cDesc = &capsuleDesc;
		cDesc->material = gMaterial;
		cDesc->contactOffset = 0.01f;
		PxCapsuleController * ctrl = static_cast<PxCapsuleController*>(mControllerManager->createController(*cDesc));
		PX_ASSERT(ctrl);
		ctrl->setFootPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
		ctrl->setContactOffset(0.01f);
		actor = ctrl->getActor();
		comp_collider.controller = ctrl;
		setupFiltering(actor, config.group, config.mask);
	}
	else
	{
		PxShape* shape = nullptr;
		PxTransform offset(PxVec3(0.f, 0.f, 0.f));
		if (config.shapeType == physx::PxGeometryType::eBOX)
		{
			shape = gPhysics->createShape(PxBoxGeometry(config.halfExtent.x, config.halfExtent.y, config.halfExtent.z), *gMaterial);
			offset.p.y = config.halfExtent.y;
			shape->setContactOffset(1.f);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);

		}
		else if (config.shapeType == physx::PxGeometryType::eSPHERE)
		{
			shape = gPhysics->createShape(PxSphereGeometry(config.radius), *gMaterial);
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			offset.p.y = config.vOffset;
		}
		else if (config.shapeType == physx::PxGeometryType::eCONVEXMESH)
		{
			TMeshLoader * collider_mesh = loadCollider(config.filename.c_str());  // Move this to a custom collider resource

			PxCookingParams params = gCooking->getParams();
			params.convexMeshCookingType = PxConvexMeshCookingType::eQUICKHULL;
			params.gaussMapLimit = 256;
			gCooking->setParams(params);

			// Setup the convex mesh descriptor
			PxConvexMeshDesc desc;
			desc.points.data = collider_mesh->vtxs.data();
			desc.points.count = collider_mesh->header.num_vertexs;
			desc.points.stride = sizeof(PxVec3);
			desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
			
			PxConvexMesh* convex = gCooking->createConvexMesh(desc, gPhysics->getPhysicsInsertionCallback());
			PxConvexMeshGeometry convex_geo = PxConvexMeshGeometry(convex, PxMeshScale(), PxConvexMeshGeometryFlags());
			actor = gPhysics->createRigidStatic(initialTrans);
			shape = actor->createShape(convex_geo, *gMaterial);

			if (config.is_trigger)
			{
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
				actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			}

			actor->attachShape(*shape);
			shape->release();
			gScene->addActor(*actor);

			setupFiltering(actor, config.group, config.mask);
			comp_collider.actor = actor;
			actor->userData = h_comp_collider.asVoidPtr();
			return;
		}
		else if (config.shapeType == physx::PxGeometryType::eTRIANGLEMESH)
		{
			TMeshLoader * collider_mesh = loadCollider(config.filename.c_str()); // Move this to a custom collider resource

			PxTriangleMeshDesc meshDesc;
			meshDesc.points.data = collider_mesh->vtxs.data();
			meshDesc.points.count = collider_mesh->header.num_vertexs;
			meshDesc.points.stride = collider_mesh->header.bytes_per_vtx;
			meshDesc.flags = PxMeshFlag::e16_BIT_INDICES | PxMeshFlag::eFLIPNORMALS;

			meshDesc.triangles.data = collider_mesh->idxs.data();
			meshDesc.triangles.count = collider_mesh->header.num_indices / 3;
			meshDesc.triangles.stride = 3 * collider_mesh->header.bytes_per_idx;

			PxTriangleMesh * tri_mesh = gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());
			PxTriangleMeshGeometry tri_geo = PxTriangleMeshGeometry(tri_mesh, PxMeshScale());
			actor = gPhysics->createRigidStatic(initialTrans);
			shape = actor->createShape(tri_geo, *gMaterial);

			if (config.is_trigger)
			{
				shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
				shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
				shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
				actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
			}

			actor->attachShape(*shape);
			shape->release();
			gScene->addActor(*actor);

			setupFiltering(actor, config.group, config.mask);
			comp_collider.actor = actor;
			actor->userData = h_comp_collider.asVoidPtr();
			return;
		}
		//....todo: more shapes

		setupFiltering(shape, config.group, config.mask);
		shape->setLocalPose(offset);
		if (config.is_dynamic)
		{
			PxRigidDynamic* body = gPhysics->createRigidDynamic(initialTrans);
			actor = body;
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
		}
		else
		{
			PxRigidStatic* body = gPhysics->createRigidStatic(initialTrans);
			actor = body;
		}

		if (config.is_trigger)
		{
			shape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, false);
			shape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		}

		assert(shape);
		assert(actor);
		actor->attachShape(*shape);
		shape->release();
		gScene->addActor(*actor);
	}

	comp_collider.actor = actor;
	actor->userData = h_comp_collider.asVoidPtr();
}

CModulePhysics::FilterGroup CModulePhysics::getFilterByName(const std::string& name)
{
	if (strcmp("player", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Player;
	}
	else if (strcmp("enemy", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Enemy;
	}
	else if (strcmp("characters", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Characters;
	}
	else if (strcmp("wall", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Wall;
	}
	else if (strcmp("floor", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Floor;
	}
	else if (strcmp("ignore", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Ignore;
	}
	else if (strcmp("scenario", name.c_str()) == 0) {
		return CModulePhysics::FilterGroup::Scenario;
	}
	return CModulePhysics::FilterGroup::All;
}

PxFilterFlags CustomFilterShader(
  PxFilterObjectAttributes attributes0, PxFilterData filterData0,
  PxFilterObjectAttributes attributes1, PxFilterData filterData1,
  PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize
)
{
    if ( (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1) )
    {
        if ( PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1) )
        {
            pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
        }
        else {
            pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND;
        }
        return PxFilterFlag::eDEFAULT;
    }
    return PxFilterFlag::eSUPPRESS;
}

bool CModulePhysics::start()
{
	gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	assert(gFoundation != nullptr);

	if (!gFoundation) {
		return false;
	}

	gPvd = PxCreatePvd(*gFoundation);

	if (!gPvd) {
		return false;
	}

	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	bool  is_ok = gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));
	gDispatcher = PxDefaultCpuDispatcherCreate(2);

	if (!gPhysics)
		fatal("PxCreatePhysics failed");

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(gravity.x, -9.81f * gravity.y, gravity.z);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = CustomFilterShader;
	sceneDesc.flags = PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS | PxSceneFlag::eENABLE_ACTIVE_ACTORS;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();

	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}

	gMaterial = gPhysics->createMaterial(0.5f, 0.2f, 0.1f);
	mControllerManager = PxCreateControllerManager(*gScene);
	gScene->setSimulationEventCallback(&customSimulationEventCallback);
	PxInitExtensions(*gPhysics, gPvd);

	// Set a default filter to do query checks
	physx::PxFilterData pxFilterData;
	pxFilterData.word0 = FilterGroup::Scenario;
	defaultFilter.data = pxFilterData;

	return true;
}

void CModulePhysics::update(float delta)
{
	if (!gScene)
		return;

	gScene->simulate(delta);
	gScene->fetchResults(true);

	PxU32 nbActorsOut = 0;
	PxActor**actors = gScene->getActiveActors(nbActorsOut);

	for (unsigned int i = 0; i < nbActorsOut; ++i) {

		if (actors[i]->is<PxRigidActor>())
		{
			PxRigidActor* rigidActor = ((PxRigidActor*)actors[i]);
			PxTransform PxTrans = rigidActor->getGlobalPose();
			PxVec3 pxpos = PxTrans.p;
			PxQuat pxq = PxTrans.q;
			CHandle h_comp_collider;
			h_comp_collider.fromVoidPtr(rigidActor->userData);

			CEntity* e = h_comp_collider.getOwner();
			TCompTransform * compTransform = e->get<TCompTransform>();
			TCompCollider* compCollider = h_comp_collider;

			if (compCollider->controller)
			{
				PxExtendedVec3 pxpos_ext = compCollider->controller->getFootPosition();
				pxpos.x = pxpos_ext.x;
				pxpos.y = pxpos_ext.y;
				pxpos.z = pxpos_ext.z;
			}
			else
			{
				compTransform->setRotation(QUAT(pxq.x, pxq.y, pxq.z, pxq.w));
			}

			compTransform->setPosition(VEC3(pxpos.x, pxpos.y, pxpos.z));
			compCollider->lastFramePosition = VEC3(pxpos.x, pxpos.y, pxpos.z);
		}
	}
}

void CModulePhysics::render()
{

}

void CModulePhysics::CustomSimulationEventCallback::onTrigger(PxTriggerPair* pairs, PxU32 count)
{
	for (PxU32 i = 0; i < count; ++i)
	{
		if (pairs[i].flags & (PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
		{
			continue;
		}

		CHandle h_trigger_comp_collider;
		h_trigger_comp_collider.fromVoidPtr(pairs[i].triggerActor->userData);

		CHandle h_other_comp_collider;
		h_other_comp_collider.fromVoidPtr(pairs[i].otherActor->userData);
		CEntity* e_trigger = h_trigger_comp_collider.getOwner();

		dbg("trigger touch\n");
		if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			e_trigger->sendMsg(TMsgTriggerEnter{ h_other_comp_collider.getOwner() });
			TCompCollider * comp = (TCompCollider*)h_trigger_comp_collider;
			TCompCollider * comp_enemy = (TCompCollider*)h_other_comp_collider;

			if(comp_enemy->config.group & FilterGroup::Player)
			{
				comp->isInside = true;
			}
		}
		else if (pairs[i].status == PxPairFlag::eNOTIFY_TOUCH_LOST)
		{
			e_trigger->sendMsg(TMsgTriggerExit{ h_other_comp_collider.getOwner() });
			TCompCollider * comp = (TCompCollider*)h_trigger_comp_collider;
			TCompCollider * comp_enemy = (TCompCollider*)h_other_comp_collider;

			if (comp_enemy->config.group & FilterGroup::Player)
			{
				comp->isInside = false;
			}
		}
	}
}

void CModulePhysics::CustomSimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	for (PxU32 i = 0; i < nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];
		//dbg("contact found\n");
	}
}

/* Auxiliar physics methods */

bool CModulePhysics::Raycast(const VEC3 & origin, const VEC3 & dir, float distance, physx::PxRaycastHit & hit, physx::PxQueryFlag::Enum flag, physx::PxQueryFilterData filterdata)
{
	PxVec3 px_origin = PxVec3(origin.x, origin.y, origin.z);
	PxVec3 px_dir = PxVec3(dir.x, dir.y, dir.z); // [in] Normalized ray direction
	PxReal px_distance = (PxReal)(distance); // [in] Raycast max distance

	PxRaycastBuffer px_hit; // [out] Raycast results
	filterdata.flags = flag;

	bool status = gScene->raycast(px_origin, px_dir, px_distance, px_hit, PxHitFlags(PxHitFlag::eDEFAULT), filterdata); // Closest hit
	hit = px_hit.block;

	return status;
}
