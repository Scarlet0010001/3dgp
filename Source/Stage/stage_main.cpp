#include "stage_main.h"
#include "User/user.h"

StageMain::StageMain()
{
    Graphics& graphics = Graphics::Instance();
    model = std::make_unique<gltf_model>(graphics.GetDevice().Get(),
		"Resources/Stage/low_poly_hole_in_the_road.glb");
	//“–‚½‚è”»’è—p
	model->collisionMesh = std::make_unique<CollisionMesh>(
		graphics.GetDevice().Get(),
		"Resources/Stage/low_poly_hole_in_the_road.glb");

    scale = { 100.0f, 100.0f, 100.0f };
    
    animeTimer = 0.0f;
    animatedNodes = model->nodes;
	transform = Math::CalcWorldMatrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);

}

StageMain::~StageMain()
{
}

void StageMain::Update(float elapsedTime)
{
    transform = Math::CalcWorldMatrix(scale, angle, position, Math::COORDINATE_SYSTEM::RHS_YUP);

}

void StageMain::Render(float elapsedTime)
{
    static DirectX::XMFLOAT4 material_color = { 1,1,1,1 };
    Graphics& graphics = Graphics::Instance();

    //ƒ‚ƒfƒ‹•`‰æ
    model->animate(0, animeTimer += elapsedTime, animatedNodes);
    model->render(graphics.Get_DC().Get(), transform, animatedNodes);
	elapsedTime_ = elapsedTime;
}

void StageMain::DebugDUI()
{
#if USE_IMGUI
	ImguiMenuBar("Stage", "stage_main", displayImgui);
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
#endif

}

bool StageMain::RayCast(const DirectX::XMFLOAT3& start, const DirectX::XMFLOAT3& end, HitResult& hit)
{
	//Å‰transform‚ª‚º‚ñ‚Ô0
	return Collision::RayVsModel(start, end, model.get(), transform, hit);

    return false;
}
