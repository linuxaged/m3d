#ifndef GUI_SYSTEM_PANEL_H
#define GUI_SYSTEM_PANEL_H


#include "../../Macros.h"
#include <string>

#include "GUIStructures.h"
#include "GUIElement.h"

namespace GUISystem
{

	typedef struct GUIPanelTextures
	{
		std::string textureName;
		std::string textureNameHover;
		std::string textureNameClicked;
		std::string textureNameChecked;
		std::string textureNameCheckedHover;

		const std::string & Get(int i) const
		{
			return this->textureName;
		};

	} GUIPanelTextures;

	class GUIPanel : public GUIElement
	{
		public:
			GUIPanel(const std::string & name);
			~GUIPanel();

			virtual GUIPanel * GetPanel();

			const std::string & GetActiveTextureName() const;
			const GUIPanelTextures & GetTextures() const;



			void SetTextures(GUIPanelTextures & textures);



		protected:
			GUIPanelTextures textures;



		private:

	};

}


#endif