#include "RayCaster.h"

#include "ImGui/Util.h"

RayCollector::RayCollector(RE::Actor* a_actor, RE::hknpBSWorld* a_physicsWorld) :
	hknpClosestHitCollector(),
	actor(a_actor),
	physicsWorld(a_physicsWorld)
{}

void RayCollector::AddHit(const RE::hknpCollisionResult& a_result)
{
	auto colLayer = a_result.hitBodyInfo.shapeCollisionFilterInfo->GetCollisionLayer();

	switch (colLayer) {
	case RE::COL_LAYER::kStatic:
	case RE::COL_LAYER::kTerrain:
	case RE::COL_LAYER::kGround:
		hknpClosestHitCollector::AddHit(a_result);
		break;
	case RE::COL_LAYER::kBiped:
	case RE::COL_LAYER::kBipedNoCC:
	case RE::COL_LAYER::kDeadBip:
	case RE::COL_LAYER::kCharController:
		{
			RE::TESObjectREFR* owner = nullptr;
			if (physicsWorld) {
				if (auto body = RE::bhkNPCollisionObject::Getbhk(physicsWorld, a_result.hitBodyInfo.bodyId)) {
					owner = RE::TESObjectREFR::FindReferenceFor3D(body->sceneObject);
				}
			}

			if (owner == actor) {
				hknpClosestHitCollector::AddHit(a_result);
			}
		}
		break;
	default:
		break;
	}
}

void RayCaster::StartPoint::Init()
{
	auto player = RE::PlayerCharacter::GetSingleton();
	if (!player) {
		return;
	}

	debug = player->GetPosition();
	debug.z += player->GetCurrentEyeLevel();

	if (auto worldRoot = RE::Main::WorldRootNode(); !worldRoot->children.empty()) {
		camera = (*worldRoot->children.begin())->local.translate;
	} else if (auto pcCamera = RE::PlayerCamera::GetSingleton(); pcCamera && pcCamera->cameraRoot) {
		camera = pcCamera->cameraRoot->world.translate;
	} else {
		camera = debug;
	}
}

RayCaster::RayCaster(RE::Actor* a_target) :
	actor(a_target)
{
	startPoint.Init();
}

RayCaster::Result RayCaster::GetResult(bool a_debugRay)
{
	if (auto root = actor->Get3D()) {
		if (!RE::Main::WorldRootCamera()->PointInFrustum(root->worldBound.center, root->worldBound.fRadius)) {
			return Result::kOffscreen;
		}
	}

	auto cell = actor->GetParentCell();
	if (!cell || cell->cellState != RE::TESObjectCELL::CELL_STATE::kAttached || !cell->loadedData) {
		return Result::kOffscreen;
	}

	auto bhkWorld = cell->GetbhkWorld();
	if (!bhkWorld || !bhkWorld->worldNP.ptr) {
		return Result::kOffscreen;
	}

	targetPoints[0] = actor->CalculateLOSLocation(RE::ACTOR_LOS_LOCATION::kEye);
	targetPoints[1] = actor->CalculateLOSLocation(RE::ACTOR_LOS_LOCATION::kHead);
	targetPoints[2] = actor->CalculateLOSLocation(RE::ACTOR_LOS_LOCATION::kTorso);
	targetPoints[3] = actor->CalculateLOSLocation(RE::ACTOR_LOS_LOCATION::kFeet);

	RE::bhkPickData pickData{};

	RayCollector collector(actor, bhkWorld->worldNP.ptr);
	pickData.collector = &collector;
	pickData.collectorType = static_cast<RE::bhkPickData::COLLECTOR_TYPE>(1);
	pickData.castQuery.filterData.collisionFilterInfo->SetCollisionLayer(RE::COL_LAYER::kLOS);
	//pickData.castQuery.filterData.collisionFilterInfo->SetSystemGroup(RE::PlayerCharacter::GetSingleton()->GetCurrentCollisionGroup());

	bool result = false;

	for (std::uint32_t i = 0; i < targetPoints.size(); ++i) {
		if (result) {
			break;
		}

		pickData.SetStartEnd(startPoint.camera, targetPoints[i]);

		auto object = RE::TES::GetSingleton()->Pick(pickData);
		auto owner = object ? RE::TESObjectREFR::FindReferenceFor3D(object) : nullptr;
		if (owner == actor) {
			result = true;
		}
		if (a_debugRay) {
			DebugRay(pickData, object, owner, targetPoints[i], debugColors[i]);
		}
	}

	return result ? Result::kVisible : Result::kObscured;
}

void RayCaster::DebugRay(const RE::bhkPickData& a_pickData, RE::NiAVObject* a_obj, [[maybe_unused]] RE::TESObjectREFR* a_owner, const RE::NiPoint3& a_targetPos, ImU32 color) const
{
	const auto hitFrac = a_pickData.GetHitFraction();
	const auto hitPos = (a_targetPos - startPoint.debug) * hitFrac + startPoint.debug;

	ImGui::DrawLine(startPoint.debug, hitPos, a_pickData.HasHit() ? color : IM_COL32_BLACK);

	if (a_obj) {
		auto text = std::format("[{}]",
			a_pickData.HasHit() ? a_pickData.result.hitBodyInfo.shapeCollisionFilterInfo->GetCollisionLayer() : RE::COL_LAYER::kUnidentified);

		ImGui::DrawTextAtPoint(hitPos, text.c_str(), color);
	}
}
