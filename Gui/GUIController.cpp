#include "GUIController.h"

#include "GUIElement.h"
#include "GUIButton.h"
#include "GUICheckBox.h"
#include "GUIScreen.h"
#include "GUIStructures.h"

#include "Matrix.h"

#include "IControls.h"

#include <climits>

using namespace GUISystem;

GUIController::GUIController(GUIScreen * sc, IControls * control)
{
	this->screen = sc;
	this->control = control;

	for (int i = 0; i < this->control->GetControlsPointsCount(); i++)
	{
		this->lastMouseOverEls.push_back(NULL);
	}
}

GUIController::~GUIController()
{
}

/*-----------------------------------------------------------
Function:	IsMouseOver (static)
Parametrs:
	[in] sc - base screen with element
	[in] el - element itself
	[in] x - x mouse coordinate in pixels
	[in] y - y mouse coordinate in pixels
Returns:
	most top element with mouse over

Test if point with pixel [x,y]  is within element at screen
-------------------------------------------------------------*/
bool GUIController::IsMouseOver(GUIScreen * sc, GUIElement * el, int x, int y)
{
	if (el->IsVisible() == false)
	{
		return false;
	}


	const ElementProportions & s = sc->GetProportions();
	const ElementProportions & p = el->GetProportions();

	//NDC is in [-1,1] x [-1,1] x [according to used API]
	float startX = MyMath::MyMathUtils::MapRange(0, 1, -1, 1, p.topLeft.X / s.width);
	float startY = MyMath::MyMathUtils::MapRange(0, 1, -1, 1, 1.0f - p.topLeft.Y / s.height);

	float endX = MyMath::MyMathUtils::MapRange(0, 1, -1, 1, p.botRight.X / s.width);
	float endY = MyMath::MyMathUtils::MapRange(0, 1, -1, 1, 1.0f - p.botRight.Y / s.height);



	float centerX = s.width  * 0.5f;
	float centerY = s.height * 0.5f;

	float mouseX =  2.0f * ((x - centerX) / s.width);
	float mouseY = -2.0f * ((y - centerY) / s.height);



	//http://www.emanueleferonato.com/2012/03/09/algorithm-to-determine-if-a-point-is-inside-a-square-with-mathematics-no-hit-test-involved/

	if (mouseX > endX) return false;
	if (mouseX < startX) return false;

	if (mouseY < endY) return false;
	if (mouseY > startY) return false;


	return true;

}

/*-----------------------------------------------------------
Function:	GetMouseOverElement
Returns:
	most top element with mouse over

Get the most top element that has mouse over it
Text captions are gnored
-------------------------------------------------------------*/
GUIElement * GUIController::GetMouseOverElement() const
{
	int x, y;
	this->control->GetPos(x, y);

    if ((x == INT_MAX) && (y == INT_MAX))
    {
        return NULL;
    }

	GUIElement * mouseOver = NULL;
	float maxDepth = -999999;

	std::vector<GUIElement *> & els = this->screen->GetElements();

	for (uint32 i = 0; i < els.size(); i++)
	{
		if (els[i]->GetTextCaption() != NULL)
		{
			//ignore text captions
			continue;
		}

		if (GUIController::IsMouseOver(this->screen, els[i], x, y))
		{
			if (els[i]->GetProportions().depth  >= maxDepth)
			{
				mouseOver = els[i];
				maxDepth = els[i]->GetProportions().depth;
			}
		}


	}

	if (mouseOver != NULL)
	{
		mouseOver->SetMouseMove(x, y);
	}

	return mouseOver;
}


void GUIController::UpdateElements()
{
	int count = this->control->GetControlsPointsCount();
	int active = this->control->GetActiveControlPoint(); //backup actual active point


	for (int i = 0; i < count; i++)
	{
		this->UpdateElementsFromControlPoint(i);
	}

	this->control->SetActiveControlPoint(active); //restore active point


}

void GUIController::UpdateElementsFromControlPoint(int index)
{
	this->control->SetActiveControlPoint(index);


	GUIElement * act = NULL;
	GUIElement * last = this->lastMouseOverEls[index];

    act = this->GetMouseOverElement();

	this->LastActDifferent(last, act);

	this->LastActSame(last, act);




	this->lastMouseOverEls[index] = act;



}



