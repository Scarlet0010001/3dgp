#pragma once
#include "graphics.h"
#include "camera.h"
#include "collision.h"
class Camera;

class Stage
{
public:
	Stage() {}
	virtual ~Stage() {}

	virtual void  Update(float elapsedTime) = 0;

	//virtual void  render(float elapsed_time, Camera* camera) = 0;
	virtual void  Render(float elapsed_time) = 0;
	//virtual void  shadow_render(float elapsed_time) = 0;

	virtual bool RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit) = 0;

	bool displayImgui = false;
};