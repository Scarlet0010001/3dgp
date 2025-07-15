#pragma once
#include "Graphics/graphics.h"
#include "Camera/camera.h"
#include "Collision/collision.h"
class Camera;

class Stage
{
public:
	Stage() {}
	virtual ~Stage() {}

	virtual void  Update(float elapsedTime) = 0;

	//virtual void  render(float elapsedTime, Camera* camera) = 0;
	virtual void  Render(float elapsedTime) = 0;
	virtual void  DebugDUI() = 0;
	//virtual void  shadow_render(float elapsedTime) = 0;

	virtual bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit) = 0;

	bool displayImgui = false;
};