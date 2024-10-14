#include "device.h"

Device::Device()
{
}

void Device::Update(HWND hwnd, float elapsed_time)
{
	mouse.Update(hwnd);
	gamePad.Update(elapsed_time);
}
