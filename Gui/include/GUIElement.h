#ifndef GUI_SYSTEM_ELEMENT_H
#define GUI_SYSTEM_ELEMENT_H

namespace GUISystem {
class GUIScreen;
class GUIBakery;
class GUIController;

class GUIPanel;
class GUIImage;
class GUIButton;
class GUICheckBox;
class GUITextCaption;
class GUITextPanel;
}

#include <vector>

#include "GUIEvents.h"
#include "Matrix.h"
#include <string>

#include "GUIStructures.h"

namespace GUISystem {

class GUIElement {
public:
    GUIElement(const std::string& name);
    virtual ~GUIElement();

    virtual GUIPanel* GetPanel();
    virtual GUIImage* GetImage();
    virtual GUIButton* GetButton();
    virtual GUICheckBox* GetCheckBox();
    virtual GUITextCaption* GetTextCaption();
    virtual GUITextPanel* GetTextPanel();

    virtual const std::string& GetActiveTextureName() const = 0;

    const std::string& GetName() const;
    const std::vector<GUIElement*>* GetChildrens() const;
    bool HasChilds() const;

    bool IsVisible() const;
    bool IsBaked() const;

    bool CanBeBaked() const;
    virtual void SetCanBaBaked(bool val);
    const m3d::math::Vector4& GetColor() const;

    void SetVisible(bool val);

    IUserData* GetUserData() const;
    void SetUserData(IUserData* data);

    const ElementProportions& GetProportions() const;
    void SetPosition(ElementPos pos);
    void SetSize(ElementDim dim);
    void SetMaxSize(int w, int h);
    void SetColor(int r, int g, int b, int a);

    void SetOnMoveCallback(OnMoveDelegate onMoveDelegate);

    void AddChild(GUIElement* el);

    friend class GUISystem::GUIScreen;
    friend class GUISystem::GUIBakery;
    friend class GUISystem::GUIController;

protected:
    std::string name;

    ElementPos pos;
    ElementDim dim;
    ElementDimMax max;

    ElementProportions proportions;

    m3d::math::Vector4 color;

    IUserData* userData;

    OnMoveDelegate onMove;

    bool visible;
    bool canBeBaked;
    bool needRebaked;

    GUIElement* parent;
    std::vector<GUIElement*> childs;

private:
    bool baked;

    void SetMouseMove(int x, int y);
    void CalculateProportions(const ElementProportions& parentProportions);
};
}

#endif