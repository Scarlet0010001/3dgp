#include "stage_main.h"
#include "user.h"

StageMain::StageMain()
{
    Graphics& graphics = Graphics::Instance();
    model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		//"Resources/Stage/ExampleStage_out/ExampleStage.gltf");
		"Resources/Stage/testStage.gltf");
	model->collisionMesh = std::make_unique<CollisionMesh>(
		graphics.GetDevice().Get(),
		//"Resources/Stage/ExampleStage_out/ExampleStage.gltf");
		"Resources/Stage/testStage.gltf");
		//"Resources/glTF-Sample-Models-master/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf");
	//modelCollision = std::make_unique<gltf_model>(graphics.GetDevice().Get(), ".\\resources\\Model\\Stage\\stage_hall_collision.fbx", 1);
    scale = { 10.0f, 10.0f, 10.0f };
   // scale = { 50.0f, 50.0f, 50.0f };
    
    animeTimer = 0.0f;
    animated_nodes = model->nodes;
	transform = Math::calc_world_matrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);

}

StageMain::~StageMain()
{
}

void StageMain::Update(float elapsedTime)
{
    transform = Math::calc_world_matrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);

}

void StageMain::Render(float elapsedTime)
{
    static DirectX::XMFLOAT4 material_color = { 1,1,1,1 };
    Graphics& graphics = Graphics::Instance();

    //モデル描画
    model->animate(0, animeTimer += elapsedTime, animated_nodes);
    model->render(graphics.Get_DC().Get(), transform, animated_nodes);

#if USE_IMGUI
	imguiMenuBar("Stage", "stage_main", displayImgui);
	if (displayImgui)
	{

		ImGui::Begin("stage_main");
		ImGui::DragFloat3("scale", &scale.x, 0.1f);
		ImGui::Checkbox("flustm_flag", &flustm_flag);
		static int num = 0;
		ImGui::DragInt("mesh_num", &num, 1, 0, model->meshes.size());
		int mesh_size = model->meshes.size();
		ImGui::DragInt("mesh_size", &mesh_size);
		//DirectX::XMFLOAT3 min = model->meshes.at(num).boundinbox[0];
		//DirectX::XMFLOAT3 max = mode->meshes.at(num).bounding_box[1];
		//ImGui::DragFloat3("bounding_min", &min.x);
		//ImGui::DragFloat3("bounding_max", &max.x);
		ImGui::End();
	}

	// フレーム表示
	{
		ImGui::Begin("##frame stage_rate");

		static float temp_value = 0;
		static float values[90] = {};
		static int values_offset = 0;
		static float refresh_time = 0.0f;
		static const float PLOT_SENSE = 0.2f;

		refresh_time += elapsedTime;
		if (static_cast<int>(refresh_time / PLOT_SENSE) >= 1)
		{
			values_offset = values_offset >= IM_ARRAYSIZE(values) ? 0 : values_offset;
			values[values_offset] = temp_value = elapsedTime * 1000.0f;

			++values_offset;
			refresh_time = 0;
		}

		char overlay[32];
		sprintf_s(overlay, "now: %d fps  %.3f ms", static_cast<int>(1000.0f / temp_value), temp_value);
		ImGui::PlotLines("##frame", values, IM_ARRAYSIZE(values), values_offset, overlay, 0, 20, ImVec2(ImGui::GetWindowSize().x * 0.75f, ImGui::GetWindowSize().y * 0.5f));

		ImGui::End();
	}
#endif

}

bool StageMain::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
	//最初transformがぜんぶ0
	return Collision::RayVsModel(start, end, model.get(), transform, hit);

    return false;
}
