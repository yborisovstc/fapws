
#include "fapopvi.h"

// Parameters of view
const TInt KViewNameLineHeight = 20;
const TInt KViewExtLineHeight = 20;
const TInt KViewExtAreaWidth = 90;
const TInt KViewCompAreaWidth = 200;
const TInt KViewCompGapHight = 20;
const TInt KViewStateWidth = 300;
const TInt KViewConnLineLen = 20;
const TInt KViewConnIdHeight = 16;
const TInt KViewConnIdWidth = 120;
const TInt KViewConnIdMaxNum = 1;
const TInt KViewConnGapWidth = 3;
const TInt KViewTransWidth = 200;
const TInt KViewTransHeight = 100;
const TInt KViewStateGapTransInp = 20;
const TInt KViewCompInpOutpGapWidth = 20;


CPV_Opvi::CPV_Opvi(MAE_View* aView): iView(aView)
{
}

CPV_Opvi::~CPV_Opvi()
{
}

void CPV_Opvi::Destroy()
{
    delete this;
}

void CPV_Opvi::SetObj(CAE_Object::Ctrl* aObj)
{
    iSys = aObj;
    iSystBva = new BvaSyst(NULL, *iSys, iView->Wnd(), iView->Name());
    iSystBva->iWnd->Show();
}

//*********************************************************
// Base view agents
//*********************************************************

Bva::Bva(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aWnd, TReType aType, const string& aName, TBool aCreateWnd): 
    iParent(aParent), iSys(aSys), iType(aType), iName(aName) 
{
    static map<TReType, string> KBvaToString;
    if (KBvaToString.empty()) {
	KBvaToString[Et_System] = "System";
	KBvaToString[Et_Header] = "Et_Header";
	KBvaToString[Et_Comp] = "Et_Comp";
	KBvaToString[Et_Outp] = "Et_Outp";
	KBvaToString[Et_Inp] = "Et_Inp";
	KBvaToString[Et_StateHeader] = "Et_StateHeader";
	KBvaToString[Et_CompHeader] = "Et_CompHeader";
	KBvaToString[Et_CompInp] = "Et_CompInp";
	KBvaToString[Et_CompOutp] = "Et_CompOutp";
	KBvaToString[Et_LeftConns] = "Et_LeftConns";
	KBvaToString[Et_RightConns] = "Et_RightConns";
	KBvaToString[Et_Conn] = "Et_Conn";
    }

    if (aCreateWnd)
	iWnd = aWnd->CreateWindow(KBvaToString[iType] + "." + iName);
    else
	iWnd = aWnd;
    iWnd->SetObserver(this);
}

Bva::~Bva()
{
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	delete it->second;
    }
    iWnd->Destroy();
    iWnd = NULL;
}

void Bva::AddBva(Bva* aBva)
{
    iBvas[TRelm(aBva->iType, aBva->iName)] = aBva;
}

Bva* Bva::GetBva(TReType aType, const string& aName)
{
    map<TRelm, Bva*>::iterator it = iBvas.find(TRelm(aType, aName));
    return (it != iBvas.end()) ? it->second : NULL; 
}

void Bva::OnCrossing(MAE_Window* aWnd, TBool aEnter)
{

}



// Base view agents for system

BvaSyst::BvaSyst(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName): 
    Bva(aParent, aSys, aOwnedWnd, Et_System, aName, ETrue) 
{
    iWnd->SetPrefSize(CAV_Rect(-1, -1));
    // Add header bva
    AddBva(new BvaHead(this, iSys, iWnd));
    // Add components bvas
    for (map<string, CAE_Object*>::iterator it = iSys.Comps().begin(); it != iSys.Comps().end(); it++) {
	CAE_Object* obj = it->second;
	if (obj != NULL) {
	    AddBva(new BvaComp(this, iSys, iWnd, obj->InstName()));
	}
    }
}

