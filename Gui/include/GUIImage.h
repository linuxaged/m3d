#ifndef GUI_SYSTEM_IMAGE_H
#define GUI_SYSTEM_IMAGE_H

#include <string>

#include "GUIElement.h"
#include "GUIStructures.h"

namespace GUISystem {

class GUIImage : public GUIElement {
public:
    GUIImage(const std::string& name);
    ~GUIImage();

    virtual GUIImage* GetImage();

    virtual void SetCanBaBaked(bool val);

    const std::string& GetActiveTextureName() const;

    void SetTextureName(const std::string& name);

protected:
    std::string textureName;

private:
};
}

#endif