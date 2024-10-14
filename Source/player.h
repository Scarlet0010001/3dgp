#pragma once
#include "device.h"
#include "camera.h"
#include "character.h"
#include "gltf_model.h"

#include "primitive.h"
#include <cereal/cereal.hpp>

//�v���C���[ :final ���̃N���X�̌p�����ł��Ȃ����Ƃ𖾎�����
class Player final :
    public Character
{
public:
    Player();
    ~Player()override;

	//����������
	void Initialize();
	//�X�V����
	void Update(float elapsed_time);
	//�`�揈��
//�f�B�t�@�[�h�Ń����_�����O����I�u�W�F�N�g
	void Render_d(float elapsed_time);
	//�t�H���[�h�����_�����O����I�u�W�F�N�g
	void Render_f(float elapsed_time);
	//�V���h�E�����_�����O����I�u�W�F�N�g
	void Render_s(float elapsed_time);
	//UI�`��
	void RenderUI(float elapsed_time);
	//�f�o�b�O�pGUI�`��
	void DebugGUI();

	//�v���C���[�̍�������̈ʒu
	DirectX::XMFLOAT3 GetWaistPosition() { return DirectX::XMFLOAT3(position.x, position.y + charaParam.height / 2, position.z); }
	//�J�������v���C���[������Ƃ��ɒ�������|�C���g
	DirectX::XMFLOAT3 GetGazingPoint() { return DirectX::XMFLOAT3(position.x, position.y + (charaParam.height + 3), position.z); }

	//�v���C���[�̃R���W�����ƓG�̓����蔻��
	void CalcCollision_vs_Enemy(Capsule capsule_collider, float colider_height);

	//�v���C���[�̍U���ƓG�̓����蔻��
	void CalcAttack_vs_Enemy(Capsule collider, AddDamageFunc damaged_func);

	//�X�L���ƓG�̓����蔻��
	void JudgeSkillCollision(Capsule object_colider, AddDamageFunc damaged_func);

private:
	//-------------�\���́A�񋓌^--------------//
	//�A�j���[�V����
	enum  PlayerAnimation
	{
		PLAYER_IDLE,//�ҋ@
		PLAYER_RUN,//����
		PLAYER_ROLL,//���
		PLAYER_JUMP,//�W�����v
		PLAYER_DAMAGE_FRONT,//�O�����_��
		PLAYER_ATK_SPRING_SLASH,//�O��]�؂�
		PLAYER_PULL_SLASH,//�G�������t���Ďa��
		PLAYER_ATK_GROUND,//�n�ʂɎ��t���Č��񂹂݂�����
		PLAYER_MAGIC_BUFF,//�o�t
		PLAYER_MAGIC_SLASH_UP,//�󒆂Ɋ����グ�a��
		PLAYER_MAGIC_BULLET,//���������@�e�ł悤��
		PLAYER_ATK_FORWARD_SLASH,//�O�i�a��
		PLAYER_ATK_AIR,//�W�����v���Ēn�ʂɖ��@����
		PLAYER_ATK_COMBO1,//�R���{2-1
		PLAYER_ATK_COMBO2,//�R���{2-2
		PLAYER_ATK_COMBO3,//�R���{2-3
		PLAYER_ATK_DODGE_BACK,//����ɉ�����Ȃ��疂�@
	};
	PlayerAnimation playerAnimation = PLAYER_IDLE;


	//�X�e�[�g
	enum class State
	{
		IDLE,
		MOVE,
		ROLL,
		JUMP,
		SHOT,
		LANDING,
		FRONT_DAMAGE,
		NORMAL_ATTACK,
		SKILL,

	};

	const float JUST_GURD_TIME = 3.0f;

	struct PlayerParam
	{
		//���N���X�̃p�����[�^�[
		CharacterParam charaInitParam;
		//�W�����v�X�s�[�h
		float jumpSpeed = 21;
		//��𑬓x
		float avoidanceSpeed = 50;
		//debug�p�^�C�}�[
		int avoidanceTimer = 0;
		//���V�x
		float floatingValue = 10.0f;
		//���G�t�F�N�g�̑��x
		float swordSwingSpeed = 1500.0f;
		//�R���{1�U���̃p�����[�^�[
		AttackParam combo_1;
		//�R���{2�̃p�����[�^�[
		AttackParam combo_2;
		//�R���{3�̃p�����[�^�[
		AttackParam combo_3;


		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				cereal::make_nvp("charaParam", charaInitParam),
				cereal::make_nvp("jumpSpeed", jumpSpeed),
				cereal::make_nvp("avoidanceSpeed", avoidanceSpeed),
				cereal::make_nvp("floatingValue", floatingValue),
				cereal::make_nvp("swordSwingSpeed", swordSwingSpeed),
				cereal::make_nvp("attack_combo_1", combo_1),
				cereal::make_nvp("attack_combo_2", combo_2),
				cereal::make_nvp("attack_combo_3", combo_3)
			);
		}
	};

	struct ShieldParam
	{
		bool isShield;//�K�[�h��Ԃ�
		bool shieldable;//�K�[�h�\��Ԃ�
		bool isBreakShield;//�V�[���h���j���Ԃ�
		float shieldTime;
		float justGuardTime;
		float recastShieldTime;
		float shieldHp;//�ϋv
		float recastRate;
		float SHIELD_HP_MAX = 50;

	};

