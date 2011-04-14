//************************************************************ 
// Finite automata programming base, level 1 prototype 
// Win32 platform port 
// Yuri Borisov  21/06/05              Initial version 
// Yuri Borisov  11/07/05  FAP_CR_001  Added link based on reconfiguring of inputs/outputs 
// Yuri Borisov  11/07/05  FAP_CR_002  Added panics 
// Yuri Borisov  13/07/05  FAP_CR_004  Corrected activity processing 
// Yuri Borisov  13/07/05  FAP_CR_005  Added Object activity processing 
// Yuri Borisov  15/07/05  FAP_CR_006  Added callback object for input transition 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging 
// Yuri Borisov  22/07/05  FAP_CR_014  Updated activating in the CAE_StateBase::SetNew 
// Yuri Borisov  25/07/05  FAP_CR_015  Reset flag "Update" if there wasn't update 
// Yuri Borisov  25/07/05  FAP_CR_016  Update CAE_Base class to move observer from state and object 
// Yuri Borisov  25/07/05  FAP_CR_017  Added operator = to the templated state 
// Yuri Borisov  25/07/05  FAPW_CR_001 Ported FAP service into Win32 platform 
// Yuri Borisov  18-Nov-08 FAPW_CR_003 Added type identification for objects
// Yuri Borisov  15-Dec-08 FAPW_CR_011 Corrected to make compatible with fapwstst1
//*************************************************************


// TODO [YB] To use full name in the logging
// TODO [YB] To condider access rules to object elem. Is it ok to access Inp state value, Out state inputs?
// TODO [YB] To improve access to object's states: only inputs of input state accessibele (but not outp), similar for outp state
// TODO [YB] How to define an access to internal object?
// TODO [YB] To implement inputs and outputs for object
// TODO [YB] To implement states group proxy
// TODO [YB] To implement ifaces in conn pins
// TODO [YB] There is strong restriction in current design of connection. It is not possible to switch extention ...
// See UC_CONN_06 in FAP Design document. The problem is that disconnection and connection is possible only
// if we have both of connectin points being connecting. But with given use case we have only connection point that
// is extended, and this conn point needs to be disconnected from the pair, and then the new conn point needs to be connected to
// that pair. 

#include <stdlib.h>
#include <stdio.h>
#include "fapplat.h"
#include "panics.h"
#include "fapbase.h"
#include "fapview.h"

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

_LIT(KFapPanic, "FAP: Error %d");
const TInt KNameSeparator  = '.';
const TInt KNameSeparatorLen = 1;
const TInt KNameMaxLen = 50;

// Parameters of XML spec
const char *KXTransAttr_State = "state";
const char *KXTransAttr_Type = "type";
const char *KXTransAttr_LogEvent = "event";
const char *KXTransAttr_LogData = "id";
const char *KXStateAttr_Transf = "transf";
const char *KXStateAttr_Init = "init";
const char *KXStateInpAttr_Id = "id";

// Length of bite automata word 
const int KBaWordLen = 32;

void Panic(TInt aRes)
{
	_IND_PANIC(KFapPanic, aRes);
}

CAE_ConnSlot::CAE_ConnSlot(const map<string, string>& aTempl): iTempl(aTempl)
{
    for (map<string, string>::const_iterator i = aTempl.begin(); i != aTempl.end(); i++) {
	iPins[i->first] = NULL;
    }
}

CAE_Base* CAE_ConnSlot::Pin(const char *aName) 
{ 
    map<string, CAE_Base*>::iterator elem = iPins.find(aName);
    return (elem == iPins.end()) ? NULL : elem->second;
};

const CAE_Base* CAE_ConnSlot::Pin(const char *aName) const
{ 
    map<string, CAE_Base*>::const_iterator elem = iPins.find(aName);
    return (elem == iPins.end()) ? NULL : elem->second;
};

TBool CAE_ConnSlot::SetPin(const char* aName, CAE_Base *aSrc) 
{
    TBool res = EFalse;
    // TODO [YB] To check type in template
    map<string, string>::const_iterator tt = iTempl.find(aName);
    if (tt != iTempl.end()) {
	iPins[tt->first] = aSrc;
	res = ETrue;
    }
    return res;
}

TBool CAE_ConnSlot::Set(CAE_ConnSlot& aSrcs) 
{
    TBool res = ETrue;
    // Go thru all dest pins
    for (map<string, string>::const_iterator i = iTempl.begin(); i != iTempl.end(); i++) {
	// Check if there is the corresponding pin in sources
	CAE_Base* pin = aSrcs.Pin(i->first.c_str());
	if (pin != NULL) {
	    iPins[i->first] = pin;
	}
	else {
	    res = EFalse; break;
	}
    }
    return res;
}

TBool CAE_ConnSlot::operator==(const CAE_ConnSlot& aSlot)
{
    TBool res = ETrue;
    // Go thru all dest pins
    for (map<string, string>::const_iterator i = iTempl.begin(); i != iTempl.end(); i++) {
	// Check if there is the corresponding pin in sources
	const CAE_Base* pin = aSlot.Pin(i->first.c_str());
	if ((pin == NULL) || (iPins[i->first] != pin)) {
	    res = EFalse; break;
	}
    }
    return res;
}

TBool CAE_ConnSlot::IsEmpty() const
{
    TBool res = ETrue;
    // Go thru all dest pins
    for (map<string, string>::const_iterator i = iTempl.begin(); i != iTempl.end(); i++) {
	// Check if there is the corresponding pin in sources
	if (Pin(i->first.c_str()) != NULL) {
	    res = EFalse; break;
	}
    }
    return res;
}

/*
CAE_ConnPoint::CAE_ConnPoint(const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl): 
    iSrcsTempl(aSrcsTempl), iDestsTempl(aDestsTempl) 
{
    iSrcs = new CAE_ConnSlot(iSrcsTempl);
}
*/

CAE_ConnPoint::CAE_ConnPoint(const string aName, CAE_EBase* aMan, const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, 
	const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl): CAE_ConnPointBase(aName, aMan),
    iSrcsTempl(aSrcsTempl), iDestsTempl(aDestsTempl) 
{
    iSrcs = new CAE_ConnSlot(iSrcsTempl);
}

CAE_ConnPoint::~CAE_ConnPoint() 
{
}

void *CAE_ConnPoint::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : NULL;
}

/*
TBool CAE_ConnPoint::ConnectConnPoint(CAE_ConnPoint *aConnPoint) 
{
    TBool res = EFalse;
    // Adding new slot
    CAE_ConnSlot *slot = new CAE_ConnSlot(iDestsTempl);
    // Copy Srcs from connected point to created slot
    res = slot->Set(*(aConnPoint->Srcs()));
    if (res) {
	iDests.push_back(slot);
    }
    return res;
}
*/

TBool CAE_ConnPoint::ConnectConnPoint(CAE_ConnPoint *aConnPoint) 
{
    TBool res = ETrue;
    for (map<string, string>::iterator idt = iDestsTempl.begin(); idt != iDestsTempl.end() && res; idt++) {
	string pinid = idt->first;
	res = ConnectPin(pinid.c_str(), aConnPoint, pinid.c_str());
    }
    return res;
}

TBool CAE_ConnPoint::Connect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = EFalse;
    // Try to get connection
    CAE_ConnPoint *pair = aConnPoint->GetFbObj(pair); 
    if (pair != NULL ) {
	res = ConnectConnPoint(pair);
    }
    else {
	// Probably it's commutating extention
	CAE_ConnPointExtC *pec = aConnPoint->GetFbObj(pec); 
	if (pec != NULL) {
	    // Connect all the dests pins
	    TBool res1 = ETrue;
	    for (map<string, string>::iterator id = iDestsTempl.begin(); id != iDestsTempl.end() && res1; id++) {
		res1 = ConnectPin(id->first.c_str(), aConnPoint, id->first.c_str());
	    }
	    res = res1;
	}
    }
    if (res)
	iConns.push_back(aConnPoint);
    return res;
}

// TODO [YB] Maybe reconsider the policy of connecting to the bus. Currently we attempt to connect the pin
// in last slot. This means we have restriction here: when inp a.a connects to outp b.b1 b.b2 (buses)
// then we need to complete all connections to b before go to c.
TBool CAE_ConnPoint::ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    TBool res = EFalse;
    TBool found = EFalse;
    CAE_ConnSlot* slot = NULL;

    CAE_ConnSlot::Template::iterator ti = iDestsTempl.find(aPin);
    if (ti != iDestsTempl.end()) {
	// Check the latest slot, where the pin is not set, then set them, otherwise create new slot
	for (vector<CAE_ConnSlot*>::iterator is = iDests.end(); is != iDests.begin() && !found; ) {
	    is--;
	    slot = *is;
	    found = slot->Pin(aPin) == NULL;
	}
	if (!found) {
	    slot = new CAE_ConnSlot(iDestsTempl);
	    iDests.push_back(slot);
	}
	// Redirect to the pin
	CAE_Base* pair_pin = aPair->GetSrcPin(aPairPin);
	// Case when pair pin is not found is normal. When connect to the bus it maybe that bus doesn't contain the pair for pin
	if (pair_pin != NULL) {
	    res = slot->SetPin(aPin, pair_pin);
	}
    }

    return res;
}

void CAE_ConnPoint::DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    TBool found = EFalse;
    CAE_ConnSlot* slot = NULL;

    CAE_ConnSlot::Template::iterator ti = iDestsTempl.find(aPin);
    if (ti != iDestsTempl.end()) {
	// Check the latest slot, if the pin is set, then reset them
	for (vector<CAE_ConnSlot*>::iterator is = iDests.end(); is != iDests.begin() && !found; ) {
	    is--;
	    slot = *is;
	    found = slot->Pin(aPin) != NULL;
	}
	// Redirect to the pin
	CAE_Base* pair_pin = aPair->GetSrcPin(aPairPin);
	CAE_Base* pin = slot->Pin(aPin);
	if (pair_pin == pin) {
	    // Activate srcs to be disconnected 
	    CAE_EBase* srce = pair_pin->GetFbObj(srce);
	    if (srce != NULL) {
		srce->SetActive();
	    }
	    slot->Pins()[aPin] = NULL;
	}
    }
}

CAE_Base* CAE_ConnPoint::GetSrcPin(const char* aName)
{
    return iSrcs->Pin(aName);
}

void CAE_ConnPoint::DisconnectConnPoint(CAE_ConnPoint *aConnPoint)
{
    CAE_ConnSlot& pair_srcs = *(aConnPoint->Srcs());
    // Go thru dests and disconnect any pins connected to pair
    for (vector<CAE_ConnSlot*>::iterator ids = iDests.begin(); ids != iDests.end(); ids++) {
	CAE_ConnSlot& dest = *(*ids);
	for (map<string, string>::iterator idt = iDestsTempl.begin(); idt != iDestsTempl.end(); idt++) {
	    string did = idt->first;
	    CAE_Base* pspin = pair_srcs.Pins()[did];
	    if (dest.Pins()[did] == pspin) {
		// Activate srcs to be disconnected 
		CAE_EBase* srce = pspin->GetFbObj(srce);
		if (srce != NULL) {
		    srce->SetActive();
		}
		dest.Pins()[did] = NULL;
	    }
	}
    }
}

void CAE_ConnPoint::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
    CAE_ConnPoint *pair = aConnPoint->GetFbObj(pair); 
    if (pair != NULL ) {
	// Disconnect conn point
	DisconnectConnPoint(pair);
    }
}

void CAE_ConnPoint::Disconnect()
{
    // Go thru dests and disconnect any pins connected to pair
    for (vector<CAE_ConnSlot*>::iterator ids = iDests.begin(); ids != iDests.end(); ids++) {
	CAE_ConnSlot& dest = *(*ids);
	for (map<string, string>::iterator idt = iDestsTempl.begin(); idt != iDestsTempl.end(); idt++) {
	    string did = idt->first;
	    CAE_Base* pin = dest.Pins()[did];
	    if (pin != NULL) {
		// Activate srcs to be disconnected 
		CAE_EBase* srce = pin->GetFbObj(srce);
		if (srce != NULL) {
		    srce->SetActive();
		}
		dest.Pins()[did] = NULL;
	    }
	}
    }
}

TBool CAE_ConnPoint::Extend(CAE_ConnPointBase *aConnPoint) 
{
    iConns.push_back(aConnPoint);
    return ETrue;
}

void CAE_ConnPoint::Disextend(CAE_ConnPointBase *aConnPoint)
{
}


void *CAE_ConnPointExt::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : ((iRef != NULL) ? iRef->GetFbObj(aName) : NULL);
}

// TODO [YB] Is it used?
void CAE_ConnPointExt::Set(CAE_ConnPointBase *aConnPoint) 
{
    _FAP_ASSERT(iRef != NULL);
    iRef = aConnPoint;
}
    
void CAE_ConnPointExt::Unset() 
{
    iRef = NULL;
}

TBool CAE_ConnPointExt::Connect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = EFalse;
    if (iRef != NULL) {
	res = iRef->Connect(aConnPoint);
	if (res) {
	    iConns.push_back(aConnPoint);
	}
    }
    return res;
}

void CAE_ConnPointExt::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
    if (iRef != NULL) {
	iRef->Disconnect(aConnPoint);
    }
}

void CAE_ConnPointExt::Disconnect()
{
    if (iRef != NULL) {
	iRef->Disconnect();
    }
}

TBool CAE_ConnPointExt::Extend(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = EFalse;
    if (iRef == NULL) { 
	iRef = aConnPoint;
       	res = ETrue;
    }
    return res;
};

CAE_Base* CAE_ConnPointExt::GetSrcPin(const char* aName)
{
    return iRef->GetSrcPin(aName);
}

void CAE_ConnPointExt::DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    if (iRef == NULL) { 
	iRef->DisconnectPin(aPin, aPair, aPairPin);
    }
}

void CAE_ConnPointExt::Disextend(CAE_ConnPointBase *aConnPoint)
{
    if (iRef == aConnPoint) { 
	aConnPoint->Disconnect();
	iRef = NULL;
    }
}



/*
TBool CAE_ConnPointExtC::Connect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = ETrue;
    // Process all the slots
    for (vector<Slot>::iterator is = iSlots.begin(); is != iSlots.end() && res; is++) {
	// Process all the dests
	for (map<string, Slot::slot_elem>::iterator id = (*is).Dests().begin(); id != (*is).Dests().end() && res; id++) {
	    CAE_ConnPointBase* cp = id->second.first;
	    string& pin_name = id->second.second;
	    const string& bus_pin_name = id->first;
	    res = cp->ConnectPin(pin_name.c_str(), aConnPoint, bus_pin_name.c_str());
	}
    }
    return res;
}
*/
TBool CAE_ConnPointExtC::Connect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = ETrue;
    // Connect all the bus pins
    for (map<string, SlotTempl::slot_templ_elem>::iterator id = Templ().Dests().begin(); id != Templ().Dests().end() && res; id++) {
	res = ConnectPin(id->first.c_str(), aConnPoint, id->first.c_str());
    }
    return res;
}

void CAE_ConnPointExtC::Disconnect(CAE_ConnPointBase *aConnPoint)
{
    // Disconnect all the bus pins
    for (map<string, SlotTempl::slot_templ_elem>::iterator id = Templ().Dests().begin(); id != Templ().Dests().end(); id++) {
	DisconnectPin(id->first.c_str(), aConnPoint, id->first.c_str());
    }
}

void CAE_ConnPointExtC::Disconnect()
{
}

TBool CAE_ConnPointExtC::ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    TBool res = ETrue;
    // Process all the slots
    for (vector<Slot>::iterator is = iSlots.begin(); is != iSlots.end() && res; is++) {
	// Redirect to connected pin
	map<string, Slot::slot_elem>::iterator id = (*is).Dests().find(aPin);
	CAE_ConnPointBase* cp = id->second.first;
	if (cp != NULL) {
	    string& pin_name = id->second.second;
	    res = cp->ConnectPin(pin_name.c_str(), aPair, aPairPin);
	}
    }
    return res;
}