void BvaSyst::Render(CAV_Rect& aRect)
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = aRect;
    // Render the header
    CAV_Rect headrc;
    Bva* bva = GetBva(Et_Header, "Header");
    if (bva != NULL) {
	CAV_Rect cdres = bva->iWnd->CalcPrefSize();
	headrc = CAV_Rect(rect.iTl, rect.Width(), cdres.Height());
	// TODO [YB] It s not clear how gtk container handles resizing. I can see that even size is set to child
	// the container anycase allocate pref size to child. So as workaround i set pref size together with size.
	// To consider the mechanism and update
	bva->iWnd->SetPrefSize(headrc);
	bva->iWnd->SetRect(headrc);
    }

    // Render the comps
    CAV_Point comprec_base((rect.iTl.iX + rect.iBr.iX)/2, headrc.iBr.iY + KViewCompGapHight);
    for (map<string, CAE_Object*>::iterator it = iSys.Comps().begin(); it != iSys.Comps().end(); it++) {
	CAE_Object* obj = it->second;
	if (obj != NULL) {
	    bva = GetBva(Et_Comp, obj->InstName());
	    if (bva != NULL) {
		//cdres = CAV_Rect(CAV_Point(rect.iTl.iX, cdres.iBr.iY + KViewCompGapHight), rect.Width(), 40);
		//bva->Render(cdres);
		CAV_Rect comprec = bva->iWnd->CalcPrefSize();
		comprec.Move(comprec_base - CAV_Point(comprec.Width()/2, 0));
		bva->iWnd->SetPrefSize(comprec);
		bva->iWnd->SetRect(comprec);
		comprec_base += CAV_Point(0, comprec.Height() + KViewCompGapHight);
	    }
	}
    }
}

void BvaSyst::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the rect
    //gc->DrawRect(rect, EFalse);
}

void BvaSyst::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaSyst::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void BvaSyst::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaSyst::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    CAV_Rect head_rec;
    Bva* bva = GetBva(Et_Header, "Header");
    head_rec = bva->iWnd->CalcPrefSize();
    // Calc size of comps
    CAV_Point comps_sz;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_Comp) {
	    Bva* bva = it->second;
	    CAV_Rect rec = bva->iWnd->CalcPrefSize();
	    comps_sz.iX = max(comps_sz.iX, rec.Width());
	    comps_sz.iY += rec.Height() + KViewCompGapHight;
	}
    }

    CAV_Point fsz;
    //fsz.iX = max(head_rec.Width(), comps_sz.iX);
    // TODO [YB] We cannot use header pref size width because the sys wnd with will be able to decrease. Consider.
    fsz.iX = comps_sz.iX;
    fsz.iY = head_rec.Height() + comps_sz.iY;
    aRect = CAV_Rect(fsz.iX, fsz.iY);
}


void BvaSyst::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
{
    if (aChild->iType == Et_Conn && aChild->iWnd->GetState() == MAE_Window::ESt_Selected && aPrevState != MAE_Window::ESt_Selected) {
	// Highligt pair via selecting
	const string& cname = aChild->iName;
	if (cname.compare("...") != 0) {
	    TBool isoutp = aChild->iParent->iType == Et_LeftConns;
	    size_t ptp = cname.find_first_of('.');
	    const string& paircompname = cname.substr(0, ptp);
	    const string& paircpname = cname.substr(ptp + 1);
	    const string& cpname = aChild->iParent->iName;
	    const string& compname = aChild->iParent->iParent->iName;
	    map<TRelm, Bva*>::iterator compit = iBvas.find(TRelm(Et_Comp, paircompname));
	    Bva* compbva = compit->second;
	    map<TRelm, Bva*>::iterator csit = compbva->iBvas.find(TRelm(isoutp ? Et_RightConns: Et_LeftConns, paircpname));
	    Bva* csbva = csit->second;
	    string ciname = compname + "." + cpname;
	    map<TRelm, Bva*>::iterator cit = csbva->iBvas.find(TRelm(Et_Conn, ciname));
	    if (cit == csbva->iBvas.end()) {
		cit = csbva->iBvas.find(TRelm(Et_Conn, "..."));
	    }
	    Bva* cbva = cit->second;
	    cbva->iWnd->SetState(MAE_Window::ESt_Selected);
	}
    }
}



// Base view agents for header

BvaHead::BvaHead(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd): Bva(aParent, aSys, aOwnedWnd, Et_Header, "Header")
{
    iWnd->SetPrefSize(CAV_Rect(0, KViewNameLineHeight));
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iSys.Object().InstName());
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    iWnd->SetPrefSize(CAV_Rect(tsize.iX, KViewNameLineHeight));
    CAE_Color bg_norm(1, 0, 40000, 0);
    iWnd->SetBg(MAE_Window::ESt_Normal, bg_norm);
    CAE_Color bg_pre(2, 50000, 0, 0);
    iWnd->SetBg(MAE_Window::ESt_Prelight, bg_pre);
    iWnd->Show();
}

void BvaHead::Render(CAV_Rect& aRect)
{
}

void BvaHead::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iSys.Object().InstName());
    CAV_Rect drc(rect.iTl + CAV_Point(1, 1), rect.iBr - CAV_Point(1, 1));
    nametl->Draw(drc.iTl);
    gc->DrawRect(drc, EFalse);
}

void BvaHead::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaHead::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	//iSys.Object().OnHeaderPress(aWnd->View());
    }
}

