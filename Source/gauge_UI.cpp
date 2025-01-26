#include "gauge_UI.h"
#include "Operators.h"
#include "user.h"

GaugeUI::GaugeUI(const wchar_t* back_filename, const wchar_t* body_filename, const wchar_t* frame_filename)
{
    Graphics& graphics = Graphics::Instance();
    back = std::make_unique<SpriteBatch>(graphics.GetDevice().Get(), back_filename, 1);
    body = std::make_unique<SpriteBatch>(graphics.GetDevice().Get(), body_filename, 2);
    if (frame_filename)
    {
        frame = std::make_unique<SpriteBatch>(graphics.GetDevice().Get(), frame_filename, 1);
    }

    nowPercent = 1.0f;
    oldPercent = 1.0f;
    diffColor = { 2.0f,2.0f, 1.0f, 1.0f };

    gauge.texsize = { static_cast<float>(back->GetTexture2dDesc().Width), static_cast<float>(back->GetTexture2dDesc().Height) };

}

void GaugeUI::Update(float elapsed_time)
{
    //ゲージパーセントが減った場合の差分ゲージの処理
    if (nowPercent < oldPercent)
    {
        const float min_rate = 0.5f;
        const float max_rate = 1.0f;
        const float diff_rate = Math::Lerp(min_rate, max_rate, (1 - (nowPercent / oldPercent))) * elapsed_time;
        oldPercent = Math::Lerp(oldPercent, nowPercent, diff_rate);
    }
}

void GaugeUI::Render(ID3D11DeviceContext* dc)
{
    gauge.angle = 0;
    //--back--//
    back->begin(dc);
    back->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
    back->end(dc);
    //--body--//
    body->begin(dc);
    //ゲージ差分
    body->render(dc, gauge.position, gauge.scale, gauge.pivot, diffColor, gauge.angle, gauge.texpos,
        { gauge.texsize.x * oldPercent, gauge.texsize.y });
    //ゲージ本体
    body->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos,
        { gauge.texsize.x * nowPercent, gauge.texsize.y });
    body->end(dc);
    //--frame--//
    if (frame)
    {
        frame->begin(dc);
        frame->render(dc, gauge.position, gauge.scale, gauge.pivot, gauge.color, gauge.angle, gauge.texpos, gauge.texsize);
        frame->end(dc);
    }
}
