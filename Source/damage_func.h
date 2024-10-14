#pragma once
#include <functional>
#include "primitive.h"
#include <cereal/cereal.hpp>
#include "camera.h"
#include "game_pad.h"
struct AttackParam
{
	bool isAttack;//攻撃中かどうか
	int power;//攻撃力
	float invinsibleTime;//攻撃対象に課す無敵時間
	Camera::CameraShakeParam cameraShake;//カメラシェイク
	Camera::HitStopParam hitStop;//ヒットストップ
	GamePad::Viberation hitViberation;
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::make_nvp("power", power),
			cereal::make_nvp("invinsible_time", invinsibleTime),
			cereal::make_nvp("camera_shake", cameraShake),
			cereal::make_nvp("hit_stop", hitStop),
			cereal::make_nvp("hit_viberation", hitViberation)
		);
	}
};

//ダメージを受けた時のひるみ方
enum class WINCE_TYPE
{
	NONE,//怯みなし
	SMALL,//小さくひるむ
	BIG,//大きく吹き飛ばされる
};
typedef std::function<bool(int, float, WINCE_TYPE)> AddDamageFunc;