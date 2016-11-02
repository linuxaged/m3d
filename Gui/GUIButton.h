#ifndef GUI_SYSTEM_BUTTON_H
#define GUI_SYSTEM_BUTTON_H

#include "../../Macros.h"
#include <string>
#include "../../Utils/header/FastDelegate.h"

#include "./GUIStructures.h"
#include "./GUIElement.h"

namespace GUISystem
{
	typedef struct GUIButtonTextures
	{
		std::string textureName;
		std::string textureNameClicked;
		std::string textureNameHover;

		const std::string & Get(int i) const
		{
			if (i == 0) return this->textureName;
			else if (i == 1) return this->textureNameClicked;
			else return this->textureNameHover;
		};


	} GUIButtonTextures;

	typedef struct GUIButtonDelegates
	{
		OnClickDelegate onClick;
		OnDownDelegate onDown;
		OnUpDelegate onUp;
		OnOverDelegate onOver;
		OnStateChangeDelegate onStateChange;

		WhileDownDelegate whileDown;
		WhileHoverDelegate whileHover;

    } GUIButtonDelegates;


	class GUIButton : public GUIElement
	{
		public:
			GUIButton(const std::string & name);
			~GUIButton();

			virtual GUIButton * GetButton();

			ButtonElementState GetState() const;

			const std::string & GetActiveTextureName() const;
			const GUIButtonTextures & GetTextures() const;

			void SetTextures(GUIButtonTextures & textures);

			void SetOnClickCallback(OnClickDelegate onClickDelegate);
			void SetOnDownCallback(OnDownDelegate onDownDelegate);
			void SetOnUpCallback(OnUpDelegate onUpDelegate);
			void SetOnOverCallback(OnOverDelegate onOverDelegate);
			void SetOnStateChangeCallback(OnStateChangeDelegate onStateChange);

			void SetWhileDownCallback(WhileDownDelegate whileDownDelegate);
			void SetWhileHoverCallback(WhileHoverDelegate whileHoverDelegate);

			void SetState(ButtonElementState state);

			void AddJoinedButton(GUIButton * btn);

		protected:
			GUIButtonTextures textures;

			GUIButtonDelegates delegates;

			ButtonElementState actState;

		private:
			bool hasBeenDown;
			std::vector<GUIButton *> joined;

			void SetStateJoined(ButtonElementState state);

	};

}


#endif