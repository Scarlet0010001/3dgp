#include "resource_manager.h"

std::shared_ptr<ModelResource> ResourceManager::Load(ID3D11Device* device, const std::string* filename, bool triangulate, float sampling_rate)
{
	// �}�b�v�̒����烂�f��������
	ModelMap::iterator iter = models.find(filename->c_str());
	if (iter != models.end())
	{
		//�����N���؂�Ă��Ȃ����m�F
		if (!iter->second.expired())  //second��map��[�l]�̕����̂���  *�L�[�Ȃ�first    expired :�����N���؂�Ă��Ȃ����̊m�F
		{
			//���f�����\�[�X��Ԃ�
			return iter->second.lock(); //lock: �Ď����Ă���shared_ptr�I�u�W�F�N�g���擾����B // https://cpprefjp.github.io/reference/memory/weak_ptr/lock.html
		}
	}
	// �����������Ă�������Ȃ����
	// �V�K���f�����\�[�X�ǂݍ���
	auto model = std::make_shared<ModelResource>(device, filename, triangulate, sampling_rate);
	// �}�b�v�ɓo�^
	models[filename->c_str()] = model;
	return model;
}
