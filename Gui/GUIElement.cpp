#include "../header/GUIElement.h"

#include "../header/GUITextCaption.h"

using namespace GUISystem;

GUIElement::GUIElement(const std::string & name)
{
	this->name = name;

	this->pos.x = 0.0;
	this->pos.y = 0.0;
	this->pos.offsetX = 0;
	this->pos.offsetY = 0;

	this->dim.w = 0.0;
	this->dim.h = 0.0;
	this->dim.pixelW = 0;
	this->dim.pixelH = 0;
	this->dim.AR = 0.0;

	this->max.pixelW = 0;
	this->max.pixelH = 0;

	this->color = MyMath::Vector4(0.0f, 0.0f, 0.0f, 0.0f);

	this->parent = NULL;

	this->baked = false;
	this->canBeBaked = true;

	this->needRebaked = true;

	this->userData = NULL;
}

GUIElement::~GUIElement()
{
	/*
	if (this->userData != NULL)
	{
		this->userData->refCount--;
		if (this->userData->refCount <= 0)
		{
			//to do
			//SAFE_DELETE(this->userData);
		}
	}
	*/
}

GUIPanel * GUIElement::GetPanel()
{
	return NULL;
}

GUIImage * GUIElement::GetImage()
{
	return NULL;
}

GUIButton * GUIElement::GetButton()
{
	return NULL;
}

GUICheckBox * GUIElement::GetCheckBox()
{
	return NULL;
}

GUITextCaption * GUIElement::GetTextCaption()
{
	return NULL;
}

GUITextPanel * GUIElement::GetTextPanel()
{
	return NULL;
}

const std::string & GUIElement::GetName() const
{
	return this->name;
}

bool GUIElement::IsBaked() const
{
	return this->baked;
}

bool GUIElement::IsVisible() const
{
	return this->visible;
}

/*-----------------------------------------------------------
Function:	CanBeBaked
Return:
	true / false

Return true if object can be baked into "baked" GUI
-------------------------------------------------------------*/
bool GUIElement::CanBeBaked() const
{
	return this->canBeBaked;
}

void GUIElement::SetCanBaBaked(bool val)
{
	this->canBeBaked = val;
}

const std::vector<GUIElement *> * GUIElement::GetChildrens() const
{
	return &this->childs;
}

bool GUIElement::HasChilds() const
{
	return this->childs.size() > 0;
}

const ElementProportions & GUIElement::GetProportions() const
{
	return this->proportions;
}

const MyMath::Vector4 & GUIElement::GetColor() const
{
	return this->color;
}

IUserData * GUIElement::GetUserData() const
{
	return this->userData;
}

void GUIElement::SetUserData(IUserData * data)
{
	if (this->userData != NULL)
	{
		//overwrite old data with new one
		//decrease refCount for old data
		this->userData->refCount--;
		if (this->userData->refCount < 0)
		{
			this->userData->refCount = 0;
		}
	}
	this->userData = data;
	this->userData->refCount++;
}

/*-----------------------------------------------------------
Function:	SetColor
Parametrs:
	[in] r - red channel
	[in] g - green channel
	[in] b - blue channel
	[in] a - alpha channel

Set button color
Color is blended with texture according to set alpha color
-------------------------------------------------------------*/
void GUIElement::SetColor(int r, int g, int b, int a)
{
	this->color.X = r / 255.0f;
	this->color.Y = g / 255.0f;
	this->color.Z = b / 255.0f;
	this->color.W = a / 255.0f;

	this->needRebaked = true;
}

/*-----------------------------------------------------------
Function:	SetVisible
Parametrs:
	[in] val - visibility

Set visibility to object and all of his childrens
-------------------------------------------------------------*/
void GUIElement::SetVisible(bool val)
{
	this->visible = val;

	std::vector<GUIElement *>::iterator it;
	for (it = this->childs.begin(); it != this->childs.end(); it++)
	{
		(*it)->SetVisible(val);
	}
}

/*-----------------------------------------------------------
Function:	SetPosition
Parametrs:
	[in] pos - element position

Set object position within its parent
-------------------------------------------------------------*/
void GUIElement::SetPosition(ElementPos pos)
{
	this->pos = pos;
	this->needRebaked = true;
}

/*-----------------------------------------------------------
Function:	SetSize
Parametrs:
	[in] dim - objects dimension

Set object dimension
-------------------------------------------------------------*/
void GUIElement::SetSize(ElementDim dim)
{
	this->dim = dim;
	this->needRebaked = true;
}

/*-----------------------------------------------------------
Function:	SetMaxSize
Parametrs:
	[in] w - max width in pixels
	[in] h - max height in pixels

Set max width and height in pixels for object
If size is bigger, size is clamped to max
(Set 0 to disable)
-------------------------------------------------------------*/
void GUIElement::SetMaxSize(int w, int h)
{
	this->max.pixelW = w;
	this->max.pixelH = h;

	this->needRebaked = true;
}

void GUIElement::SetOnMoveCallback(OnMoveDelegate onMoveDelegate)
{
	this->onMove = onMoveDelegate;
}

/*-----------------------------------------------------------
Function:	AddChild
Paramaters:
	[in] el - child element

Add children element to current element
-------------------------------------------------------------*/
void GUIElement::AddChild(GUIElement * el)
{
	el->parent = this;
	this->childs.push_back(el);

	if (this->visible == false)
	{
		el->SetVisible(false);
	}

}

void GUIElement::SetMouseMove(int x, int y)
{
	if (this->onMove.empty() == false)
	{
		float relX = x - this->proportions.topLeft.X;
		float relY = y - this->proportions.topLeft.Y;

		//trigger action for "mouse move"
		this->onMove(this, relX, relY);
	}
}

/*-----------------------------------------------------------
Function:	CalculateProportions
Paramaters:
	[in] parentProportions - proportions of parent object

Calculate real size of object based on set uped
dimension and position
Recursively called for each child of element
Proportions are calculated in pixels
-------------------------------------------------------------*/
void GUIElement::CalculateProportions(const ElementProportions & parentProportions)
{

	if (this->needRebaked)
	{
		float x = parentProportions.topLeft.X;
		x += this->pos.x * parentProportions.width;
		x += this->pos.offsetX;

		float y = parentProportions.topLeft.Y;
		y += this->pos.y * parentProportions.height;
		y += this->pos.offsetY;

		float w = parentProportions.width;
		w *= this->dim.w;
		w += this->dim.pixelW;

		float h = parentProportions.height;
		h *= this->dim.h;
		h += this->dim.pixelH;

		if (this->dim.AR != 0.0f)
		{
			h = (1.0f / this->dim.AR) * w;
		}

		if ((this->max.pixelW > 0) && (w > this->max.pixelW))
		{
			w = static_cast<float>(this->max.pixelW);
		}

		if ((this->max.pixelH > 0) && (h > this->max.pixelH))
		{
			h = static_cast<float>(this->max.pixelH);
		}

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

		//this will overflow from parent element
		this->proportions.topLeft = MyMath::Vector2(x, y);
		this->proportions.botRight = MyMath::Vector2(x + w, y + h);
		this->proportions.width = w;
		this->proportions.height = h;
		this->proportions.depth = parentProportions.depth + 0.01f;

		if (this->GetTextCaption() == NULL)
		{
			if (this->GetTextPanel() == NULL)
			{
				this->needRebaked = false;
			}
		}
	}

	std::vector<GUIElement *>::iterator it;
	for (it = this->childs.begin(); it != this->childs.end(); it++)
	{
		(*it)->CalculateProportions(this->proportions);
	}
}