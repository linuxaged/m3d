#ifndef GUI_SYSTEM_CONTROLLER_H
#define GUI_SYSTEM_CONTROLLER_H

namespace GUISystem {
class IControls;
class GUIElement;
class GUIScreen;
class GUIButton;
class GUICheckBox;
}

#include <string>

#include "./GUIStructures.h"

namespace GUISystem {

class GUIController {
public:
    GUIController(GUIScreen* sc, IControls* control);
    ~GUIController();

    static bool IsMouseOver(GUIScreen* sc, GUIElement* el, int x, int y);

    void UpdateElements();
    GUIElement* GetMouseOverElement() const;

protected:
    GUIScreen* screen;
    IControls* control;

private:
    std::vector<GUIElement*> lastMouseOverEls;

    void UpdateElementsFromControlPoint(int index);

    void LastActSame(GUIElement* last, GUIElement* act);
    void LastActDifferent(GUIElement* last, GUIElement* act);

    void UpdateGUIButtonState(ControlState ctrl, GUIButton* btn);
    void UpdateGUICheckBoxState(ControlState ctrl, GUICheckBox* btn);
};
}

#endif