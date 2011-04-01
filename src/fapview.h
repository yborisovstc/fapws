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
	virtual MAE_Window* Wnd() = 0;
	virtual void SetObserver(MAE_ViewObserver* aObs) = 0;
	virtual void SetDetLevel(TInt aLevel) = 0;
	virtual TInt DetLevel() = 0;
};

class MAE_Gc;
class MAE_Window
{
    public:
	virtual CAV_Rect Rect() = 0;
	virtual MAE_Gc* Gc() = 0;
};

class MAE_Gc
{
    public:
	virtual void DrawRect(CAV_Rect aRect, TBool aFilled) = 0;
	virtual void DrawText(const string& aText, CAV_Rect aRect) = 0;
};

class MAE_ViewObserver
{
    public:
	virtual void OnExpose(MAE_View* aView, CAV_Rect aRect) = 0;
};


#endif // __FAP_VIEW_H
