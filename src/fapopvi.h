#ifndef __FAP_OPVI_H
#define __FAP_OPVI_H

// Object proxy for base view based on view interface
// This is initial variant of proxy, not sure it can be implemented properly
// This is because the view iface is very limited

#include "fapbase.h"


class Bva;
class CPV_Opvi: public MAE_Opv
{
    public:
	CPV_Opvi(MAE_View* aView);
	virtual ~CPV_Opvi();
	// From MAE_Opv
	virtual void Destroy();
	virtual void SetRoot(CAE_Object::Ctrl* aObj) {};
	virtual void SetObj(CAE_Object::Ctrl* aObj);
	virtual void UnsetObj(CAE_Object::Ctrl* aObj) {};
	virtual void UnsetRoot(CAE_Object* aObj) {};
    private:
	CAE_Object::Ctrl* iSys;
	Bva* iSystBva; // Base view agent for system
	MAE_View* iView;
};

// Base view rendering element type
enum TReType {
    Et_Unknown,
    Et_System,
    Et_Header,
    Et_Comp,
    Et_Inp,
    Et_Outp,
    Et_StateHeader,
    Et_CompHeader,
    Et_CompInp,
    Et_LeftConns,
    Et_RightConns,
    Et_CompOutp,
    Et_Conn
};
// Type of rendering elements
typedef pair<TReType, string> TRelm;

// Base view agents
class Bva: public MAE_ViewObserver 
{
    public:
	Bva(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aWnd, TReType aType, const string& aName, TBool aCreateWnd = ETrue);
	virtual ~Bva();
	Bva* GetBva(TReType aType, const string& aName);
	void AddBva(Bva* aBva);
	// Renders the childs, returns hint for its rect
	virtual void Render(CAV_Rect& aRect) {};
	// TODO [YB] To remove Draw
	virtual void Draw() {};
	virtual void OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState) {}; 
	void SetRect(const CAV_Rect& aRect) {iWnd->SetRect(aRect);};
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect) {};
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt) {};
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect) {};
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect) {};
	virtual void OnMotion(MAE_Window* aWnd, const CAV_Point& aCoord) {};
	virtual void OnCrossing(MAE_Window* aWnd, TBool aEnter);
	virtual void OnStateChanged(MAE_Window* aWnd, MAE_Window::TState aPrevState) {};
    public:
	Bva* iParent;
	CAE_Object::Ctrl& iSys;
	MAE_Window* iWnd;
	TReType iType;
	string iName;
	map<TRelm, Bva*> iBvas; 
};

// Base view agent for header
class BvaHead : public Bva
{
    public:
	BvaHead(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for system
class BvaSyst : public Bva
{
    public:
	BvaSyst(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	virtual void OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState); 
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for component
class BvaComp : public Bva
{
    public:
	BvaComp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	virtual void OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState); 
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for components header
class BvaCompHead : public Bva
{
    public:
	BvaCompHead(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for components inputs
class BvaCompInp : public Bva
{
    public:
	BvaCompInp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for conns (Et_LeftConns - placed left from conn point, Et_RightConns - placed right)
class BvaConns : public Bva
{
    public:
	BvaConns(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, TReType aType, const string& aName, 
		const vector<CAE_ConnPointBase*>& aConns);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	virtual void OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState); 
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};

// Base view agent for components outputs
class BvaCompOutp : public Bva
{
    public:
	BvaCompOutp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
};
// Base view agent for connection id
class BvaConn : public Bva
{
    public:
	BvaConn(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName);
	virtual ~BvaConn();
	virtual void Render(CAV_Rect& aRect);
	virtual void Draw();
	// From MAE_ViewObserver
	virtual void OnExpose(MAE_Window* aWnd, CAV_Rect aRect);
	virtual TBool OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt);
	virtual void OnResized(MAE_Window* aWnd, CAV_Rect aRect);
	virtual void OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect);
	virtual void OnCrossing(MAE_Window* aWnd, TBool aEnter);
	virtual void OnStateChanged(MAE_Window* aWnd, MAE_Window::TState aPrevState);
};

#endif 
