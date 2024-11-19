#include "device.h"

Device::Device()
{
}

void Device::Update(HWND hwnd, float elapsedTime)
{
	mouse.Update(hwnd);
	gamePad.Update(elapsedTime);
}
