#include "ImageAdjust.h"

void ImageAdjustEffect::UpdateConstants() {
	bool isExterior = TheShaderManager->isExterior;
	float transitionCurve = TheShaderManager->transitionCurve;

	Constants.Data.x = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "Brightness", isExterior, transitionCurve);
	Constants.Data.y = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "Contrast", isExterior, transitionCurve);
	Constants.Data.z = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "Saturation", isExterior, transitionCurve);
	Constants.Data.w = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "Strength", isExterior, transitionCurve);

	Constants.DarkColor.x = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "DarkColorR", isExterior, transitionCurve);
	Constants.DarkColor.y = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "DarkColorG", isExterior, transitionCurve);
	Constants.DarkColor.z = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "DarkColorB", isExterior, transitionCurve);

	Constants.LightColor.x = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "LightColorR", isExterior, transitionCurve);
	Constants.LightColor.y = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "LightColorG", isExterior, transitionCurve);
	Constants.LightColor.z = TheSettingManager->GetSettingTransition("Shaders.ImageAdjust", "LightColorB", isExterior, transitionCurve);

}