#ifndef GUI_SYSTEM_STRUCTURES_H
#define GUI_SYSTEM_STRUCTURES_H

namespace GUISystem
{
	class GUIElement;
}

namespace MyGraphics
{
	class G_GraphicsObject;
}

#include "Matrix.h"

#include "Delegate.h"

namespace GUISystem
{

	typedef enum ORIGIN
	{
		TL = 0,
		TR = 1,
		BL = 2,
		BR = 3,
		C = 4

	} ORIGIN;

	//Position of element
	//(0, 0, 100, 200) - element at position [100, 200]
	//(0.5, 0.6, 100, 200) - element at position [50% of screen_w + 100, 60% of screen_h + 200]
	typedef struct ElementPos
	{
		ElementPos() : x(0.0f), y(0.0f), offsetX(0), offsetY(0), origin(TL) {};
		ElementPos(float x, float y) : x(x), y(y), offsetX(0), offsetY(0), origin(TL)  {};
		ElementPos(float x, float y, int offsetX, int offsetY) : x(x), y(y), offsetX(offsetX), offsetY(offsetY), origin(TL)  {};

		float x;	//relative X value in %
		float y;    //relative Y value in %

		int offsetX;	//absolute X offset from relative position in pixels
		int offsetY;    //absolute Y offset from relative position in pixels

		ORIGIN origin; //origin, where point [0,0] is

	} ElementPos;


	//Dimension of slement
	//(0.1, 0.1, 0, 0) - 10% of parent size in w, 10% of parent size in h
	//(0.1, 0.0, 0, 0, 4/3) - 10% of parent size in w, 10% * 4/3 of parent size in h
	typedef struct ElementDim
	{
		ElementDim() : w(0.0f), h(0.0f), pixelW(0), pixelH(0), AR(0.0) {};
		ElementDim(float w, float h, int pixelW, int pixelH) : w(w), h(h), pixelW(pixelW), pixelH(pixelH), AR(0.0f) {};
		ElementDim(float w, int pixelW, float AR) : w(w), h(0), pixelW(pixelW), pixelH(0), AR(AR) {};

		float w;
		float h;

		int pixelW;
		int pixelH;

		float AR; //aspect ratio - if set (different value than 1.0), h and pixelH is ignored
				  //and recalculated from w and pixelW to keep AR

	} ElementDim;

	//Maximal pixel size of element
	typedef struct ElementDimMax
	{
		int pixelW;
		int pixelH;

	} ElementDimMax;


	//Real size of elements in current resolution and screen size
	typedef struct ElementProportions
	{
		M3D::Math::Vector2 topLeft;
		M3D::Math::Vector2 botRight;

		float width;
		float height;
		float depth;

	} ElementProportions;


	//Baked GUI informations
	//render all baked objects at once
	typedef struct BakedGUIScreen
	{
		static const int BAKED_SIZE = 64;

		std::string textureName;

		MyGraphics::G_GraphicsObject * bakedGUI;
		int bakedStates[BakedGUIScreen::BAKED_SIZE];

	} BakedGUIScreen;

	//non baked GUI elements - each element has its own graphics
	//each object is rendere by single draw call
	typedef struct RawGUIScreen
	{
		std::vector<GUIElement *> el;
		std::vector<MyGraphics::G_GraphicsObject *> g;

	} RawGUIScreen;

	//States of button
	typedef enum ButtonElementState
	{
		BTN_STATE_NON_ACTIVE = 0,
		BTN_STATE_OVER = 1,
		BTN_STATE_CLICKED = 2,

        BTN_STATE_DUMMY = 3 //dummy state used for touch controls or disabling state
                            //equivalent to NON_ACTIVE state

	} ButtonElementState;

	//States of check box
	typedef enum CheckBoxElementState
	{
		CHCK_STATE_NON_CHECKED = 0,
		CHCK_STATE_HOVERED = 1,
		CHCK_STATE_CLICKED = 2,
		CHCK_STATE_CHECKED = 3,
		CHCK_STATE_CHECKED_HOVERED = 4,
		CHCK_STATE_CHECKED_CLICKED = 5,

        CHCK_STATE_DUMMY = 6 //dummy state used for touch controls or disabling state
                             //equivalent to NON_CHECKED state


	} CheckBoxElementState;


	//Control state (eg. mouse)
	typedef enum ControlState
	{
		CONTROL_NONE = 0, //nothing active
		CONTROL_OVER = 1, //only "mouse over"
		CONTROL_CLICK = 2 //control has been pressed ("mouse button down")

	} ControlState;


	//Trigger action for click - control down / control up
	//on element
	typedef delegate<void(GUIElement *)> OnClickDelegate;

	//Trigger state change from control
	typedef delegate<void(GUIElement *)> OnOverDelegate;
	typedef delegate<void(GUIElement *)> OnDownDelegate;
	typedef delegate<void(GUIElement *)> OnUpDelegate;
	typedef delegate<void(GUIElement *)> OnStateChangeDelegate;

	//Trigger repeated actions
	//While state is not changed, call trigger
	typedef delegate<void(GUIElement *)> WhileDownDelegate;
	typedef delegate<void(GUIElement *)> WhileHoverDelegate;

	//Universal triggers for movements
	typedef delegate<void(GUIElement *, float, float)> OnMoveDelegate;


	struct IUserData
	{
		IUserData() : refCount(0) {};
		virtual ~IUserData() {};

		friend class GUISystem::GUIElement;

		private:
			int refCount;
    };

}


#endif