/*-----------------------------------------------------------
Function:	LastActDifferent
Parametrs:
	[in] last - last active element
	[in] act - actual active element

If last and currently active elements are different,
update last and current elements states accordingly
-------------------------------------------------------------*/
void GUIController::LastActDifferent(GUIElement * last, GUIElement * act)
{
	if (last == act)
	{
		//both are same - do nothing with last
		return;
	}

    //last and actual element differs

	if (last != NULL)
	{
        if (last->GetButton() != NULL)
        {
             //last time we were over button
             //now we are with point click over different element,
             //than last time
             // => unhover last element (button)

             this->UpdateGUIButtonState(CONTROL_NONE, last->GetButton());
        }

        if (last->GetCheckBox() != NULL)
        {
            //last time we were over check box
            //now we are with point click over different element,
            //than last time
            // => unhover last element (button)
            this->UpdateGUICheckBoxState(CONTROL_NONE, last->GetCheckBox());
        }

	}


	if (this->control->IsPressed())
	{
		//if control is down, do not trigger state change for mouse over
		return;
	}



	if (act != NULL)
	{
		if (act->GetButton() != NULL)
		{
			//we are over new button with mouse
			// => hover it
			this->UpdateGUIButtonState(CONTROL_OVER, act->GetButton());
		}

		if (act->GetCheckBox() != NULL)
		{
			//we are over new check box with mouse
			// => hover it

			this->UpdateGUICheckBoxState(CONTROL_OVER, act->GetCheckBox());
		}
	}


}

/*-----------------------------------------------------------
Function:	LastActSame
Parametrs:
	[in] last - last active element
	[in] act - actual active element

If last and currently active elements are same,
update last and current elements states accordingly
-------------------------------------------------------------*/
void GUIController::LastActSame(GUIElement * last, GUIElement * act)
{

	if (last != act)
	{
		//last and actual elemnts are not the same - can not
		//perform clicking on it
		return;
	}

	if (act == NULL)
	{
		//no selected element
		// - no clicking on it
		return;
	}


	//====================================================

	if (this->control->IsPressed())
	{
		if (act->GetButton() != NULL)
		{
			this->UpdateGUIButtonState(CONTROL_CLICK, act->GetButton());
		}

		if (act->GetCheckBox() != NULL)
		{
			this->UpdateGUICheckBoxState(CONTROL_CLICK, act->GetCheckBox());
		}
	}

	//====================================================

	if (this->control->IsReleased())
	{
		if (act->GetButton() != NULL)
		{
			this->UpdateGUIButtonState(CONTROL_OVER, act->GetButton());
		}

		if (act->GetCheckBox() != NULL)
		{
			this->UpdateGUICheckBoxState(CONTROL_OVER, act->GetCheckBox());
		}
	}

	//====================================================
}

/*-----------------------------------------------------------
Function:	UpdateGUIButtonState
Parametrs:
	[in] ctrl - control state
	[in] btn - button to be updated

Update button state based on ControlState
State loop:
NON_ACTIVE - button has no focus
HOVERED - focus
CLICKED - focus + clicked (eg. mouse button)
NON_ACTIVE -
-------------------------------------------------------------*/
void GUIController::UpdateGUIButtonState(ControlState ctrl, GUIButton * btn)
{

	//PROBLEM:
	//dotykove ovladani - co kdyz budou dva prsty na jednom tlacitku
	//a jeden se zvedne a druhy zustane
	//to mi vyvola onRelease akci.. ale ta by mela nastat az kdyz zadny prst nebude aktivni

    //touch control has no CONTROL_OVER state !

	if (ctrl == CONTROL_OVER) //element has focus from mouse
	{
        #ifdef __APPLE__
            //touch control has no CONTROL_OVER state !
            //CONTROL_OVER => element has been touched => CONTROL_CLICK
            //convert it to CONTROL_CLICK
            ctrl = CONTROL_CLICK;
        #else
            //should not occur for touch control
            if (btn->GetState() == BTN_STATE_CLICKED) //last was clicked
            {
                btn->SetState(BTN_STATE_NON_ACTIVE); //trigger actions for onRelease
                btn->SetState(BTN_STATE_OVER); //hover it - mouse stays on top of element after click
            }
            else
            {
                btn->SetState(BTN_STATE_OVER); //hover element
            }
        #endif
	}


	if (ctrl == CONTROL_CLICK) //element has focus from mouse and we have touched mouse button
	{
		btn->SetState(BTN_STATE_CLICKED); //trigger actions for onClick
	}


	if (ctrl == CONTROL_NONE) //element has no mouse focus
	{
        #ifndef __APPLE__
            btn->SetState(BTN_STATE_OVER); //deactivate (on over)
        #endif

        // #ifdef __APPLE__
        if (this->control->IsPressed())
        {
            btn->SetState(BTN_STATE_DUMMY); //deactivate
        }

		btn->SetState(BTN_STATE_NON_ACTIVE); //deactivate
	}
}