void CAE_ConnPointExtC::DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    // Process all the slots
    for (vector<Slot>::iterator is = iSlots.begin(); is != iSlots.end(); is++) {
	// Redirect to disconnecting pin
	map<string, Slot::slot_elem>::iterator id = (*is).Dests().find(aPin);
	CAE_ConnPointBase* cp = id->second.first;
	if (cp != NULL) {
	    string& pin_name = id->second.second;
	    cp->DisconnectPin(pin_name.c_str(), aPair, aPairPin);
	}
    }
}

CAE_Base* CAE_ConnPointExtC::GetSrcPin(const char* aName)
{
    CAE_Base* res = NULL;
    // Redirect to connected pin
    Slot& slot = iSlots.at(0);
    map<string, Slot::slot_elem>::iterator it = slot.Srcs().find(aName);
    if (it != slot.Srcs().end()) {
	CAE_ConnPointBase* cp = it->second.first;
	string& pin_name = it->second.second;
	res = cp->GetSrcPin(pin_name.c_str());
    }
    return res;
}

TBool CAE_ConnPointExtC::Extend(CAE_ConnPointBase *aConnPoint)
{
    // Just extend all the pins to given connpoint with the bus pin names
    TBool res = EFalse;
    CAE_ConnPointExtC::Slot slot(Templ());
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = iSlotTempl.Dests().begin(); it != iSlotTempl.Dests().end(); it++) {
	slot.Dests()[it->first] = Slot::slot_elem(aConnPoint, it->first);
    }
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = iSlotTempl.Srcs().begin(); it != iSlotTempl.Srcs().end(); it++) {
	slot.Srcs()[it->first] = Slot::slot_elem(aConnPoint, it->first);
    }
    Slots().push_back(slot);
    res = ETrue;
    return res;
}

void *CAE_ConnPointExtC::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : NULL;
}

TInt CAE_ConnPointExtC::SlotTempl::SrcCnt(const char* aName) {return iSrcs.count(aName);};
TInt CAE_ConnPointExtC::SlotTempl::DestCnt(const char* aName) {return iDests.count(aName);};

CAE_ConnPointExtC::Slot::Slot(const SlotTempl& aTempl)
{
    // Copy Dest and Srcs from template
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = aTempl.Dests().begin(); it != aTempl.Dests().end(); it++) {
	Dests()[it->first] = slot_elem(NULL, "");
    }
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = aTempl.Srcs().begin(); it != aTempl.Srcs().end(); it++) {
	Srcs()[it->first] = slot_elem(NULL, "");
    }
}

// CAE_EBase

CAE_EBase::CAE_EBase(const char* aInstName, CAE_Object* aMan): CAE_Base(), 
	iInstName(NULL), iTypeName(NULL), iMan(aMan), iUpdated(ETrue), iActive(ETrue), iQuiet(EFalse), iLogSpec(NULL)
{
    if (aInstName != NULL) 
	iInstName = strdup(aInstName);
};


void CAE_EBase::SetActive() 
{ 
	iActive= ETrue; 
	if (iMan && !iMan->IsActive()) iMan->SetActive();
};

void CAE_EBase::SetUpdated()
{ 
	iUpdated= ETrue; if (iMan) iMan->SetUpdated();
};

void CAE_EBase::SetName(const char *aName)
{
    if (iInstName != NULL)
	free(iInstName);
    if (aName != NULL) 
	iInstName = strdup(aName);
}

void CAE_EBase::SetType(const char *aName)
{
    if (iTypeName != NULL)
	free(iTypeName);
    if (aName != NULL) 
	iTypeName = strdup(aName);
}

FAPWS_API CAE_EBase::~CAE_EBase()
{
    if (iMan != NULL)
	iMan->UnregisterComp(this);
    if (iInstName != NULL)
	free(iInstName);
    if (iTypeName != NULL)
	free(iTypeName);
    if (iLogSpec != NULL)
	delete iLogSpec;
}

void CAE_EBase::AddLogSpec(TInt aEvent, TInt aData)
{
    if (iLogSpec == NULL)
    {
	iLogSpec = new vector<TLogSpecBase>;
    }
    _FAP_ASSERT (iLogSpec != NULL);
    iLogSpec->push_back(TLogSpecBase(aEvent, aData));
}

TInt CAE_EBase::GetLogSpecData(TInt aEvent) const
{	
    TInt res = KBaseDa_None;
    for (TInt i = 0; iLogSpec && i < iLogSpec->size(); i++)
    {
	const TLogSpecBase& spec = iLogSpec->at(i);
	if (spec.iEvent == aEvent) {
	    res = spec.iData; break;
	}
    }
    return res;
}

const char* CAE_EBase::MansName(TInt aLevel) const 
{ 
    CAE_EBase *man = iMan; 
    for (TInt i=aLevel; i > 0 && man != NULL && man->iMan != NULL; i--) man = man->iMan;
    return (man == NULL)? "": man->InstName();
}

// From CAE_Base
void *CAE_EBase::DoGetFbObj(const char *aName)
{
    if ((iTypeName != NULL) && (strcmp(iTypeName, aName) == 0) || (strcmp(aName, Type()) == 0))
	return this;
    else
	return NULL;
}

TTransInfo::TTransInfo(const string& aETrans): iFun(NULL), iOpInd(0), iHandle(NULL), iId(NULL)
{
    FormatEtrans(aETrans, iETrans);
}

void TTransInfo::FormatEtrans(const string& aTrans, string& aRes)
{
    // Remove leading tabs
    size_t pb = 0, pe = 0;
    TBool fin = EFalse;
    pb = aTrans.find_first_not_of("\n\t ", pe);
    if (pb == string::npos) fin = ETrue;
    while (!fin) {
	pe = aTrans.find('\n', pb);
	if (pe == string::npos || pe == aTrans.size()) {
	    pe = aTrans.size();
	    fin = ETrue;
	}
	string line = aTrans.substr(pb, pe - pb);
	pb = aTrans.find_first_not_of("\n\t ", pe);
	if (pb == string::npos) fin = ETrue;
	aRes += line + (fin ? "" : "\n");
    };
}

// ******************************************************************************
// CAE_StateBase - state base
// ******************************************************************************


CAE_StateBase::CAE_StateBase(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_EBase(aInstName, aMan), iTrans(aTrans)
{
    _FAP_ASSERT (iMan != NULL);
}


CAE_StateBase::~CAE_StateBase()
{
    // Delete inputs and outputs
    for (map<string, CAE_ConnPointBase*>::iterator i = iInputs.begin(); i != iInputs.end(); i++) {
	CAE_ConnPointBase* cpoint = i->second;
	if (cpoint != NULL)
	    delete cpoint;
    }
    if (iOutput != NULL) {
	delete iOutput;
	iOutput = NULL;
    }
}

void CAE_StateBase::ConstructL()
{
    if (iMan)
	iMan->RegisterCompL(this);
    CAE_ConnSlot::Template dt, st;
    dt["_1"]  = "State";
    st["_1"]  = "State";
    CAE_ConnPoint *outp = new CAE_ConnPoint("output", this, st, dt); 
    iOutput = outp;
    outp->Srcs()->SetPin("_1", this);
    if (Logger())
	Logger()->WriteFormat("State created:: Name: %s", iInstName);
}

CAE_StateBase::mult_point_inp_iterator CAE_StateBase::MpInput_begin(const char *aName) 
{ 
    CAE_ConnPoint *cp = iInputs.find(aName)->second->GetFbObj(cp); 
    return mult_point_inp_iterator(cp->Dests(), 0);
};

CAE_StateBase::mult_point_inp_iterator CAE_StateBase::MpInput_end(const char *aName) 
{ 
    CAE_ConnPoint *cp = iInputs.find(aName)->second->GetFbObj(cp); 
    return mult_point_inp_iterator(cp->Dests(), cp->Dests().size());
}

void CAE_StateBase::LogUpdate(TInt aLogData)
{
    if (Logger() == NULL) return;
    char *buf_cur = DataToStr(ETrue);
    if (buf_cur == NULL)
	buf_cur = strdup("undefined");
    char *buf_new = DataToStr(EFalse);
    if (buf_new == NULL)
	buf_new = strdup("undefined");
    if (aLogData & KBaseDa_New && aLogData & KBaseDa_Curr)
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s <= %s", MansName(1), MansName(0), InstName(), buf_new, buf_cur);
    else if (aLogData & KBaseDa_New)
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s", MansName(1), MansName(0), iInstName, buf_new);
    // Loggin inputs
    if (aLogData & KBaseDa_Dep)
    {
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s <<<<< ", MansName(1), MansName(0), iInstName, buf_new);
	for (map<string, CAE_ConnPointBase*>::iterator i = iInputs.begin(); i != iInputs.end(); i++)
	{
	    CAE_ConnPoint* cpoint = i->second->GetFbObj(cpoint);
	    map<string, CAE_Base*>::iterator it = cpoint->Slot(0)->Pins().begin();
	    CAE_StateBase* state = (*it).second->GetFbObj(state);
	    if (state != NULL) { /* input can be empty in case of ext input */
		char* buf = state->DataToStr(ETrue);
		if (Logger())
		    Logger()->WriteFormat(">>>> [%s]: %s", i->first.c_str(), buf);
		free(buf);	
	    }
	}
    }
    free (buf_cur);
    free (buf_new);
}

// TODO [YB] This logging doesn't make sense - to remove
void CAE_StateBase::LogTrans(TInt aLogData)
{
    if (Logger() == NULL) return;
    char *buf_cur = DataToStr(ETrue);
    char *buf_new = DataToStr(EFalse);
    if (aLogData & KBaseDa_Curr)
	Logger()->WriteFormat("Transf state [%s.%s.%s]: ?? <= %s", MansName(1), MansName(0), InstName(), buf_cur);
    // Loggin inputs
    if (aLogData & KBaseDa_Dep)
    {
	Logger()->WriteFormat("Transf state [%s.%s.%s]: ?? <= %s via ", MansName(1), MansName(0), iInstName, buf_cur);
	for (map<string, CAE_ConnPointBase*>::iterator i = iInputs.begin(); i != iInputs.end(); i++)
	{
	    CAE_ConnPoint* cpoint = i->second->GetFbObj(cpoint);
	    map<string, CAE_Base*>::iterator it = cpoint->Slot(0)->Pins().begin();
	    CAE_StateBase* state = (*it).second->GetFbObj(state);
	    if (state != NULL) { /* input can be empty in case of ext input */
		char* buf = state->DataToStr(ETrue);
		if (Logger())
		    Logger()->WriteFormat(">>>> [%s]: %s", i->first.c_str(), buf);
		free(buf);	
	    }
	}
    }
    free (buf_cur);
    free (buf_new);
}

CAE_StateBase* CAE_StateBase::Input(const char* aName)
{
    CAE_StateBase *res = NULL;
    map<string, CAE_ConnPointBase*>::iterator it = iInputs.find(aName);
    if (it == iInputs.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), aName);
    }
    else {
	CAE_ConnPoint *inp = it->second->GetFbObj(inp);
	if (inp->Dests().size() > 0 ) {
	    CAE_Base* refb = inp->Slot(0)->Pins().begin()->second;
	    res = (refb != NULL) ? refb->GetFbObj(res) : NULL;
	}
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), InstName(), aName);
	}
    }
    return res;
}

void CAE_StateBase::AddInputL(const char* aName) 
{
    TBool exists = iInputs.find(aName) != iInputs.end();
    if (exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already exists", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(!exists);
    }
    CAE_ConnSlot::Template dt, st;
    st["_1"]  = "State";
    dt["_1"]  = "State";
    CAE_ConnPoint *cpoint = new CAE_ConnPoint(aName, this, st, dt);
    cpoint->Srcs()->SetPin("_1", this);
    pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = iInputs.insert(pair<string, CAE_ConnPoint*>(aName, cpoint));
    bool bres = res.second;
}


void CAE_StateBase::SetInputL(const char *aName, CAE_StateBase* aState)
{
    // Check if the name is the name of extended input 
    char *name = strdup(aName);
    char *extp = strchr(name, '.');
    if (extp != NULL) {
	// Extended, form the name template for search
	*(extp + 1) = '*'; *(extp + 2) = 0;
    }
    // Check if the input is specified for the state
    TBool exists = iInputs.find(name) != iInputs.end();
    if (!exists) {
	if (Logger())
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), name);
	_FAP_ASSERT(name);
    }
    // For extended input add the input because only template input was added on creation
    if (extp != NULL) {
	AddInputL(aName);
    }
    CAE_ConnPointBase *inp = iInputs[aName];
    CAE_ConnPointBase *out = aState->Output();
    if (!inp->Connect(out) || !out->Connect(inp)) {
	if (Logger())
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already connected", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(EFalse);
	    };
    free(name);
}

// TODO [YB] It's possible to simply add extended input - not consistent with SetInput
void CAE_StateBase::AddInputL(const char *aName, CAE_StateBase* aState) 
{
    AddInputL(aName);
    SetInputL(aName, aState);
}

void CAE_StateBase::Set(void* aData)
{
    DoSet(aData);
}

void CAE_StateBase::SetFromStr(const char *aStr)
{
    DoSetFromStr(aStr);
}

TBool CAE_StateBase::SetTrans(TTransInfo aTinfo) 
{ 
	iTrans = aTinfo; 
	return ETrue;
};


void CAE_StateBase::DoTrans() 
{
    if (iTrans.iFun != NULL) 
	iTrans.iFun(iMan, this);
    else if (iTrans.iOpInd != 0)
	DoOperation();
    else if (!iTrans.iETrans.empty()) {
	iMan->DoTrans(this);
    }
};

// The base class has the trivial implementation of operation
void CAE_StateBase::DoOperation()
{
}

void CAE_StateBase::Update()
{
    TInt logdata = GetLogSpecData(KBaseLe_Trans);
    if (logdata != KBaseDa_None)
	LogTrans(logdata);
    DoTrans();
}

FAPWS_API TOperationInfo CAE_StateBase::OperationInfo(TUint8 aId) const
{
	TOperationInfo nfo = {aId, 0, 0, 0};
	return nfo;
}

void *CAE_StateBase::DoGetFbObj(const char *aName)
{
    if (strcmp(aName, Type()) == 0)
	return this;
    else
	return CAE_EBase::DoGetFbObj(aName);
}

const string CAE_StateBase::ValStr() const 
{ 
    char* data = DataToStr(ETrue); 
    string res(data); 
    free(data);
    return res;
}

CSL_ExprBase* CAE_StateBase::GetExpr(const string& aTerm, const string& aRtype) 
{ 
    return iMan->GetExpr(aTerm, aRtype);
};

multimap<string, CSL_ExprBase*>::iterator CAE_StateBase::GetExprs(const string& aName, const string& aRtype, multimap<string,
       	CSL_ExprBase*>::iterator& aEnd)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    if (iMan != NULL)
	res = iMan->GetExprs(aName, aRtype, aEnd);
    else {
	_FAP_ASSERT(EFalse);
    }
    return res;
}

// ******************************************************************************
// CAE_State - state handling owned data
// ******************************************************************************
	
void CAE_State::ConstructL()
{
    CAE_StateBase::ConstructL();
    iCurr = (void*) new  TUint8[iLen];
    iNew = (void*) new  TUint8[iLen];
    memset(iCurr, 0x00, iLen);
    memset(iNew, 0x00, iLen);
}	

CAE_State::~CAE_State()
{
    free(iCurr);
    free(iNew);
}

void CAE_State::Confirm()
{
    if (memcmp(iCurr, iNew, iLen))
    {
	CAE_ConnPoint *outp = iOutput->GetFbObj(outp);
	_FAP_ASSERT(outp != NULL);
	for (TInt i = 0; i < outp->Dests().size(); i++) {
	    CAE_StateBase* sout =  outp->Slot(i)->Pin("_1")->GetFbObj(sout);
	    _FAP_ASSERT(sout != NULL);
	    sout->SetActive();
	}
    }
    memcpy(iCurr, iNew, iLen); // iCurr << iNew
}

char* CAE_State::DataToStr(TBool aCurr) const
{
    return FmtData(aCurr ? iCurr : iNew, iLen);
}

void CAE_State::DataFromStr(const char* aStr, void *aData) const
{
    char *str = strdup(aStr);
    char *bf = strtok(str, " ");
    int elem;
    int res = 0;
    int i = 0;
    while (bf != NULL && res != EOF && i < iLen)
    {
	res = sscanf(bf, "%02x", &elem);
	((TUint8 *) aData)[i++] = elem;
	bf = strtok(NULL, " ");
    }
    free(str);
}

