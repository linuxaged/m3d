#include "GUICheckBox.h"

using namespace GUISystem;

GUICheckBox::GUICheckBox(const std::string& name)
    : GUIElement(name)
{
    this->state = CHCK_STATE_NON_CHECKED;
}

GUICheckBox::~GUICheckBox()
{
}

GUICheckBox* GUICheckBox::GetCheckBox()
{
    return this;
}

CheckBoxElementState GUICheckBox::GetState() const
{
    return this->state;
}

const std::string& GUICheckBox::GetActiveTextureName() const
{
    if (this->state == CHCK_STATE_CHECKED) {
        return this->textures.textureNameChecked;
    } else if (this->state == CHCK_STATE_CLICKED) {
        return this->textures.textureNameClicked;
    } else if (this->state == CHCK_STATE_HOVERED) {
        return this->textures.textureNameHover;
    } else if (this->state == CHCK_STATE_CHECKED_HOVERED) {
        return this->textures.textureNameCheckedHover;
    } else if (this->state == CHCK_STATE_CHECKED_CLICKED) {
        return this->textures.textureNameCheckedHover;
    } else {
        return this->textures.textureName;
    }
}

const GUICheckBoxTextures& GUICheckBox::GetTextures() const
{
    return this->textures;
}

void GUICheckBox::SetTextures(GUICheckBoxTextures& textures)
{
    this->textures = textures;
}

void GUICheckBox::SetState(CheckBoxElementState newState)
{
    if (this->state == newState) {
        return;
    }

    if ((newState == CHCK_STATE_CHECKED) && (this->state == CHCK_STATE_CLICKED)) {
        printf("checkbox onCheck");
    }

    if ((newState == CHCK_STATE_NON_CHECKED) && (this->state == CHCK_STATE_CHECKED_CLICKED)) {
        printf("checkbox onUnCheck");
    }

    this->state = newState;
}