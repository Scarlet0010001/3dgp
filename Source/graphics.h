#pragma once
#include<d3d11.h>
#include <map>
#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include <sstream>
#include <mutex>
#include "misc.h"
#include "debug_renderer.h"
#include "mesh_shader.h"

#if _DEBUG
CONST LONG SCREEN_WIDTH{ 1280 };
CONST LONG SCREEN_HEIGHT{ 720 };
#else
CONST LONG SCREEN_WIDTH{ 1920 };
CONST LONG SCREEN_HEIGHT{ 1080 };
#endif
CONST BOOL FULLSCREEN{ FALSE };

#define ST_SAMPLER Graphics::SAMPLER_STATE
#define ST_DEPTH Graphics::DEPTH_STATE
#define ST_BLEND Graphics::BLEND_STATE
#define ST_RASTERIZER Graphics::RASTERIZER_STATE
#define SHADER_TYPE Graphics::SHADER_TYPES
#define RENDER_TYPE MeshShader::RenderType

class Graphics
{
public:
	//------------<�萔>-----------//
	enum SAMPLER_STATE
	{
		//�|�C���g�T���v�����O�A���[�v�A�h���b�V���O
		//�e�N�X�`�����g��k�������Ƃ��A�ł��߂��e�N�Z���̐F��I�����܂��B����̓s�N�Z���A�[�g�ȂǁA�e�N�X�`������𑜓x�̏ꍇ�ɓK���Ă��܂�
		POINT_SAMPLE = 0,

		//���`��ԁA���[�v�A�h���b�V���O
		//�e�N�X�`�����g��k�������Ƃ��A���͂̃e�N�Z���̐F����`��Ԃ��Ċ��炩�Ȍ��ʂ𐶂ݏo���܂��B����͈�ʓI�ȃe�N�X�`���}�b�s���O�ɓK���Ă��܂�
		LINEAR,

		//�A�C�\�g���s�b�N�t�B���^�����O�A���[�v�A�h���b�V���O
		//�e�N�X�`�����g��k�������Ƃ��A�A�C�\�g���s�b�N�t�B���^�����O���g�p���Ċ��炩�ō��i���ȕ�Ԃ��s���܂��B��Ɋp�x�Ɉˑ����Ȃ����ʂ����߂���ꍇ�Ɏg�p����܂�
		ANISOTROPIC,

		LINEAR_BORDER_BLACK,

		LINEAR_BORDER_WHITE,

		//�|�C���g�T���v�����O�A�N�����v�A�h���b�V���O
		//�e�N�X�`�����g��k�������Ƃ��A�ł��߂��e�N�Z���̐F��I�����܂��B����̓s�N�Z���A�[�g�ȂǁA�e�N�X�`������𑜓x�̏ꍇ�ɓK���Ă��܂�
		//�e�N�X�`�����W�� [0, 1] �͈̔͊O�ɂȂ�ƁA�͈͓��Ɏ��߂����ɁA�[�̐F�ŃN�����v���܂�
		CLAMP,

		SHADOW_MAP,
		SAMPLER_COUNT,
	};

	enum DEPTH_STATE
	{
		//�[�x�e�X�g���L���ŁA�[�x�o�b�t�@�ւ̏������݂�������Ă��܂��B�ʏ��3D�`��Ɏg�p���܂��B
		DepthON_WriteON = 0,

		//�[�x�e�X�g�������ŁA�[�x�o�b�t�@�ւ̏������݂�������Ă��܂��B�f�v�X�e�X�g�𖳌��ɂ����ʏ�̕`��Ɏg�p���܂��B
		DepthOFF_WriteON,

		//�[�x�e�X�g���L���ŁA�[�x�o�b�t�@�ւ̏������݂������ł��B�e�̕`��ȂǁA�[�x���̍X�V���s�v�ȏꍇ�Ɏg�p���܂��B
		DepthON_WriteOFF,

