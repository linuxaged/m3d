#ifndef GUI_SYSTEM_CHECK_BOX_H
#define GUI_SYSTEM_CHECK_BOX_H


#include "../../Macros.h"
#include "../../Utils/header/MyString.h"

#include "./GUIStructures.h"
#include "./GUIElement.h"

namespace GUISystem
{

	typedef struct GUICheckBoxTextures
	{
		std::string textureName;
		std::string textureNameHover;
		std::string textureNameClicked;
		std::string textureNameChecked;
		std::string textureNameCheckedHover;

		const std::string & Get(int i) const
		{
			if (i == 0) return this->textureName;
			else if (i == 1) return this->textureNameHover;
			else if (i == 2) return this->textureNameClicked;
			else if (i == 3) return this->textureNameChecked;
			else return this->textureNameCheckedHover;
		};

	} GUICheckBoxTextures;

	class GUICheckBox : public GUIElement
	{
		public:
			GUICheckBox(const std::string & name);
			~GUICheckBox();

			virtual GUICheckBox * GetCheckBox();

			CheckBoxElementState GetState() const;

			const std::string & GetActiveTextureName() const;
			const GUICheckBoxTextures & GetTextures() const;

			void SetTextures(GUICheckBoxTextures & textures);

			void SetState(CheckBoxElementState state);

		protected:

			GUICheckBoxTextures textures;

			CheckBoxElementState state;

		private:

	};

}


#endif