char *CAE_State::FmtData(void *aData, int aLen)
{
    char* buf = (char *) malloc(aLen*4);
    memset(buf, 0, aLen*4);
    char fmtsmb[10] = "";
    for (TInt i=0; i < aLen; i++)
    {
	int symb = ((TUint8*) aData)[i];
	fmtsmb[0] = 0;
	sprintf(fmtsmb, "%02x ", symb);
	strcat(buf, fmtsmb);
    }
    return buf;
}

void CAE_State::DoSet(void* aNew) 
{
    if (memcmp(aNew, iNew, iLen))
    {
	SetUpdated();
	memcpy(iNew, aNew, iLen); 
	TInt logdata = GetLogSpecData(KBaseLe_Updated);
	if (logdata != KBaseDa_None)
	    LogUpdate(logdata);
    }
};

void CAE_State::DoSetFromStr(const char *aStr)
{
    void *data = malloc(iLen);
    memset(data, 0, iLen);
    DataFromStr(aStr, data);
    Set(data);
    free(data);
}

const char* KCAE_StateName = "State";

CAE_State::CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_StateBase(aInstName, aMan, aTrans), iLen(aLen)
{
}

CAE_State* CAE_State::NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans)
{
	CAE_State* self = new CAE_State(aInstName, aLen, aMan, aTrans);
	self->ConstructL();
	return self;
}

void CAE_State::DoOperation()
{
}

// From CAE_Base
void *CAE_State::DoGetFbObj(const char *aName)
{
    if ((iTypeName != NULL) && (strcmp(iTypeName, aName) == 0) || (strcmp(aName, Type()) == 0))
	return this;
    else
	return CAE_StateBase::DoGetFbObj(aName);
}



//*********************************************************
// State handling data not owned but referenced
//*********************************************************

CAE_StateRef::CAE_StateRef(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_StateBase(aInstName, aMan, aTrans), iRef(NULL)
{
    CAE_StateBase::ConstructL();
}

void *CAE_StateRef::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : CAE_StateBase::DoGetFbObj(aName);
}

//*********************************************************
// State - controller of object
//*********************************************************

CAE_StateCtr::CAE_StateCtr(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_StateRef(aInstName, aMan, aTrans) 
{
};

CAE_StateCtr* CAE_StateCtr::New(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans)
{
    return new CAE_StateCtr(aInstName, aMan, aTrans);
}

void *CAE_StateCtr::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : CAE_StateRef::DoGetFbObj(aName);
}

char* CAE_StateCtr::DataToStr(TBool aCurr) const
{
    char * res = strdup("Chromo updated");
    return res;
}

void CAE_StateCtr::DataFromStr(const char* aStr, void *aData) const
{
}

void CAE_StateCtr::DoSet(void* aData)
{
    iRef = (CAE_Base*) aData;
    CAE_Object::ChromoPx* chromo = iRef->GetFbObj(chromo);
    _FAP_ASSERT(chromo != NULL);
}

void CAE_StateCtr::DoSetFromStr(const char *aStr)
{
}

void CAE_StateCtr::Confirm()
{
    CAE_Object::ChromoPx* chromo = iRef->GetFbObj(chromo);
    _FAP_ASSERT(chromo != NULL);
    CAE_ChromoNode& mut = chromo->Mut().Root();
    if (mut.Begin() != mut.End())
    {
	CAE_ConnPoint *outp = iOutput->GetFbObj(outp);
	_FAP_ASSERT(outp != NULL);
	for (TInt i = 0; i < outp->Dests().size(); i++) {
	    CAE_StateBase* sout =  outp->Slot(i)->Pin("_1")->GetFbObj(sout);
	    _FAP_ASSERT(sout != NULL);
	    sout->SetActive();
	}
    }
 
}

//*********************************************************
// CAE element logging formatter
//*********************************************************

CAE_Formatter::CAE_Formatter(int aUid, TLogFormatFun aFun):
    iStateDataUid(aUid), iFun(aFun)
{
}


CAE_Formatter::~CAE_Formatter()
{
}

//*********************************************************
// Node of chromo spec
//*********************************************************

const string CAE_ChromoNode::Attr(TNodeAttr aAttr) 
{
    char* sattr = iMdl.GetAttr(iHandle, aAttr); 
    string res; 
    if (sattr != NULL)
	res.assign(sattr);
    free(sattr); 
    return res; 
};

const string CAE_ChromoNode::Attr(TNodeAttr aAttr) const
{
    char* sattr = iMdl.GetAttr(iHandle, aAttr); 
    string res; 
    if (sattr != NULL)
	res.assign(sattr);
    free(sattr); 
    return res; 
};

TInt CAE_ChromoNode::AttrInt(TNodeAttr aAttr) const 
{
    string attr = Attr(aAttr); 
    _FAP_ASSERT(!attr.empty()); 
    return atoi(attr.c_str());
};

TBool CAE_ChromoNode::AttrBool(TNodeAttr aAttr) const 
{ 
    string attr = Attr(aAttr); 
    _FAP_ASSERT(!attr.empty() || attr == "yes" || attr == "no"); 
    return (attr == "yes"); 
};

// TODO [YB] Node name should not contain ".". Reconsider nodes "conn" etc.
CAE_ChromoNode::Iterator CAE_ChromoNode::Find(NodeType aType, const string& aName) 
{ 
    CAE_ChromoNode::Iterator res = End();
    size_t pos = aName.find("/");
    string name = aName.substr(0, pos);
    for (CAE_ChromoNode::Iterator it = Begin(); it != End(); it++) {
	if (((*it).Type() == aType) && (name.compare((*it).Name()) == 0)) {
	    res = it;  break;
	}
    }
    if ((res != End()) &&  (pos != string::npos)) {
	res = (*res).Find(aType, aName.substr(pos + 1));	
    }
    return res;
};

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(NodeType aType, TNodeAttr aAttr, const string& aAttrVal)
{
    CAE_ChromoNode::Iterator res = End();
    for (CAE_ChromoNode::Iterator it = Begin(); it != End(); it++) {
	if (((*it).Type() == aType) && (*it).AttrExists(aAttr) && (aAttrVal.compare((*it).Attr(aAttr)) == 0)) {
	    res = it;  break;
	}
    }
    return res;

};


//*********************************************************
// CAE_Object
//*********************************************************

FAPWS_API CAE_Object::CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv):
    CAE_EBase(aInstName, aMan), iChromX(NULL), iEnv(aEnv), iMut(NULL), iChromo(NULL), iChromoIface(*this)
{
    if (iMan != NULL) 
	iMan->RegisterCompL(this);
}

void *CAE_Object::DoGetFbObj(const char *aName)
{
    CAE_EBase *res = NULL;
    if (strcmp(aName, Type()) == 0) {
	res = this;
    }
    return res;
}


void *CAE_Object::ChromoPx::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : iMaster.DoGetFbObj(aName);
}

FAPWS_API CAE_Object* CAE_Object::NewL(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv)
{
    CAE_Object* self = new CAE_Object(aInstName, aMan, aEnv);
    self->Construct();
    return self;
}

FAPWS_API CAE_Object* CAE_Object::NewL(const char* aInstName, CAE_Object* aMan, const void* aChrom, MAE_Env* aEnv)
{
	CAE_Object* self = new CAE_Object(aInstName, aMan, aEnv);
	self->SetChromosome(EChromOper_Copy, aChrom);
	// TODO [YB] To add chrom mutation
	self->ConstructL(NULL);
	return self;
}

FAPWS_API void CAE_Object::SetChromosome(TChromOper aOper, const void* aChrom1, const char* aChrom2)
{
    if (aChrom1 != NULL)
    {
	iEnv->Chman()->CopySpec(aChrom1, &iChromX);	
    }
}

void CAE_Object::Construct()
{
    iMut = iEnv->Provider()->CreateChromo();
    iChromo = iEnv->Provider()->CreateChromo();
    iChromo->Init(ENt_Robject);
    CAE_ChromoNode croot = iChromo->Root();
}

// On construction phase object is created from given chromosome
// If chromosome isn't given and provider exists then chromosome is created randomly
// There is restrictions of mutation implemented on construction phase:
// If there is the locus with the unallowed allele then it's considered as mutation
// and the allele is generated randomly in the legal value interval   

// TODO [YB] Do we need ConstructL? It's just wrapper of ConstructFromChromXL
void CAE_Object::ConstructL(const void* aChrom)
{
    if (aChrom != NULL)
	ConstructFromChromXL(aChrom);
    else if (iChromX != NULL)
	ConstructFromChromXL(iChromX);
}

// TODO [YB] Prevent registering the same comp again. Migrate to map.
void  CAE_Object::RegisterCompL(CAE_EBase* aComp) 
{ 
    iCompReg.push_back(aComp); 
    aComp->SetMan(this);
    if (aComp->IsUpdated()) SetUpdated();
};

// TODO [YB] To support referencing to object in spec instead of direct definition
// TODO [YB] To add object proxy state 
FAPWS_API void CAE_Object::ConstructFromChromXL(const void* aChromX)
{
    MAE_ChroMan *chman = iEnv->Chman();
    MAE_Provider *prov = iEnv->Provider();
    _FAP_ASSERT(chman != NULL);
    char *name = chman->GetName((void *) aChromX);
    char *type = chman->GetType((void *) aChromX);
    if (iInstName == NULL && name != NULL && strlen(name) != 0)
    {
	SetName(name);
    }
    if (iTypeName == NULL && type != NULL && strlen(type) != 0)
    {
	SetType(type);
    }
    Logger()->WriteFormat("Object constructing. Name: %s, Parent: %s", InstName(), TypeName());
    // Pass thru children
    void *child = NULL;
    // Go thru children list in spec and create the children
    for (child = chman->GetChild((void *) aChromX); child != NULL; child = chman->GetNext(child))
    {
	TCaeElemType ftype = chman->FapType(child);
	// TODO [YB] Incorrect approach for creating subsystem. It should be created clean or cloned and then mutated.
	if (ftype == ECae_Object)
	{
	    char *sparent = chman->GetStrAttr(child, KXTransAttr_Type);
	    char *sname = chman->GetName(child); 
	    char *squiet = chman->GetStrAttr(child, "quiet");
	    if (strcmp(sparent, "none") == 0) {
		CAE_Object *obj = CAE_Object::NewL(NULL, this, child, iEnv);
		if (obj == NULL) {
		    Logger()->WriteFormat("ERROR: Creating object [%s]", sname);
		}
		else {
		    obj->SetQuiet(squiet != NULL);
		    if (squiet != NULL) {
			Logger()->WriteFormat("ATTENTION: Object [%s] created as quiet", sname);
		    }
		}
	    }
	    else {
		CAE_Object *parent = GetComp(sparent);
		if (parent == NULL) {
		    Logger()->WriteFormat("ERROR: Creating object [%s] - parent [%s] not found", sname, sparent);
		}
		else {
		    parent->CreateNewL(child, sname, this);
		}
	    }
	}
	else if (ftype == ECae_State)
	{
	    CAE_StateBase *state = NULL;
	    TCaeMut mut = chman->MutType(child);
	    _FAP_ASSERT(mut != ECaeMut_None);
	    char *stype = chman->GetStrAttr(child, KXTransAttr_Type);
	    char *name = chman->GetName(child); 
	    int len = chman->GetLen(child); 
	    char *transf_name = chman->GetStrAttr(child, KXStateAttr_Transf);
	    const TTransInfo *trans = NULL;
	    if (transf_name != NULL)
	    {
		trans = prov->GetTransf(transf_name);
		if (trans == NULL)
		    Logger()->WriteFormat("ERROR: Transition [%s] not found", name);
		_FAP_ASSERT(trans != NULL);
	    }
	    char *init = chman->GetStrAttr(child, KXStateAttr_Init);
	    if (mut == ECaeMut_Add) {
		state = prov->CreateStateL(stype, name, this);  
		if (state == NULL)
		    Logger()->WriteFormat("ERROR: Creating state [%s] failed", name);
	    }
	    else if (mut == ECaeMut_Change) {
		state = (CAE_StateBase *) FindByName(name);
		if (state == NULL)
		    Logger()->WriteFormat("ERROR: Changing state [%s]: state not found", name);
	    }
	    else /* mut == ECaeMut_Del */ {
	    }
	    if (state != NULL)
	    {
		if (trans != NULL)
		    state->SetTrans(*trans);
		// Go thru state elements
		for (void *stelem = chman->GetChild(child); stelem != NULL; stelem = chman->GetNext(stelem))
		{
		    TCaeElemType stetype = chman->FapType(stelem);
		    if (stetype == ECae_Logspec)
		    {
			// Set logspec 
			char *sevent = chman->GetStrAttr(stelem, KXTransAttr_LogEvent);
			TInt event = LsEventFromStr(sevent);
			// Get logging data
			TInt ldata = 0;
			for (void *lselem = chman->GetChild(stelem); lselem != NULL; lselem = chman->GetNext(lselem))
			{
			    TCaeElemType lstype = chman->FapType(lselem);
			    if (lstype == ECae_Logdata)
			    {
				char *sdata = chman->GetStrAttr(lselem, KXTransAttr_LogData);
				TInt data = LsDataFromStr(sdata);
				ldata |= data;
			    }
			    else
			    {
				Logger()->WriteFormat("ERROR: Unknown type [%s]", chman->GetName(lselem));
			    }
			}
			state->AddLogSpec(event, ldata);
		    }
		    else if (stetype == ECae_Stinp) {
			// Set inputs
			char *sinpid = chman->GetStrAttr(stelem, KXStateInpAttr_Id);
			if (sinpid == NULL)
			    Logger()->WriteFormat("ERROR: Creating state [%s]: empty input name", name);
			else {
			    CreateStateInp(stelem, state);
			    //state->AddInputL(sinpid);
			}
		    }
		    else
		    {
			Logger()->WriteFormat("ERROR: Unknown type [%s]", chman->GetName(stelem));
		    }
		}
		if (init != NULL) {
		    state->SetFromStr(init);
		    state->Confirm();
		}
	    }
	}
	// State mutation
	// TODO [YB] This is obsolete - to remove
	else if (ftype == ECae_State_Mut)
	{
	    char *name = chman->GetName(child); 
	    char *transf_name = chman->GetStrAttr(child, KXStateAttr_Transf);
	    char *init = chman->GetStrAttr(child, KXStateAttr_Init);
	    CAE_StateBase* state = (CAE_StateBase *) FindByName(name);
	    if (state == NULL) {
		Logger()->WriteFormat("ERROR: Mutating state [%s]: state not found", name);
	    }
	    if (init != NULL) {
		state->SetFromStr(init);
		state->Confirm();
	    }
	}
	// Object input
	else if (ftype == ECae_Stinp) {
	    CreateConn(child, iInputs);
	}
	// Object output
	else if (ftype == ECae_Soutp) {
	    CreateConn(child, iOutputs);
	}
	// Connection
	else if (ftype == ECae_Conn)
	{
	    char *p1_name = chman->GetStrAttr(child,"id"); 
	    char *p2_name = chman->GetStrAttr(child,"pair"); 
	    if ((p1_name != NULL) && (p2_name != NULL))
	    {
		CAE_ConnPointBase* p1 = GetConn(p1_name);
		CAE_ConnPointBase* p2 = GetConn(p2_name);
		if ((p1 != NULL) && (p2 != NULL))
		{
		    if (!p1->Connect(p2) || ! p2->Connect(p1))
		    {
			Logger()->WriteFormat("ERROR: Connecting [%s] <> [%s]: failure", p1_name, p2_name);
		    }
		}
		else if (p2 == NULL) {
		    Logger()->WriteFormat("ERROR: Connecting [%s] <> [%s]: [%s] not found", p1_name, p2_name, p2_name);
		}
		else {
		    Logger()->WriteFormat("ERROR: Connecting [%s] <> [%s]: [%s] not found", p1_name, p2_name, p1_name);
		}
	    }
	}
	// Connecting extention
	else if (ftype == ECae_Cext)
	{
	    char *p1_name = chman->GetStrAttr(child,"id"); 
	    char *p2_name = chman->GetStrAttr(child,"pair"); 
	    if ((p1_name != NULL) && (p2_name != NULL))
	    {
		CAE_ConnPointBase* p1 = GetConn(p1_name);
		CAE_ConnPointBase* p2 = GetConn(p2_name);
		if ((p1 != NULL) && (p2 != NULL))
		{
		    if (!p1->Extend(p2))
		    {
			Logger()->WriteFormat("ERROR: Extention [%s] <- [%s]: failure", p1_name, p2_name);
		    }
		}
		else if (p2 == NULL) {
		    Logger()->WriteFormat("ERROR: Extending [%s] <-  [%s]: [%s] not found", p1_name, p2_name, p2_name);
		}
		else {
		    Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: [%s] not found", p1_name, p2_name, p1_name);
		}
	    }
	}
	// Custom Connecting extention
	else if (ftype == ECae_Cextc)
	{
	    char *exname = chman->GetStrAttr(child,"id"); 
	    if (exname != NULL)
	    {
		CAE_ConnPointBase* epb = GetConn(exname);
		CAE_ConnPointExtC* ep = epb->GetFbObj(ep);
		if (ep != NULL) {
		    // Create slog
		    CAE_ConnPointExtC::Slot slot(ep->Templ());
		    // Go thru custom extention elements
		    for (void *elem = chman->GetChild(child); elem != NULL; elem = chman->GetNext(elem))
		    {
			TCaeElemType etype = chman->FapType(elem);
			if (etype == ECae_CextcSrc)
			{
			    char *pin_name = chman->GetStrAttr(elem,"id"); 
			    char *pair_name = chman->GetStrAttr(elem,"pair"); 
			    CAE_ConnPointBase* pairb = GetConn(pair_name);
			    if (ep->Templ().Srcs().count(pin_name) > 0) {
				if (pairb != NULL) {
				    // [YB] So far the default pin in pair is used
				    slot.Srcs()[pin_name] = CAE_ConnPointExtC::Slot::slot_elem(pairb, "_1");
				}
				else {
				    Logger()->WriteFormat("ERROR: Custom extenion [%s],[%s]: pair [%s] not found", exname, pin_name, pair_name);
				}
			    }
			    else {
				Logger()->WriteFormat("ERROR: Custom extenion [%s]: pin [%s]  not found", exname, pin_name);
			    }
			}
			else if (etype == ECae_CextcDest)
			{
			    char *pin_name = chman->GetStrAttr(elem,"id"); 
			    char *pair_name = chman->GetStrAttr(elem,"pair"); 
			    CAE_ConnPointBase* pairb = GetConn(pair_name);
			    if (ep->Templ().Dests().count(pin_name) > 0) {
				if (pairb != NULL) {
				    // [YB] So far the default pin in pair is used
				    slot.Dests()[pin_name] = CAE_ConnPointExtC::Slot::slot_elem(pairb, "_1");
				}
				else {
				    Logger()->WriteFormat("ERROR: Custom extenion [%s],[%s]: pair [%s] not found", exname, pin_name, pair_name);
				}
			    }
			    else {
				Logger()->WriteFormat("ERROR: Custom extenion [%s]: pin [%s]  not found", exname, pin_name);
			    }
			}
			else {
			    Logger()->WriteFormat("ERROR: Custom extenion [%s]: unknown spec element", exname);
			}

		    }
		    // Add the slot
		    ep->Slots().push_back(slot);
		}
		else {
		    Logger()->WriteFormat("ERROR: Custom extenion [%s] : conn point not found", exname);
		}
	    }
	}
	else
	{
	    // TODO Exception to be raised
	}
    }
}

