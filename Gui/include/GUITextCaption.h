#ifndef GUI_SYSTEM_TEXT_CAPTION_H
#define GUI_SYSTEM_TEXT_CAPTION_H

namespace GUISystem {
class GUIFontRenderer;
}

#include <string>

#include "GUIElement.h"
#include "GUIStructures.h"

namespace GUISystem {

class GUITextCaption : public GUIElement {
public:
    GUITextCaption(const std::string& name);
    ~GUITextCaption();

    virtual GUITextCaption* GetTextCaption();

    virtual void SetCanBaBaked(bool val);

    const std::string& GetActiveTextureName() const;
    const std::string& GetText() const;
    const std::string& GetFontFace() const;
    int GetFontSize() const;
    float GetRotationAngle() const;
    float GetFontPercentSize() const;

    void SetFontFace(const std::string& fontFace);
    void SetText(const std::string& text);
    void SetFontSize(float size);
    void SetRotationAngle(float angle);

    void Update(const ElementProportions& parentProportions, GUIFontRenderer* fontRenderer);

protected:
    std::string caption;
    std::string fontFace;
    int fontSize;
    float percentSize;

    float angle;

    std::string fontAtlasName;

private:
};
}

#endif