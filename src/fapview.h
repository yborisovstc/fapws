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
	virtual void SetDetLevel(TInt aLevel) = 0;
	virtual TInt DetLevel() = 0;
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
    public:
	virtual const MAE_View* View() = 0;
	virtual void Destroy() = 0;
	virtual CAV_Rect Rect() = 0;
	virtual void SetRect(const CAV_Rect& aRect) = 0;
	virtual void SetPrefRect(const CAV_Rect& aRect) = 0;
	virtual MAE_Gc* Gc() = 0;
	virtual void Clear() = 0;
	virtual const string& Name() = 0;
	virtual MAE_Window* CreateWindow(const string& aName) = 0;
	virtual MAE_Window* Wnd(const string& aName) = 0;
	virtual void SetObserver(MAE_ViewObserver* aObs) = 0;
	virtual void ResetObserver(MAE_ViewObserver* aObs) = 0;
	virtual void Show() = 0;
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
};

#endif // __FAP_VIEW_H