private:

	//------------�J��--------------//
	void TransitionIdleState();//�ҋ@
	void TransitionMoveState();//����
	void TransitionAvoidanceState();//���
	void TransitionJumpState();//�W�����v
	void TransitionShotState();//�ˌ�


	//--------�e�X�e�[�g�̃A�b�v�f�[�g--------//r_�̓��[�g���[�V�����t��
	void UpdateIdleState(float elapsedTime);//�ҋ@
	void UpdateMoveState(float elapsedTime);//����
	void UpdateAvoidanceState(float elapsedTime);//���
	void UpdateJumpState(float elapsedTime);//�W�����v
	void UpdateShotState(float elapsedTime);//�ˌ�


	//�X�V�֐��̊֐��|�C���^�̒�`
	typedef void (Player::* ActUpdate)(float elapsedTime);

	//�v���C���[�̈ړ����͏���
	bool InputMove(float elapsedTime);

	//�����t���̈ړ��i�U�����Ȃǂ̈ړ����́j
	bool InputMove(float elapsedTime, float restrictionMove, float restrictionTurn);
	const DirectX::XMFLOAT3 GetMoveVec(Camera* camera) const;
	//�W�����v���͏���
	void InputJump();
	//������
	void InputAvoidance();

	//���n������
	void OnLanding()override;
	//���S�����Ƃ��̏���
	void OnDead() override;
	//�_���[�W���󂯂����̏���
	void OnDamaged(WINCE_TYPE type) override;
	//�_���[�W���󂯂鏈��
	bool ApplyDamage(int damage, float invincible_time, WINCE_TYPE type)override;
	//���[�g���[�V����
	//void RootMotion(DirectX::XMFLOAT3 dir, float speed);
	//void RootMotionManual(DirectX::XMFLOAT3 dir, float speed);

	//�������x�𗎂Ƃ�
	bool Floating();

	//���V����
	bool Flying();

	//--------------------�ϐ�--------------------------
	//�֐��|�C���^�̐錾
	ActUpdate p_update = &Player::UpdateIdleState;

	PlayerParam param;
	State state;

	GamePad* gamePad;
	Mouse* mouse;
	Camera* camera;

	std::unique_ptr <gltf_model> model;

	//������W�����v���Ă邩
	int jump_count = 0;
	//�W�����v�\��
	const int jump_limit = 1;

	bool displayPlayerImgui = false;


	//------------------�U���֘A--------------------------

	AttackParam attackParam;


	DirectX::XMFLOAT3 forward;


	//------------------�f�o�b�O-------------------------
	bool isFallDawn = false;
public:
	//�_���[�W���󂯂��Ƃ��ɌĂ΂�� *�֐����ĂԂ̂̓_���[�W��^�����I�u�W�F�N�g
	AddDamageFunc damagedFunction;
	Capsule collider;


};

