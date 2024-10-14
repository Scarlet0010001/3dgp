#include "light_manager.h"
#include "shader.h"
#include "user.h"

void LightManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	HRESULT hr{};
	//hr = create_ps_from_cso(graphics.GetDevice().Get(), "shaders/deferred_light.cso", deferred_light.GetAddressOf());
	//hr = create_ps_from_cso(graphics.GetDevice().Get(), "shaders/deferred_light_shadow.cso", shadow_map_light.GetAddressOf());
	lightScreen = std::make_unique<fullscreen_quad>(graphics.GetDevice().Get());
	lights.clear();
#if CAST_SHADOW
	DirectX::XMFLOAT3 shadow_light_dir = { 1.0f, -1.0f, -1.0 };
	DirectX::XMFLOAT3 shadow_color = { 0.2f, 0.2f, 0.2f };
	shadowDirLight = std::make_shared<DirLight>(shadow_light_dir, shadow_color);
	LightManager::Instance().Register("shadow_dir_light", shadowDirLight);
#endif
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

}

LightManager::~LightManager()
{
	lights.clear();
}

void LightManager::Register(std::string name, std::shared_ptr<Light> light)
{
	//ID
	int id = 0;
	//���̖��O
	std::string  uniqueName = name;
	for (auto& l : lights)
	{
		//�������O�����Ԃ��Ă�����
		if (name == l.first)
		{
			//���̖��O�̌��ɐ�����t������
			id++;
			name = uniqueName + std::to_string(id);
		}
	}
	//�}�b�v�ɓo�^
	lights[name] = light;
	//���C�g�ɕύX�������O��o�^
	light->name = name;

}

void LightManager::Draw(ID3D11ShaderResourceView** rtv, int rtv_num)
{
	Graphics& graphics = Graphics::Instance();
	//�e�p���C�g�`��
#if CAST_SHADOW
	//shadow_dir_light->light_constants->bind(graphics.get_dc().Get(), 7);
	//light_screen->blit(graphics.get_dc().Get(), rtv, 0, rtv_num, shadow_map_light.Get());
#endif
	//�ʏ탉�C�g�`��
	for (auto& light : lights)
	{
		//�����N���؂�Ă��Ȃ����`�F�b�N
		if (!light.second.expired())
		{
			//�Ď����Ă��郉�C�g�̊֐�����
			light.second.lock()->lightConstants->Bind(graphics.Get_DC().Get(), 7);
			lightScreen->blit(graphics.Get_DC().Get(), rtv, 0, rtv_num, deferredLight.Get());
		}
		//�v�f���Ȃ��̂�map�̗̈���Ƃ��Ă���Ƃ��Ɍx�����o��
		_ASSERT_EXPR(!light.second.expired(), L"light_map��nullptr�����݂��Ă��܂�\n delete_light()���ĂіY��Ă���\��������܂�");
	}

}

void LightManager::DebugGUI()
{
	imguiMenuBar("Lights", "Light", displayImgui);
	if (displayImgui)
	{
#if CAST_SHADOW
		shadowDirLight->DebugGUI("shadow");
#endif
		for (auto& l : lights)
		{
			if (!l.second.expired())
			{
				l.second.lock()->DebugGUI(l.first);
			}
		}
	}

}

void LightManager::DeleteLight(std::string name)
{
	//�w��̃L�[�̗v�f���}�b�v����폜
	lights.erase(name);

}