		//�[�x�e�X�g�������ŁA�[�x�o�b�t�@�ւ̏������݂������ł��B�ʏ�̕`��Ńf�v�X�e�X�g��[�x��񂪕s�v�ȏꍇ�Ɏg�p���܂��B
		DepthOFF_WriteOFF,

		DEPTH_STATE_COUNT
	};

	enum BLEND_STATE
	{
		/// <summary>
		/// <para>�ł���ʓI�ȃu�����f�B���O���[�h</para>
		/// <para>�I�u�W�F�N�g�������ŁA�A���t�@�`�����l�������ꍇ�Ɏg�p����܂��B</para>
		/// �I�u�W�F�N�g�̓����x�ɉ����Ĕw��̃I�u�W�F�N�g��������悤�ɂ��܂��B
		/// </summary>
		NORMAL = 0,

		/// <summary>
		/// <para>�ʏ�u�����f�B���O�Ɠ����ł����A�A���t�@�`�����l���̒l�Ɋ�Â��č������܂��B</para>
		/// <para>�A���t�@ �`�����l���̒l���Ⴂ�قǁA�I�u�W�F�N�g�������ɂȂ�܂��B</para>
		/// </summary>
		ALPHA,

		/// <summary>
		/// <para>�F�����Z���č������܂��B�������ʂȂǂ�\������̂Ɏg�p����܂��B</para>
		/// <para>�I�u�W�F�N�g��������˂���悤�Ɍ����܂��B</para>
		/// </summary>
		ADD,

		/// <summary>
		/// <para>�F�����Z���č������܂��B���≊�̌��ʂ�\�����邽�߂Ɏg�p����܂��B</para>
		/// <para>�I�u�W�F�N�g�������z������悤�Ɍ����܂��B</para>
		/// </summary>
		SUBTRACTIVE,

		/// <summary>
		/// <para>�F����Z���č������܂��B�e��F�̍����Ɏg�p����܂��B</para>
		/// <para>�I�u�W�F�N�g�̐F���w�i�ɉe����^���܂��B</para>
		/// </summary>
		MULTIPLY,

		BLEND_STATE_COUNT
	};

	enum RASTERIZER_STATE
	{
		SOLID_ONESIDE,                   // SOLID, �ʃJ�����O�Ȃ�, ���v��肪�\
		CULL_NONE,               // SOLID, �ʃJ�����O����
		SOLID_COUNTERCLOCKWISE,  // SOLID, �ʃJ�����O�Ȃ�, �����v��肪�\
		WIREFRAME_CULL_BACK,     // WIREFRAME, �������̎O�p�`��`�悵�Ȃ�
		WIREFRAME_CULL_NONE,     // WIREFRAME, ��ɂ��ׂĂ̎O�p�`��`�悷��

		RASTERIZER_STATE_COUNT
	};

	//�V�F�[�_�[�^�C�v
	enum SHADER_TYPES
	{
		LAMBERT,
		PBR,
		SHADOW
	};
private:
	Graphics() {}
	~Graphics();
public:
	static Graphics& Instance()
	{
		static Graphics instance;
		return instance;
	}