void CAE_Object::CreateStateInp(void* aSpecNode, CAE_StateBase *aState) 
{
    TBool r_dest_spec = EFalse;
    MAE_ChroMan *chman = iEnv->Chman();
    char *name = chman->GetStrAttr(aSpecNode, KXStateInpAttr_Id);
    if (name == NULL)
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", InstName(), name);
    else {
	if (aState->Inputs().count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), InstName(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnSlot::Template st, dt;
	st["_1"] = "State";
	if (chman->GetChild(aSpecNode) != NULL) {
	    // There are pins spec, processing
	    for (void *elem = chman->GetChild(aSpecNode); elem != NULL; elem = chman->GetNext(elem)) {
		TCaeElemType etype = chman->FapType(elem);
		if (etype == ECae_CpDest)
		{
		    char *name = chman->GetStrAttr(elem,"id"); 
		    char *type = chman->GetStrAttr(elem,"type"); 
		    // Create default templ if type isnt specified
		    dt[name] = type ? type: "State";
		    r_dest_spec = ETrue;
		}
	    }
	}
	if (!r_dest_spec) {
	    dt["_1"] = "State";
	}
	CAE_ConnPoint *cpoint = new CAE_ConnPoint(name, aState, st, dt);
	cpoint->Srcs()->SetPin("_1", aState);
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aState->Inputs().insert(pair<string, CAE_ConnPointBase*>(name, cpoint));
    }
}

void CAE_Object::CreateStateInp(const CAE_ChromoNode& aSpec, CAE_StateBase *aState) 
{
    TBool r_dest_spec = EFalse;
    const string sname = aSpec.Name();
    const char *name = sname.c_str();
    if (sname.empty())
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", InstName(), name);
    else {
	if (aState->Inputs().count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), InstName(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnSlot::Template st, dt;
	st["_1"] = "State";
	for (CAE_ChromoNode::Const_Iterator elemit = aSpec.Begin(); elemit != aSpec.End(); elemit++) {
	    CAE_ChromoNode elem = *elemit;
	    if (elem.Type() == ENt_CpDest)
	    {
		const string sname = elem.Name();
		const char *name = sname.c_str(); 
		const string stype = elem.Attr(ENa_PinType);
		const char *type = stype.c_str(); 
		// Create default templ if type isnt specified
		dt[name] = type ? type: "State";
		r_dest_spec = ETrue;
	    }
	}
	if (!r_dest_spec) {
	    dt["_1"] = "State";
	}
	CAE_ConnPoint *cpoint = new CAE_ConnPoint(name, aState, st, dt);
	cpoint->Srcs()->SetPin("_1", aState);
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aState->Inputs().insert(pair<string, CAE_ConnPointBase*>(name, cpoint));
    }
}


void CAE_Object::CreateConn(void* aSpecNode, map<string, CAE_ConnPointBase*>& aConns) 
{
    MAE_ChroMan *chman = iEnv->Chman();
    char *name = chman->GetStrAttr(aSpecNode, KXStateInpAttr_Id);
    if (name == NULL)
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", InstName(), name);
    else {
	if (aConns.count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), InstName(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnPointBase* cp = NULL;
	if (chman->GetChild(aSpecNode) != NULL) {
	    // There are pins spec - Commutating extender
	    CAE_ConnPointExtC *cpoint = new CAE_ConnPointExtC(name, this);
	    cp = cpoint;
	    for (void *elem = chman->GetChild(aSpecNode); elem != NULL; elem = chman->GetNext(elem)) {
		TCaeElemType etype = chman->FapType(elem);
		if (etype == ECae_CpSource)
		{
		    char *name = chman->GetStrAttr(elem,"id"); 
		    cpoint->Templ().Srcs()[name] = "";
		}
		else if (etype == ECae_CpDest)
		{
		    char *name = chman->GetStrAttr(elem,"id"); 
		    cpoint->Templ().Dests()[name] = "";
		}
	    }
	}
	else {
	    // Simple extender
	    cp = new CAE_ConnPointExt(name, this);
	}
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aConns.insert(pair<string, CAE_ConnPointBase*>(name, cp));
    }
}

void CAE_Object::AddConn(const CAE_ChromoNode& aSpecNode, map<string, CAE_ConnPointBase*>& aConns) 
{
    const string sname = aSpecNode.Name();
    const char* name = sname.c_str();
    if (sname.empty())
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", InstName(), name);
    else {
	if (aConns.count(sname) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), InstName(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnPointBase* cp = NULL;
	if ((aSpecNode.Find(ENt_CpSource) != aSpecNode.End()) || (aSpecNode.Find(ENt_CpDest) != aSpecNode.End()))  {
	    // There are pins spec - Commutating extender
	    CAE_ConnPointExtC *cpoint = new CAE_ConnPointExtC(sname, this);
	    for (CAE_ChromoNode::Const_Iterator it = aSpecNode.Begin(); it != aSpecNode.End(); it++) {
		CAE_ChromoNode node = (*it);
		if (node.Type() == ENt_CpSource) {
		    cpoint->Templ().Srcs()[node.Name()] = "";
		}
		else if (node.Type() == ENt_CpDest) {
		    cpoint->Templ().Dests()[node.Name()] = "";
		}
	    }
	    cp = cpoint;
	}
	else {
	    // Simple extender
	    cp = new CAE_ConnPointExt(sname, this);
	}
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aConns.insert(pair<string, CAE_ConnPointBase*>(sname, cp));
    }
}

FAPWS_API CAE_Object::~CAE_Object()
{
	TInt count = iCompReg.size();
	// Use the revers orders recalling that deleting object cause unregister it from parent
	for (TInt i = count-1; i >= 0; i-- )
	{
		CAE_EBase* obj = (CAE_EBase*) iCompReg.at(i);
		delete obj;
	}
	iCompReg.clear();
	if (iChromX != NULL) {
	    delete (char *) iChromX;
	    iChromX = NULL;
	}
	iEnv = NULL; // Not owned

	if (iMut != NULL) {
	    delete iMut;
	    iMut = NULL;
	}
	if (iChromo != NULL) {
	    delete iChromo;
	    iChromo = NULL;
	}
	for (map<string, MAE_View*>::iterator it = iViews.begin(); it != iViews.end(); it++) {
	    delete it->second;
	}
};

TInt CAE_Object::LsEventFromStr(const char *aStr)
{
    if (strcmp(aStr, "upd") == 0) return KBaseLe_Updated;
    else if (strcmp(aStr, "cre") == 0) return KBaseLe_Creation;
    else if (strcmp(aStr, "any") == 0) return KBaseLe_Any;
    else if (strcmp(aStr, "tran") == 0) return KBaseLe_Trans;
    else return KBaseLe_None;
}

TInt CAE_Object::LsDataFromStr(const char *aStr)
{
    if (strcmp(aStr, "cur") == 0) return KBaseDa_Curr;
    else if (strcmp(aStr, "new") == 0) return KBaseDa_New;
    else if (strcmp(aStr, "dep") == 0) return KBaseDa_Dep;
    else if (strcmp(aStr, "trex") == 0) return KBaseDa_Trex;
    else return KBaseDa_None;
}

void CAE_Object::Confirm()
{
    // Confirm the elements
    for (TInt i = 0; i < iCompReg.size(); i++ )
    {
	CAE_EBase* obj = (CAE_EBase*) iCompReg.at(i);
	if (obj->IsUpdated() && !obj->IsQuiet())
	{
	    obj->Confirm();
	    obj->ResetUpdated();
	}
    }
    // Mutate if required
    if (iMut != NULL) {
	CAE_ChromoNode mut = iMut->Root();
	if (mut.Begin() != mut.End())
	{
	    Mutate();
	    // Reactivate all the components
	    for (TInt i = 0; i < iCompReg.size(); i++ )
	    {
		CAE_EBase* elem = (CAE_EBase*) iCompReg.at(i);
		if (elem != NULL) {
		    elem->SetActive();
		}
	    }
	}
    }
};

void CAE_Object::SetMutation(const CAE_ChromoNode& aMuta)
{
    iMut->Set(aMuta);
}

void CAE_Object::AddObject(const CAE_ChromoNode& aNode)
{
    string sparent = aNode.Attr(ENa_Type);
    string sname = aNode.Name();
    TBool squiet = aNode.AttrExists(ENa_ObjQuiet) && aNode.AttrBool(ENa_ObjQuiet);
    CAE_Object* obj = NULL;
    if (sparent == "none") 
    {
	obj = CAE_Object::NewL(aNode.Name().c_str(), this, iEnv);
    }
    else
    {
	CAE_Object *parent = GetComp(sparent.c_str());
	if (parent == NULL) {
	    Logger()->WriteFormat("ERROR: Creating object [%s] - parent [%s] not found", sname.c_str(), sparent.c_str());
	}
	else {
	    // Create heir from the parent
	    obj = parent->CreateHeir(sname.c_str(), this);
	}
    }
    if (obj == NULL) {
	Logger()->WriteFormat("ERROR: Creating object [%s]", sname.c_str());
    }
    else {
	obj->SetQuiet(squiet);
	if (squiet) {
	    Logger()->WriteFormat("ATTENTION: Object [%s] created as quiet", sname.c_str());
	}
	// Mutate object if needed
	for (CAE_ChromoNode::Const_Iterator imut = aNode.Begin(); imut != aNode.End(); imut++) {
	    if ((*imut).Type() == ENt_Mut) {
		obj->SetMutation(*imut);
		obj->Mutate();
	    }
	}
    }
}

void CAE_Object::AddState(const CAE_ChromoNode& aSpec)
{
    MAE_Provider *prov = iEnv->Provider();
    CAE_StateBase *state = NULL;
    string atype = aSpec.Attr(ENa_Type);
    const char *stype = atype.c_str();
    string aname = aSpec.Name(); 
    const char *name = aname.c_str(); 
    TBool lenexists = aSpec.AttrExists(ENa_StLen);
    int len = lenexists ? aSpec.AttrInt(ENa_StLen) : NULL; 
    string atransf = aSpec.Attr(ENa_Transf);
    const TTransInfo *trans = NULL;
    if (!atransf.empty())
    {
	const char *transf_name = atransf.c_str();
	trans = prov->GetTransf(transf_name);
	if (trans == NULL)
	    Logger()->WriteFormat("ERROR: Transition [%s] not found", name);
	_FAP_ASSERT(trans != NULL);
    }
    string ainit = aSpec.Attr(ENa_StInit);
    const char *init = ainit.c_str();
    state = prov->CreateStateL(stype, name, this);  
    if (state == NULL)
	Logger()->WriteFormat("ERROR: Creating state [%s] failed", name);
    else
    {
	if (trans != NULL)
	    state->SetTrans(*trans);
	// Go thru state elements
	for (CAE_ChromoNode::Const_Iterator stelemit = aSpec.Begin(); stelemit != aSpec.End(); stelemit++)
	{
	    const CAE_ChromoNode stelem = *stelemit;
	    string stelemname = stelem.Name();
	    if (stelem.Type() == ENt_Logspec)
	    {
		// Set logspec 
		string aevnet = stelem.Attr(ENa_Logevent);
		const char *sevent = aevnet.c_str();
		TInt event = LsEventFromStr(sevent);
		// Get logging data
		TInt ldata = 0;
		for (CAE_ChromoNode::Const_Iterator lselemit = stelem.Begin(); lselemit != stelem.End(); lselemit++)
		{
		    CAE_ChromoNode lselem = *lselemit;
		    if (lselem.Type() == ENt_Logdata)
		    {
			string adata = lselem.Name();
			TInt data = LsDataFromStr(adata.c_str());
			ldata |= data;
		    }
		    else
		    {
			string aname = lselem.Name();
			Logger()->WriteFormat("ERROR: Unknown type [%s]", aname.c_str());
		    }
		}
		state->AddLogSpec(event, ldata);
	    }
	    else if (stelem.Type() == ENt_Stinp) {
		// Set inputs
		string ainpid = stelem.Name();
		const char *sinpid = ainpid.c_str();
		if (sinpid == NULL)
		    Logger()->WriteFormat("ERROR: Creating state [%s]: empty input name", name);
		else {
		    CreateStateInp(stelem, state);
		}
	    }
	    else if (stelem.Type() == ENt_Trans) {
		// Embedded transition
		CAE_ChromoNode::Const_Iterator nodetxt = stelem.FindText();
		TTransInfo tinfo((*nodetxt).Content());
		state->SetTrans(tinfo);
	    }
	    else
	    {
		Logger()->WriteFormat("ERROR: Unknown type [%s]", stelemname.c_str());
	    }
	}
	if (init != NULL) {
	    CAE_StateCtr* ctr = state->GetFbObj(ctr);
	    if (ctr != NULL) {
		CAE_Object* obj = (strcmp(init, "self") == 0) ? this: GetComp(init);
		ctr->Set(obj->ChromoIface());
	    }
	    else {
		state->SetFromStr(init);
		state->Confirm();
	    }
	}
    }
}

void CAE_Object::AddConn(const CAE_ChromoNode& aSpec)
{
    string sname = aSpec.Name();
    const char *p1_name = sname.c_str(); 
    string spair = aSpec.Attr(ENa_ConnPair);
    const char *p2_name = spair.c_str(); 
    if (!sname.empty() && !spair.empty())
    {
	CAE_ConnPointBase* p1 = GetConn(p1_name);
	CAE_ConnPointBase* p2 = GetConn(p2_name);
	if ((p1 != NULL) && (p2 != NULL))
	{
	    if (!p1->Connect(p2) || ! p2->Connect(p1))
	    {
		Logger()->WriteFormat("ERROR: Connecting [%s] <> [%s]: failure", p1_name, p2_name);
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Connecting [%s] <> [%s]: [%s] not found", p1_name, p2_name, (p1 == NULL) ? p1_name: p2_name);
	}
    }
}

void CAE_Object::AddTrans(const CAE_ChromoNode& aSpec)
{
    CAE_ChromoNode::Const_Iterator iconnode = aSpec.FindText();
    SetTrans((*iconnode).Content());
    iEnv->Tranex()->EvalTrans(this, this, iTransSrc);
    iTrans = iEnv->Tranex()->Exprs();
}

// TODO [YB] For now it is not possible to have "multipoint" output that extends multiple output conn points. To implement.
void CAE_Object::AddExt(const CAE_ChromoNode& aSpec)
{
    string sname = aSpec.Name();
    const char *p1_name = sname.c_str(); 
    string spair = aSpec.Attr(ENa_ConnPair);
    const char *p2_name = spair.c_str(); 
    if (!sname.empty() && !spair.empty())
    {
	CAE_ConnPointBase* p1 = GetConn(p1_name);
	CAE_ConnPointBase* p2 = GetConn(p2_name);
	if ((p1 != NULL) && (p2 != NULL))
	{
	    if (!p1->Extend(p2) || !(p2->Extend(p1)))
	    {
		Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: failure", p1_name, p2_name);
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: [%s] not found", p1_name, p2_name, (p1 == NULL) ? p1_name: p2_name);
	}
    }
}

// TODO [YB] There is functionality in ExtC of extending multiple connection point
// But this functionality missed in simple extender
void CAE_Object::AddExtc(const CAE_ChromoNode& aSpec)
{
    string stexname = aSpec.Name();
    const char *exname = stexname.c_str(); 
    if (!stexname.empty())
    {
	CAE_ConnPointBase* epb = GetConn(exname);
	CAE_ConnPointExtC* ep = epb->GetFbObj(ep);
	if (ep != NULL) {
	    // Create slog
	    CAE_ConnPointExtC::Slot slot(ep->Templ());
	    // Go thru custom extention elements
	    for (CAE_ChromoNode::Const_Iterator elemit = aSpec.Begin(); elemit != aSpec.End(); elemit++)
	    {
		CAE_ChromoNode elem = *elemit;
		NodeType etype = elem.Type();
		if (etype == ENt_CextcSrc)
		{
		    string spin_name = elem.Name();
		    const char *pin_name = spin_name.c_str(); 
		    string spair_name = elem.Attr(ENa_ConnPair);
		    const char *pair_name = spair_name.c_str(); 
		    CAE_ConnPointBase* pairb = GetConn(pair_name);
		    if (ep->Templ().Srcs().count(pin_name) > 0) {
			if (pairb != NULL) {
			    // [YB] So far the default pin in pair is used
			    slot.Srcs()[pin_name] = CAE_ConnPointExtC::Slot::slot_elem(pairb, "_1");
			}
			else {
			    Logger()->WriteFormat("ERROR: Custom extenion [%s],[%s]: pair [%s] not found", exname, pin_name, pair_name);
			}
		    }
		    else {
			Logger()->WriteFormat("ERROR: Custom extenion [%s]: pin [%s]  not found", exname, pin_name);
		    }
		}
		else if (etype == ENt_CextcDest)
		{
		    string spin_name = elem.Name();
		    const char *pin_name = spin_name.c_str(); 
		    string spair_name = elem.Attr(ENa_ConnPair);
		    const char *pair_name = spair_name.c_str(); 
		    CAE_ConnPointBase* pairb = GetConn(pair_name);
		    if (ep->Templ().Dests().count(pin_name) > 0) {
			if (pairb != NULL) {
			    // [YB] So far the default pin in pair is used
			    slot.Dests()[pin_name] = CAE_ConnPointExtC::Slot::slot_elem(pairb, "_1");
			}
			else {
			    Logger()->WriteFormat("ERROR: Custom extenion [%s],[%s]: pair [%s] not found", exname, pin_name, pair_name);
			}
		    }
		    else {
			Logger()->WriteFormat("ERROR: Custom extenion [%s]: pin [%s]  not found", exname, pin_name);
		    }
		}
		else {
		    Logger()->WriteFormat("ERROR: Custom extenion [%s]: unknown spec element", exname);
		}

	    }
	    // Add the slot
	    ep->Slots().push_back(slot);
	}
	else {
	    Logger()->WriteFormat("ERROR: Custom extenion [%s] : conn point not found", exname);
	}
    }
}


void CAE_Object::ChangeAttr(const CAE_ChromoNode& aSpec, const CAE_ChromoNode& aCurr)
{
    NodeType ntype = aSpec.AttrNtype(ENa_Type);
    string nname = aSpec.Name();
    TNodeAttr mattr = aSpec.AttrNatype(ENa_MutChgAttr);
    string mattrs = aSpec.Attr(ENa_MutChgAttr);
    string mval = aSpec.Attr(ENa_MutChgVal);
    if (ntype == ENt_State)
    {
	// TODO [YB] To do find amongs states only
	CAE_EBase* elem = FindByName(nname.c_str());
	CAE_StateBase* state = elem->GetFbObj(state);
	if (state != NULL) {
	    if (mattr == ENa_StInit) {
		state->SetFromStr(mval.c_str());
		state->Confirm();
	    }
	    else {
		Logger()->WriteFormat("ERROR: Changing state [%s] - changing attr [%s] not supported", state->InstName(), mattrs.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Changing state [%s] - not found", nname.c_str());
	}
    }
    else if (ntype == ENt_Cext)
    {
	CAE_ConnPointBase* ext = GetConn(nname.c_str());
	if (ext != NULL) {
	    if (mattr == ENa_ConnPair) {
		string cpair_name = aCurr.Attr(ENa_ConnPair);
		CAE_ConnPointBase* cpair = GetConn(cpair_name.c_str());
		if (cpair != NULL) {
		    ext->Disextend(cpair);
		    CAE_ConnPointBase* npair = GetConn(mval.c_str());
		    if (npair != NULL) {
			ext->Extend(npair);
		    }
		    else {
			Logger()->WriteFormat("ERROR: Changing extention [%s] - new pair [%s] not found", nname.c_str(), mval.c_str());
		    }
		}
		else {
		    Logger()->WriteFormat("ERROR: Changing extention [%s] - ext with pair [%s] not found", nname.c_str(), cpair_name.c_str());
		}
	    }
	    else {
		Logger()->WriteFormat("ERROR: Changing extention [%s] - changing attr [%s] not supported", nname.c_str(), mattrs.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Changing extention [%s] - not found", nname.c_str());
	}
    }
}

// TODO [YB] To consider keeping connection as object's elem (id required). It would simplilify handling of conn.
void CAE_Object::RemoveElem(const CAE_ChromoNode& aSpec)
{
    NodeType type = aSpec.AttrNtype(ENa_Type);
    string name = aSpec.Name();
    if (type == ENt_Conn) {
	CAE_ChromoNode& chr = iChromo->Root();
	CAE_ChromoNode::Iterator conni = chr.Find(type, name);
	if (conni != chr.End()) {
	    // TODO [YB] There can be several connections with the same origin point, needs to have unique id of conn.
	    string pairname = (*conni).Attr(ENa_ConnPair);
	    CAE_ConnPointBase* conn = GetConn(name.c_str());
	    CAE_ConnPointBase* pair = GetConn(pairname.c_str());
	    if (conn != NULL) {
		conn->Disconnect(pair);
		pair->Disconnect(conn);
	    }
	    else {
		Logger()->WriteFormat("ERROR: Deleting connection from  [%s] - cannot find connection [%s]", InstName(), name.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Deleting connection from  [%s] - cannot find connection [%s]", InstName(), name.c_str());
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Deleting element from  [%s] - unsupportd element type for deletion", InstName());
    }
}

void CAE_Object::Mutate()
{
    DoMutation();
    // Clear mutation
    CAE_ChromoNode& root = iMut->Root();
    for (CAE_ChromoNode::Iterator mit = root.Begin(); mit != root.End(); mit++)
    {
	root.RmChild(*mit);
    }
}

void CAE_Object::AddLogspec(const CAE_ChromoNode& aSpec)
{
    // Set logspec 
    string aevnet = aSpec.Attr(ENa_Logevent);
    const char *sevent = aevnet.c_str();
    TInt event = LsEventFromStr(sevent);
    // Get logging data
    TInt ldata = 0;
    for (CAE_ChromoNode::Const_Iterator lselemit = aSpec.Begin(); lselemit != aSpec.End(); lselemit++)
    {
	CAE_ChromoNode lselem = *lselemit;
	if (lselem.Type() == ENt_Logdata)
	{
	    string adata = lselem.Name();
	    TInt data = LsDataFromStr(adata.c_str());
	    ldata |= data;
	}
	else
	{
	    string aname = lselem.Name();
	    Logger()->WriteFormat("ERROR: Unknown type [%s]", aname.c_str());
	}
    }
    AddLogSpec(event, ldata);
}

// TODO [YB] To carefully consider the concept:
// 1. We use FindByName - not sure it should have an access to all nodes
void CAE_Object::DoMutation()
{
    CAE_ChromoNode& root = iMut->Root();
    TBool emnode = root.AttrExists(ENa_MutNode);
    string mnode = root.Attr(ENa_MutNode);
    CAE_EBase* node = (!emnode || (mnode.compare("self") == 0)) ? this: FindByName(mnode.c_str());
    CAE_ChromoNode& chrroot = iChromo->Root();
    // Handling mutations
    for (CAE_ChromoNode::Iterator mit = root.Begin(); mit != root.End(); mit++)
    {
	CAE_ChromoNode mno = (*mit);
	if (node != NULL)
	{
	    if (mno.Type() == ENt_MutAdd) 
	    {
		for (CAE_ChromoNode::Iterator mait = mno.Begin(); mait != mno.End(); mait++)
		{
		    CAE_ChromoNode mano = (*mait);
		    if (mano.Type() == ENt_Object) {
			AddObject(mano);
		    }
		    else if (mano.Type() == ENt_Stinp) {
			AddConn(mano, iInputs);
		    }
		    else if (mano.Type() == ENt_Soutp) {
			AddConn(mano, iOutputs);
		    }
		    else if (mano.Type() == ENt_State) {
			AddState(mano);
		    }
		    else if (mano.Type() == ENt_Conn) {
			AddConn(mano);
		    }
		    else if (mano.Type() == ENt_Cext) {
			AddExt(mano);
		    }
		    else if (mano.Type() == ENt_Cextc) {
			AddExtc(mano);
		    }
		    else if (mano.Type() == ENt_Trans) {
			AddTrans(mano);
		    }
		    else if (mano.Type() == ENt_Logspec) {
			AddLogspec(mano);
		    }
		    else {
			Logger()->WriteFormat("ERROR: Mutating object [%s] - unknown element to be added", InstName());
		    }
		    chrroot.AddChild(mano);
		}
	    }
	    else if (mno.Type() == ENt_MutRm) 
	    {
		for (CAE_ChromoNode::Iterator mrmit = mno.Begin(); mrmit != mno.End(); mrmit++)
		{
		    CAE_ChromoNode mrno = *mrmit;
		    if (mrno.Type() == ENt_Node) {
			RemoveElem(mrno);
			NodeType type = mrno.AttrNtype(ENa_Type);
			string name = mrno.Name();
			CAE_ChromoNode::Iterator remit = chrroot.Find(type, name);
			if (remit != chrroot.End()) {
			    chrroot.RmChild(*remit);
			}
			else {
			    Logger()->WriteFormat("ERROR: Mutating object [%s] - cannot find node [%s] to remove", InstName(), name.c_str());
			}
		    }
		    else {
			Logger()->WriteFormat("ERROR: Mutating object [%s] - wrong node to be removed", InstName());
		    }
		}
	    }
	    else if (mno.Type() == ENt_MutChange) 
	    {
		NodeType type = mno.AttrNtype(ENa_Type);
		string name = mno.Name();
		CAE_ChromoNode::Iterator curr = chrroot.Find(type, name);
		if (curr != chrroot.End()) {
		    ChangeAttr(mno, *curr);
		}
		else {
		    Logger()->WriteFormat("ERROR: Mutating object [%s] - cannot find node [%s] to change", InstName(), name.c_str());
		}
	    }
	    else
	    {
		Logger()->WriteFormat("ERROR: Mutating object [%s] - inappropriate mutation", InstName());
	    }
	}
	else
	{
	    Logger()->WriteFormat("ERROR: Mutating object [%s] - unknown node to be mutated", InstName(), mnode.c_str());
	}
    }
}

void CAE_Object::Update()
{
    for (TInt i = 0; i < iCompReg.size(); i++ )
    {
	CAE_EBase* obj = (CAE_EBase*) iCompReg.at(i);
	if (obj->IsActive() && !obj->IsQuiet())
	{
	    obj->SetUpdated();
	    obj->ResetActive();
	    obj->Update();
	}
    }
}


void CAE_Object::UnregisterComp(CAE_EBase* aComp)
{
    for (TInt i = 0; i < iCompReg.size(); i++ )
	if (iCompReg.at(i) == aComp) {iCompReg.erase(iCompReg.begin() + i); break; };
}

FAPWS_API CAE_Object* CAE_Object::GetComp(const char* aName, TBool aGlob)
{
    CAE_Object* res = NULL;
    for (TInt i = 0; i < iCompReg.size(); i++) {
	CAE_Object* comp = (CAE_Object*) iCompReg.at(i);
	if (strcmp(aName, comp->InstName()) == 0) {
	    res = comp; break;
	}
    }
    if (res == NULL && aGlob) {
	for (TInt i = 0; i < iCompReg.size() && res == NULL; i++) {
	    CAE_Object* comp = (CAE_Object*) iCompReg.at(i);
	    res = comp->GetComp(aName, ETrue);
	}
    }
    return res;
}

FAPWS_API TInt CAE_Object::CountCompWithType(const char *aType)
{
    TInt res = 0;
    if (aType == NULL) {
	res = iCompReg.size();
    }
    else {
	for (TInt i = 0; i < iCompReg.size(); i++) {
	    CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
	    if ((comp != NULL) && (comp->TypeName() != NULL) && (strcmp(comp->TypeName(), aType) == 0)) {
		res++;
	    }
	}
    }
    return res;
}

// TODO [YB] Type to be supposed as FAP base type. Then what's the sense of this method?
FAPWS_API CAE_Object* CAE_Object::GetNextCompByFapType(const char *aType, int* aCtx) const
{
    CAE_Object *res = NULL, *elem = NULL;
    for (TInt i = aCtx?*aCtx:0; i < iCompReg.size(); i++)
    {
	CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
	if ((elem = (CAE_Object*) comp->GetFbObj(aType)) != NULL)
	{
	    res = elem;
	    *aCtx = i+1;
	    break;
	}
    }
    return res;
}

FAPWS_API CAE_Object* CAE_Object::GetNextCompByType(const char *aType, int* aCtx) const
{
    CAE_Object *res = NULL, *elem = NULL;
    for (TInt i = aCtx?*aCtx:0; i < iCompReg.size(); i++) {
	CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
	if ((comp->TypeName() != NULL) && (strcmp(comp->TypeName(), aType) == 0) && (elem = comp->GetFbObj(elem)) != NULL) {
	    res = elem;
	    *aCtx = i+1;
	    break;
	}
    }
    return res;
}


FAPWS_API CAE_EBase* CAE_Object::FindByName(const char* aName)
{
    CAE_EBase* res = NULL;
    char name[KNameMaxLen];
    const char* tail = strchr(aName, KNameSeparator);
    TInt namelen = tail?tail - aName:strlen(aName);
    if (namelen < KNameMaxLen)
    {
	strncpy(name, aName, namelen);
	name[namelen] = 0x00;

	if (tail == 0x00)
	{ // Simple name
	    for (TInt i = 0; i < iCompReg.size(); i++)
	    {
		CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
		if ((comp->InstName() != NULL) && (strcmp(name, comp->InstName()) == 0))
		{
		    res = comp; break;
		}
	    }
	}
	else
	{ // Full name
	    for (TInt i = 0; i < iCompReg.size(); i++)
	    {
		CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
		if (comp && strcmp(name, comp->InstName()) == 0)
		{
		    CAE_Object* obj = comp->GetFbObj(obj);
		    if (obj != NULL) {
			res = obj->FindByName(tail+1); 
			break;
		    }
		}
	    }
	}
    }
    return res;
}

CAE_ConnPointBase* CAE_Object::GetConn(const char *aName)
{
    CAE_ConnPointBase *res = GetInpN(aName);
    if (res == NULL)
	res = GetOutpN(aName);
    return res;
}

CAE_ConnPointBase* CAE_Object::GetInpN(const char *aName)
{
    CAE_EBase *elem = NULL;
    CAE_ConnPointBase * res = NULL;
    string ss(aName);
    string ename;
    string iname;
    map<string, CAE_ConnPointBase*>::iterator it;
    size_t found = ss.find_last_of(".");
    if (found != string::npos) 
    {
	ename = ss.substr(0, found);
	elem = FindByName(ename.c_str());
	iname = ss.substr(found + 1);
    }
    else
    {
	elem = this;
	iname = ss;
    }
    if (elem != NULL) {
	CAE_StateBase *state = elem->GetFbObj(state);
	if (state != NULL)
	{
	    it = state->Inputs().find(iname);
	    if (it != state->Inputs().end())
		res = it->second;
	}
	else 
	{
	    CAE_Object *obj = elem->GetFbObj(obj);
	    if (obj != NULL)
	    {
		it = obj->Inputs().find(iname);
		if (it != obj->Inputs().end())
		    res = it->second;
	    }
	}
    }
    return res;
}

CAE_ConnPointBase* CAE_Object::GetOutpN(const char *aName)
{
    CAE_EBase *elem = NULL;
    CAE_ConnPointBase * res = NULL;
    string ss(aName);
    string ename;
    string iname;
    map<string, CAE_ConnPointBase*>::iterator it;
    size_t found = ss.find_last_of(".");
    if (found != string::npos) 
    {
	ename = ss.substr(0, found);
	elem = FindByName(ename.c_str());
	iname = ss.substr(found + 1);
    }
    else
    {
	elem = this;
	iname = ss;
    }
    if (elem != NULL) {
	CAE_StateBase *state = elem->GetFbObj(state);
	if (state != NULL)
	{
	    if (iname.compare("output") == 0) {
		res = state->Output();
	    }
	}
	else 
	{
	    CAE_Object *obj = elem->GetFbObj(obj);
	    if (obj != NULL)
	    {
		it = obj->Outputs().find(iname);
		if (it != obj->Outputs().end())
		    res = it->second;
	    }
	}
    }
    return res;
}


FAPWS_API void CAE_Object::LinkL(CAE_StateBase* aInp, CAE_StateBase* aOut, TTransFun aTrans)
{
    //_FAP_ASSERT(aTrans || (aInp->Len() == aOut->Len())); // Panic(EFapPan_IncorrLinksSize)); 
    _FAP_ASSERT(aTrans); // Panic(EFapPan_IncorrLinksSize)); 
    aInp->AddInputL("_Inp_1");
    aInp->SetInputL("_Inp_1", aOut);
	aInp->SetTrans(TTransInfo(aTrans));
}

FAPWS_API CAE_StateBase* CAE_Object::GetStateByName(const char *aName)
{
    CAE_StateBase* res = NULL;
    for (TInt i = 0; i < iCompReg.size(); i++)
    {
	CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
	CAE_StateBase* state = comp->GetFbObj(state);
	if (state != NULL && (strcmp(aName, state->InstName()) == 0))
	{
	    res = state; 
	    break;
	}
    }
    return res;
}


TInt CAE_Object::ChromLen(const TUint8* aChrom) const
{
	TInt len = 1;
	TUint8 num = aChrom[0] & ~KAdp_ObjChromLen_SSD;
	TUint8 ssd = aChrom[0] & KAdp_ObjChromLen_SSD;
	if (ssd)
	{
		for (TInt i = 0; i < num; i++)
		{
			len += ChromLen(aChrom + len);
		}
	}
	else 
		len += num;
	return len;
}

// The inheritor is created by clone the parent and mutate it for now
// TODO [YB] Incorrect approach. Parent can only clone but not mutate. This should be done by owned system.
FAPWS_API CAE_Object* CAE_Object::CreateNewL(const void* aSpec, const char *aName, CAE_Object *aMan)
{
    CAE_Object* heir = new CAE_Object(aName, aMan, iEnv);
    heir->SetType(InstName());
    heir->SetChromosome(EChromOper_Copy, iChromX, NULL);
    // Construct from native parents chromosome
    heir->ConstructL();
    // Construct from original chromosome if specified
    if (aSpec != NULL)
	heir->ConstructL(aSpec);
    return heir;
}

CAE_Object* CAE_Object::CreateHeir(const char *aName, CAE_Object *aMan)
{
    CAE_Object* heir = new CAE_Object(aName, aMan, iEnv);
    heir->SetType(InstName());
    heir->Construct();
    // Mutate bare child with original parent chromo
    MAE_Chromo& mut = heir->Mut();
    mut.Init(ENt_Mut);
    CAE_ChromoNode mroot = mut.Root();
    mroot.SetAttr(ENa_MutNode, "self");
    CAE_ChromoNode madd = mroot.AddChild(ENt_MutAdd);
    const CAE_ChromoNode croot = iChromo->Root();
    for (CAE_ChromoNode::Const_Iterator it = croot.Begin(); it != croot.End(); it++) {
	CAE_ChromoNode elem = *it;
	madd.AddChild(elem);
    }
    heir->Mutate();
    return heir;
}

CAE_StateBase* CAE_Object::GetOutpState(const char* aName)
{
    CAE_StateBase *res = NULL;
    map<string, CAE_ConnPointBase*>::iterator it = iOutputs.find(aName);
    if (it == iOutputs.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), aName);
    }
    else {
	CAE_ConnPoint *cp = it->second->GetFbObj(cp);
	res = cp->Srcs()->Pins().begin()->second->GetFbObj(res);
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), InstName(), aName);
	}
    }
    return res;
}

CAE_StateBase* CAE_Object::GetInpState(const char* aName)
{
    CAE_StateBase *res = NULL;
    map<string, CAE_ConnPointBase*>::iterator it = iInputs.find(aName);
    if (it == iInputs.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), aName);
    }
    else {
	CAE_ConnPoint *cp = it->second->GetFbObj(cp);
	res = cp->Srcs()->Pins().begin()->second->GetFbObj(res);
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), InstName(), aName);
	}
    }
    return res;
}

void CAE_Object::DoTrans(CAE_StateBase* aState)
{
    iEnv->Tranex()->EvalTrans(this, aState, aState->iTrans.iETrans);
}

// TODO [YB] This method is used for state init. Actually init is trans that performs on creation phase.
// Why do not use the same spec notation as for trans?
void CAE_Object::DoTrans(CAE_StateBase* aState, const string& aInit)
{
    iEnv->Tranex()->EvalTrans(this, aState, aInit);
}

CSL_ExprBase* CAE_Object::GetExpr(const string& aName, const string& aRtype)
{
    CSL_ExprBase* res = NULL;
    if (res == NULL) {
	string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
	multimap<string, CSL_ExprBase*>::iterator it = iTrans.find(key);
	if (it != iTrans.end()) {
	    res = it->second;
	}
    }
    return (res != NULL) ? res : (iMan != NULL ? iMan->GetExpr(aName, aRtype) : NULL);
}

multimap<string, CSL_ExprBase*>::iterator CAE_Object::GetExprs(const string& aName, const string& aRtype,
	multimap<string, CSL_ExprBase*>::iterator& aEnd)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    // Look at emb terms first then for current
    string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
    multimap<string, CSL_ExprBase*>::iterator it = iTrans.find(key);
    if (it != iTrans.end()) {
	res = it; aEnd = iTrans.end();
    }
    else if (iMan != NULL) {
	multimap<string, CSL_ExprBase*>::iterator it = iMan->GetExprs(aName, aRtype, aEnd);
	if (it != aEnd) {
	    res = it;
	}
	else {
	    aEnd = iTrans.end(); res = aEnd;
	}
    }
    else {
	aEnd = iTrans.end(); res = aEnd;
    }
    return res;
}

CSL_ExprBase* CAE_Object::CreateDataExpr(const string& aType)
{
    return iEnv->Tranex()->GetExpr(aType, "");
}

void CAE_Object::SetTrans(const string& aTrans)
{
    TTransInfo::FormatEtrans(aTrans, iTransSrc);
}

void CAE_Object::AddBva(Bva* aBva)
{
    iBvas[TRelm(aBva->iType, aBva->iName)] = aBva;
}

CAE_Object::Bva* CAE_Object::GetBva(TReType aType, const string& aName)
{
    map<TRelm, Bva*>::iterator it = iBvas.find(TRelm(aType, aName));
    return (it != iBvas.end()) ? it->second : NULL; 
}

void CAE_Object::AddView(MAE_View* aView)
{
    _FAP_ASSERT (iViews[aView->Name()] == 0);
    iViews[aView->Name()] = aView;
    BvaSyst* bva = new BvaSyst(NULL, *this, aView->Wnd(), aView->Name());
    AddBva(bva);
    // TODO [YB] Hack
//    CAV_Rect rec = aView->Wnd()->Rect();
//    bva->Render(rec);
    bva->iWnd->Show(ETrue);
}

void CAE_Object::ResetView()
{
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	delete it->second;
    }
    iBvas.clear();
}

/*
TBool CAE_Object::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{
    vector<string> relm;
    if (aEvent == EBte_Press) {
	TReType retype = Et_Unknown;
	Render(aWnd, aWnd->Rect(), EFalse, aPt, retype, relm);
	if (retype == Et_CompHeader) {
	    // Set view to component
	    aWnd->ResetObserver(this);
	    ResetView();
	    MAE_View* view = iViews[aWnd->View()->Name()];
	    iViews[view->Name()] = NULL;
	    string cname = relm[0];
	    CAE_EBase* comp = FindByName(cname.c_str());
	    CAE_Object* obj = comp->GetFbObj(obj);
	    aWnd->Clear();
	    obj->AddView(view);
	    obj->OnExpose(view->Wnd(), CAV_Rect());
	}
    }
    return ETrue;
}
*/

void CAE_Object::Render(MAE_Window* aWnd, CAV_Rect aRect, TBool aDraw, CAV_Point aPt, TReType& aReType, vector<string>& aRelm)
{
    MAE_Window* wnd = aWnd;
    MAE_Gc* gc = wnd->Gc();
    CAV_Rect rect = wnd->Rect();
    // Draw the rect
    gc->DrawRect(rect, EFalse);
    // Draw head
    Bva* bva = GetBva(Et_Header, "Header");
    if (bva == NULL) {
	//AddBva(bva = new BvaHead(this, *this, wnd));
    }
    CAV_Rect cdres = rect;
    bva->Render(cdres);
    CAV_Rect headrc = CAV_Rect(rect.iTl, rect.Width(), KViewNameLineHeight);
    bva->iWnd->SetPrefSize(headrc);
    bva->iWnd->SetRect(headrc);
    if (aDraw)
	bva->Draw();
    /*
    CAV_Rect headrect = CAV_Rect(CAV_Point(0, 0), CAV_Point(rect.iBr.iX, KViewNameLineHeight));
    if (aDraw) {
	gc->DrawRect(headrect, EFalse);
	// Draw the name
	MAE_TextLayout* nametl = gc->CreateTextLayout();
	nametl->SetText(InstName());
	nametl->Draw(headrect.iTl);
    }
    else if (headrect.Intersects(aPt)) {
	aReType = Et_Header;
    }
    */

    CAV_Rect baserc = CAV_Rect(CAV_Point(rect.iTl + CAV_Point(0, KViewNameLineHeight)), rect.iBr);
    // Draw Inputs/Outputs areas
    /*
    CAV_Rect inprect = CAV_Rect(baserc.iTl + CAV_Point(baserc.iBr.iX - KViewExtAreaWidth, 0), baserc.iBr);
    gc->DrawRect(inprect, EFalse);
    //CAV_Rect outprect = CAV_Rect(CAV_Point(rect.iTl.iX, rect.iTl.iY + KViewNameLineHeight), CAV_Point(rect.iTl.iX + KViewExtAreaWidth, rect.iBr.iY));
    CAV_Rect outprect = CAV_Rect(baserc.iTl, baserc.iBr - CAV_Point(baserc.iBr.iX - KViewExtAreaWidth, 0));
    gc->DrawRect(outprect, EFalse);
    */

    // Draw outputs
    cdres = baserc.iTl + CAV_Point(0, KViewCompGapHight);
    for (map<string, CAE_ConnPointBase*>::iterator it = iOutputs.begin(); it != iOutputs.end(); it++) {
	RenderEpoint(*(it->second), EFalse, *gc, cdres, aDraw, aPt, aReType, aRelm);
	cdres.Move(CAV_Point(0, cdres.Height()));
    }

    // Draw inputs
    cdres = baserc.iTl + CAV_Point(baserc.Width() - KViewExtAreaWidth, KViewCompGapHight);
    for (map<string, CAE_ConnPointBase*>::iterator it = iInputs.begin(); it != iInputs.end(); it++) {
	RenderEpoint(*(it->second), ETrue, *gc, cdres, aDraw, aPt, aReType, aRelm);
	cdres.Move(CAV_Point(0, cdres.Height()));
    }

    // Draw transitions
    cdres = baserc.iTl + CAV_Point(baserc.iBr.iX/2, KViewCompGapHight);
    RenderTrans(iTransSrc, *gc, cdres, aDraw, aPt, aReType, aRelm);

    // Draw states
    cdres = baserc.iTl + CAV_Point(baserc.iBr.iX/2, cdres.iBr.iY + KViewCompGapHight);
    for (vector<CAE_EBase*>::iterator it = iCompReg.begin(); it != iCompReg.end(); it++) {
	CAE_StateBase* comp = (*it)->GetFbObj(comp);
	if (comp != NULL) {
	    RenderState(*comp, *gc, cdres, aDraw, aPt, aReType, aRelm);
	    cdres.Move(CAV_Point(0, cdres.Height() + KViewCompGapHight));
	}
    }

    // Draw subsystems
    //cdinipt = baserc.iTl + CAV_Point((baserc.iBr.iX - KViewCompAreaWidth)/2, cdres.iBr.iY + KViewCompGapHight);
    cdres = baserc.iTl + CAV_Point((baserc.iBr.iX - KViewCompAreaWidth)/2, cdres.iBr.iY + KViewCompGapHight);
    for (vector<CAE_EBase*>::iterator it = iCompReg.begin(); it != iCompReg.end(); it++) {
	CAE_Object* obj = (*it)->GetFbObj(obj);
	if (obj != NULL) {
	    RenderComp(*obj, *gc, cdres, aDraw, aPt, aReType, aRelm);
	    cdres.Move(CAV_Point(0, cdres.Height() + KViewCompGapHight));
	}
    }
}

void CAE_Object::RenderTrans(const string& aTrans, MAE_Gc& aGc, CAV_Rect& aRect,
	TBool aDraw, CAV_Point aPt, TReType& aReType, vector<string>& aRelm)
{
    MAE_TextLayout* txt = aGc.CreateTextLayout();
    txt->SetText(aTrans);
    CAV_Point tsize;
    txt->GetSizePu(tsize);
    CAV_Point tl = aRect.iTl - CAV_Point(tsize.iX/2, 0);
    CAV_Rect rect(tl, tl + tsize);
    if (aDraw) {
	txt->Draw(rect.iTl);
	aGc.DrawRect(rect, EFalse);
    }
    aRect = rect;
}

void CAE_Object::RenderComp(const CAE_Object& aComp, MAE_Gc& aGc, CAV_Rect& aRect, TBool aDraw, CAV_Point aPt, 
	TReType& aReType, vector<string>& aRelm)
{
    // Draw head
    CAV_Rect headrect(aRect.iTl, aRect.iTl + CAV_Point(KViewCompAreaWidth, KViewNameLineHeight));
    if (aDraw) {
	aGc.DrawRect(headrect, EFalse);
    }
    else if (headrect.Intersects(aPt)) {
	aReType = Et_CompHeader;
	aRelm.push_back(aComp.InstName());
    }
    // Draw the name
    aGc.DrawText(aComp.InstName(), headrect);
    // Draw inputs
    CAV_Rect iresrc = headrect.iBr - CAV_Point(KViewExtAreaWidth, 0);
    for (map<string, CAE_ConnPointBase*>::const_iterator it = aComp.Inputs().begin(); it != aComp.Inputs().end(); it++) {
	RenderCpoint(*(it->second), ETrue, EFalse, aGc, iresrc, aDraw, aPt, aReType, aRelm);
	iresrc.Move(CAV_Point(0, iresrc.Height()));
    }
    // Draw outputs
    iresrc = headrect.iBr - CAV_Point(headrect.Width(), 0);
    CAV_Rect oresrc = headrect;
    for (map<string, CAE_ConnPointBase*>::const_iterator it = aComp.Outputs().begin(); it != aComp.Outputs().end(); it++) {
	RenderCpoint(*(it->second), EFalse, EFalse, aGc, iresrc, aDraw, aPt, aReType, aRelm);
	iresrc.Move(CAV_Point(0, oresrc.Height()));
    }
    TInt maxy = max(iresrc.iBr.iY, oresrc.iBr.iY);
    CAV_Rect baserc(headrect.iBr - CAV_Point(headrect.Width(), 0), CAV_Point(headrect.iBr.iX, maxy));
    aGc.DrawRect(baserc, EFalse);
    CAV_Rect fullrc(aRect.iTl, baserc.iBr);
    aRect = fullrc;
}

void CAE_Object::RenderState(const CAE_StateBase& aComp, MAE_Gc& aGc, CAV_Rect& aRect, TBool aDraw, CAV_Point aPt, 
	TReType& aReType, vector<string>& aRelm)
{
    // Render trans
    CAV_Rect trnrec = aRect;
    RenderTrans(aComp.iTrans.iETrans, aGc, trnrec, EFalse, aPt, aReType, aRelm);

    // Draw head
    CAV_Point headipt = aRect.iTl - CAV_Point((trnrec.Width() + KViewExtAreaWidth + KViewStateGapTransInp)/2, 0);
    CAV_Rect headrect(headipt, headipt + CAV_Point(trnrec.Width() + KViewExtAreaWidth + KViewStateGapTransInp, KViewNameLineHeight));
    if (aDraw) {
	aGc.DrawRect(headrect, EFalse);
    }
    else if (headrect.Intersects(aPt)) {
	aReType = Et_StateHeader;
	aRelm.push_back(aComp.InstName());
    }
    trnrec = headrect.iTl + CAV_Point(trnrec.Width()/2, headrect.Height());
    RenderTrans(aComp.iTrans.iETrans, aGc, trnrec, aDraw, aPt, aReType, aRelm);
    // Draw the name
    if (aDraw) {
	aGc.DrawRect(headrect, EFalse);
	aGc.DrawText(aComp.InstName(), headrect);
    }
    // Draw inputs
    CAV_Rect iresrc = headrect.iBr - CAV_Point(KViewExtAreaWidth, 0);
    for (map<string, CAE_ConnPointBase*>::const_iterator it = aComp.Inputs().begin(); it != aComp.Inputs().end(); it++) {
	RenderCpoint(*(it->second), ETrue, ETrue, aGc, iresrc, aDraw, aPt, aReType, aRelm);
	iresrc.Move(CAV_Point(0, iresrc.Height()));
    }
   
    CAV_Rect baserc(headrect.iBr - CAV_Point(headrect.Width(), 0), CAV_Point(headrect.iBr.iX, iresrc.iBr.iY));
    aGc.DrawRect(baserc, EFalse);
    CAV_Rect fullrc(aRect.iTl, baserc.iBr);

    // Draw output
    iresrc = baserc.iTl + CAV_Point(0, baserc.Height()/2);
    CAV_Rect oresrc = headrect;
    RenderCpoint(*(aComp.Output()), EFalse, ETrue, aGc, iresrc, aDraw, aPt, aReType, aRelm);
    iresrc.Move(CAV_Point(0, oresrc.Height()));

    aRect = fullrc;
}

void CAE_Object::RenderCpoint(const CAE_ConnPointBase& aCpoint, TBool aInp, TBool aState, MAE_Gc& aGc, CAV_Rect& aRect,
	TBool aDraw, CAV_Point aPt, TReType& aReType, vector<string>& aRelm)
{
    CAV_Rect headrect;
    if (aState && !aInp) {
	headrect = CAV_Rect(aRect.iTl - CAV_Point(0, KViewExtLineHeight/2), aRect.iTl + CAV_Point(KViewExtAreaWidth, KViewExtLineHeight));
    }
    else {
	headrect = CAV_Rect(aRect.iTl, aRect.iTl + CAV_Point(KViewExtAreaWidth, KViewExtLineHeight));
	aGc.DrawRect(headrect, EFalse);
	// Draw the name
	aGc.DrawText(aCpoint.Name(), headrect);
	aRect = headrect;
    }
    // Draw the connection line
    CAV_Point pt1, pt2;
    if (aInp) {
	pt1 = headrect.iBr - CAV_Point(0, headrect.Height() / 2);
	pt2 = pt1 + CAV_Point(KViewConnLineLen, 0);
    }
    else {
	pt1 = headrect.iTl + CAV_Point(0, headrect.Height() / 2);
	pt2 = pt1 - CAV_Point(KViewConnLineLen, 0);
    }
    aGc.DrawLine(pt1, pt2);
    // Draw conn Ids
    CAV_Rect cidrc;
    CAV_Point ciinitpt = pt2;
    TInt count = KViewConnIdMaxNum + 1;
    for (vector<CAE_ConnPointBase*>::const_iterator it = aCpoint.Conns().begin(); it != aCpoint.Conns().end() && count > 0; it++, count--) {
	CAE_ConnPointBase* cp = *it;
	const CAE_EBase& mgr = cp->Man();
	string pair_name = (&mgr == NULL) ? "?" : mgr.InstName();
	string ftxt = (count == 1) ? "..." : pair_name + "." + cp->Name();
	MAE_TextLayout* nametl = aGc.CreateTextLayout();
	nametl->SetText(ftxt);
	CAV_Point tsize;
	nametl->GetSizePu(tsize);
	if (aInp) {
	    cidrc = CAV_Rect(ciinitpt - CAV_Point(0, KViewConnIdHeight/2), ciinitpt + CAV_Point(tsize.iX, KViewConnIdHeight/2));
	    ciinitpt += CAV_Point(cidrc.Width(), 0);
	}
	else {
	    cidrc = CAV_Rect(ciinitpt - CAV_Point(tsize.iX, KViewConnIdHeight/2), ciinitpt + CAV_Point(0, KViewConnIdHeight/2));
	    ciinitpt -= CAV_Point(cidrc.Width(), 0);
	}
	aGc.DrawRect(cidrc, EFalse);
	nametl->Draw(cidrc.iTl);
    }
}

void CAE_Object::RenderEpoint(CAE_ConnPointBase& aCpoint, TBool aInp, MAE_Gc& aGc, CAV_Rect& aRect,
	TBool aDraw, CAV_Point aPt, TReType& aReType, vector<string>& aRelm)
{
    CAV_Rect headrect;
    headrect = CAV_Rect(aRect.iTl, aRect.iTl + CAV_Point(KViewExtAreaWidth, KViewExtLineHeight));
    aGc.DrawRect(headrect, EFalse);
    // Draw the name
    aGc.DrawText(aCpoint.Name(), headrect);
    aRect = headrect;
    // Draw the connection line
    CAV_Point pt1, pt2;
    if (!aInp) {
	pt1 = headrect.iBr - CAV_Point(0, headrect.Height() / 2);
	pt2 = pt1 + CAV_Point(KViewConnLineLen, 0);
    }
    else {
	pt1 = headrect.iTl + CAV_Point(0, headrect.Height() / 2);
	pt2 = pt1 - CAV_Point(KViewConnLineLen, 0);
    }
    aGc.DrawLine(pt1, pt2);
    // Draw conn Ids
    CAV_Rect cidrc;
    CAV_Point ciinitpt = pt2;
    CAE_ConnPointExt* ext = aCpoint.GetFbObj(ext);
    if (ext != NULL) {
	// Simple ext point
	CAE_ConnPointBase* cp = ext->Ref();
	const CAE_EBase& mgr = cp->Man();
	string pair_name = (&mgr == NULL) ? "?" : mgr.InstName();
	string ftxt = pair_name + "." + cp->Name();
	MAE_TextLayout* nametl = aGc.CreateTextLayout();
	nametl->SetText(ftxt);
	CAV_Point tsize;
	nametl->GetSizePu(tsize);
	if (!aInp) {
	    cidrc = CAV_Rect(ciinitpt - CAV_Point(0, KViewConnIdHeight/2), ciinitpt + CAV_Point(tsize.iX, KViewConnIdHeight/2));
	}
	else {
	    cidrc = CAV_Rect(ciinitpt - CAV_Point(tsize.iX, KViewConnIdHeight/2), ciinitpt + CAV_Point(0, KViewConnIdHeight/2));
	}
	nametl->Draw(cidrc.iTl);
	aGc.DrawRect(cidrc, EFalse);
    }
    else {
	CAE_ConnPointExtC* extc = aCpoint.GetFbObj(extc);
	for (map<string, CAE_ConnPointExtC::SlotTempl::slot_templ_elem>::iterator pit = extc->Templ().Srcs().begin(); 
		pit != extc->Templ().Srcs().end(); pit++) {
	    string pinname = (*pit).second;
	    for (vector<CAE_ConnPointExtC::Slot>::iterator sit = extc->Slots().begin(); sit != extc->Slots().end(); sit++) {
		CAE_ConnPointBase* pint = (*sit).Srcs()[pinname].first;
		// Multipin ext point
		TInt count = KViewConnIdMaxNum + 1;
		for (vector<CAE_ConnPointBase*>::const_iterator it = aCpoint.Conns().begin(); it != aCpoint.Conns().end() && count > 0; it++, count--) {
		    CAE_ConnPointBase* cp = *it;
		    const CAE_EBase& mgr = cp->Man();
		    string pair_name = (&mgr == NULL) ? "?" : mgr.InstName();
		    string ftxt = (count == 1) ? "..." : pair_name + "." + cp->Name();
		    MAE_TextLayout* nametl = aGc.CreateTextLayout();
		    nametl->SetText(ftxt);
		    CAV_Point tsize;
		    nametl->GetSizePu(tsize);
		    if (!aInp) {
			cidrc = CAV_Rect(ciinitpt - CAV_Point(0, KViewConnIdHeight/2), ciinitpt + CAV_Point(tsize.iX, KViewConnIdHeight/2));
			ciinitpt += CAV_Point(cidrc.Width(), 0);
		    }
		    else {
			cidrc = CAV_Rect(ciinitpt - CAV_Point(tsize.iX, KViewConnIdHeight/2), ciinitpt + CAV_Point(0, KViewConnIdHeight/2));
			ciinitpt -= CAV_Point(cidrc.Width(), 0);
		    }
		    aGc.DrawRect(cidrc, EFalse);
		    nametl->Draw(cidrc.iTl);
		}
	    }
	}
    }
}

void CAE_Object::OnHeaderPress(const MAE_View* aView)
{
    // Return view to super-system
    MAE_View* view = iViews[aView->Name()];
    iViews[view->Name()] = NULL;
    //view->Wnd()->ResetObserver(this);
    view->Wnd()->Clear();
    CAE_Object* obj = iMan;
    obj->AddView(view);
    ResetView();
    //obj->OnExpose(view->Wnd(), CAV_Rect());
}

//*********************************************************
// Base view agents
//*********************************************************

CAE_Object::Bva::Bva(Bva* aParent, CAE_Object& aSys, MAE_Window* aWnd, TReType aType, const string& aName, TBool aCreateWnd): 
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

CAE_Object::Bva::~Bva()
{
    iWnd->Destroy();
    iWnd = NULL;
    for (map<TRelm, Bva*>::iterator it = iBvas.begin(); it != iBvas.end(); it++) {
	delete it->second;
    }
}

void CAE_Object::Bva::AddBva(Bva* aBva)
{
    iBvas[TRelm(aBva->iType, aBva->iName)] = aBva;
}

CAE_Object::Bva* CAE_Object::Bva::GetBva(TReType aType, const string& aName)
{
    map<TRelm, Bva*>::iterator it = iBvas.find(TRelm(aType, aName));
    return (it != iBvas.end()) ? it->second : NULL; 
}

void CAE_Object::Bva::OnCrossing(MAE_Window* aWnd, TBool aEnter)
{

}



// Base view agents for system

CAE_Object::BvaSyst::BvaSyst(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName): 
    Bva(aParent, aSys, aOwnedWnd, Et_System, aName, EFalse) 
{
    iWnd->SetPrefSize(CAV_Rect(-1, -1));
    // Add header bva
    AddBva(new BvaHead(this, iSys, iWnd));
    // Add components bvas
    for (vector<CAE_EBase*>::iterator it = iSys.iCompReg.begin(); it != iSys.iCompReg.end(); it++) {
	CAE_Object* obj = (*it)->GetFbObj(obj);
	if (obj != NULL) {
	    AddBva(new BvaComp(this, iSys, iWnd, obj->InstName()));
	}
    }
}

void CAE_Object::BvaSyst::Render(CAV_Rect& aRect)
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
    for (vector<CAE_EBase*>::iterator it = iSys.iCompReg.begin(); it != iSys.iCompReg.end(); it++) {
	CAE_Object* obj = (*it)->GetFbObj(obj);
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

void CAE_Object::BvaSyst::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the rect
    //gc->DrawRect(rect, EFalse);
}

void CAE_Object::BvaSyst::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaSyst::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void CAE_Object::BvaSyst::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaSyst::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
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


void CAE_Object::BvaSyst::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
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

CAE_Object::BvaHead::BvaHead(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd): Bva(aParent, aSys, aOwnedWnd, Et_Header, "Header")
{
    iWnd->SetPrefSize(CAV_Rect(0, KViewNameLineHeight));
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iSys.InstName());
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    iWnd->SetPrefSize(CAV_Rect(tsize.iX, KViewNameLineHeight));
    CAE_Color bg_norm(1, 0, 40000, 0);
    iWnd->SetBg(MAE_Window::ESt_Normal, bg_norm);
    CAE_Color bg_pre(2, 50000, 0, 0);
    iWnd->SetBg(MAE_Window::ESt_Prelight, bg_pre);
    iWnd->Show();
}

