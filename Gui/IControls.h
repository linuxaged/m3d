#pragma once

namespace GUISystem
{
	class IControls
	{
	protected:
		IControls();

	public:
		virtual				~IControls();
		// forbit copying
		IControls(IControls const&) = delete;
		IControls& operator=(IControls const&) = delete;

	//private:
		virtual int 		GetControlsPointsCount() = 0;
		virtual void 		GetPos(int& x, int& y) = 0;
		virtual int 		GetActiveControlPoint() = 0;
		virtual void 		SetActiveControlPoint(int active) = 0;

		virtual bool 		IsPressed() = 0;
		virtual bool 		IsReleased() = 0;

	
	};
}