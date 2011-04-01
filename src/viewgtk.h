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
	MAE_ViewObserver* Observer() { return iObserver;};
	// From MAE_View
	virtual TType Type() {return iType;};
	virtual MAE_Window* Wnd();
	virtual void SetObserver(MAE_ViewObserver* aObs);
	virtual void ResetObserver(MAE_ViewObserver* aObs);
	virtual void SetDetLevel(TInt aLevel);
	virtual TInt DetLevel() { return iDetLevel; };
    private:
	TType iType;
	CAV_WindowGtk* iWnd;
	GtkWidget *iWidget;
	MAE_ViewObserver* iObserver;
	// Detalization level
	TInt iDetLevel;
};

class CAV_Gc;
class CAV_WindowGtk: public MAE_Window
{
    friend class CAV_Gc;
    public:
	CAV_WindowGtk(GdkWindow* aGdkWnd, PangoContext* aPContext);
	virtual ~CAV_WindowGtk();
	// From MAE_Window
	virtual CAV_Rect Rect();
	virtual MAE_Gc* Gc();
    private:
	GdkWindow* iGdkWnd;
	CAV_Gc* iGc;
	PangoContext* iPContext;
};

class CAV_Gc: public MAE_Gc
{
    public:
	CAV_Gc(CAV_WindowGtk* aWnd, GdkGC* aGdkGc);
	// From MAE_Gc
	virtual void DrawRect(CAV_Rect aRect, TBool aFilled);
	virtual void DrawText(const string& aText, CAV_Rect aRect);
	virtual void DrawLine(CAV_Point aPt1, CAV_Point aPt2);
    private:
	CAV_WindowGtk* iWnd;
	GdkGC* iGdkGc;
};

class CAE_ViewGtkUtils
{
    public:
	static CAV_Rect Rect(GdkRectangle aRect);
};


#endif // __FAP_VIEWGTK_H
