#ifndef __FAP_VIEWGTK_H
#define __FAP_VIEWGTK_H

// View GTK based

#include <gtk/gtk.h>
#include <string.h>
#include "fapbase.h"
#include "fapview.h"

class CAV_WindowGtk;
class CAE_ViewGtk: public MAE_View
{
    public:
	CAE_ViewGtk(TType aType, GtkLayout* aWidget);
	virtual ~CAE_ViewGtk();
	// From MAE_View
	virtual TType Type() {return iType;};
	virtual const string& Name() const;
	virtual void SetName(const string& aName);
	virtual MAE_Window* Wnd();
	virtual void SetDetLevel(TInt aLevel);
	virtual TInt DetLevel() { return iDetLevel; };
	CAV_WindowGtk* WndGtk() { return iWnd;}
    private:
	TType iType;
	string iName;
	CAV_WindowGtk* iWnd;
	GtkLayout *iWidget;
	// Detalization level
	TInt iDetLevel;
};

class CAV_Gc;
class CAV_WindowGtk: public MAE_Window
{
    friend class CAV_Gc;
    public:
	CAV_WindowGtk(const MAE_View* aView, CAV_WindowGtk* aParent, const string& aName, GtkLayout* aWidget, PangoContext* aPContext);
	virtual ~CAV_WindowGtk();
	// From MAE_Window
	virtual const MAE_View* View() { return iView;};
	virtual const string& Name();
	virtual void Destroy();
	virtual CAV_Rect Rect();
	virtual void SetPrefSize(const CAV_Rect& aRect);
	virtual CAV_Rect GetPrefSize();
	virtual CAV_Rect CalcPrefSize();
	virtual void SetRect(const CAV_Rect& aRect);
	virtual MAE_Gc* Gc();
	virtual void Clear();
	virtual MAE_Window* CreateWindow(const string& aName);
	virtual MAE_Window* Wnd(const string& aName);
	virtual void Show(TBool aAll = EFalse);
	virtual void SetObserver(MAE_ViewObserver* aObs);
	virtual void ResetObserver(MAE_ViewObserver* aObs);
	virtual TState GetState();
	virtual void SetState(TState aState);
	virtual void SetBg(TState aState, const CAE_Color& aColor);
	void RemoveChild(CAV_WindowGtk* aWnd);
	MAE_ViewObserver* Observer() {return iObserver;};
    private:
	const MAE_View* iView;
	string iName;
	CAV_WindowGtk* iParent;
	GtkLayout* iWidget;
	CAV_Gc* iGc;
	PangoContext* iPContext;
	map<string, CAV_WindowGtk*> iChilds;
	MAE_ViewObserver* iObserver;
};

class CAV_Gc: public MAE_Gc
{
    friend class CAV_TextLayoutGtk;
    public:
	CAV_Gc(CAV_WindowGtk* aWnd, GdkGC* aGdkGc);
	GdkWindow* GdkWnd();
	PangoContext* PContext();
	// From MAE_Gc
	virtual void DrawRect(CAV_Rect aRect, TBool aFilled);
	virtual void DrawText(const string& aText, CAV_Rect aRect);
	virtual void DrawLine(CAV_Point aPt1, CAV_Point aPt2);
	virtual MAE_TextLayout* CreateTextLayout();
    private:
	CAV_WindowGtk* iWnd;
	GdkGC* iGdkGc;
};

class CAV_TextLayoutGtk: public MAE_TextLayout
{
    public: 
	CAV_TextLayoutGtk(CAV_Gc& aGc);
	virtual ~CAV_TextLayoutGtk();
	// From MAE_TextLayout
	virtual void SetText(const string& aText);
	virtual void Draw(CAV_Point aPt);
	virtual void GetSizePu(CAV_Point& aSize);
    private:
	PangoLayout* iGdkTextLayout;
	CAV_Gc& iGc;
};


class CAE_ViewGtkUtils
{
    public:
	static CAV_Rect Rect(GdkRectangle aRect);
	static void ColorToGtk(const CAE_Color& aColor, GdkColor* aGdkColor);
	static GtkStateType StateToGtk(MAE_Window::TState aState);
	static MAE_Window::TState GtkToState(GtkStateType aGtkState);
};


#endif // __FAP_VIEWGTK_H
