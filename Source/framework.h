#pragma once

#include <d3d11.h>
#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include <sstream>

#include "sprite.h"
#include "sprite_batch.h"
#include "geometric_primitive.h"
#include "static_mesh.h"
#include "skinned_mesh.h"
#include "framebuffer.h"
#include "fullscreen_quad.h"
#include "misc.h"
#include "high_resolution_timer.h"

#include "device.h"

//gltf
#include "gltf_model.h"

#ifdef USE_IMGUI
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

CONST LPCWSTR APPLICATION_NAME{ L"X3DGP" };

class framework
{
public:
	CONST HWND hwnd;


	framework(HWND hwnd);
	~framework();

	framework(const framework&) = delete;
	framework& operator=(const framework&) = delete;
	framework(framework&&) noexcept = delete;
	framework& operator=(framework&&) noexcept = delete;

	int run();

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef USE_IMGUI
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps{};
			BeginPaint(hwnd, &ps);

			EndPaint(hwnd, &ps);
		}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE)
			{
				PostMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
		case WM_ENTERSIZEMOVE:
			tictoc.stop();
			break;
		case WM_EXITSIZEMOVE:
			tictoc.start();
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private:
	bool initialize();
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);
	bool uninitialize();

	//Microsoft::WRL::ComPtr<ID3D11Device> device;
	//Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediate_context;
	//Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain;
	//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[8];
	//
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffers[8];
	//
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shaders[8];
	//
	//Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[4];
	//
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states[4];
	//
	//Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[4];
	//
	//Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states[4];
	//
	//std::unique_ptr<Sprite> sprites[8];
	//
	//std::unique_ptr<Sprite_Batch> sprite_batches[8];
	//
	//std::unique_ptr<geometric_primitive> geometric_primitives[8];
	//
	//std::unique_ptr<static_mesh> static_meshes[8];
	//
	//std::unique_ptr<skinned_mesh> skinned_meshes[8];
	//
	//std::unique_ptr<gltf_model> gltf_models[8];
	//
	//std::unique_ptr<framebuffer> framebuffers[8];
	//
	//std::unique_ptr<fullscreen_quad> bit_block_transfer;

	struct scene_constants
	{
		DirectX::XMFLOAT4X4 view_projection; //�r���[�E�v���W�F�N�V�����ϊ��s��
		DirectX::XMFLOAT4 light_direction; //���C�g�̌���
		DirectX::XMFLOAT4 camera_position;

		float smoothstep_minEdge;						//�G�b�W�̉����B���̒l�ȉ��ł� smoothstep �̌��ʂ� 0 �ɂȂ�܂��B
		float smoothstep_maxEdge;						//�G�b�W�̏���B���̒l�ȏ�ł� smoothstep �̌��ʂ� 1 �ɂȂ�܂��B
		
		float gaussian_sigma;
		float bloom_intensity;

		float exposure;
		float dummy1;
		float dummy2;
		float dummy3;

	};


	DirectX::XMFLOAT4 camera_position{ 0.0f, 0.0f, -10.0f, 1.0f };
	DirectX::XMFLOAT4 light_direction{ 0.0f, 0.0f, 1.0f, 0.0f };

	//luminance_extraction_ps
	float smoothstep_minEdge;						//�G�b�W�̉����B���̒l�ȉ��ł� smoothstep �̌��ʂ� 0 �ɂȂ�܂��B
	float smoothstep_maxEdge;						//�G�b�W�̏���B���̒l�ȏ�ł� smoothstep �̌��ʂ� 1 �ɂȂ�܂��B

	//�K�E�V�A�����z�̕W���΍�
	//�ڂ����̋�����ڂ����̍L����	�̒���
	float gaussian_sigma = 2.0f;

	//�u���[���i��䊁j���ʂ̋��x�𐧌䂷��
	//bloom_intensity �̒����ɂ��A�V�[���S�̂����̌�������̌������������\�������
	float bloom_intensity;

	float exposure = 1.2f;

	DirectX::XMFLOAT3 translation{ 0, 0, 0 };
	DirectX::XMFLOAT3 scaling{ 1, 1, 1 };
	DirectX::XMFLOAT3 rotation{ 0, 0, 0 };
	DirectX::XMFLOAT4 material_color{ 1 ,1, 1, 1 };

	float factor = 0.5f;
	int clip_count = 0;
	int clip_Max = 0;

#if 0
	DirectX::XMFLOAT3 axis{ 1, 0, 0 };
	DirectX::XMFLOAT3 kf_Translation{ 0, 0, 0 };
#endif


private:
	high_resolution_timer tictoc;
	uint32_t frames{ 0 };
	float elapsed_time{ 0.0f };
	void calculate_frame_stats()
	{
		if (++frames, (tictoc.time_stamp() - elapsed_time) >= 1.0f)
		{
			float fps = static_cast<float>(frames);
			std::wostringstream outs;
			outs.precision(6);
			outs << APPLICATION_NAME << L" : FPS : " << fps << L" / " << L"Frame Time : " << 1000.0f / fps << L" (ms)";
			SetWindowTextW(hwnd, outs.str().c_str());

			frames = 0;
			elapsed_time += 1.0f;
		}
	}
};

