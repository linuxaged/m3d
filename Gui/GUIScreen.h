#pragma once

namespace GUISystem
{
	class GUIScreen
	{
	public:
		const ElementProportions&		GetProportions();
		std::vector<GUIElement *>&		GetElements();
	};
}