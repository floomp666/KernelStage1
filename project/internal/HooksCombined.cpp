// Small hook implementations, consolidated without changing function bodies.

// ---- HooksCombined.cpp ----
#line 1 "HooksCombined.cpp"
// Small hook implementations, consolidated without changing function bodies.

// ---- AddPunch_hk.cpp ----
#line 1 "AddPunch_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::AddPunch_hk(SDK::HeldEntity* attackEntity, Vector3 amount, float duration) {
    Vector3 amount_new = amount;

    __try {
        if (entity_data::local_player && entity_data::local_player->input() && entity_data::local_player->input()->state() && entity_data::local_player->input()->state()->current()) {

            if (setting::misc::no_recoil) {
                amount_new = amount * 0.0f;
            }

            entity_data::local_player->input()->state()->current()->aimAngles() += (amount - amount_new);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    if (attackEntity) {
        attackEntity->AddPunch(amount_new, duration);
    }
}

// ---- BodyLeanOffset_hk.cpp ----
#line 1 "BodyLeanOffset_hk.cpp"
#include "Hooks.hpp"
#include "manipulator.hpp"

Vector3 hooks::methods::BodyLeanOffset_hk(SDK::PlayerEyes* a1) {
    __try {
        if (g_in_camera) {
            return a1->BodyLeanOffset();
        }
        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
            return current_manipulate_angle;
        }
    }
    __except (true) {
    }

    return a1->BodyLeanOffset();
}


// ---- DoAttack_hk.cpp ----
#line 1 "DoAttack_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::DoAttack_hk(SDK::FlintStrikeWeapon* weapon_info) {
    if (!weapon_info) return;

    __try {
        if (entity_data::local_player) {
            if (setting::misc::insta_eoka) {
                weapon_info->_didSparkThisFrame() = true;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        if (entity_data::local_player) {
            if (!setting::misc::insta_eoka) {
                weapon_info->_didSparkThisFrame() = false;
            }
        }
    }

    weapon_info->DoAttack();
}


// ---- DoFirstPersonCamera_hk.cpp ----
#line 1 "DoFirstPersonCamera_hk.cpp"
#include "Hooks.hpp"
#include "manipulator.hpp"

void hooks::methods::DoFirstPersonCamera_hk(SDK::PlayerEyes* a1, SDK::UnityEngine::Component* cam) {
    if (!a1 || !cam) return;

    __try {
        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
            g_in_camera = true; a1->DoFirstPersonCamera(cam); g_in_camera = false;

            Vector3 re_p = entity_data::local_player->GetTransform()->GetPosition() + entity_data::local_player->GetTransform()->up() * (SDK::PlayerEyes::EyeOffset().y + entity_data::local_player->eyes()->viewOffset().y);
            cam->GetTransform()->set_position(re_p);
        } 
        else {
            a1->DoFirstPersonCamera(cam);
        }
    }
    __except (true) {
    }
}


// ---- DoHit_hk.cpp ----
#line 1 "DoHit_hk.cpp"
#include "Hooks.hpp"

bool hooks::methods::DoHit_hk(SDK::Projectile* projectile, SDK::HitTest* hitTest, Vector3 point, Vector3 normal) {
	__try {
		if (setting::misc::wall_shot) {
			if (hitTest) {
				auto lol = hitTest->HitEntity();

				if (!lol) {
					return false;
				}

				auto classname = lol->GetClassName();
				if (classname &&
					!m_strcmp(classname, "BasePlayer") &&
					!m_strcmp(classname, "BaseAnimalNPC") &&
					!m_strcmp(classname, "Boar") &&
					!m_strcmp(classname, "Bear") &&
					!m_strcmp(classname, "BaseNPC") &&
					!m_strcmp(classname, "ScientistNPC") &&
					!m_strcmp(classname, "NPCPlayer") &&
					!m_strcmp(classname, "Wolf")) {
					return false;
				}
			}
		}
	}
	__except (true)
	{
	}
	return projectile->DoHit(hitTest, point, normal);
}


// ---- DoHitNotify_hk.cpp ----
#line 1 "DoHitNotify_hk.cpp"
#include "Hooks.hpp"
#pragma comment(lib, "winmm.lib") 

void hooks::methods::DoHitNotify_hk(SDK::BaseCombatEntity* entity, SDK::HitInfo* hitInfo, SDK::Projectile* projectile) {
	__try
	{
		if (entity->IsFrom("BasePlayer")) {
			
			if (setting::menu::hit_sound) {
				ui::play_sound(1);
			}

			return;
		}
	}
	__except (true) {
	}
	return entity->DoHitNotify(entity, hitInfo, projectile);
}