void CAE_Object::BvaHead::Render(CAV_Rect& aRect)
{
}

void CAE_Object::BvaHead::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    // Draw the name
    CAV_Rect tr = rect;
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iSys.InstName());
    CAV_Rect drc(rect.iTl + CAV_Point(1, 1), rect.iBr - CAV_Point(1, 1));
    nametl->Draw(drc.iTl);
    gc->DrawRect(drc, EFalse);
}

void CAE_Object::BvaHead::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaHead::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	iSys.OnHeaderPress(aWnd->View());
    }
}

void CAE_Object::BvaHead::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaHead::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

// Base view agents for component

CAE_Object::BvaComp::BvaComp(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName):
    Bva(aParent, aSys, aOwnedWnd, Et_Comp, aName) 
{
    CAE_EBase* elem = iSys.FindByName(aName.c_str());
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
}

void CAE_Object::BvaComp::Render(CAV_Rect& aRect)
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

void CAE_Object::BvaComp::Draw()
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

void CAE_Object::BvaComp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaComp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void CAE_Object::BvaComp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaComp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
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

void CAE_Object::BvaComp::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
{
    iParent->OnChildStateChanged(aChild, aPrevState);
}


// Base view agents for components header

CAE_Object::BvaCompHead::BvaCompHead(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName):
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

