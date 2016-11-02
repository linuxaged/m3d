#include "GUITextCaption.h"
#include "GUIFontRenderer.h"
#include "../../Utils/header/Logger.h"
#include "Matrix.h"
#include "../../OpenGL/G_Device.h"

using namespace GUISystem;

GUITextCaption::GUITextCaption(const std::string & name) : GUIElement(name)
{
	this->color = MyMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	this->canBeBaked = false;
	this->fontSize = 10;
	this->fontAtlasName = "";
	this->percentSize = -1;
}

GUITextCaption::~GUITextCaption()
{
}

GUITextCaption * GUITextCaption::GetTextCaption()
{
	return this;
}


void GUITextCaption::SetCanBaBaked(bool val)
{
	if (val)
	{
		MyUtils::Logger::LogError("GUI Text Caption can not be baked. Settings will be ignored.");
	}
}

const std::string & GUITextCaption::GetActiveTextureName() const
{
	//melo by vracet jmeno font textury ?
	return this->fontAtlasName;
}

const std::string & GUITextCaption::GetText() const
{
	return this->caption;
}

const std::string & GUITextCaption::GetFontFace() const
{
	return this->fontFace;
}

float GUITextCaption::GetRotationAngle() const
{
	return this->angle;
}

float GUITextCaption::GetFontPercentSize() const
{
	return this->percentSize;
}

int GUITextCaption::GetFontSize() const
{
	return this->fontSize;
}

void GUITextCaption::SetRotationAngle(float angle)
{
	this->angle = angle;
}

void GUITextCaption::SetFontFace(const std::string & fontFace)
{
	this->fontFace = fontFace;

	this->needRebaked = true;
}

void GUITextCaption::SetText(const std::string & text)
{
	this->caption = text;

	this->needRebaked = true;
}



void GUITextCaption::SetFontSize(float size)
{
	if (size <= 0)
	{
		return;
	}

	if (this->fontSize == static_cast<int>(size))
	{
		return;
	}

	if (size < 1.0)
	{
		//size is in percent of parent size
		if (this->parent == NULL)
		{
			this->percentSize = size;
		}
		else
		{
			this->fontSize = static_cast<int>(this->parent->GetProportions().height * size);
			this->percentSize = size;
		}

	}
	else
	{
		//real size
		this->fontSize = static_cast<int>(size);
		this->percentSize = -1;
	}

	this->needRebaked = true;
}


void GUITextCaption::Update(const ElementProportions & parentProportions, GUIFontRenderer * fontRenderer)
{
	if (this->needRebaked == false)
	{
		return;
	}

	//--- special only for text
	//if size of element has been set in %
	//renew it according to parent

	float size = parentProportions.height * this->GetFontPercentSize();
	this->SetFontSize(size);


	fontRenderer->SetFontSize(this->fontSize);
	GUISystem::ElementDim dim = fontRenderer->GetFontDimension(this->caption);
	float w = dim.w;
	float h = dim.h; // static_cast<float>(this->fontSize);

	//-----

	float x = parentProportions.topLeft.X;
	x += this->pos.x * parentProportions.width;
	x += this->pos.offsetX;

	float y = parentProportions.topLeft.Y;
	y += this->pos.y * parentProportions.height;
	y += this->pos.offsetY;

	//change position based on origin
	if (this->pos.origin == TL)
	{
		//do nothing - top left is default
	}
	else if (this->pos.origin == TR)
	{
		//swap x coordinate
		x = parentProportions.botRight.X - (x - parentProportions.topLeft.X); //put x back to top left
		x -= w;
	}
	else if (this->pos.origin == BL)
	{
		//swap y coordinate
		y = parentProportions.botRight.Y - (y - parentProportions.topLeft.Y); //put y back to top left
		y -= h;
	}
	else if (this->pos.origin == BR)
	{
		//swap x & y coordinate
		x = parentProportions.botRight.X - (x - parentProportions.topLeft.X); //put x back to top left
		y = parentProportions.botRight.Y - (y - parentProportions.topLeft.Y); //put y back to top left

		x -= w;
		y -= h;
	}
	else if (this->pos.origin == C)
	{
		//center of parent element
		x = x + (parentProportions.botRight.X - parentProportions.topLeft.X) * 0.5f; //put x back to top left
		y = y + (parentProportions.botRight.Y - parentProportions.topLeft.Y) * 0.5f; //put y back to top left

		x -= (w * 0.5f);
		y -= (h * 0.5f);
	}

	this->proportions.topLeft = MyMath::Vector2(x, y);
	this->proportions.botRight = MyMath::Vector2(x + w, y + h);
	this->proportions.width = w;
	this->proportions.height = h;

	this->needRebaked = false;
}