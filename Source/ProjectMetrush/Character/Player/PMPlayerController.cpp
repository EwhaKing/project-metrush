#include "PMPlayerController.h"
#include "Camera/PMPlayerCameraManager.h"

APMPlayerController::APMPlayerController()
{
	PlayerCameraManagerClass = APMPlayerCameraManager::StaticClass();
}