void CAE_Object::BvaCompHead::Render(CAV_Rect& aRect)
{
    MAE_Gc* gc = iWnd->Gc();
    MAE_TextLayout* nametl = gc->CreateTextLayout();
    nametl->SetText(iName);
    CAV_Point tsize;
    nametl->GetSizePu(tsize);
    CAV_Rect rect = CAV_Rect(aRect.iTl, tsize.iX, KViewNameLineHeight);
}

void CAE_Object::BvaCompHead::Draw()
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

void CAE_Object::BvaCompHead::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaCompHead::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	iSys.OnHeaderPress(aWnd->View());
    }
}

void CAE_Object::BvaCompHead::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}


void CAE_Object::BvaCompHead::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = iWnd->GetPrefSize();
}

// Base view agents for components inputs

CAE_Object::BvaCompInp::BvaCompInp(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName):
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

void CAE_Object::BvaCompInp::Render(CAV_Rect& aRect)
{
}

void CAE_Object::BvaCompInp::Draw()
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

void CAE_Object::BvaCompInp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaCompInp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void CAE_Object::BvaCompInp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaCompInp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = CAV_Rect(0, KViewConnLineLen);
}


// Base view agents for components outputs

CAE_Object::BvaCompOutp::BvaCompOutp(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName):
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

