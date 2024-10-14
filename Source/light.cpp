#include "light.h"
#include "user.h"

//--------------------------�R���X�g���N�^----------------------//
Light::Light()
{
	Graphics& graphics = Graphics::Instance();
	lightConstants = std::make_unique<Constants<DirLightParam>>(graphics.GetDevice().Get());
}

//���s��
DirLight::DirLight(DirectX::XMFLOAT3 dir, DirectX::XMFLOAT3 color)
{
	//���s���́o����x�A����y�A����z�A0.0�p
	lightConstants->data.dirLightDirection = { dir.x, dir.y, dir.z, 0.0f };
	lightConstants->data.dirLightColor = { color.x, color.y, color.z, 1.0f };
}

PointLight::PointLight(DirectX::XMFLOAT3 position, float distance, DirectX::XMFLOAT3 color)
{
	//�_�����́o�ʒux�A�ʒuy�A�ʒuz�A���������p
	lightConstants->data.dirLightDirection = { position.x, position.y, position.z, distance };
	lightConstants->data.dirLightColor = { color.x, color.y, color.z, 1.0f };
}


//---------------------------DebugImGUI--------------------------//
void DirLight::DebugGUI(std::string name)
{
#if USE_IMGUI
	ImGui::Begin("light");
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::DragFloat3(std::string("Direction " + name).c_str(),
			&lightConstants->data.dirLightDirection.x, 0.1f);
		ImGui::DragFloat3(std::string("color " + name).c_str(),
			&lightConstants->data.dirLightColor.x, 0.1f, 0.0f);
	}
	ImGui::End();
#endif // 0

}

void PointLight::DebugGUI(std::string name)
{
#if USE_IMGUI
	ImGui::Begin("light");
	if (ImGui::CollapsingHeader(name.c_str()))
	{
		ImGui::DragFloat3(std::string("position " + name).c_str(),
			&lightConstants->data.dirLightDirection.x, 0.5f);
		ImGui::DragFloat3(std::string("color " + name).c_str(),
			&lightConstants->data.dirLightColor.x, 0.1f, 0);
		ImGui::DragFloat(std::string("distance " + name).c_str(),
			&lightConstants->data.dirLightDirection.w, 0.1f, 1, 500);
	}
	ImGui::End();
#endif // 0

}


//---------------------�Q�b�^�[�ƃZ�b�^�[-------------------------//
void DirLight::SetDirection(DirectX::XMFLOAT3 direction)
{
	lightConstants->data.dirLightDirection = { direction.x, direction.y, direction.z, 0.0f };
}

void DirLight::SetColor(DirectX::XMFLOAT3 color)
{
	lightConstants->data.dirLightColor = { color.x, color.y, color.z, 1.0f };
}

DirectX::XMFLOAT3 DirLight::GetDirection()
{
	return DirectX::XMFLOAT3(
		lightConstants->data.dirLightDirection.x,
		lightConstants->data.dirLightDirection.y, 
		lightConstants->data.dirLightDirection.z);
}

DirectX::XMFLOAT3 DirLight::GetColor()
{
	return DirectX::XMFLOAT3(
		lightConstants->data.dirLightColor.x,
		lightConstants->data.dirLightColor.y,
		lightConstants->data.dirLightColor.z);
}

void PointLight::SetPosition(DirectX::XMFLOAT3 position)
{
	lightConstants->data.dirLightDirection.x = position.x;
	lightConstants->data.dirLightDirection.y = position.y;
	lightConstants->data.dirLightDirection.z = position.z;
}

void PointLight::SetColor(DirectX::XMFLOAT3 color)
{
	lightConstants->data.dirLightColor = { color.x, color.y, color.z, 1.0f };
}

void PointLight::SetDistance(float d)
{
	lightConstants->data.dirLightDirection.w = d;
}

DirectX::XMFLOAT3 PointLight::GetPosition()
{
	return DirectX::XMFLOAT3(
		lightConstants->data.dirLightDirection.x,
		lightConstants->data.dirLightDirection.y,
		lightConstants->data.dirLightDirection.z);
}

DirectX::XMFLOAT3 PointLight::GetColor()
{
	return DirectX::XMFLOAT3(
		lightConstants->data.dirLightColor.x,
		lightConstants->data.dirLightColor.y,
		lightConstants->data.dirLightColor.z);
}

float PointLight::GetDistance()
{
	return lightConstants->data.dirLightDirection.w;
}