	//------------<�Q�b�^�[/�Z�b�^�[>-----------//
	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() { return device; }							//DirectX11�̋@�\�ɃA�N�Z�X���邽�߂̃f�o�C�X�B���̃f�o�C�X����`��ɕK�v�ȃI�u�W�F�N�g�̐����Ȃǂ��s��		
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Get_DC() { return immediateContext; }				//�`��R�}���h�̐����┭�s���Ǘ�������BD3D11CreateDeviceAndSwapChain�Ő��������̂�Immediate�BImmediate�ł̓R�}���h�̐�������GPU�ւ̔��s�܂ōs���B
	Microsoft::WRL::ComPtr<IDXGISwapChain> GetSwapChain() { return swapChain; }						//�����_�����O���ʂ��o�͂��邽�߂̃I�u�W�F�N�g
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() { return renderTargetView; }			//�����_�[�^�[�Q�b�g�r���[���o�͌����X�e�[�W�Ƀo�C���h�ł���
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() { return depthStencilView; }	//�[�x�X�e���V���r���[�C���^�[�t�F�C�X�́A�[�x�X�e���V���e�X�g���Ƀe�N�X�`���[���\�[�X�ɃA�N�Z�X����B
	Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_STATE s) { return samplerStates[static_cast<int>(s)]; }
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTH_STATE d) { return depthStencilStates[static_cast<int>(d)]; }	//�[�x�X�e���V���r���[�C���^�[�t�F�C�X�́A�[�x�X�e���V���e�X�g���Ƀe�N�X�`���[���\�[�X�ɃA�N�Z�X����B
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerRtate(RASTERIZER_STATE r) { return rasterizerStates[static_cast<int>(r)]; }
	Microsoft::WRL::ComPtr<ID3D11BlendState> GetBlendState(BLEND_STATE b) { return blendStates[static_cast<int>(b)]; }

	// �f�o�b�O�����_���擾
	DebugRenderer* GetDebugRenderer() const { return debugRenderer.get(); }

	//------------<�֐�>-----------//
public:
		void Initialize(HWND hwnd);
		void DebugGui();

	//------------<�ϐ�>-----------//
private:
	//COM�I�u�W�F�N�g
	Microsoft::WRL::ComPtr<ID3D11Device> device;							//DirectX11�̋@�\�ɃA�N�Z�X���邽�߂̃f�o�C�X�B���̃f�o�C�X����`��ɕK�v�ȃI�u�W�F�N�g�̐����Ȃǂ��s��		
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;				//�`��R�}���h�̐����┭�s���Ǘ�������BD3D11CreateDeviceAndSwapChain�Ő��������̂�Immediate�BImmediate�ł̓R�}���h�̐�������GPU�ւ̔��s�܂ōs���B
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;						//�����_�����O���ʂ��o�͂��邽�߂̃I�u�W�F�N�g
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;			//�����_�[�^�[�Q�b�g�r���[���o�͌����X�e�[�W�Ƀo�C���h�ł���
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;		//�[�x�X�e���V���r���[�C���^�[�t�F�C�X�́A�[�x�X�e���V���e�X�g���Ƀe�N�X�`���[���\�[�X�ɃA�N�Z�X����B
	
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[SAMPLER_STATE::SAMPLER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[DEPTH_STATE::DEPTH_STATE_COUNT];
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[RASTERIZER_STATE::RASTERIZER_STATE_COUNT];
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[BLEND_STATE::BLEND_STATE_COUNT];

	std::unique_ptr<DebugRenderer> debugRenderer;

	//--maps--//
	//std::map<SHADER_TYPES, std::shared_ptr<MeshShader>> shaders;

public:
	//std::shared_ptr<MeshShader> shader = nullptr;
	void SetDepthStencilState(DEPTH_STATE z_stencil);
	void SetBlendState(BLEND_STATE blend);
	void SetRasterizerState(RASTERIZER_STATE rasterizer);
	void SetGraphicStatePriset(DEPTH_STATE z_stencil, BLEND_STATE blend, RASTERIZER_STATE rasterizer);
	void ShaderActivate(SHADER_TYPES sh, RENDER_TYPE rt);

	inline void SetHwnd(HWND hwnd) { this->hwnd = hwnd; }
	//�~���[�e�b�N�X�擾
	std::mutex& GetMutex() { return mutex_; }
	//�V�F�[�_�[�̃��R���p�C��
	//BOOL get_file_name(HWND hWnd, TCHAR* fname, int sz, TCHAR* initDir);
	//bool recompile_pixel_shader(ID3D11PixelShader** pixel_shader, std::string id);
private:
	std::mutex mutex_;

	HWND hwnd;

};