void BvaHead::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaHead::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

// Base view agents for component

BvaComp::BvaComp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_Comp, aName) 
{
    CAE_EBase* elem = iSys.Object().FindByName(aName.c_str());
    CAE_Object* comp = elem->GetFbObj(comp);
    iWnd->SetPrefSize(CAV_Rect(-1, -1));
    // Add header
    AddBva(new BvaCompHead(this, iSys, iWnd, iName));
    // Add Inputs and inputs connections
    for (map<string, CAE_ConnPointBase*>::const_iterator it = comp->Inputs().begin(); it != comp->Inputs().end(); it++) {
	CAE_ConnPointBase* cp = it->second;
	AddBva(new BvaCompInp(this, iSys, iWnd, it->first));
	AddBva(new BvaConns(this, iSys, iWnd, Et_RightConns, it->first, cp->Conns()));
    }
    // Add ouptuts
    for (map<string, CAE_ConnPointBase*>::const_iterator it = comp->Outputs().begin(); it != comp->Outputs().end(); it++) {
	CAE_ConnPointBase* cp = it->second;
	AddBva(new BvaCompOutp(this, iSys, iWnd, it->first));
	AddBva(new BvaConns(this, iSys, iWnd, Et_LeftConns, it->first, cp->Conns()));
    }
    iWnd->Show();
}

void BvaComp::Render(CAV_Rect& aRect)
{
    // TODO [YB] We cannot use iWnd->Rect() here
    CAV_Rect rect(CAV_Point(0, 0), aRect.Width(), aRect.Height());
    // Calculate size first
    // Calculate size for header
    Bva* header = GetBva(Et_CompHeader, iName);
    CAV_Rect head_rec = header->iWnd->CalcPrefSize();
    // Calculate inputs width first
    TInt inp_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompInp) {
	    Bva* inp = it->second;
	    CAV_Rect inp_rc = inp->iWnd->CalcPrefSize();
	    if (inp_rc.Width() > inp_w) inp_w = inp_rc.Width();
	}
    }
    // Calculate outputs width first
    TInt outp_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompOutp) {
	    outp_w = max(outp_w, it->second->iWnd->CalcPrefSize().Width());
	}
    }
    // Calculate outputs conns size
    TInt outpsc_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_LeftConns) {
	    Bva* bva = it->second;
	    CAV_Rect rec = bva->iWnd->CalcPrefSize();
	    outpsc_w = max(outpsc_w, rec.Width());
	}
    }


    // Render now
    // Render the header
    TInt head_w = max(head_rec.Width(), outp_w + inp_w + KViewCompInpOutpGapWidth);
    CAV_Rect headrc = CAV_Rect(rect.iTl + CAV_Point(outpsc_w, 0), head_w, head_rec.Height());
    header->iWnd->SetPrefSize(headrc);
    header->iWnd->SetRect(headrc);
    // Render inputs
    CAV_Point inp_base(headrc.iBr.iX - inp_w, headrc.Height());
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompInp) {
	    Bva* inp = it->second;
	    CAV_Rect inp_rc = inp->iWnd->CalcPrefSize();
	    CAV_Rect inp_frec(inp_base, inp_w, inp_rc.Height());
	    inp->iWnd->SetPrefSize(inp_frec);
	    inp->iWnd->SetRect(inp_frec);
	    map<TRelm, Bva*>::iterator connit = iBvas.find(TRelm(Et_RightConns, it->first.second));
	    Bva* conn = connit->second;
	    CAV_Rect conn_rc = conn->iWnd->CalcPrefSize();
	    CAV_Rect conn_frc = conn_rc.Move(inp_frec.iBr - CAV_Point(0, inp_frec.Height()/2 + conn_rc.Height()/2));
	    conn->iWnd->SetPrefSize(conn_frc);
	    conn->iWnd->SetRect(conn_frc);
	    inp_base += CAV_Point(0, inp_frec.Height());
	}
    }
    CAV_Point outp_base(headrc.iTl.iX, headrc.Height());
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompOutp) {
	    Bva* outp = it->second;
	    CAV_Rect outp_rc = outp->iWnd->CalcPrefSize();
	    CAV_Rect outp_frec(outp_base, outp_w, outp_rc.Height());
	    outp->iWnd->SetPrefSize(outp_frec);
	    outp->iWnd->SetRect(outp_frec);
	    map<TRelm, Bva*>::iterator connit = iBvas.find(TRelm(Et_LeftConns, it->first.second));
	    Bva* conn = connit->second;
	    CAV_Rect conn_rc = conn->iWnd->CalcPrefSize();
	    conn_rc.Move(CAV_Point(outp_frec.iTl.iX - conn_rc.Width(), outp_frec.iTl.iY + outp_frec.Height()/2 - conn_rc.Height()/2));
	    conn->iWnd->SetPrefSize(conn_rc);
	    conn->iWnd->SetRect(conn_rc);
	    outp_base += CAV_Point(0, outp_frec.Height());
	}
    }
}

