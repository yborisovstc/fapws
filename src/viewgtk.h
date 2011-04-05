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
	CAE_ViewGtk(TType aType, GtkWidget* aWidget);
	virtual ~CAE_ViewGtk();
	// From MAE_View
	virtual TType Type() {return iType;};
	virtual const string& Name();
	virtual void SetName(const string& aName);
	virtual MAE_Window* Wnd();
	virtual void SetDetLevel(TInt aLevel);
	virtual TInt DetLevel() { return iDetLevel; };
	CAV_WindowGtk* WndGtk() { return iWnd;}
    private:
	TType iType;
	string iName;
	CAV_WindowGtk* iWnd;
	GtkWidget *iWidget;
	// Detalization level
	TInt iDetLevel;
};

class CAV_Gc;
class CAV_WindowGtk: public MAE_Window
{
    friend class CAV_Gc;
    public:
	CAV_WindowGtk(CAV_WindowGtk* aParent, const string& aName, GdkWindow* aGdkWnd, PangoContext* aPContext);
	virtual ~CAV_WindowGtk();
	// From MAE_Window
	virtual const string& Name();
	virtual void Destroy();
	virtual CAV_Rect Rect();
	virtual void SetRect(const CAV_Rect& aRect);
	virtual MAE_Gc* Gc();
	virtual void Clear();
	virtual MAE_Window* CreateWindow(const string& aName);
	virtual MAE_Window* Wnd(const string& aName);
	virtual void Show();
	virtual void SetObserver(MAE_ViewObserver* aObs);
	virtual void ResetObserver(MAE_ViewObserver* aObs);
	void OnExpose(MAE_View* aView, CAV_Rect aRect);
	TBool OnButton(MAE_View* aView, MAE_Window* aWnd, MAE_ViewObserver::TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	void RemoveChild(CAV_WindowGtk* aWnd);
    private:
	string iName;
	CAV_WindowGtk* iParent;
	GdkWindow* iGdkWnd;
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
};


#endif // __FAP_VIEWGTK_H
