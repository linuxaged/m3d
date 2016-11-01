#include "../header/GUIImage.h"

#include "../../Utils/header/Logger.h"

using namespace GUISystem;


/*-----------------------------------------------------------
Function:	ctor
Parametrs:
	[in] name - element name

Create GUI image
Image is never baked and uses dynamic image
-------------------------------------------------------------*/
GUIImage::GUIImage(const std::string & name) : GUIElement(name)
{
	this->canBeBaked = false;
}

GUIImage::~GUIImage()
{
}

GUIImage * GUIImage::GetImage()
{
	return this;
}

void GUIImage::SetCanBaBaked(bool val)
{
	if (val)
	{
		MyUtils::Logger::LogError("GUI Image can not be baked. Settings will be ignored.");
	}
}

const std::string & GUIImage::GetActiveTextureName() const
{
	return this->textureName;
}

/*-----------------------------------------------------------
Function:	SetTextureName
Paramaters:
	[in] name - texture name

Set name of texture that is displayed on image
Texture must be loaded manually elsewhere into texture pool
-------------------------------------------------------------*/
void GUIImage::SetTextureName(const std::string & name)
{
	this->textureName = name;
}