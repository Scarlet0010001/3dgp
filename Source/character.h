#pragma once
#include <DirectXMath.h>
#include "damage_func.h"

class Character
{
public:
	//----------------------------------------------------------------
	// �\���́A�񋓌^
	//----------------------------------------------------------------
	struct CharacterParam
	{
		//�L�����N�^�[�̔��a
		float radius = 1.0f;
		//�L�����̏c�̑傫��
		float height = 2.6f;
		//�ő�̗�
		int maxHealth = 1000;
		//���C��
		float friction = 1.0f;
		//��C��R
		float airControl = 0.3f;
		//�����x
		float acceleration = 1.5f;
		//�ő呬�x
		float maxMoveSpeed = 30.0f;
		//��]���x
		float turnSpeed = DirectX::XMConvertToRadians(720);
		//�ړ����x
		float moveSpeed = 5.0f;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("radius", radius),
				cereal::make_nvp("height", height),
				cereal::make_nvp("max_health", maxHealth),
				cereal::make_nvp("friction", friction),
				cereal::make_nvp("air_control", airControl),
				cereal::make_nvp("acceleration", acceleration),
				cereal::make_nvp("max_move_speed", maxMoveSpeed),
				cereal::make_nvp("turn_speed", turnSpeed),
				cereal::make_nvp("move_speed", moveSpeed)
			);
		}

	};

	Character() {}
	virtual ~Character() {}

	//---------------------------------------------------------------
	//�Z�b�^�[�ƃQ�b�^�[
	//---------------------------------------------------------------

	//�ʒu�擾
	const DirectX::XMFLOAT3& GetPosition() const { return position; }
	//�ʒu�ݒ�
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }
	// ��]�擾
	const DirectX::XMFLOAT3& GetAngle() const { return angle; }
	//��]�ݒ�
	void SetAngle(const DirectX::XMFLOAT3& angle) { this->angle = angle; }
	// �X�P�[���擾
	const DirectX::XMFLOAT3& GetScale() const { return scale; }
	//�X�P�[���ݒ�
	void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }
	//velocity�擾
	const DirectX::XMFLOAT3& GetVelocity() const { return velocity; }
	//velocity�Z�b�g
	void SetVelocity(const DirectX::XMFLOAT3& v) { this->velocity = v; }
	// ���a
	float GetRadius() const { return charaParam.radius; }
	// HP
	int GetHealth() const { return health; }
	// �ő�HP
	int GetMaxHealth() const { return charaParam.maxHealth; }
	//�̂̐��ʂƐi�s���ʂƂ̊p�x
	float GetTurnAngle() const { return turnAngle; }
	//HP�p�[�Z���e�[�W
	float GetHpPercent() const { return health <= 0 ? 0.0f : static_cast<float>(health) / static_cast<float>(charaParam.maxHealth); }
	// �n�ʔ���
	bool GetIsGround() const { return isGround; }
	//�����擾
	float GetHeight() const { return charaParam.height; }
	//�g�����X�t�H�[���̃Q�b�^�[
	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }
	//�Ռ���^����
	void AddImpulse(const DirectX::XMFLOAT3& impulse);
	//�_���[�W��^����
	virtual bool ApplyDamage(int damage, float invincibleTime, WINCE_TYPE type);

protected:
	void Move(float vx, float vz, float speed);
	void Turn(float elapsed_time, float vx, float vz, float speed);//�I�C���[
	void Turn(float elapsed_time, DirectX::XMFLOAT3 move_vec, float speed, DirectX::XMFLOAT4& orien);//�N�H�[�^�j�I��
	//�W�����v����
	void Jump(float speed);
	//���͏����X�V
	void UpdateVelocity(float elapsed_time, DirectX::XMFLOAT3& position);
	virtual void OnLanding() {}
	//���S�����Ƃ��ɌĂ΂��
	virtual void OnDead() {}
	virtual void OnDamaged(WINCE_TYPE type) {}
	void UpdateInvicibleTimer(float elapsed_time);

	//-----------�ϐ�--------------//

	DirectX::XMFLOAT3	position = { 0, 0, 0 };
	DirectX::XMFLOAT3	angle = { 0, 0, 0 };
	DirectX::XMFLOAT3	scale = { 1, 1, 1 };
	DirectX::XMFLOAT4 orientation{ 0,0,0,1 };
	DirectX::XMFLOAT4X4	transform = {
	1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
	};

	CharacterParam charaParam;

	float stepOffset = 0.7f;
	DirectX::XMFLOAT3 velocity = { 0, 0, 0 };
	//�n�ʂɓ������Ă��邩
	bool isGround = false;

	float invincibleTimer = 0.0f;
	float moveVec_x = 0.0f;
	float moveVec_z = 0.0f;

	//�̗�
	int32_t health;


	float vs_wall_ray_power = 5.0f;
	//��̖@��
	DirectX::XMFLOAT3 slopeNormal = {};
	float gravity = -1.0f;
	float turnAngle;			//�̂�������Ƃ��̉�]�p
	//-----------�v���C�x�[�g�֐�--------------//
private:
	//�������͍X�V����
	void UpdateVerticalVelocity(float elapsed_frame);
	//�����ړ��X�V����
	void UpdateVerticalMove(float elapsed_time, DirectX::XMFLOAT3& position);
	//�������͍X�V����
	void UpdateHorizontalVelocity(float elapsed_frame);
	//�����ړ��X�V����
	void UpdateHorizontalMove(float elapsed_time, DirectX::XMFLOAT3& position);

};

