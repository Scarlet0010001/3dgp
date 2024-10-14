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
	//------------<定数>-----------//
	enum SAMPLER_STATE
	{
		//ポイントサンプリング、ワープアドレッシング
		//テクスチャが拡大縮小されるとき、最も近いテクセルの色を選択します。これはピクセルアートなど、テクスチャが低解像度の場合に適しています
		POINT_SAMPLE = 0,

		//線形補間、ワープアドレッシング
		//テクスチャが拡大縮小されるとき、周囲のテクセルの色を線形補間して滑らかな効果を生み出します。これは一般的なテクスチャマッピングに適しています
		LINEAR,

		//アイソトロピックフィルタリング、ワープアドレッシング
		//テクスチャが拡大縮小されるとき、アイソトロピックフィルタリングを使用して滑らかで高品質な補間を行います。主に角度に依存しない効果が求められる場合に使用されます
		ANISOTROPIC,

		LINEAR_BORDER_BLACK,

		LINEAR_BORDER_WHITE,

		//ポイントサンプリング、クランプアドレッシング
		//テクスチャが拡大縮小されるとき、最も近いテクセルの色を選択します。これはピクセルアートなど、テクスチャが低解像度の場合に適しています
		//テクスチャ座標が [0, 1] の範囲外になると、範囲内に収める代わりに、端の色でクランプします
		CLAMP,

		SHADOW_MAP,
		SAMPLER_COUNT,
	};

	enum DEPTH_STATE
	{
		//深度テストが有効で、深度バッファへの書き込みが許可されています。通常の3D描画に使用します。
		DepthON_WriteON = 0,

		//深度テストが無効で、深度バッファへの書き込みが許可されています。デプステストを無効にした通常の描画に使用します。
		DepthOFF_WriteON,

		//深度テストが有効で、深度バッファへの書き込みが無効です。影の描画など、深度情報の更新が不要な場合に使用します。
		DepthON_WriteOFF,

		//深度テストが無効で、深度バッファへの書き込みも無効です。通常の描画でデプステストや深度情報が不要な場合に使用します。
		DepthOFF_WriteOFF,

		DEPTH_STATE_COUNT
	};

	enum BLEND_STATE
	{
		/// <summary>
		/// <para>最も一般的なブレンディングモード</para>
		/// <para>オブジェクトが透明で、アルファチャンネルを持つ場合に使用されます。</para>
		/// オブジェクトの透明度に応じて背後のオブジェクトを見えるようにします。
		/// </summary>
		NORMAL = 0,

		/// <summary>
		/// <para>通常ブレンディングと同じですが、アルファチャンネルの値に基づいて合成します。</para>
		/// <para>アルファ チャンネルの値が低いほど、オブジェクトが透明になります。</para>
		/// </summary>
		ALPHA,

		/// <summary>
		/// <para>色を加算して合成します。発光効果などを表現するのに使用されます。</para>
		/// <para>オブジェクトが光を放射するように見えます。</para>
		/// </summary>
		ADD,

		/// <summary>
		/// <para>色を減算して合成します。光や炎の効果を表現するために使用されます。</para>
		/// <para>オブジェクトが光を吸収するように見えます。</para>
		/// </summary>
		SUBTRACTIVE,

		/// <summary>
		/// <para>色を乗算して合成します。影や色の混合に使用されます。</para>
		/// <para>オブジェクトの色が背景に影響を与えます。</para>
		/// </summary>
		MULTIPLY,

		BLEND_STATE_COUNT
	};

	enum RASTERIZER_STATE
	{
		SOLID_ONESIDE,                   // SOLID, 面カリングなし, 時計回りが表
		CULL_NONE,               // SOLID, 面カリングあり
		SOLID_COUNTERCLOCKWISE,  // SOLID, 面カリングなし, 反時計回りが表
		WIREFRAME_CULL_BACK,     // WIREFRAME, 後ろ向きの三角形を描画しない
		WIREFRAME_CULL_NONE,     // WIREFRAME, 常にすべての三角形を描画する

		RASTERIZER_STATE_COUNT
	};

	//シェーダータイプ
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

	//------------<ゲッター/セッター>-----------//
	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() { return device; }							//DirectX11の機能にアクセスするためのデバイス。このデバイスから描画に必要なオブジェクトの生成などを行う		
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Get_DC() { return immediateContext; }				//描画コマンドの生成や発行を管理をする。D3D11CreateDeviceAndSwapChainで生成されるのはImmediate。Immediateではコマンドの生成からGPUへの発行まで行う。
	Microsoft::WRL::ComPtr<IDXGISwapChain> GetSwapChain() { return swapChain; }						//レンダリング結果を出力するためのオブジェクト
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() { return renderTargetView; }			//レンダーターゲットビューを出力結合ステージにバインドできる
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() { return depthStencilView; }	//深度ステンシルビューインターフェイスは、深度ステンシルテスト中にテクスチャーリソースにアクセスする。
	Microsoft::WRL::ComPtr<ID3D11SamplerState> GetSamplerState(SAMPLER_STATE s) { return samplerStates[static_cast<int>(s)]; }
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> GetDepthStencilState(DEPTH_STATE d) { return depthStencilStates[static_cast<int>(d)]; }	//深度ステンシルビューインターフェイスは、深度ステンシルテスト中にテクスチャーリソースにアクセスする。
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetRasterizerRtate(RASTERIZER_STATE r) { return rasterizerStates[static_cast<int>(r)]; }
	Microsoft::WRL::ComPtr<ID3D11BlendState> GetBlendState(BLEND_STATE b) { return blendStates[static_cast<int>(b)]; }

	// デバッグレンダラ取得
	DebugRenderer* GetDebugRenderer() const { return debugRenderer.get(); }

	//------------<関数>-----------//
public:
		void Initialize(HWND hwnd);
		void DebugGui();

	//------------<変数>-----------//
private:
	//COMオブジェクト
	Microsoft::WRL::ComPtr<ID3D11Device> device;							//DirectX11の機能にアクセスするためのデバイス。このデバイスから描画に必要なオブジェクトの生成などを行う		
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;				//描画コマンドの生成や発行を管理をする。D3D11CreateDeviceAndSwapChainで生成されるのはImmediate。Immediateではコマンドの生成からGPUへの発行まで行う。
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;						//レンダリング結果を出力するためのオブジェクト
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;			//レンダーターゲットビューを出力結合ステージにバインドできる
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;		//深度ステンシルビューインターフェイスは、深度ステンシルテスト中にテクスチャーリソースにアクセスする。
	
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
	//ミューテックス取得
	std::mutex& GetMutex() { return mutex_; }
	//シェーダーのリコンパイル
	//BOOL get_file_name(HWND hWnd, TCHAR* fname, int sz, TCHAR* initDir);
	//bool recompile_pixel_shader(ID3D11PixelShader** pixel_shader, std::string id);
private:
	std::mutex mutex_;

	HWND hwnd;

};

