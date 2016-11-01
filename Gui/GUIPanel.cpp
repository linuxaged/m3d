#include "../header/GUIPanel.h"


using namespace GUISystem;

GUIPanel::GUIPanel(const std::string & name) : GUIElement(name)
{	
}

GUIPanel::~GUIPanel()
{

}


GUIPanel * GUIPanel::GetPanel()
{
	return this;
}

const std::string & GUIPanel::GetActiveTextureName() const
{
	return this->textures.textureName;
}

const GUIPanelTextures & GUIPanel::GetTextures() const
{
	return this->textures;
}



void GUIPanel::SetTextures(GUIPanelTextures & textures)
{
	this->textures = textures;
}