/*-----------------------------------------------------------
Function:	UpdateGUICheckBoxState
Parametrs:
	[in] ctrl - control state
	[in] chck - check box to be updated

Update check box state based on ControlState
State loop:
NON_CHECKED
HOVERED
CLICKED
CHECKED
CHECKED_HOVER
CHECKED_CLICKED
NON_CHECKED
-------------------------------------------------------------*/
void GUIController::UpdateGUICheckBoxState(ControlState ctrl, GUICheckBox * chck)
{
	if (ctrl == CONTROL_OVER) //element has focus from mouse
	{
		if (chck->GetState() == CHCK_STATE_CLICKED)
		{
			chck->SetState(CHCK_STATE_CHECKED); //trigger actions for onCheck
		}
		else if (chck->GetState() == CHCK_STATE_CHECKED_CLICKED)
		{
			chck->SetState(CHCK_STATE_NON_CHECKED); //trigger actions for onUnCheck
			#if defined(_WIN32) || defined(TARGET_COMPUTER)
				chck->SetState(CHCK_STATE_HOVERED);
			#endif
		}

		else if (chck->GetState() == CHCK_STATE_CHECKED)
		{
			chck->SetState(CHCK_STATE_CHECKED_HOVERED);
		}
		else if (chck->GetState() == CHCK_STATE_CHECKED_HOVERED)
		{
			chck->SetState(CHCK_STATE_CHECKED_HOVERED);
		}
		else
		{
            #ifdef __APPLE__
                ctrl = CONTROL_CLICK; //click element with touch control
            #else
                chck->SetState(CHCK_STATE_HOVERED); //hover element
            #endif
		}
	}


	if (ctrl == CONTROL_CLICK) //element has focus from mouse and we have touched mouse button
	{
        #ifdef __APPLE__
            //touch control - emulate "hover" on first touch
            if (chck->GetState() == CHCK_STATE_NON_CHECKED)
            {
                chck->SetState(CHCK_STATE_HOVERED); //hover element
            }
        #endif

		if (chck->GetState() == CHCK_STATE_CHECKED)
		{
			chck->SetState(CHCK_STATE_CHECKED_CLICKED); //trigger actions for onClick
		}
		if (chck->GetState() == CHCK_STATE_CHECKED_HOVERED)
		{
			chck->SetState(CHCK_STATE_CHECKED_CLICKED); //trigger actions for onClick
		}
		else if (chck->GetState() == CHCK_STATE_CHECKED_CLICKED)
		{
			chck->SetState(CHCK_STATE_CHECKED_CLICKED); //trigger actions for onClick
		}
		else
		{
			chck->SetState(CHCK_STATE_CLICKED); //trigger actions for onClick
		}

	}


	if (ctrl == CONTROL_NONE) //element has no mouse focus
	{
		if (chck->GetState() == CHCK_STATE_CHECKED_HOVERED)
		{
			chck->SetState(CHCK_STATE_CHECKED);
		}
		else if (chck->GetState() == CHCK_STATE_CHECKED_CLICKED)
		{
			chck->SetState(CHCK_STATE_CHECKED);
		}
		else
		{
			chck->SetState(CHCK_STATE_HOVERED); //deactivate
			chck->SetState(CHCK_STATE_NON_CHECKED); //deactivate
		}
	}
}