void BvaComp::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    Bva* header = GetBva(Et_CompHeader, iName);
    CAV_Rect head_rec = header->iWnd->Rect();
    // Calculate outputs conns size
    TInt outpsc_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_LeftConns) {
	    Bva* bva = it->second;
	    CAV_Rect rec = bva->iWnd->CalcPrefSize();
	    outpsc_w = max(outpsc_w, rec.Width());
	}
    }

    // Draw the rect for body
    CAV_Rect brec(CAV_Point(outpsc_w, head_rec.Height()), head_rec.Width() - 1, rect.Height() - head_rec.Height() -1);
    gc->DrawRect(brec, EFalse);
}

void BvaComp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaComp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void BvaComp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaComp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    CAV_Point sz;
    Bva* bva = GetBva(Et_CompHeader, iName);
    CAV_Rect head_rec = bva->iWnd->CalcPrefSize();
    // Calculate inputs size
    CAV_Point inps_sz;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompInp) {
	    Bva* inp = it->second;
	    CAV_Rect inp_rec = inp->iWnd->CalcPrefSize();
	    inps_sz.iX = max(inps_sz.iX, inp_rec.Width());
	    inps_sz.iY += inp_rec.Height();
	}
    }
    // Calculate inputs conns size
    TInt inpsc_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_RightConns) {
	    Bva* bva = it->second;
	    CAV_Rect rec = bva->iWnd->CalcPrefSize();
	    inpsc_w = max(inpsc_w, rec.Width());
	}
    }

    // Calculate outputs size
    CAV_Point outps_sz;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_CompOutp) {
	    CAV_Rect outp_rec = it->second->iWnd->CalcPrefSize();
	    outps_sz.iX = max(outps_sz.iX, outp_rec.Width());
	    outps_sz.iY += outp_rec.Height();
	}
    }

    // Calculate outputs conns size
    TInt outpsc_w = 0;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_LeftConns) {
	    Bva* bva = it->second;
	    CAV_Rect rec = bva->iWnd->CalcPrefSize();
	    outpsc_w = max(outpsc_w, rec.Width());
	}
    }

    sz.iX = max(head_rec.Width(), outps_sz.iX + inps_sz.iX + KViewCompInpOutpGapWidth) + inpsc_w + outpsc_w;
    sz.iY = head_rec.Height() + max(outps_sz.iY, inps_sz.iY);
    aRect = CAV_Rect(sz.iX, sz.iY);
}

void BvaComp::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
{
    iParent->OnChildStateChanged(aChild, aPrevState);
}


// Base view agents for components header

BvaCompHead::BvaCompHead(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_CompHeader, aName) 
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect = CAV_Rect(tsize.iX, KViewNameLineHeight);
    iWnd->SetPrefSize(rect);
    iWnd->Show();
}

void BvaCompHead::Render(CAV_Rect& aRect)
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect = CAV_Rect(aRect.iTl, tsize.iX, KViewNameLineHeight);
}

void BvaCompHead::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Rect drc(rect.iTl, rect.iBr - CAV_Point(1, 1));
    nametl->Draw(drc.iTl);
    gc->DrawRect(drc, EFalse);
}

void BvaCompHead::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaCompHead::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	//iSys.OnCompHeaderPress(aWnd->View(), iName);
    }
}

void BvaCompHead::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}


void BvaCompHead::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = iWnd->GetPrefSize();
}

// Base view agents for components inputs

BvaCompInp::BvaCompInp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_CompInp, aName) 
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect(tsize.iX, KViewConnLineLen);
    iWnd->SetPrefSize(rect);
    iWnd->Show();
}

void BvaCompInp::Render(CAV_Rect& aRect)
{
}

void BvaCompInp::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Rect drc(rect.iTl, rect.iBr - CAV_Point(1, 1));
    nametl->Draw(drc.iTl);
    gc->DrawRect(drc, EFalse);
}

void BvaCompInp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaCompInp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void BvaCompInp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaCompInp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = CAV_Rect(0, KViewConnLineLen);
}


// Base view agents for components outputs

BvaCompOutp::BvaCompOutp(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_CompOutp, aName) 
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect(tsize.iX, KViewExtLineHeight);
    iWnd->SetPrefSize(rect);
    iWnd->Show();
}