void CAE_Object::BvaCompOutp::Render(CAV_Rect& aRect)
{
}

void CAE_Object::BvaCompOutp::Draw()
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

void CAE_Object::BvaCompOutp::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaCompOutp::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void CAE_Object::BvaCompOutp::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaCompOutp::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
    aRect = CAV_Rect(0, KViewConnLineLen);
}

// Base view agents for connections

CAE_Object::BvaConns::BvaConns(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, TReType aType, const string& aName,
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

void CAE_Object::BvaConns::Render(CAV_Rect& aRect)
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

void CAE_Object::BvaConns::Draw()
{
    MAE_Gc* gc = iWnd->Gc();
    CAV_Rect rect = iWnd->Rect();
    CAV_Point start((iType == Et_LeftConns) ? (rect.Width() - KViewConnLineLen) : 0, rect.Height()/2);
    CAV_Point end = start + CAV_Point(KViewConnLineLen, 0);
    gc->DrawLine(start, end);
}

void CAE_Object::BvaConns::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaConns::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
    }
}

void CAE_Object::BvaConns::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaConns::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

void CAE_Object::BvaConns::OnChildStateChanged(const Bva* aChild, MAE_Window::TState aPrevState)
{
    iParent->OnChildStateChanged(aChild, aPrevState);
}


