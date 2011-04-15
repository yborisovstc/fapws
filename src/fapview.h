#ifndef __FAP_VIEW_H
#define __FAP_VIEW_H

// View interface

#include "fapplat.h"
#include <string.h>
#include <string>
#include "fapviewprim.h"

using namespace std;

class MAE_Window;
class MAE_ViewObserver;
;

class MAE_View
{
    public:
	enum TType {
	    Et_Base,
	    Et_Regular
	};
    public:
	virtual TType Type() = 0;
	virtual const string& Name() const = 0;
	virtual void SetName(const string& aName) = 0;
	virtual MAE_Window* Wnd() = 0;
	virtual MAE_Window* CreateRootWnd() = 0;
	virtual void SetDetLevel(TInt aLevel) = 0;
	virtual TInt DetLevel() = 0;
};

class CAE_Color
{
    public:
	CAE_Color(TUint32 aValue, TInt aRed, TInt aGreen, TInt aBlue): iValue(aValue), iRed(aRed), iGreen(aGreen), iBlue(aBlue) {};
    public:
	TUint32 iValue;
	TInt iRed;
	TInt iGreen;
	TInt iBlue;
};

class MAE_Gc;
class MAE_Window
{
    public: 
	class Attr 
	{
	    public:
	    CAV_Rect iRect;
	};
	enum TState {
	    ESt_Normal,
	    ESt_Active, // active window, such as a depressed button
	    ESt_Prelight, // pointer is over the window and the window will respond to mouse clicks
	    ESt_Selected, // selected item, such the selected row in a list
	    ESt_Insens, // unresponsive to user actions
	};
	enum TGcType {
	    EGt_Cur,  // Current 
	    EGt_Fg,   // Predefined (from style), foreground
	    EGt_Bg   // Predefined (from style), background
	};
    public:
	virtual const MAE_View* View() = 0;
	virtual void Destroy() = 0;
	// TODO [YB] Actually Rect returns size, so it is not that rect that is set by SetRect, consider renaming to avoid confusion
	virtual CAV_Rect Rect() = 0;
	virtual void SetRect(const CAV_Rect& aRect) = 0;
	// Sets preferred size. (-1, -1) means the size is undefined
	virtual void SetPrefSize(const CAV_Rect& aRect) = 0;
	virtual CAV_Rect GetPrefSize() = 0;
	// Calculate preferred size, uses pref size set explicitly. If it is undefined that requests observer via OnPrefSizeRequested
	virtual CAV_Rect CalcPrefSize() = 0;
	virtual MAE_Gc* Gc(TGcType aGcType = EGt_Cur) = 0;
	virtual void Clear() = 0;
	virtual const string& Name() = 0;
	virtual MAE_Window* CreateWindow(const string& aName) = 0;
	virtual MAE_Window* Wnd(const string& aName) = 0;
	virtual void SetObserver(MAE_ViewObserver* aObs) = 0;
	virtual void ResetObserver(MAE_ViewObserver* aObs) = 0;
	virtual void Show(TBool aAll = EFalse) = 0;
	virtual void SetBg(TState aState, const CAE_Color& aColor) = 0;
	virtual TState GetState() = 0;
	virtual void SetState(TState aState) = 0;
};

class MAE_TextLayout;
class MAE_Gc
{
    public:
	virtual void DrawRect(CAV_Rect aRect, TBool aFilled) = 0;
	virtual void DrawLine(CAV_Point aPt1, CAV_Point aPt2) = 0;
	virtual void DrawText(const string& aText, CAV_Rect aRect) = 0;
	virtual MAE_TextLayout* CreateTextLayout() = 0;
};

class MAE_TextLayout
{
    public:
	virtual void SetText(const string& aText) = 0;
	virtual void Draw(CAV_Point aPt) = 0;
	virtual void GetSizePu(CAV_Point& aSize) = 0;
};

class MAE_ViewObserver
{
    public:
	enum TBtnEv {
	    EBte_Press,
	    EBte_DoublePress,
	    EBte_Release
	};
    public:
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect) = 0;
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt) = 0;
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect) = 0;
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect) = 0;
	virtual void OnMotion(MAE_Window* aWnd, const CAV_Point& aCoord) = 0;
	virtual void OnCrossing(MAE_Window* aWnd, TBool aEnter) = 0;
	virtual void OnStateChanged(MAE_Window* aWnd, MAE_Window::TState aPrevState) = 0;
};

#endif // __FAP_VIEW_H