// ---- DoMovement_hk.cpp ----
#line 1 "DoMovement_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::DoMovement_hk(SDK::Projectile* projectile, float deltaTime) {
    if (!projectile) {
        return;
    }

    if (projectile->owner() == entity_data::local_player) {
        auto currentVelocity = projectile->currentVelocity() * deltaTime;
        auto intmag = currentVelocity.Magnitude();

        if (intmag > 0.0001f) {
            auto intnum = 1.f / intmag;
            auto speed = currentVelocity * intnum;
            auto lineposition = projectile->currentPosition() + speed * currentVelocity.Magnitude();

            if (setting::misc::bullet_tp) {
                SDK::BasePlayer* projectileTarget = bullets_target_info.bullet_target_player;
                if (projectileTarget) {
                    SDK::UnityEngine::Transform* playerTransform = bullets_target_info.bullet_target_transform;
                    SDK::HitTest* hitTest = projectile->hitTest();

                    if (playerTransform && hitTest) {
                        Vector3 playerPos = playerTransform->GetPosition();
                        Vector3 bulletPos = projectile->currentPosition();
                        Vector3 nextBulletPos = projectile->currentPosition() + (projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime());

                        SDK::Line updateLine = SDK::Line(bulletPos, nextBulletPos);
                        Vector3 closestUpdateToPlayer = updateLine.ClosestPoint(playerPos);

                        Vector3 yplayerPos;
                        if (closestUpdateToPlayer.y - 1 > playerTransform->GetPosition().y)
                            yplayerPos = playerTransform->GetPosition() - Vector3(0, 0.2, 0);
                        else
                            yplayerPos = playerTransform->GetPosition();

                        float distanceToPlayer = closestUpdateToPlayer.Distance(yplayerPos);

                        Vector3 newBulletPos = Vector3::MoveTowards(closestUpdateToPlayer, playerPos, 0.99f);
                        Vector3 newBulletPosx2 = Vector3::MoveTowards(newBulletPos, playerPos, 0.99f);

                        if (distanceToPlayer < 3.2f) {
                            Vector3 pointStart = newBulletPos;
                            Vector3 vector = newBulletPosx2;
                            vector -= projectile->currentVelocity().Normalized() * 0.001f;
                            Vector3 vector2 = vector;
                            Vector3 b2 = (pointStart - closestUpdateToPlayer).Normalized() * 0.01f;
                            Vector3 b3 = (vector2 - pointStart).Normalized() * 0.01f;

                            if (SDK::UnityEngine::LineOfSight(bulletPos, closestUpdateToPlayer) &&
                                SDK::UnityEngine::LineOfSight(closestUpdateToPlayer, playerPos) &&
                                SDK::UnityEngine::LineOfSight(closestUpdateToPlayer - b2, pointStart + b2) &&
                                SDK::UnityEngine::LineOfSight(pointStart - b3, vector2) &&
                                SDK::UnityEngine::LineOfSight(vector2, vector))
                            {
                                SDK::ProtoBuf::PlayerProjectileUpdate* ppu = SDK::ProtoBuf::PlayerProjectileUpdate::New();
                                if (ppu && hitTest && playerTransform && projectileTarget) {
                                    if (projectile->projectileID() != 0) {
                                        Vector3 velocityPerTick = projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime();
                                        float velocityPerTickSpeed = velocityPerTick.Length();

                                        if (velocityPerTickSpeed > 0.0001f) {
                                            float speedPerSecond = 1.f / velocityPerTickSpeed;
                                            float distanceTraveled = (closestUpdateToPlayer - bulletPos).Length();
                                            float traveledTime = distanceTraveled * speedPerSecond * SDK::UnityEngine::Time::GetFixedDeltaTime();

                                            projectile->traveledDistance() += distanceTraveled;
                                            projectile->traveledTime() += traveledTime;

                                            ppu->projectileId() = projectile->projectileID();
                                            ppu->travelTime() = projectile->traveledTime() + traveledTime;
                                            ppu->curVelocity() = projectile->currentVelocity();
                                            ppu->curPosition() = closestUpdateToPlayer;

                                            if (entity_data::local_player) {
                                                entity_data::local_player->SendProjectileUpdate(ppu);
                                                ppu->curPosition() = newBulletPos;
                                                entity_data::local_player->SendProjectileUpdate(ppu);
                                            }

                                            hitTest->DidHit() = true;
                                            hitTest->HitEntity() = projectileTarget;
                                            hitTest->HitTransform() = playerTransform;
                                            hitTest->HitPoint() = playerTransform->InverseTransformPoint(newBulletPosx2);
                                            hitTest->HitNormal() = playerTransform->InverseTransformDirection(newBulletPosx2);
                                            hitTest->AttackRay() = SDK::UnityEngine::Ray(newBulletPos, Vector3());

                                            auto matStr = SDK::System::String::New("Flesh");
                                            if (matStr) {
                                                hitTest->HitMaterial() = matStr;
                                            }

                                            if (check_hit(newBulletPosx2, projectile)) {
                                                projectile->DoHit(hitTest, newBulletPosx2, Vector3());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Vector3 bulletPos = projectile->currentPosition();
            Vector3 nextBulletPos = projectile->currentPosition() + (projectile->currentVelocity() * SDK::UnityEngine::Time::GetFixedDeltaTime());
        }
    }

    return projectile->DoMovement(deltaTime);
}

// ---- EyePositionForPlayer_hk.cpp ----
#line 1 "EyePositionForPlayer_hk.cpp"
#include "Hooks.hpp"
#include "manipulator.hpp"

Vector3 hooks::methods::EyePositionForPlayer_hk(SDK::BaseMountable* mount, SDK::BasePlayer* player, SDK::UnityEngine::Quaternion lookRot) {
    __try {
        if (player && player->userID()) {
            if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key)) {
                return mount->EyePositionForPlayer(player, lookRot) + current_manipulate_angle;
            }
        }
    }
    __except (true) {
    }

    return mount->EyePositionForPlayer(player, lookRot);
}

// ---- GetModifiedAimConeDirection_hk.cpp ----
#line 1 "GetModifiedAimConeDirection_hk.cpp"
#include "Hooks.hpp"

Vector3 hooks::methods::GetModifiedAimConeDirection_hk(float aimCone, Vector3 inputVec, bool canAddCone) {
    __try {
        if (entity_data::local_player) {
            if (is_active(setting::combat::aim_key) && setting::combat::aim_enable) {
                if (setting::combat::use_pSilent) {
                    if (target_info.has_target) {
                        inputVec = target_info.aim_direction;
                    }
                }
            }

            if (setting::misc::no_spread) {
                aimCone *= 0.0f;
            }
        }
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return SDK::BaseProjectile::GetModifiedAimConeDirection_(aimCone, inputVec, canAddCone);
}

// ---- ProjectileUpdate_hk.cpp ----
#line 1 "ProjectileUpdate_hk.cpp"
#include "Hooks.hpp"

static __declspec(noinline) void ProjectileUpdateBody(SDK::Projectile* projectile) {

        projectile->Update();

        if (entity_data::local_player) {
            if (projectile->owner() != entity_data::local_player) return;

            SDK::BasePlayer* best_target = nullptr;
            Vector3 best_hit_pos = { 0,0,0 };

            Vector3 projectile_pos = projectile->currentPosition();
            Vector3 projectile_vel = projectile->currentVelocity();

            SDK::HitTest* hitTest = projectile->hitTest();
            SDK::HitInfo* hitInfo = projectile->hitInfo();

            if (setting::combat::aim_enable) {
                if (projectile->traveledTime() <= 0.4f) {
                    Vector3 velocity = projectile->currentVelocity();
                    float speed = velocity.Length();

                    if (speed > 1.0f) {
                        weapon_info.launch_speed = speed;
                    }
                    weapon_info.gravity_modifier = projectile->gravityModifier();
                    weapon_info.drag = projectile->drag();
                }
            }
            if (setting::visuals::draw_bullet_tracers) {
                void* projectile_id = (void*)projectile;
                float current_time = SDK::UnityEngine::Time::GetTime();
                float traveled = projectile->traveledTime();
                Vector3 pos = projectile->currentPosition();

                auto it = last_bulletPosition.find(projectile_id);
                if (it != last_bulletPosition.end()) {
                    const auto& info = it->second;
                    float dist = (pos - info.pos).Length();
                    float time_delta = current_time - info.time;

                    bool continuous =
                        traveled > 0.015f &&
                        time_delta < 0.15f &&
                        dist > 0.6f && dist < 90.0f;

                    if (continuous) {
                        tracers.emplace_back(TracerSegment{
                            info.pos,
                            pos,
                            current_time
                            });
                    }
                }
                last_bulletPosition[projectile_id] = { pos, current_time };
            }
        }

}

void hooks::methods::ProjectileUpdate_hk(SDK::Projectile* projectile) {
    if (!projectile) return;
    __try { ProjectileUpdateBody(projectile); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
}

// ---- SendClientTick_hk.cpp ----
#line 1 "SendClientTick_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::SendClientTick_hk(SDK::BasePlayer* player) {
    if (player && entity_data::local_player != 0 && player->userID() == entity_data::local_player->userID()) {
        __try {
            if (setting::movement::anti_aim) {
                if (player->input() && player->input()->state() && player->input()->state()->current()) {
                    player->input()->state()->current()->aimAngles() = Vector3(-999, rand() % 999, rand() % 999);
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (SDK::BasePlayer::SendClientTick_) {
        SDK::BasePlayer::SendClientTick_(player);
    }
}

// ---- TryToMove_hk.cpp ----
#line 1 "TryToMove_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::TryToMove_hk(SDK::ItemIcon* a1, void* method) {
    if (!a1) return;

    __try {
        a1->TryToMove();

        if (entity_data::local_player) {
            if (setting::movement::fast_loot) {
                if (a1->queuedForLooting()) {
                    a1->RunTimedAction();
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// ---- UpdateAmbient_hk.cpp ----
#line 1 "UpdateAmbient_hk.cpp"
#include "Hooks.hpp"

void hooks::methods::UpdateAmbient_hk(SDK::TOD_Sky* sky) {
    if (!sky) return;

    __try {
        sky->UpdateAmbient();
        if (entity_data::local_player) {
            auto stars = sky->Stars();

            if (setting::visuals::bright_night) {
                SDK::UnityEngine::Color night_color;
                night_color.r = setting::visuals::night_color[0];
                night_color.g = setting::visuals::night_color[1];
                night_color.b = setting::visuals::night_color[2];
                night_color.a = setting::visuals::night_color[3];

                sky->set_ambientMode(static_cast<int>(SDK::TOD_Sky::AmbientMode::Flat));
                sky->set_ambientIntensity(6.0f);
                sky->set_ambientLight(night_color);
            }
            if (setting::visuals::star_changer_enabled) {
                if (stars) {
                    stars->Size() = setting::visuals::star_size;
                    stars->Brightness() = setting::visuals::star_brightness;
                }
            }
            if (!setting::visuals::star_changer_enabled) {
                if (stars) {
                    stars->Size() = 1.0f;
                    stars->Brightness() = 1.0f;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// ---- UpdateVelocity_hk.cpp ----
#line 1 "UpdateVelocity_hk.cpp"
#include "Hooks.hpp"

ImVec2 rotatePoint1(const ImVec2& point, const ImVec2& center, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    ImVec2 rotatedPoint;
    rotatedPoint.x = (point.x - center.x) * c - (point.y - center.y) * s + center.x;
    rotatedPoint.y = (point.x - center.x) * s + (point.y - center.y) * c + center.y;

    return rotatedPoint;
}

void hooks::methods::UpdateVelocity_hk(SDK::PlayerWalkMovement* movement) {
    if (!movement) return;

    __try {
        if (entity_data::local_player) {
            if (!movement->flying()) {
                auto TargetMovement = movement->TargetMovement();

                if (setting::movement::infinite_jump) {
                    movement->landTime() = 0.0f;
                    movement->jumpTime() = 0.0f;
                    movement->groundTime() = 9999.0f;
                }
                if (setting::movement::enable_tp_to_head) {
                    auto* local_head_bone = Get::bone_transform(entity_data::local_player, BoneList::head);
                    Vector3 local_head = Get::transform_pos(local_head_bone);

                    if (!local_head.Empty()) {
                        SDK::BasePlayer* target_player = nullptr;
                        Vector3 target_pos;
                        float best_dist = 4.5f;

                        for (const auto& player : entity_data::players_list) {
                            if (!player.pawn) continue;

                            SDK::BasePlayer* pawn = (SDK::BasePlayer*)player.pawn;
                            if (!pawn) continue;

                            auto* head_bone = Get::bone_transform(pawn, BoneList::head);
                            if (!head_bone) continue;

                            Vector3 head_pos = Get::transform_pos(head_bone);
                            if (head_pos.Empty()) continue;

                            float dist = local_head.Distance(head_pos);
                            if (dist < best_dist) {
                                best_dist = dist;
                                target_player = pawn;
                                target_pos = head_pos;
                            }
                        }

                        if (target_player && best_dist <= 3.f && is_active(setting::movement::tp_to_head_key)) {
                            ImVec2 center(target_pos.x, target_pos.z);
                            float radius = 0.55f;

                            static float rotation = 0.0f;
                            static float speed = 5.0f;

                            float delta_time = SDK::UnityEngine::Time::GetDeltaTime();
                            if (delta_time <= 0.0f || delta_time > 0.1f) delta_time = 0.016f;

                            rotation += speed * delta_time;

                            ImVec2 startPoint = rotatePoint1(
                                ImVec2(center.x + radius, center.y),
                                center,
                                rotation
                            );

                            Vector3 teleportPos(startPoint.x, target_pos.y + 0.04f, startPoint.y);
                            movement->TeleportTo(teleportPos);
                        }
                    }
                }
                if (setting::movement::no_mini_sprint) {
                    if (TargetMovement.Length() > 0.1f) {
                        float speedMult;

                        if (movement->swimming() || GetAsyncKeyState(VK_LCONTROL)) {
                            speedMult = 1.7f;
                        }
                        else {
                            speedMult = 5.6f;
                        }

                        if (is_active(setting::movement::speed_key) && setting::movement::enable_speedHack) {
                            speedMult += setting::movement::speed_value;
                        }

                        Vector3 newVel(TargetMovement.x / TargetMovement.Length() * speedMult,
                            TargetMovement.y, TargetMovement.z / TargetMovement.Length() * speedMult);

                        movement->TargetMovement() = newVel;
                    }
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }

    movement->UpdateVelocity();
}


// ---- ClientInput_hk.cpp ----
#line 1 "ClientInput_hk.cpp"
#include "Hooks.hpp"
#include "custommodel.hpp"
#include "chams.hpp"

#include <cmath>
#include <unordered_map>
#include "manipulator.hpp"

namespace predict {
    float CalcBulletDrop(float time, float gravity) {
        return 0.5f * gravity * time * time;
    }

    bool SolveQuadratic(float a, float b, float c, float& t0, float& t1) {
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return false;
        float sqrt_disc = std::sqrt(disc);
        t0 = (-b - sqrt_disc) / (2.0f * a);
        t1 = (-b + sqrt_disc) / (2.0f * a);
        return true;
    }

    Vector3 CalculateRustPrediction(
        Vector3 start_pos,
        Vector3 target_pos,
        Vector3 target_velocity,
        const WeaponInfo& weapon)
    {
        float bullet_speed = weapon.launch_speed > 1.0f ? weapon.launch_speed : 375.0f;
        bullet_speed *= 0.975f;

        float gravity = 9.81f * (weapon.gravity_modifier > 0.001f ? weapon.gravity_modifier : 1.0f);

        if (std::isnan(target_velocity.x) || std::isnan(target_velocity.y) || std::isnan(target_velocity.z))
            target_velocity = Vector3{ 0.f, 0.f, 0.f };

        Vector3 to_target = target_pos - start_pos;
        float dist = to_target.Length();
        if (dist < 0.1f)
            return target_pos;

        Vector3 relative_vel = target_velocity;
        Vector3 delta = to_target;

        float a = relative_vel.Dot(relative_vel) - bullet_speed * bullet_speed;
        float b = 2.0f * (delta.Dot(relative_vel));
        float c = delta.Dot(delta);

        float t0 = 0.0f, t1 = 0.0f;
        float time = dist / bullet_speed;

        if (std::fabs(a) > 0.0001f && SolveQuadratic(a, b, c, t0, t1)) {
            float best_t = 9999.0f;
            if (t0 > 0.01f && t0 < best_t) best_t = t0;
            if (t1 > 0.01f && t1 < best_t) best_t = t1;
            if (best_t < 8.0f)
                time = best_t;
        }

        time = std::clamp(time, 0.01f, 8.0f);

        Vector3 predicted = target_pos + target_velocity * time;
        float drop = CalcBulletDrop(time, gravity);
        predicted.y += drop;

        for (int iter = 0; iter < 5; ++iter) {
            Vector3 delta_p = predicted - start_pos;
            float d = delta_p.Length();
            float new_time = d / bullet_speed;
            new_time = std::clamp(new_time, 0.01f, 8.0f);

            float alpha = 0.6f;
            time = time * (1.0f - alpha) + new_time * alpha;

            predicted = target_pos + target_velocity * time;
            drop = CalcBulletDrop(time, gravity);
            predicted.y += drop;
        }

        if (dist > 70.0f) {
            float dist_factor = 1.0f - ((dist - 70.0f) * 0.0007f);
            dist_factor = std::clamp(dist_factor, 0.92f, 1.0f);
            Vector3 lead = predicted - target_pos;
            predicted = target_pos + lead * dist_factor;
        }

        Vector3 lead = predicted - target_pos;
        float max_possible_lead = bullet_speed * time * 0.94f;
        float lead_len = lead.Length();
        if (lead_len > max_possible_lead && lead_len > 0.01f) {
            predicted = target_pos + lead * (max_possible_lead / lead_len);
        }

        return predicted;
    }

    void GetAimAnglesAndPredict() {
        target_info.has_target = false;
        target_info.target_player = nullptr;
        target_info.target_transform = nullptr;

        matrix4x4 v_matrix = Get::view_matrix();
        float screen_w = static_cast<float>(SDK::UnityEngine::Screen::GetWidth());
        float screen_h = static_cast<float>(SDK::UnityEngine::Screen::GetHeight());
        Vector2 screen_center{ screen_w * 0.5f, screen_h * 0.5f };

        Vector3 eyes_pos = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));
        if (eyes_pos.Empty())
            return;

        if (is_active(setting::combat::manipulated_key) && setting::combat::enable_manipulated && !current_manipulate_angle.Empty()) {
            eyes_pos = eyes_pos + current_manipulate_angle;
        }

        float current_fov = SDK::Graphics::_fov();
        if (current_fov <= 0.0f)
            current_fov = 90.0f;

        float fov_radius = setting::combat::aim_fov * (90.0f / current_fov);
        float min_screen_dist = fov_radius;

        SDK::BasePlayer* best_target = nullptr;
        SDK::UnityEngine::Transform* best_bone_trans = nullptr;
        Vector3 best_predicted_pos{ 0.f, 0.f, 0.f };

        for (const auto& player : entity_data::players_list) {
            auto player_pawn = reinterpret_cast<SDK::BasePlayer*>(player.pawn);
            if (!player_pawn)
                continue;

            if (setting::combat::ignore_team_combat_type && Get::team(player_pawn, entity_data::local_player))
                continue;
            if (setting::combat::ignore_npc_combat_type && player.is_npc)
                continue;
            if (setting::combat::ignore_sleepers_combat_type && Get::is_sleeper(player_pawn))
                continue;

            auto bone_trans = Get::bone_transform(player_pawn, setting::combat::target_bone);
            if (!bone_trans)
                continue;

            Vector3 world_pos = Get::transform_pos(bone_trans);
            if (world_pos.Empty())
                continue;

            if (setting::combat::visible_check_combat_type) {
                if (!SDK::UnityEngine::LineOfSight(eyes_pos, world_pos)) continue;
            }

            Vector3 target_velocity = Get::player_velocity(player_pawn);
            if (std::isnan(target_velocity.x) || std::isnan(target_velocity.y) || std::isnan(target_velocity.z))
                target_velocity = Vector3{ 0.f, 0.f, 0.f };

            Vector3 temp_predicted = CalculateRustPrediction(eyes_pos, world_pos, target_velocity, weapon_info);
            if (std::isnan(temp_predicted.x) || std::isnan(temp_predicted.y) || std::isnan(temp_predicted.z))
                temp_predicted = world_pos;

            Vector2 screen_world{}, screen_pred{};
            bool world_ok = world_pos.world_to_screen(screen_world, v_matrix.matrix);
            bool pred_ok = temp_predicted.world_to_screen(screen_pred, v_matrix.matrix);

            float world_dist = world_ok ? std::hypot(screen_world.x - screen_center.x, screen_world.y - screen_center.y) : 99999.f;
            float pred_dist = pred_ok ? std::hypot(screen_pred.x - screen_center.x, screen_pred.y - screen_center.y) : 99999.f;



            bool in_fov = (world_ok && world_dist <= fov_radius) || (pred_ok && pred_dist <= fov_radius);
            if (in_fov) {
                float current = (std::min)(world_dist, pred_dist);
                if (current < min_screen_dist) {
                    min_screen_dist = current;
                    best_target = player_pawn;
                    best_predicted_pos = temp_predicted;
                    best_bone_trans = bone_trans;
                }
            }
        }


        if (best_target && !best_predicted_pos.Empty() && best_bone_trans) {
            target_info.has_target = true;
            target_info.predicted_pos = best_predicted_pos;
            target_info.aim_direction = (best_predicted_pos - eyes_pos).Normalized();
            target_info.target_player = best_target;
            target_info.target_transform = best_bone_trans;
        }
    }
}
void GetBulletTPTarget() {
    if (!setting::misc::bullet_tp) {
        bullets_target_info.bullet_target_player = nullptr;
        bullets_target_info.bullet_target_transform = nullptr;
        return;
    }

    Vector3 local_pos = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));

    SDK::BasePlayer* best_target = nullptr;
    SDK::UnityEngine::Transform* best_transform = nullptr;
    float best_dist = 99999.f;

    for (const auto& player : entity_data::players_list) {
        auto player_pawn = (SDK::BasePlayer*)player.pawn;
        if (!player_pawn || player_pawn == entity_data::local_player) continue;

        if (setting::combat::ignore_team_combat_type && Get::team(player_pawn, entity_data::local_player)) continue;
        if (setting::combat::ignore_npc_combat_type && player.is_npc) continue;
        if (setting::combat::ignore_sleepers_combat_type && Get::is_sleeper(player_pawn)) continue;
        if (player_pawn->lifeState() == LifeState::Dead) continue;
        if (player_pawn->health() <= 0.f) continue;

        SDK::UnityEngine::Transform* player_bone = Get::bone_transform(player_pawn, BoneList::head);
        if (!player_bone) continue;

        Vector3 player_pos = Get::transform_pos(player_bone);
        if (player_pos.Empty()) continue;

        float dist = local_pos.Distance(player_pos);
        if (dist < best_dist) {
            best_dist = dist;
            best_target = player_pawn;
            best_transform = player_bone;
        }
    }

    bullets_target_info.bullet_target_player = best_target;
    bullets_target_info.bullet_target_transform = best_transform;
}

void hooks::methods::ClientInput_hk(SDK::BasePlayer* local_player, uintptr_t state) {
    SDK::BasePlayer::ClientInput_(local_player, state);

    __try {
        entity_data::local_player = SDK::LocalPlayer::GetEntity();
        Get::entity_loop();

        if (entity_data::local_player) {
            reset(false);

            auto active_item = Get::active_item(entity_data::local_player);
            auto held_raw = active_item ? active_item->heldEntity() : 0;
            auto* held_entity = held_raw ? reinterpret_cast<SDK::BaseProjectile*>(held_raw) : nullptr;
            auto* held_base = held_raw ? reinterpret_cast<SDK::BaseEntity*>(held_raw) : nullptr;
            int id = active_item && active_item->info() ? active_item->info()->itemid() : 0;

            auto movement = entity_data::local_player->movement();
            if (!movement) return;

            auto walk_movement = (SDK::PlayerWalkMovement*)movement;
            if (!walk_movement) return;

            Vector3 localEyes = Get::transform_pos(Get::bone_transform(entity_data::local_player, BoneList::head));
            if (localEyes.Empty()) return;

            if (setting::visuals::custommodel::enabled) {
                custommodel::tick();
            }
            if (setting::visuals::chams::enabled) {
                chams::tick();
            }

            if (setting::combat::aim_enable) {
                predict::GetAimAnglesAndPredict();
            }
            if (setting::misc::bullet_tp) {
                GetBulletTPTarget();
            }

            if (setting::movement::no_mini_sprint) {
                entity_data::local_player->add_modelstate_flag(SDK::ModelState::Flags::Sprinting);
            }
            if (setting::movement::insta_pickup_player) {
                for (const auto& player : entity_data::players_list) {
                    auto player_pawn = (SDK::BasePlayer*)player.pawn;

                    if (player_pawn->HasPlayerFlag(SDK::BasePlayer::PlayerFlags::Wounded) && localEyes.Distance(Get::transform_pos(Get::bone_transform(player_pawn, BoneList::head))) < 3.f && entity_data::local_player->GetKeyState(RustButton::USE)) {
                        AssistPlayer(player_pawn);
                    }
                }
            }
            if (setting::visuals::time_changer) {
                SDK::Admin::admintime() = setting::visuals::time_value;
            }
            if (setting::misc::silent_reload) {
                if (held_entity && held_base) {
                    if (id != 795236088 && id != 200773292 && id != 1525520776) {
                        float last_shot_time = held_entity->lastShotTime();

                        static float prev_last_shot = 0.0f;
                        static uint64_t prev_item = 0;
                        uint64_t cur_item = entity_data::local_player->clActiveItem();

                        if (cur_item != prev_item) {
                            prev_item = cur_item;
                            prev_last_shot = last_shot_time;
                            did_reload = false;
                            just_shot = false;
                            time_since_last_shot = 0;
                        }

                        if (last_shot_time != prev_last_shot && last_shot_time > 0.0f) {
                            just_shot = true;
                            did_reload = false;
                            fixed_time_last_shot = SDK::UnityEngine::Time::GetTime();
                            prev_last_shot = last_shot_time;
                        }

                        if (!did_reload) {
                            time_since_last_shot = SDK::UnityEngine::Time::GetTime() - fixed_time_last_shot;
                        }
                        if (just_shot && (time_since_last_shot > 0.2f)) {
                            held_base->ServerRPC("StartReload");
                            held_base->SendSignalBroadcast(SDK::BaseEntity::Signal::Reload);
                            just_shot = false;
                        }
                        if (time_since_last_shot > (held_entity->reloadTime() - (held_entity->reloadTime() / 10))
                            && !did_reload) {
                            held_base->ServerRPC("Reload");
                            did_reload = true;
                            time_since_last_shot = 0;
                        }
                    }
                }
            }
            if (setting::misc::no_sway) {
                if (held_entity != 0) {
                    held_entity->aimSway() = 0.0f;
                    held_entity->aimSwaySpeed() = 0.0f;
                    held_entity->aimconePenalty() = 0.0f;
                    held_entity->stancePenalty() = 0.0f;
                }
            }

            if (setting::visuals::fov_changer || is_active(setting::visuals::zoom_key) || !setting::visuals::fov_changer) {
                float targetFOV = 90.0f;

                if (is_active(setting::visuals::zoom_key)) {
                    targetFOV = setting::visuals::zoom_value;
                }
                else if (setting::visuals::fov_changer) {
                    targetFOV = setting::visuals::fov_value;
                }

                SDK::Graphics::_fov() = targetFOV;
            }
            if (setting::movement::enable_spider && is_active(setting::movement::spider_key) ||
                setting::movement::enable_airStack && is_active(setting::movement::airStack_key) ||
                setting::movement::enable_tp_to_head && is_active(setting::movement::tp_to_head_key)) {
                walk_movement->groundAngle() = 0.0f;
                walk_movement->groundAngleNew() = 0.0f;
            }

            if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key)) {
                find_manipulate_angle();
            }
            if (setting::movement::enable_airStack && is_active(setting::movement::airStack_key)) {
                walk_movement->gravityMultiplier() = 0.0f;
                walk_movement->gravityMultiplierSwimming() = -4.0f;
                walk_movement->jumpTime() = 0.0f;
                walk_movement->jumping() = false;
            }
            if (setting::combat::aim_enable && is_active(setting::combat::aim_key) && target_info.has_target) {
                if (!setting::combat::use_pSilent) {
                    auto playerInput = entity_data::local_player->input();
                    if (playerInput) {
                        Vector3 current_angles = playerInput->bodyAngles();
                        Vector3 target_angles = Vector3::Angles(localEyes, target_info.predicted_pos);

                        Vector3 delta = target_angles - current_angles;

                        while (delta.x > 180.0f)  delta.x -= 360.0f;
                        while (delta.x < -180.0f) delta.x += 360.0f;
                        while (delta.y > 180.0f)  delta.y -= 360.0f;
                        while (delta.y < -180.0f) delta.y += 360.0f;

                        float smooth = max(1.0f, setting::combat::aim_smooth);
                        Vector3 smoothed_angles = current_angles + (delta / smooth);

                        playerInput->bodyAngles() = smoothed_angles.ClampAngles();
                    }
                }
            }
            if (setting::combat::auto_fire && is_active(setting::combat::aim_key)) {
                if (setting::combat::aim_enable && setting::combat::use_pSilent) {
                    if (target_info.has_target && target_info.target_player) {
                        if (held_entity && active_item) {
                            const char* classname = held_entity->GetClassName();
                            bool isProjectile = (
                                strcmp(classname, "BaseProjectile") == 0 ||
                                strcmp(classname, "BowWeapon") == 0 ||
                                strcmp(classname, "CompoundBowWeapon") == 0 ||
                                strcmp(classname, "BaseLauncher") == 0 ||
                                strcmp(classname, "CrossbowWeapon") == 0 ||
                                strcmp(classname, "FlintStrikeWeapon") == 0);

                            if (isProjectile) {
                                float current_time = SDK::UnityEngine::Time::GetTime();
                                if (!held_entity->HasReloadCooldown() && target_info.target_player && held_entity->nextAttackTime() <= current_time) {

                                    auto target_playerPos = target_info.predicted_pos;
                                    Vector3 shot_eyes = localEyes;
                                    bool can_shoot = false;

                                    if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                        shot_eyes = localEyes + current_manipulate_angle;
                                    }

                                    if (SDK::UnityEngine::LineOfSight(shot_eyes, target_playerPos)) {
                                        Vector3 test_eye = entity_data::local_player->eyes()->position();
                                        if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                            test_eye = test_eye + current_manipulate_angle;
                                        }
                                        if (check_eye(test_eye)) {
                                            can_shoot = true;
                                        }
                                    }

                                    if (!can_shoot && setting::combat::enable_hitscan) {
                                        HitscanResult hs = hitscan_from_eye(target_playerPos);
                                        if (hs.found) {
                                            shot_eyes = hs.hit_point;
                                            if (SDK::UnityEngine::LineOfSight(shot_eyes, target_playerPos)) {
                                                Vector3 test_eye = entity_data::local_player->eyes()->position();
                                                if (setting::combat::enable_manipulated && is_active(setting::combat::manipulated_key) && !current_manipulate_angle.Empty()) {
                                                    test_eye = test_eye + (shot_eyes - localEyes);
                                                }
                                                if (check_eye(test_eye)) {
                                                    can_shoot = true;
                                                }
                                            }
                                        }
                                    }

                                    if (can_shoot) {
                                        held_entity->SendSignalBroadcast(SDK::BaseEntity::Signal::Attack, "");
                                        held_entity->doAttack();
                                        held_entity->LaunchProjectile();
                                        held_entity->ShotFired();
                                        held_entity->DidAttackClientside();
                                        held_entity->UpdateAmmoDisplay();
                                        held_entity->nextAttackTime() = current_time + held_entity->repeatDelay();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (auto mounted = entity_data::local_player->mounted()) {
                if (mounted) {
                    mounted->canWieldItems() = setting::movement::can_wield_items;
                }
            }
        }
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

// ---- Present_hk.cpp ----
#line 1 "Present_hk.cpp"
#include "Hooks.hpp"
#include "gui/gui.hpp"
#include "manipulator.hpp"
#include "IconManager.hpp"

void Visuals() {
    if (!entity_data::local_player) {
        if (!entity_data::players_list.empty()) {
            entity_data::players_list.clear();
        }

        return;
    }

    auto draw_list = ImGui::GetBackgroundDrawList();

    matrix4x4 view_matrix = Get::view_matrix();
    if (!view_matrix.matrix) return;

    float screen_width = (float)SDK::UnityEngine::Screen::GetWidth();
    float screen_height = (float)SDK::UnityEngine::Screen::GetHeight();

    auto get_u32_color = [](float color[3], float alpha = 1.0f) {
        return ImGui::ColorConvertFloat4ToU32({ color[0], color[1], color[2], alpha });
        };

    const ImU32 box_color = get_u32_color(setting::visuals::box_color);
    const ImU32 name_color = get_u32_color(setting::visuals::name_color);
    const ImU32 bullet_color = get_u32_color(setting::visuals::bullet_tracers_color);

    const ImU32 dist_color = get_u32_color(setting::visuals::distance_color);
    const ImU32 skeleton_color = get_u32_color(setting::visuals::skeleton_color);
    const ImU32 aim_target_color = get_u32_color(setting::combat::aim_target_color);

    const ImU32 npc_color = get_u32_color(setting::visuals::npc_color);
    const ImU32 team_color = get_u32_color(setting::visuals::team_color);
    const ImU32 sleepers_color = get_u32_color(setting::visuals::sleepers_color);

    ImU32 map_org;
    ImU32 box_org;
    ImU32 name_org;
    ImU32 dist_org;
    ImU32 skeleton_org;

    auto color_black = ImColor(0, 0, 0);
    auto color_green = ImColor(0, 255, 0);
    auto color_red = ImColor(255, 0, 0);

    if (setting::visuals::enable_esp) {
        for (const auto& player : entity_data::players_list) {
            if (!player.pawn) continue;

            SDK::BasePlayer* player_pawn = (SDK::BasePlayer*)player.pawn;
            if (!player_pawn) continue;

            float health = player_pawn->health();
            if (health < 0.0f || health > 1000.0f) continue;

            auto is_sleeper = Get::is_sleeper(player_pawn);
            auto is_team = Get::team(player_pawn, entity_data::local_player);

            if (setting::visuals::ignore_team_visuals_type && is_team) continue;
            if (setting::visuals::ignore_npc_visuals_type && player.is_npc) continue;
            if (setting::visuals::ignore_sleepers_visuals_type && is_sleeper) continue;

            ImColor box_org, name_org, dist_org, skeleton_org;
            if (is_team != 0) {
                box_org = team_color;
                name_org = team_color;
                dist_org = team_color;
                skeleton_org = team_color;
            }
            else if (is_sleeper != 0) {
                box_org = sleepers_color;
                name_org = sleepers_color;
                dist_org = sleepers_color;
                skeleton_org = sleepers_color;
            }
            else if (player.is_npc) {
                box_org = npc_color;
                name_org = npc_color;
                dist_org = npc_color;
                skeleton_org = npc_color;
            }
            else {
                box_org = box_color;
                name_org = name_color;
                dist_org = dist_color;
                skeleton_org = skeleton_color;
            }

            float min_x = FLT_MAX, min_y = FLT_MAX;
            float max_x = -FLT_MAX, max_y = -FLT_MAX;
            bool bone_visible = false;

            for (int bone_id : bounds_bones) {
                SDK::UnityEngine::Transform* bone = Get::bone_transform(player.pawn, bone_id);
                if (!bone) continue;

                Vector3 world_position = Get::transform_pos(bone);
                if (world_position.Empty()) continue;

                for (int i = 0; i < 8; i++) {
                    Vector3 corner = {
                        world_position.x + ((i & 1) ? 0.15f : -0.15f),
                        world_position.y + ((i & 2) ? 0.20f : -0.20f),
                        world_position.z + ((i & 4) ? 0.15f : -0.15f)
                    };

                    Vector2 screen_position;
                    if (!corner.world_to_screen(screen_position, view_matrix.matrix)) continue;

                    bone_visible = true;

                    min_x = min(min_x, screen_position.x);
                    max_x = max(max_x, screen_position.x);
                    min_y = min(min_y, screen_position.y);
                    max_y = max(max_y, screen_position.y);
                }
            }
            if (!bone_visible) continue;

            if (setting::visuals::box_esp) {
                draw_list->AddRect({ min_x, min_y }, { max_x, max_y }, box_org, 1.0f);
            }
            if (setting::visuals::name_esp) {
                SDK::System::String* displayName = player_pawn->displayName();

                if (displayName && displayName->len > 0 && displayName->buffer) {
                    char name_buffer[256] = { 0 };

                    int safe_len = (displayName->len > 120) ? 120 : displayName->len;

                    int converted_size = WideCharToMultiByte(
                        CP_UTF8,
                        0,
                        displayName->buffer,
                        safe_len,
                        name_buffer,
                        sizeof(name_buffer) - 1,
                        nullptr,
                        nullptr
                    );

                    if (converted_size > 0) {
                        name_buffer[converted_size] = '\0';

                        ImVec2 text_size = ImGui::CalcTextSize(name_buffer);

                        float text_x = min_x + (max_x - min_x) * 0.5f - text_size.x * 0.5f;
                        float text_y = min_y - text_size.y - 2.0f;

                        draw_list->AddText({ text_x, text_y }, name_org, name_buffer);
                    }
                }
            }

            if (setting::visuals::health_esp) {
                float clamped_health = std::clamp(health, 0.0f, 100.0f);
                float hp_pc = clamped_health / 100.0f;

                float bar_width = 3.0f;
                float bx = min_x - bar_width - 4.0f;

                float bar_height = max_y - min_y;
                draw_list->AddRectFilled({ bx - 1, min_y }, { bx + bar_width + 1, max_y }, ImColor(0, 0, 0, 255));

                ImColor health_color((int)(255 * (1.0f - hp_pc)), (int)(255 * hp_pc), 0, 255);
                draw_list->AddRectFilled({ bx, max_y - (bar_height * hp_pc) }, { bx + bar_width, max_y }, health_color);
            }
            if (setting::visuals::distance_esp) {
                SDK::UnityEngine::Transform* local_eyes = Get::bone_transform(entity_data::local_player, BoneList::head);
                SDK::UnityEngine::Transform* target_eyes = Get::bone_transform(player_pawn, BoneList::head);

                if (local_eyes && target_eyes) {
                    Vector3 local_pos = Get::transform_pos(local_eyes);
                    Vector3 target_pos = Get::transform_pos(target_eyes);

                    int distance = (int)(local_pos - target_pos).Length();
                    char dist_str[16];

                    sprintf_s(dist_str, "%dm", distance);

                    float dist_x = max_x + 3.0f;
                    float dist_y = min_y;

                    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize() * 0.85f, { dist_x, dist_y }, dist_org, dist_str);
                }
            }
            if (setting::visuals::skeleton_esp) {
                for (const auto& connection : skeleton_connections) {
                    SDK::UnityEngine::Transform* bone_start = Get::bone_transform(player.pawn, connection.first);
                    SDK::UnityEngine::Transform* bone_end = Get::bone_transform(player.pawn, connection.second);

                    if (!bone_start || !bone_end) continue;
                    Vector2 point_start, point_end;

                    if (Get::transform_pos(bone_start).world_to_screen(point_start, view_matrix.matrix) &&
                        Get::transform_pos(bone_end).world_to_screen(point_end, view_matrix.matrix)) {
                        draw_list->AddLine({ point_start.x, point_start.y }, { point_end.x, point_end.y }, skeleton_org, 1.0f);
                    }
                }
            }
            if (setting::visuals::inventory_esp || setting::menu::render_menu) {
                SDK::BasePlayer* nearest = nullptr;
                float best_dist = FLT_MAX;
                Vector2 center = { screen_width * 0.5f, screen_height * 0.5f };

                if (setting::visuals::inventory_esp) {
                    for (const auto& player : entity_data::players_list) {
                        if (!player.pawn) continue;

                        SDK::BasePlayer* pp = (SDK::BasePlayer*)player.pawn;
                        if (!pp) continue;

                        auto bone = Get::bone_transform(pp, BoneList::head);
                        if (!bone) continue;

                        Vector3 world = Get::transform_pos(bone);
                        if (world.Empty()) continue;

                        Vector2 screen;
                        if (!world.world_to_screen(screen, view_matrix.matrix)) continue;

                        float dx = screen.x - center.x;
                        float dy = screen.y - center.y;
                        float d = dx * dx + dy * dy;

                        if (d < best_dist) {
                            best_dist = d;
                            nearest = pp;
                        }
                    }
                }

                auto wear = nearest ? Get::wear_items(nearest) : std::vector<SDK::Item*>{};
                auto belt = nearest ? Get::belt_items(nearest) : std::vector<SDK::Item*>{};

                float icon_sz = setting::visuals::inventory_icon_size;
                if (icon_sz < 16.f) icon_sz = 16.f;
                if (icon_sz > 64.f) icon_sz = 64.f;

                float pad = 4.f;
                float slot_gap = 3.f;
                float wear_w = 7 * icon_sz + 6 * slot_gap + pad * 2;
                float belt_w = 6 * icon_sz + 5 * slot_gap + pad * 2;
                float total_w = max(wear_w, belt_w);
                float total_h = icon_sz * 2 + pad * 3 + slot_gap;

                bool movable = setting::menu::render_menu;
                ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

                if (!movable)
                    flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs;

                if (setting::visuals::reset_inventory_pos) {
                    ImGui::SetNextWindowPos(setting::visuals::inventory_pos, ImGuiCond_Always);
                    setting::visuals::reset_inventory_pos = false;
                }
                else {
                    ImGui::SetNextWindowPos(setting::visuals::inventory_pos, ImGuiCond_Once);
                }

                ImGui::SetNextWindowSize({ total_w, total_h });

                if (ImGui::Begin("##inventory_esp", nullptr, flags)) {
                    setting::visuals::inventory_pos = ImGui::GetWindowPos();

                    ImDrawList* d = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    ImGuiStyle& style = ImGui::GetStyle();

                    ImU32 slot_bg = ImGui::GetColorU32(ImVec4(
                        style.Colors[ImGuiCol_FrameBg].x,
                        style.Colors[ImGuiCol_FrameBg].y,
                        style.Colors[ImGuiCol_FrameBg].z,
                        0.45f
                    ));

                    ImU32 border_col = ImGui::GetColorU32(ImVec4(
                        style.Colors[ImGuiCol_Border].x,
                        style.Colors[ImGuiCol_Border].y,
                        style.Colors[ImGuiCol_Border].z,
                        0.6f
                    ));

                    float start_x_wear = p.x + (total_w - wear_w) * 0.5f + pad;
                    float y_wear = p.y + pad;

                    for (int i = 0; i < 7; i++) {
                        float sx = start_x_wear + i * (icon_sz + slot_gap);
                        float sy = y_wear;

                        d->AddRectFilled({ sx, sy }, { sx + icon_sz, sy + icon_sz }, slot_bg, 3.f);
                        d->AddRect({ sx, sy }, { sx + icon_sz, sy + icon_sz }, border_col, 3.f, 0, 1.0f);

                        if (i < (int)wear.size() && wear[i]) {
                            std::string sn = Get::item_shortname(wear[i]);
                            if (!sn.empty()) {
                                ImTextureID tex = IconManager::Get().GetTextureID(sn);
                                if (tex)
                                    d->AddImage(tex, { sx + 1, sy + 1 }, { sx + icon_sz - 1, sy + icon_sz - 1 });
                            }
                        }
                    }

                    float start_x_belt = p.x + (total_w - belt_w) * 0.5f + pad;
                    float y_belt = y_wear + icon_sz + pad;

                    for (int i = 0; i < 6; i++) {
                        float sx = start_x_belt + i * (icon_sz + slot_gap);
                        float sy = y_belt;

                        d->AddRectFilled({ sx, sy }, { sx + icon_sz, sy + icon_sz }, slot_bg, 3.f);
                        d->AddRect({ sx, sy }, { sx + icon_sz, sy + icon_sz }, border_col, 3.f, 0, 1.0f);

                        if (i < (int)belt.size() && belt[i]) {
                            std::string sn = Get::item_shortname(belt[i]);
                            if (!sn.empty()) {
                                ImTextureID tex = IconManager::Get().GetTextureID(sn);
                                if (tex)
                                    d->AddImage(tex, { sx + 1, sy + 1 }, { sx + icon_sz - 1, sy + icon_sz - 1 });
                            }
                        }
                    }

                    ImGui::End();
                }
            }
        }
    }
    if (setting::visuals::resource_esp) {
        for (const auto& res : entity_data::resource_list) {
            if (!res.pawn) continue;

            bool should_draw = false;
            ImU32 ore_color = 0;

            if (res.is_hemp && setting::visuals::draw_hemp) {
                should_draw = true;
                ore_color = get_u32_color(setting::visuals::hemp_color);
            }
            else if (!res.is_hemp) {
                if (res.type == OreType::stone && setting::visuals::draw_stone) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::stone_color);
                }
                else if (res.type == OreType::metal && setting::visuals::draw_metal) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::metal_color);
                }
                else if (res.type == OreType::sulfur && setting::visuals::draw_sulfur) {
                    should_draw = true;
                    ore_color = get_u32_color(setting::visuals::sulfur_color);
                }
            }

            if (!should_draw) continue;

            Vector3 draw_pos = res.position;
            draw_pos.y += 1.0f;

            Vector2 screen_pos;
            if (!draw_pos.world_to_screen(screen_pos, view_matrix.matrix)) continue;

            const char* label = nullptr;
            if (res.is_hemp) {
                label = "Hemp";
            }
            else if (res.type == OreType::stone) {
                label = "Stone";
            }
            else if (res.type == OreType::metal) {
                label = "Metal";
            }
            else if (res.type == OreType::sulfur) {
                label = "Sulfur";
            }

            if (!label) continue;

            auto font = ImGui::GetFont();
            float text_height = 0.2;

            ImVec2 text_size = ImGui::CalcTextSize(label);

            float text_x = screen_pos.x - text_size.x * 0.5f;
            float text_y = screen_pos.y - text_height * 0.5f;

            draw_list->AddText({ text_x - 1, text_y }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x + 1, text_y }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y - 1 }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y + 1 }, IM_COL32(0, 0, 0, 200), label);
            draw_list->AddText({ text_x, text_y }, ore_color, label);
        }
    }

    if (setting::visuals::crosshair) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);

        auto* draw_list = ImGui::GetForegroundDrawList();
        float sz = setting::visuals::crosshair_size;
        float thin = 0.5f;

        ImU32 color;
        if (setting::visuals::rainbow_crosshair) {
            float speed = 0.25f;
            float hue = fmodf((float)ImGui::GetTime() * speed, 1.0f);
            ImVec4 col_rainbow;
            ImGui::ColorConvertHSVtoRGB(hue, 1.0f, 1.0f, col_rainbow.x, col_rainbow.y, col_rainbow.z);
            color = ImGui::ColorConvertFloat4ToU32(ImVec4(col_rainbow.x, col_rainbow.y, col_rainbow.z, 1.0f));
        }
        else {
            color = ImGui::ColorConvertFloat4ToU32(ImVec4(setting::visuals::crosshair_color[0], setting::visuals::crosshair_color[1], setting::visuals::crosshair_color[2], 1.0f));
        }

        switch (setting::visuals::crosshair_type) {
        case 0: {
            float sz = setting::visuals::crosshair_size;
            float thin = 1.0f;

            draw_list->AddLine(ImVec2(center.x - sz, center.y), ImVec2(center.x + sz + 1.0f, center.y), color, thin);
            draw_list->AddLine(ImVec2(center.x, center.y - sz), ImVec2(center.x, center.y + sz + 1.0f), color, thin);

            break;
        }
        case 1: {
            float sz = setting::visuals::crosshair_size;
            float radius = sz * 0.4f;

            draw_list->AddCircleFilled(center, radius, color, 64);
            break;
        }
        case 2: {
            float rot_speed = 2.0f;
            float angle = (float)ImGui::GetTime() * rot_speed;
            float cos_a = cosf(angle);
            float sin_a = sinf(angle);

            auto rotate = [&](ImVec2 p) {
                float x = p.x - center.x;
                float y = p.y - center.y;
                return ImVec2(center.x + x * cos_a - y * sin_a, center.y + x * sin_a + y * cos_a);
                };

            ImVec2 p1 = rotate(ImVec2(center.x, center.y - sz));
            ImVec2 p2 = rotate(ImVec2(center.x, center.y + sz));
            ImVec2 p3 = rotate(ImVec2(center.x - sz, center.y));
            ImVec2 p4 = rotate(ImVec2(center.x + sz, center.y));

            draw_list->AddLine(p1, p2, color, thin);
            draw_list->AddLine(p3, p4, color, thin);
            draw_list->AddLine(p1, rotate(ImVec2(center.x + sz, center.y - sz)), color, thin);
            draw_list->AddLine(p2, rotate(ImVec2(center.x - sz, center.y + sz)), color, thin);
            draw_list->AddLine(p3, rotate(ImVec2(center.x - sz, center.y - sz)), color, thin);
            draw_list->AddLine(p4, rotate(ImVec2(center.x + sz, center.y + sz)), color, thin);
        }
              break;
        }
    }
    if (setting::combat::draw_manipulated) {
        draw_manipulator_esp();
    }
    if (setting::visuals::draw_bullet_tracers) {
        const float now = SDK::UnityEngine::Time::GetTime();
        const float max_life = setting::visuals::max_bullet_lifetime;

        for (auto it = last_bulletPosition.begin(); it != last_bulletPosition.end();) {
            if (now - it->second.time > 1.2f)
                it = last_bulletPosition.erase(it);
            else
                ++it;
        }

        for (auto it = tracers.begin(); it != tracers.end();) {
            float age = now - it->time;
            if (age > max_life) {
                it = tracers.erase(it);
                continue;
            }

            Vector2 s, e;
            if (it->start.world_to_screen(s, view_matrix.matrix) &&
                it->end.world_to_screen(e, view_matrix.matrix)) {
                float alpha = 1.0f - (age / max_life);
                alpha *= alpha;

                ImU32 col = ImGui::ColorConvertFloat4ToU32({
                    setting::visuals::bullet_tracers_color[0],
                    setting::visuals::bullet_tracers_color[1],
                    setting::visuals::bullet_tracers_color[2],
                    alpha
                    });

                draw_list->AddLine(
                    ImVec2(s.x, s.y),
                    ImVec2(e.x, e.y),
                    col,
                    setting::visuals::bullets_tracers_thickness
                );
            }
            ++it;
        }

        if (tracers.size() > 350)
            tracers.erase(tracers.begin(), tracers.begin() + (tracers.size() - 350));
    }

    if (setting::combat::aim_enable && setting::combat::draw_aim_fov) {
        float current_fov = SDK::Graphics::_fov();
        float fov_radius = setting::combat::aim_fov * (90.0f / current_fov);

        draw_list->AddCircle({ screen_width / 2.0f, screen_height / 2.0f }, fov_radius, ImColor(setting::combat::aim_fov_color[0], setting::combat::aim_fov_color[1], setting::combat::aim_fov_color[2]), 120, 1.0f);
    }
    if (setting::misc::reload_indicator && entity_data::local_player) {
        float fraction = 0.0f;
        bool has_reload = false;

        auto active_item = Get::active_item(entity_data::local_player);
        if (!active_item) return;

        auto held_raw = active_item->heldEntity();
        if (!held_raw) return;

        auto* held = reinterpret_cast<SDK::BaseProjectile*>(held_raw);
        if (!held) return;

        if (held->HasReloadCooldown()) {
            float time_left = held->nextReloadTime() - SDK::UnityEngine::Time::GetTime();
            float time_full = held->CalculateCooldownTime(held->nextReloadTime(), held->reloadTime()) - SDK::UnityEngine::Time::GetTime();
            if (time_full > 0.0f)
                fraction = clamp(time_left / time_full, 0.0f, 1.0f);
            fraction = 1.0f - fraction;
            has_reload = true;
        }
        else if (!did_reload && time_since_last_shot > 0.0f && time_since_last_shot <= (held->reloadTime() - (held->reloadTime() / 10))) {
            float time_full = held->reloadTime() - (held->reloadTime() / 10);
            fraction = clamp(time_since_last_shot / time_full, 0.0f, 1.0f);
            has_reload = true;
        }
        bool show_indicator = has_reload || setting::menu::render_menu;
        if (show_indicator) {
            float bar_width = 120.0f;

            float bar_height = 6.0f;
            float rounding = 3.0f;

            ImGuiStyle& style = ImGui::GetStyle();
            ImU32 bg_col = ImGui::GetColorU32(style.Colors[ImGuiCol_FrameBg]);

            ImU32 accent_col = ImGui::GetColorU32(style.Colors[ImGuiCol_Scheme]);
            bool movable = setting::menu::render_menu;

            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
            if (!movable) flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMouseInputs;

            if (setting::misc::reset_reload_indicator_pos) {
                ImGui::SetNextWindowPos(setting::misc::reload_indicator_pos, ImGuiCond_Always);
                setting::misc::reset_reload_indicator_pos = false;
            }
            ImGui::SetNextWindowSize({ bar_width + 4, bar_height + 4 });
            if (ImGui::Begin("##reload_indicator", nullptr, flags)) {

                setting::misc::reload_indicator_pos = ImGui::GetWindowPos();
                ImDrawList* d = ImGui::GetWindowDrawList();

                ImVec2 p = ImGui::GetCursorScreenPos();
                ImVec2 bar_p = ImVec2(p.x, p.y);

                d->AddRectFilled({ bar_p.x, bar_p.y }, { bar_p.x + bar_width, bar_p.y + bar_height }, bg_col, rounding);
                if (fraction > 0.0f) d->AddRectFilled({ bar_p.x, bar_p.y }, { bar_p.x + bar_width * fraction, bar_p.y + bar_height }, accent_col, rounding);

                ImGui::End();
            }
        }
    }
    if (setting::combat::aim_enable && is_active(setting::combat::aim_key)) {
        if (setting::combat::draw_aim_target && target_info.has_target) {
            Vector2 screen_pos;

            if (target_info.predicted_pos.world_to_screen(screen_pos, view_matrix.matrix)) {
                float point_size = 2.5f;

                ImU32 dot_color = get_u32_color(setting::combat::aim_target_color, 1.0f);
                draw_list->AddCircleFilled({ screen_pos.x, screen_pos.y }, point_size, dot_color);
            }
        }
    }
}

HRESULT hooks::methods::hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!oPresent) return S_OK;
    ImGuiIO& io = ImGui::GetIO();

    if (GetAsyncKeyState(VK_HOME) & 1 || GetAsyncKeyState(VK_INSERT) & 1) {
        setting::menu::render_menu = !setting::menu::render_menu;
    }

    if (!imgui_init) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = sd.OutputWindow;

            ID3D11Texture2D* pBackBuffer;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();

            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            
            InitImGui();
            GUI::get().initialize(pDevice);

            imgui_init = true;
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    intro::draw_intro();

    if (!intro::intro_active) {

        io.MouseDrawCursor = setting::menu::render_menu;
        if (setting::menu::render_menu) {
            ClipCursor(NULL);

            POINT m_pos;
            GetCursorPos(&m_pos);
            ScreenToClient(window, &m_pos);
            io.MousePos.x = (float)m_pos.x;
            io.MousePos.y = (float)m_pos.y;

            GUI::get().draw();
        }

        ui::bind_list();
        ui::watermark();
        Visuals();
    }

    ImGui::Render();
    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// ---- WndProc.cpp ----
#line 1 "WndProc.cpp"
#include "Hooks.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT hooks::methods::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (setting::menu::render_menu) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

        if (uMsg == WM_SETCURSOR) return TRUE;

        switch (uMsg) {
        case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL: case WM_MOUSEMOVE:
        case WM_INPUT:
            return TRUE;
        }
    }

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// ---- Hooks.cpp ----
#line 1 "Hooks.cpp"
#include "Hooks.hpp"
#include "IconManager.hpp"

#define SetHook(cls_name, method_name, args_count, original_ptr, hook_func) \
    { \
        void* method_ptr = find_method("", cls_name, method_name, args_count); \
        if (method_ptr) { \
            if (hookengine::hook( \
                method_ptr, \
                reinterpret_cast<void**>(&(original_ptr)), \
                reinterpret_cast<void*>(hook_func) \
            )) { \
                char buf[128]; \
                snprintf(buf, sizeof(buf), "[+]  %s", #method_name); \
                intro::SetHookLog(hook_index++, buf, ImColor(46, 204, 113)); \
            } else { \
                char buf[128]; \
                snprintf(buf, sizeof(buf), "[!]  %s", #method_name); \
                intro::SetHookLog(hook_index++, buf, ImColor(231, 76, 60)); \
            } \
        } else { \
            char buf[128]; \
            snprintf(buf, sizeof(buf), "[-]  %s", #method_name); \
            intro::SetHookLog(hook_index++, buf, ImColor(241, 196, 15)); \
        } \
    }

bool hooks::init() {
    if (MH_Initialize() != MH_OK) return false;
    int hook_index = 0;

    if (!il2cpp_attached) {
        Dissector::Attach();
        il2cpp_attached = true;
    }

    auto find_method = [](const char* ns, const char* cls, const char* method, int args = -1) -> void* {
        auto* klass = Dissector::FindClass(ns, cls);
        if (!klass) klass = Dissector::FindClass("", cls);
        if (!klass) return nullptr;

        auto* m = Dissector::FindMethod(klass, method, args);
        if (!m) return nullptr;

        return reinterpret_cast<void*>(m->methodPtr);
    };

    SetHook("PlayerWalkMovement", "UpdateVelocity", 0, SDK::PlayerWalkMovement::UpdateVelocity_, hooks::methods::UpdateVelocity_hk);
    SetHook("Projectile", "Update", 0, SDK::Projectile::Update_, hooks::methods::ProjectileUpdate_hk);
    SetHook("BasePlayer", "ClientInput", 1, SDK::BasePlayer::ClientInput_, hooks::methods::ClientInput_hk);
    SetHook("TOD_Sky", "UpdateAmbient", 0, SDK::TOD_Sky::UpdateAmbient_, hooks::methods::UpdateAmbient_hk);
    SetHook("AimConeUtil", "GetModifiedAimConeDirection", 3, SDK::BaseProjectile::GetModifiedAimConeDirection_, hooks::methods::GetModifiedAimConeDirection_hk);
    SetHook("HeldEntity", "AddPunch", 2, SDK::HeldEntity::AddPunch_, hooks::methods::AddPunch_hk);
    SetHook("FlintStrikeWeapon", "DoAttack", 0, SDK::FlintStrikeWeapon::DoAttack_, hooks::methods::DoAttack_hk);
    SetHook("ItemIcon", "TryToMove", 0, SDK::ItemIcon::TryToMove_, hooks::methods::TryToMove_hk);
    SetHook("Projectile", "DoHit", 3, SDK::Projectile::DoHit_, hooks::methods::DoHit_hk);
    SetHook("BasePlayer", "SendClientTick", 0, SDK::BasePlayer::SendClientTick_, hooks::methods::SendClientTick_hk);
    SetHook("BaseCombatEntity", "DoHitNotify", 1, SDK::BaseCombatEntity::DoHitNotify_, hooks::methods::DoHitNotify_hk);
    SetHook("Projectile", "DoMovement", 1, SDK::Projectile::DoMovement_, hooks::methods::DoMovement_hk);

    SetHook("PlayerEyes", "get_BodyLeanOffset", 0, SDK::PlayerEyes::BodyLeanOffset_, hooks::methods::BodyLeanOffset_hk);
    SetHook("PlayerEyes", "DoFirstPersonCamera", 1, SDK::PlayerEyes::DoFirstPersonCamera_, hooks::methods::DoFirstPersonCamera_hk);
    SetHook("BaseMountable", "EyePositionForPlayer", 2, SDK::BaseMountable::EyePositionForPlayer_, hooks::methods::EyePositionForPlayer_hk);
#undef SetHook

    return true;
}

void hooks::shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    kiero::shutdown();
}
void hooks::InitImGui() {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 14.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->Build();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);

    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    IconManager::Get().Initialize(pDevice);
}


// ---- Getter.cpp ----
#line 1 "Getter.cpp"
#include "Getter.hpp"

matrix4x4 Get::view_matrix() {
    auto camera = SDK::UnityEngine::Camera::get_main();
    if (!camera) return {};

    auto camera_addr = *(uintptr_t*)((uintptr_t)camera + 0x10);
    if (!camera_addr) return {};

    auto view_matrix = *(matrix4x4*)(camera_addr + 0x2E4);
    return view_matrix;
}
Vector3 Get::player_velocity(SDK::BasePlayer* player) {
    if (!player) return { 0, 0, 0 };

    if (player == entity_data::local_player) {
        if (!player->movement()) return { 0, 0, 0 };
        auto walk_movement = (SDK::PlayerWalkMovement*)player->movement();
        if (!walk_movement) return { 0, 0, 0 };

        auto velocity = walk_movement->TargetMovement();
        if (velocity.Length() < 0.1f) {
            velocity = walk_movement->velocity();
        }

        return velocity;
    }

    auto base_trans = player->GetTransform();
    if (!base_trans) return { 0, 0, 0 };

    auto current_pos = base_trans->GetPosition();
    if (current_pos.Empty()) return { 0, 0, 0 };

    auto player_ptr = reinterpret_cast<uintptr_t>(player);
    auto& track = g_velocity_map[player_ptr];

    float current_time = SDK::UnityEngine::Time::GetTime();
    float delta_time = current_time - track.last_update_time;

    if (track.last_position.Length() > 0.1f) {
        if (delta_time > 0.0001f && delta_time < 0.5f) {
            Vector3 raw_velocity = (current_pos - track.last_position) / delta_time;

            if (raw_velocity.Length() < 60.0f) {
                track.calculated_velocity = track.calculated_velocity * 0.3f + raw_velocity * 0.7f;
            }
        }
    }

    track.last_position = current_pos;
    track.last_update_time = current_time;

    return track.calculated_velocity;
}

SDK::UnityEngine::Transform* Get::bone_transform(SDK::BaseEntity* player_pawn, int index) {
    if (!player_pawn) return nullptr;

    auto model = player_pawn->model();
    if (!model) return nullptr;

    auto bone_array = model->boneTransforms();
    if (!bone_array) return nullptr;

    int bone_count = bone_array->ArraySize();
    if (index < 0 || index >= bone_count) return nullptr;

    return bone_array->GetArray(index);
}
Vector3 Get::transform_pos(SDK::UnityEngine::Transform* transform) {
    if (!transform) return Vector3{ 0, 0, 0 };
    return transform->GetPosition();
}

bool Get::team(SDK::BasePlayer* player_pawn, SDK::BasePlayer* local_player) {
    if (!player_pawn || !local_player) return false;

    auto playerTeam = player_pawn->currentTeam();
    auto localTeam = local_player->currentTeam();

    return (localTeam != 0 && localTeam == playerTeam);
}
bool Get::is_sleeper(SDK::BasePlayer* player_pawn) {
    int flags = static_cast<int>(player_pawn->playerFlags());

    return (flags & static_cast<int>(SDK::BasePlayer::PlayerFlags::Sleeping)) != 0;
}

SDK::Item* Get::active_item(SDK::BasePlayer* player) {
    auto inventory = entity_data::local_player->inventory();
    if (!inventory) return nullptr;

    auto belt = inventory->containerBelt();
    if (!belt) return nullptr;

    auto items = belt->itemList();
    if (!items) return nullptr;

    int count = items->GetSize();
    if (count < 1) return nullptr;

    for (int i = 0; i < count; i++) {
        auto item = items->GetArray(i);
        if (!item) continue;

        if (item->uid() == entity_data::local_player->clActiveItem()) {
            return item;
        }
    }

    return nullptr;
}
std::string Get::item_shortname(SDK::Item* item) {
    if (!item) return "";

    auto info = item->info();
    if (!info) return "";

    auto sn = info->shortname();
    if (!sn || sn->len <= 0 || !sn->buffer) return "";

    char name_buf[128] = {};
    int len = (sn->len > 60) ? 60 : sn->len;

    WideCharToMultiByte(CP_UTF8, 0, sn->buffer, len, name_buf, sizeof(name_buf) - 1, nullptr, nullptr);
    return std::string(name_buf);
}

std::vector<SDK::Item*> Get::belt_items(SDK::BasePlayer* player) {
    std::vector<SDK::Item*> result(6, nullptr);
    if (!player) return result;

    auto inventory = player->inventory();
    if (!inventory) return result;

    auto belt = inventory->containerBelt();
    if (!belt) return result;

    auto items = belt->itemList();
    if (!items) return result;

    int count = items->GetSize();
    for (int i = 0; i < count && i < 6; i++) {
        result[i] = items->GetArray(i);
    }
    return result;
}
std::vector<SDK::Item*> Get::wear_items(SDK::BasePlayer* player) {
    std::vector<SDK::Item*> result(7, nullptr);
    if (!player) return result;

    auto inventory = player->inventory();
    if (!inventory) return result;

    auto wear = inventory->containerWear();
    if (!wear) return result;

    auto items = wear->itemList();
    if (!items) return result;

    int count = items->GetSize();
    for (int i = 0; i < count && i < 7; i++) {
        result[i] = items->GetArray(i);
    }
    return result;
}

bool Get::is_npc(SDK::BasePlayer* player) {
    if (!player) return false;

    if (player->IsFrom("NPCPlayer") ||
        player->IsFrom("ScientistNPC") ||
        player->IsFrom("BaseNPC") ||
        player->IsFrom("HumanNPC") ||
        player->IsFrom("TunnelDweller") ||
        player->IsFrom("UnderwaterDweller") ||
        player->IsFrom("ScarecrowNPC") ||
        player->IsFrom("BanditGuard") ||
        player->IsFrom("GingerbreadNPC") ||
        player->IsFrom("FrankensteinPet") ||
        player->IsFrom("HTNPlayer")) {
        return true;
    }

    return false;
}
void Get::entity_loop() {
    std::vector<RustPlayer> player_list;

    auto client_entities = SDK::BaseNetworkable::clientEntities();
    if (!client_entities || IsBadReadPtr(client_entities, 8)) return;

    auto entity_realm = client_entities->entityList();
    if (!entity_realm || IsBadReadPtr(entity_realm, 8)) return;

    auto buffer_list = entity_realm->GetValues();
    if (!buffer_list || IsBadReadPtr(buffer_list, 8)) return;

    int size = buffer_list->GetSize();
    Vector3 eyes_pos = transform_pos(bone_transform(entity_data::local_player, BoneList::head));

    for (int i = 0; i < size; i++) {
        RustPlayer rustPlayer;

        void* element = buffer_list->GetArray(i);
        if (!element) continue;

        auto* pawn = (SDK::BasePlayer*)(element);
        if (pawn == entity_data::local_player) continue;

        auto lifeState = pawn->lifeState();
        if (lifeState == LifeState::Dead) continue;

        if (pawn->IsFrom("BasePlayer") || is_npc(pawn)) {
            auto bone = bone_transform(pawn, BoneList::head);
            auto world_pos = transform_pos(bone);

            if (!bone) {
                rustPlayer.is_visible = false;
                continue;
            }
            if (world_pos.Empty()) {
                rustPlayer.is_visible = false;
                continue;
            }

            rustPlayer.pawn = pawn;
            rustPlayer.is_npc = is_npc(pawn);
            rustPlayer.is_visible = SDK::UnityEngine::LineOfSight(eyes_pos, world_pos);

            player_list.push_back(rustPlayer);
        }
    }

    entity_data::players_list = player_list;

    if (setting::visuals::resource_esp) {
        std::vector<RustResource> resource_list;

        for (int i = 0; i < size; i++) {
            void* element = buffer_list->GetArray(i);
            if (!element) continue;

            SDK::BaseEntity* ent = reinterpret_cast<SDK::BaseEntity*>(element);

            auto transform = ent->GetTransform();
            if (!transform) continue;

            Vector3 pos = transform->GetPosition();
            if (pos.Empty()) continue;

            float dist = (eyes_pos - pos).Length();
            if (dist > setting::visuals::ore_distance) continue;

            auto prefabName = ent->GetShortPrefabName();
            if (!prefabName || prefabName->len <= 0 || !prefabName->buffer) continue;

            char name_buf[128] = {};
            int len = (prefabName->len > 60) ? 60 : prefabName->len;
            WideCharToMultiByte(CP_UTF8, 0, prefabName->buffer, len, name_buf, sizeof(name_buf) - 1, nullptr, nullptr);
            std::string pname(name_buf);

            bool has_hemp = pname.find("hemp") != std::string::npos;
            bool has_metal = pname.find("metal") != std::string::npos;
            bool has_sulfur = pname.find("sulfur") != std::string::npos;
            bool has_ore = pname.find("ore") != std::string::npos;
            bool has_stone = pname.find("stone") != std::string::npos;

            bool is_hemp_ent = has_hemp && (ent->IsFrom("CollectibleEntity") || ent->IsFrom("OreResource"));
            bool is_ore_ent = (has_metal || has_sulfur || has_ore || has_stone) && !has_hemp;

            if (!is_hemp_ent && !is_ore_ent) continue;

            RustResource resource;
            resource.pawn = reinterpret_cast<uintptr_t>(ent);
            resource.position = pos;
            resource.name = pname;
            resource.is_hemp = false;

            if (has_hemp) {
                resource.is_hemp = true;
            }
            else if (has_metal) {
                resource.type = OreType::metal;
            }
            else if (has_sulfur) {
                resource.type = OreType::sulfur;
            }
            else {
                resource.type = OreType::stone;
            }

            bool duplicate = false;
            for (size_t j = 0; j < resource_list.size(); j++) {
                auto& existing = resource_list[j];
                float dx = pos.x - existing.position.x;
                float dy = pos.y - existing.position.y;
                float dz = pos.z - existing.position.z;

                float d = sqrtf(dx * dx + dy * dy + dz * dz);
                if (d < 1.0f) {
                    if (!resource.is_hemp && !existing.is_hemp) {
                        if (existing.type != OreType::stone && resource.type == OreType::stone) {
                            duplicate = true;
                            break;
                        }
                        if (resource.type != OreType::stone && existing.type == OreType::stone) {
                            existing = resource;
                            duplicate = true;
                            break;
                        }
                    }
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) {
                resource_list.push_back(resource);
            }
        }

        entity_data::resource_list = resource_list;
    }
}

// ---- server_side.cpp ----
#line 1 "server_side.cpp"
#include "server_side.hpp"
#include "Hooks.hpp"
#include <algorithm>

bool check_hit(Vector3 HitPositionWorld, SDK::Projectile* projectile) {
    if (!projectile || !target_info.has_target || !target_info.target_player) return false;

    if (projectile->integrity() <= 0.f) {
        return false;
    }

    SDK::BasePlayer* target_ply = target_info.target_player;
    if (!target_ply) return false;

    float timeSinceLastTick = (SDK::UnityEngine::Time::GetRealtimeSinceStartup() - entity_data::local_player->lastSentTickTime());
    float last_tick_time = (std::max)(0.f, (std::min)(timeSinceLastTick, 1.f));

    Vector3 parentVelocity = target_ply->GetParentVelocity();
    float magnitude = parentVelocity.Magnitude();

    float num2 = 1.f + AntiHackProtection::projectile_forgiveness;
    float num9 = AntiHackProtection::projectile_clientframes / 60.f;
    float num10 = AntiHackProtection::projectile_serverframes * (std::max)(SDK::UnityEngine::Time::GetDeltaTime(), SDK::UnityEngine::Time::GetSmoothDeltaTime());
    float num12 = (last_tick_time + num9 + num10) * num2;

    float num14 = projectile->currentPosition().Distance(HitPositionWorld) + 0.01f;
    float num19 = target_ply->BoundsPadding() + num12 * magnitude + AntiHackProtection::tickhistoryforgiveness;
    float num20 = target_ply->Distance(HitPositionWorld);

    bool flag2 = (target_ply->playerFlags() & SDK::BasePlayer::PlayerFlags::Sleeping) != 0;
    bool flag3 = (target_ply->playerFlags() & SDK::BasePlayer::PlayerFlags::Wounded) != 0;
    bool flag4 = (target_ply->mounted() != nullptr);

    float maxVel = target_ply->MaxVelocity();
    float num16 = maxVel + parentVelocity.Magnitude();
    float num17 = target_ply->BoundsPadding() + num12 * num16;
    float num18 = target_ply->Distance(HitPositionWorld);

    float num21 = maxVel + parentVelocity.Magnitude();
    float num22 = num12 * num21;
    float num24 = projectile->traveledDistance() + 1.f + projectile->currentPosition().Magnitude() + num22;

    if (num20 > num19) {
        return false;
    }

    if (num14 > num24) {
        return false;
    }

    return true;
}

bool check_eye(Vector3 eyePos) {
    float timeSinceLastTick = (SDK::UnityEngine::Time::GetRealtimeSinceStartup() - entity_data::local_player->lastSentTickTime());
    float last_tick_time = (std::max)(0.f, (std::min)(timeSinceLastTick, 1.f));
    bool hit = true;
    float num = 1.f + AntiHackProtection::eye_forgiveness;
    float eye_clientframes = AntiHackProtection::eye_clientframes;
    float eye_serverframes = AntiHackProtection::eye_serverframes;
    float num2 = eye_clientframes / 60.f;

    float num3 = eye_serverframes * (std::max)({
        SDK::UnityEngine::Time::GetDeltaTime(),
        SDK::UnityEngine::Time::GetSmoothDeltaTime(),
        SDK::UnityEngine::Time::GetFixedDeltaTime()
        });

    float num4 = (last_tick_time + num2 + num3) * num;
    float num5 = entity_data::local_player->MaxVelocity() + entity_data::local_player->GetParentVelocity().Magnitude();
    float num6 = entity_data::local_player->BoundsPadding() + num4 * num5;
    float num7 = entity_data::local_player->eyes()->position().Distance(eyePos) + 0.01f;
    if (num7 > num6)
    {
        hit = false;
    }
    float num8 = abs(entity_data::local_player->GetMountVelocity().y + entity_data::local_player->GetParentVelocity().y);
    float num9 = entity_data::local_player->BoundsPadding() + num4 * num8 + entity_data::local_player->GetJumpHeight();
    float num10 = abs(entity_data::local_player->eyes()->position().y - eyePos.y) + 0.01f;
    if (num10 > num9)
    {
        hit = false;
    }
    return hit;
}