// Base view agents for connection id

CAE_Object::BvaConn::BvaConn(Bva* aParent, CAE_Object& aSys, MAE_Window* aOwnedWnd, const string& aName):
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

CAE_Object::BvaConn::~BvaConn()
{
}

void CAE_Object::BvaConn::Render(CAV_Rect& aRect)
{
}

void CAE_Object::BvaConn::Draw()
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

void CAE_Object::BvaConn::OnExpose(MAE_Window* aWnd, CAV_Rect aRect)
{
    Draw();
}

TBool CAE_Object::BvaConn::OnButton(MAE_Window* aWnd, TBtnEv aEvent, TInt aBtn, CAV_Point aPt)
{	    
    if (aEvent == EBte_Press) {
	MAE_Window::TState state = aWnd->GetState();
	if (state == MAE_Window::ESt_Selected)
	    aWnd->SetState(MAE_Window::ESt_Prelight);
	else
	    aWnd->SetState(MAE_Window::ESt_Selected);
    }
}

void CAE_Object::BvaConn::OnResized(MAE_Window* aWnd, CAV_Rect aRect)
{
    CAV_Rect rec = aRect;
    Render(rec);
}

void CAE_Object::BvaConn::OnPrefSizeRequested(MAE_Window* aWnd, CAV_Rect& aRect)
{
}

void CAE_Object::BvaConn::OnCrossing(MAE_Window* aWnd, TBool aEnter)
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

void CAE_Object::BvaConn::OnStateChanged(MAE_Window* aWnd, MAE_Window::TState aPrevState)
{
    if (iWnd->GetState() == MAE_Window::ESt_Selected) {
	iParent->OnChildStateChanged(this, aPrevState);
    }
}

//*********************************************************
// CAE_ObjectBa - bit automata
//*********************************************************


FAPWS_API CAE_ObjectBa::CAE_ObjectBa(const char* aInstName, CAE_Object* aMan, TInt aLen, TInt aInpInfo, TInt aOutInfo):
    CAE_Object(aInstName, aMan), iLen(aLen), iTrans(NULL), iNew(NULL), iCurr(NULL), 
    iInpInfo(aInpInfo), iOutInfo(aOutInfo)
{
}

FAPWS_API CAE_ObjectBa::~CAE_ObjectBa()
{
    if (iTrans != NULL)
    {
	delete[] iTrans;
	iTrans = NULL;
    }
    if (iNew != NULL)
	{
		delete[] iNew;
		iNew = NULL;
	}
	if (iCurr != NULL)
	{
		delete[] iCurr;
		iCurr = NULL;
	}
}

FAPWS_API CAE_ObjectBa* CAE_ObjectBa::NewL(const char* aInstName, CAE_Object* aMan, TInt aLen, TInt aInpInfo, TInt aOutInfo)
{
	CAE_ObjectBa* self = new CAE_ObjectBa(aInstName, aMan, aLen, aInpInfo, aOutInfo);
	self->ConstructL();
	return self;
}

FAPWS_API void CAE_ObjectBa::ConstructL()
{
	_FAP_ASSERT(iInpInfo < iLen); 
	CAE_Object::ConstructL();
	TInt stwnum = iLen?(iLen-1)/32 +1:1;
	if (iNew == NULL)
	{
		iNew = new TUint32[stwnum];
	}
	memset(iNew, 0, stwnum*4);
	if (iCurr == NULL)
	{
		iCurr = new TUint32[stwnum];
	}
	memset(iCurr, 0, stwnum*4);
	if (iTrans == NULL)
	{
		iTrans = new TUint32[iLen*stwnum];
	}
	iInp = (CAE_TState<TInt>*) CAE_State::NewL("Inp", sizeof(TInt), this, TTransInfo());
	iStInp = (CAE_TState<TInt>*) CAE_State::NewL("StInp", sizeof(TInt), this, CAE_TRANS(UpdateInput));
	iStInp->AddInputL("_Inp");
	iStInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("StInpReset", sizeof(TUint8), this, CAE_TRANS(UpdateInpReset));
	iStInpReset->AddInputL("_InpReset");
	iInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("InpReset", sizeof(TUint8), this, TTransInfo());
	iStFitness = (CAE_TState<TInt>*) CAE_State::NewL("Fitness", sizeof(TInt), this, TTransInfo());
	iOut = (CAE_TState<TInt>*) CAE_State::NewL("Out", sizeof(TInt), this, TTransInfo());
	iStInp->SetInputL("_Inp", iInp);
	iStInpReset->SetInputL("_InpReset", iInpReset);
}

void CAE_ObjectBa::UpdateInput(CAE_StateBase* aState)
{
	TInt data = iInp->Value();
	*iStInp = data;
}

void CAE_ObjectBa::UpdateInpReset(CAE_StateBase* aState)
{
	TUint8 reset = iInpReset->Value();
	if (reset == KCommand_On)
	{
		TInt stwnum = iLen?(iLen-1)/32 +1:1;
		for (int i = 0; i < stwnum; i++)
		{
			iNew[i] = 0;
		}
	}
	*iStInpReset = 0;
}

FAPWS_API void CAE_ObjectBa::SetTrans(const TUint32* aTrans)
{
	if (aTrans == NULL)
	{
		SetTransRandomly();
	}
	else
	{
		TInt stwnum = iLen?(iLen-1)/32 +1:1;
		for (TInt i = 0; i < iLen; i++)
		{
			for (TInt j = 0; j < stwnum; j++)
				iTrans[stwnum*i+j] = aTrans[stwnum*i+j];
		}
	}
}

void CAE_ObjectBa:: SetTransRandomly()
{
	TInt stwnum = iLen?(iLen-1)/32 +1:1;
	TUint32 mask = (1 << iLen) -1;
	for (TInt i = 0; i < iLen; i++)
	{
		for (TInt j = 0; j < stwnum; j++)
			iTrans[stwnum*i+j] = (rand() << 17 | rand() << 2 | (rand()&0x3)) & mask ;
	}
}


FAPWS_API void CAE_ObjectBa::Update()
{
	if (iTrans != NULL)
	{
		TInt stwnum = iLen?(iLen-1)/32 +1:1;
		TBool snew = EFalse;
		TUint32 mask = 0x00000001;
		for (TInt i = 0; i < iLen; i++)
		{
			TInt wdind = i/32;
			snew = EFalse;
			TUint32* strans = &(iTrans[stwnum*i]);
			for (TInt j = 0; j < stwnum; j++)
			{
				snew |= strans[j]&~iCurr[j];
			}
			iNew[wdind] &= ~ mask;
			if (snew) iNew[wdind] |= mask;
			mask = (mask==0x80000000)?0x00000001:mask << 1;
		}
		// Added input
		TInt data = iStInp->Value();
		stwnum = iInpInfo/32;
		TUint32 msk = data?1 << (iInpInfo % 32):0;
		iNew[stwnum] |= msk;
		// Update output
		TInt wnum = iOutInfo/32;
		msk = 1 << (iOutInfo % 32);
		TUint32 outp = iNew[wnum] & msk;
		*iOut = outp;
	}
	CAE_Object::Update();
}

FAPWS_API void CAE_ObjectBa::Confirm()
{
	TInt stwnum = iLen?(iLen-1)/32 +1:1;
	if (memcmp(iCurr, iNew, stwnum*4))
	{
		SetActive();
	}
	memcpy(iCurr, iNew, stwnum*4); // iCurr << iNew
	CAE_Object::Confirm();
}

void CAE_ObjectBa::DoMutationOfTrans()
{
	// Mutation affects the only one gene in the chromosome
	// Randomly select the gene index in the chromosome
	TInt srnd1 = rand();
	TInt srnd2 = rand();
	TInt wordidx = (iLen*srnd1)/RAND_MAX; // Index of word in the gene words array 
	TInt bitidx = (iLen*srnd2)/RAND_MAX; // Index of bit in the gene word 
	TUint32 mask = 1 << bitidx;
	iTrans[wordidx] ^= mask;
	
}


FAPWS_API CAE_ObjectBa* CAE_ObjectBa::CreateNew(const char* aInstName)
{
	CAE_ObjectBa* sinheritor = CAE_ObjectBa::NewL(aInstName, iMan, iLen, iInpInfo, iOutInfo);
	sinheritor->SetTrans(iTrans);
	sinheritor->DoMutationOfTrans();
	return sinheritor;
}


// CAE_Link

const char* KLinkInputName = "LinkInput";
const char* KLinkOutputName = "LinkOutput";

CAE_Link::CAE_Link(const char* aInstName, CAE_Object* aMan):CAE_Object(aInstName, aMan)
{
}

FAPWS_API CAE_Link* CAE_Link::NewL(const char* aInstName, CAE_Object* aMan, CAE_State* aInp, CAE_State* aOut)
{
	CAE_Link* self = new CAE_Link(aInstName, aMan);
	self->ConstructL(aInp, aOut);
	return self;
};

void CAE_Link::ConstructL(CAE_State* aInp, CAE_State* aOut)
{
	CAE_Object::ConstructL();
//	iInp = CAE_State::NewL(KLinkInputName, aInp, this, NULL, CAE_State::TType_Input);
//	iOut = CAE_State::NewL(KLinkOutputName, aOut, this, UpdateOutS, CAE_State::TType_Output);
	iOut->AddInputL("_Inp");
	iOut->SetInputL("_Inp", iInp);
};

CAE_Link::~CAE_Link()
{
	delete iInp;
	delete iOut;
}

void CAE_Link::UpdateOut() { iOut->Set(iInp->iCurr); };


// CAE_StateASrv

CAE_StateASrv::CAE_StateASrv(const char* aInstName,CAE_Object* aMan):
	CAE_Object(aInstName, aMan),
		iReqMon(NULL)	
{
}

FAPWS_API CAE_StateASrv::~CAE_StateASrv()
{
	iReqMon->Cancel();
	delete iReqMon;
	delete iStCancel;
	delete iStSetActive;
	delete iOutStatusPtr;
	delete iOutStatus;
	delete iInpSetActive;
	delete iInpCancel;
}

FAPWS_API CAE_StateASrv* CAE_StateASrv::NewL(const char* aInstName, CAE_Object* aMan, TInt aPriority)
{
	CAE_StateASrv* self = new CAE_StateASrv(aInstName, aMan);
	self->ConstructL(aPriority);
	return self;
}

void CAE_StateASrv::ConstructL(TInt aPriority)
{
	CAE_Object::ConstructL();
	iReqMon = new CAE_ReqMon(aPriority, this);
	iOutStatusPtr = (CAE_TState<TInt>*) CAE_State::NewL("OStatusPtr", sizeof(void*), this, TTransInfo());
	iOutStatus = CAE_State::NewL("OStatus", sizeof(TInt), this, TTransInfo());
	iInpSetActive = (CAE_TState<TUint8>*) CAE_State::NewL("ISetAct", sizeof(TUint8), this, TTransInfo());
	iInpCancel = (CAE_TState<TUint8>*) CAE_State::NewL("ICancel", sizeof(TUint8), this, TTransInfo());
	iStSetActive = (CAE_TState<TUint8>*) CAE_State::NewL("SSetAct", sizeof(TUint8), this, CAE_TRANS(UpdateSetActive));
	iStCancel = (CAE_TState<TUint8>*) CAE_State::NewL("SCancel", sizeof(TUint8), this, CAE_TRANS(UpdateCancel));
	iStSetActive->AddInputL("_InpSetActive", iInpSetActive); 
	iStCancel->AddInputL("_InpCancel", iInpCancel);
	iOutStatusPtr->Set((void*) iReqMon);
}

void CAE_StateASrv::RunL()
{
	iOutStatus->Set(&(iReqMon->iStatus));
}

void CAE_StateASrv::UpdateSetActive(CAE_StateBase* aState)
{
	if (iInpSetActive->Value() > 0)
	{
		iReqMon->iStatus = KRequestPending;
		iReqMon->SetActive();
	}
}

void CAE_StateASrv::UpdateCancel(CAE_StateBase* aState)
{
	if (iInpCancel->Value() > 0)
		iReqMon->Cancel();
}

// CAE_ObjectAs

CAE_ObjectAs::CAE_ObjectAs(const char* aInstName,CAE_Object* aMan):
	CAE_Object(aInstName, aMan),
		iReqMon(NULL)	
{
}

FAPWS_API CAE_ObjectAs::~CAE_ObjectAs()
{
	iReqMon->Cancel();
	delete iReqMon;
	delete iStAsStatus;
}

FAPWS_API CAE_ObjectAs* CAE_ObjectAs::NewL(const char* aInstName, CAE_Object* aMan, TInt aPriority)
{
	CAE_ObjectAs* self = new CAE_ObjectAs(aInstName, aMan);
	self->ConstructL(aPriority);
	return self;
}

void CAE_ObjectAs::ConstructL(TInt aPriority)
{
	CAE_Object::ConstructL();
	iReqMon = new CAE_ReqMonAs(aPriority, this);
	iStAsStatus = (CAE_TState<TInt>*) CAE_State::NewL("SAsStatus", sizeof(TInt), this, TTransInfo());
	// Need ti init AsStatus with unic value. Otherwise thread AO can quickly set it to KErrNone
	// so status will become unchanged
	*(TInt*) (iStAsStatus->iCurr) = KRequestPending + 1;
}

void CAE_ObjectAs::RunL()
{
	iStAsStatus->Set(&(iReqMon->iStatus));
}


