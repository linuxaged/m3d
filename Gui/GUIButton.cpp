#include "GUIButton.h"
using namespace GUISystem;

GUIButton::GUIButton(const std::string & name) : GUIElement(name)
{
	this->actState = BTN_STATE_NON_ACTIVE;
	this->hasBeenDown = false;
}

GUIButton::~GUIButton()
{

}


GUIButton * GUIButton::GetButton()
{
	return this;
}

ButtonElementState GUIButton::GetState() const
{
	return this->actState;
}

/*-----------------------------------------------------------
Function:	GetActiveTextureName
Return:
	active texture name for current state

Get texture that should be renderer on button with
current active state
-------------------------------------------------------------*/
const std::string & GUIButton::GetActiveTextureName() const
{
	if (this->actState == BTN_STATE_CLICKED)
	{
		return this->textures.textureNameClicked;
	}
	else if (this->actState == BTN_STATE_OVER)
	{
		return this->textures.textureNameHover;
	}
	else
	{
		return this->textures.textureName;
	}
}

/*-----------------------------------------------------------
Function:	GetTextures
Return:
	struct of texture name

Get all textures set to button
-------------------------------------------------------------*/
const GUIButtonTextures & GUIButton::GetTextures() const
{
	return this->textures;
}


void GUIButton::SetTextures(GUIButtonTextures & textures)
{
	this->textures = textures;

	//we need to "rebake" it, because new textures can be in atlas
	//and we need new positions
	this->needRebaked = true;
}

void GUIButton::AddJoinedButton(GUIButton * btn)
{
	this->joined.push_back(btn);
}

void GUIButton::SetOnClickCallback(OnClickDelegate onClickDelegate)
{
	this->delegates.onClick = onClickDelegate;
}

void GUIButton::SetOnDownCallback(OnDownDelegate onDownDelegate)
{
	this->delegates.onDown = onDownDelegate;
}

void GUIButton::SetOnOverCallback(OnOverDelegate onOverDelegate)
{
	this->delegates.onOver = onOverDelegate;
}

void GUIButton::SetOnUpCallback(OnUpDelegate onUpDelegate)
{
	this->delegates.onUp = onUpDelegate;
}

void GUIButton::SetWhileDownCallback(WhileDownDelegate whileDownDelegate)
{
	this->delegates.whileDown = whileDownDelegate;
}

void GUIButton::SetWhileHoverCallback(WhileHoverDelegate whileHoverDelegate)
{
	this->delegates.whileHover = whileHoverDelegate;
}

void GUIButton::SetOnStateChangeCallback(OnStateChangeDelegate onStateChange)
{
	this->delegates.onStateChange = onStateChange;
}

void GUIButton::SetState(ButtonElementState newState)
{

	for (uint32_t i = 0; i < this->joined.size(); i++)
	{
		this->joined[i]->SetStateJoined(newState);
	}

	this->SetStateJoined(newState);


}

void GUIButton::SetStateJoined(ButtonElementState newState)
{

	if (this->actState == newState)
	{
		//call repeat triggers
		if ((this->hasBeenDown) && (this->actState == BTN_STATE_CLICKED))
		{
			//printf("whileDown button (x) ");
			// TODO:
			if (this->delegates.whileDown.empty() == false)
			{
				this->delegates.whileDown(this);
			}
		}

		if (this->actState == BTN_STATE_OVER)
		{
			//printf("whileHover button (x) ");
			// TODO:
			if (this->delegates.whileHover.empty() == false)
			{
				this->delegates.whileHover(this);
			}
		}


		return;
	}

    if (newState == BTN_STATE_DUMMY)
    {
        //dummy state to "erase" safely states without trigger
        //delegates associated with action
        //dummy = NON_ACTIVE state
        this->actState = BTN_STATE_NON_ACTIVE;
        return;
    }
	// TODO:
	if (this->delegates.onStateChange.empty() == false)
	{
		//trigger action for state change
		this->delegates.onStateChange(this);
	}


    //was not active => not mouse over
	if ((this->actState == BTN_STATE_NON_ACTIVE) && (newState == BTN_STATE_OVER))
	{
		//printf("onHover button \n");
		if (this->delegates.onOver.empty() == false)
		{
			this->delegates.onOver(this);
		}
        if (this->delegates.whileHover.empty() == false)
        {
            this->delegates.whileHover(this);
        }
	}

	//was clicked => now non active
	if ((this->actState == BTN_STATE_CLICKED) && (newState == BTN_STATE_NON_ACTIVE))
	{
        if (this->hasBeenDown)
        {
           // printf("onClick button \n");
            if (this->delegates.onClick.empty() == false)
            {
                this->delegates.onClick(this);
            }
        }
        else
        {
           // printf("onUp button \n");
            if (this->delegates.onUp.empty() == false)
            {
                this->delegates.onUp(this);
            }
        }
	}



#ifdef __APPLE__
    //no hover state on touch control
    //go directly from NON_ACTIVE to CLICKED
    if ((this->actState == BTN_STATE_NON_ACTIVE) && (newState == BTN_STATE_CLICKED))
#else
    //go from mouse OVER state to CLICKED
	if ((this->actState == BTN_STATE_OVER) && (newState == BTN_STATE_CLICKED))
#endif
	{
		this->hasBeenDown = true;
		//printf("onDown button \n");
		if (this->delegates.onDown.empty() == false)
		{
			this->delegates.onDown(this);
		}
        if (this->delegates.whileDown.empty() == false)
        {
            this->delegates.whileDown(this);
        }
	}
	else
    {
        this->hasBeenDown = false;
    }


	this->actState = newState;



}