void BvaCompOutp::Render(CAV_Rect& aRect)
{
}

void BvaCompOutp::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Rect drc(rect.iTl, rect.iBr - CAV_Point(1, 1));
    nametl->Draw(drc.iTl);
    gc->DrawRect(drc, EFalse);
}

void BvaCompOutp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaCompOutp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void BvaCompOutp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaCompOutp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = CAV_Rect(0, KViewConnLineLen);
}

// Base view agents for connections

BvaConns::BvaConns(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, TReType aType, const string& aName,
	const vector<CAE_ConnPointBase*>& aConns):
    Bva(aParent, aSys, aOwnedWnd, aType, aName) 
{
    // Create conn ids
    TInt count = KViewConnIdMaxNum + 1;
    TInt connw = 0;
    for (vector<CAE_ConnPointBase*>::const_iterator it = aConns.begin(); it != aConns.end() && count > 0; it++, count--) {
	CAE_ConnPointBase* cp = *it;
	const CAE_EBase& mgr = cp->Man();
	string pair_name = (&mgr == NULL) ? "?" : mgr.InstName();
	string ftxt = (count == 1) ? "..." : pair_name + "." + cp->Name();
	Bva* bva = NULL;
	AddBva(bva = new BvaConn(this, iSys, iWnd, ftxt));
	connw += bva->iWnd->CalcPrefSize().Width() + KViewConnGapWidth;
    }
    CAV_Rect rect(connw + KViewConnLineLen - KViewConnGapWidth, KViewConnIdHeight);
    iWnd->SetPrefSize(rect);
    iWnd->Show();
}

void BvaConns::Render(CAV_Rect& aRect)
{
    CAV_Point base((iType == Et_LeftConns) ? 0: KViewConnLineLen, 0);
    // Render now
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	if (it->first.first == Et_Conn) {
	    Bva* bva = it->second;
	    CAV_Rect rc = bva->iWnd->CalcPrefSize();
	    CAV_Rect frec(base, rc.Width(), rc.Height());
	    bva->iWnd->SetPrefSize(frec);
	    bva->iWnd->SetRect(frec);
	    base += CAV_Point(rc.Width() + KViewConnGapWidth, 0);
	}
    }
}

void BvaConns::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    CAV_Point start((iType == Et_LeftConns) ? (rect.Width() - KViewConnLineLen) : 0, rect.Height()/2);
    CAV_Point end = start + CAV_Point(KViewConnLineLen, 0);
    gc->DrawLine(start, end);
}

void BvaConns::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaConns::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void BvaConns::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaConns::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

void BvaConns::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
{
    iParent->OnChildStateChanged(aChild, aPrevState);
}


// Base view agents for connection id

BvaConn::BvaConn(Bva* aParent, CAE_Object::Ctrl& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_Conn, aName) 
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect(tsize.iX, KViewConnIdHeight);
    iWnd->SetPrefSize(rect);
    iWnd->Show();
}

BvaConn::~BvaConn()
{
}

void BvaConn::Render(CAV_Rect& aRect)
{
}

void BvaConn::Draw()
{
    MAE_Gc* bggc = iWnd->Gc(MAE_Window::EGt_Bg);
    MAE_Gc* fggc = iWnd->Gc(MAE_Window::EGt_Fg);
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = fggc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Rect drc(rect.iTl, rect.iBr - CAV_Point(1, 1));
    bggc->DrawRect(drc, ETrue);
    fggc->DrawRect(drc, EFalse);
    nametl->Draw(drc.iTl);
}

void BvaConn::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool BvaConn::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	MAE_Window::TState state = aWnd->GetState();
	if (state == MAE_Window::ESt_Selected)
	    aWnd->SetState(MAE_Window::ESt_Prelight);
	else
	    aWnd->SetState(MAE_Window::ESt_Selected);
    }
}

void BvaConn::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void BvaConn::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

void BvaConn::OnCrossing(MAE_Window* aWnd, TBool aEnter)
{
    MAE_Window::TState state = aWnd->GetState();
    if (aEnter) {
	if (state == MAE_Window::ESt_Normal)
	    aWnd->SetState(MAE_Window::ESt_Prelight);
    }
    else {
	if (state == MAE_Window::ESt_Prelight)
	    aWnd->SetState(MAE_Window::ESt_Normal);
    }
}

void BvaConn::OnStateChanged(MAE_Window* aWnd, MAE_Window::TState aPrevState)
{
    if (iWnd->GetState() == MAE_Window::ESt_Selected) {
	iParent->OnChildStateChanged(this, aPrevState);
    }
}
