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
// TODO [YB] Completely wrong approach for mutation. Mutation should occur while creating new instance. It is not possible
// just to change the existing one. So mutation shoud be: first - changing of chromo, second - produce new instance.

#include <stdlib.h>
#include <stdio.h>
#include "fapplat.h"
#include "panics.h"
#include "fapbase.h"
#include "fapview.h"
#include "fapopvi.h"

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
		for (vector<CAE_ConnPointBase*>::iterator it = iConns.begin(); it != iConns.end(); it++) {
		    if ((*it) == aConnPoint) {
			iConns.erase(it); break;
		    }
		}
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
    _FAP_ASSERT(0);
    return EFalse;
}

TBool CAE_ConnPoint::Disextend(CAE_ConnPointBase *aConnPoint)
{
    _FAP_ASSERT(0);
    return EFalse;
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
	iExts.push_back(iRef);
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

TBool CAE_ConnPointExt::Disextend(CAE_ConnPointBase *aConnPoint)
{
    TBool res = EFalse;
    if (iRef == aConnPoint) { 
	aConnPoint->Disconnect();
	iRef = NULL;
       	res = ETrue;
    }
    return res;
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

void CAE_ChromoNode::ParseTname(const string& aTname, NodeType& aType, string& aName)
{
    size_t tpos = aTname.find("%");
    if (tpos != string::npos) {
	string tid = aTname.substr(0, tpos);
	aType = iMdl.GetType(tid);
	aName = aTname.substr(tpos+1);
    }
    else {
	aType = ENt_Object;
	aName = aTname;
    }
}

string CAE_ChromoNode::GetName(const string& aTname)
{
    size_t tpos = aTname.find("%");
    return (tpos != string::npos) ? aTname.substr(tpos+1) : aTname;
}

string CAE_ChromoNode::GetTName(NodeType aType, const string& aName)
{
    if (aType == ENt_Object) {
	return aName;
    }
    else {
	return iMdl.GetTypeId(aType) + "%" + aName;
    }
}

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(const string& aName) 
{ 
    Iterator res = End();
    NodeType type = ENt_Unknown;
    size_t pos = aName.find(".");
    string tname = aName.substr(0, pos);
    string name;
    ParseTname(tname, type, name);
    for (CAE_ChromoNode::Iterator it = Begin(); it != End(); it++) {
	if (((*it).Type() == type) && (name.compare((*it).Name()) == 0)) {
	    res = it;  break;
	}
    }
    if ((res != End()) &&  (pos != string::npos)) {
	res = (*res).Find(aName.substr(pos + 1));	
    }
    // TODO [YB] Return of "res" directly doesn't work, why ??
    //return res;
    return Iterator(res);
};

// TODO [YB] Node name should not contain ".". Reconsider nodes "conn" etc.
// TODO [YB] Unclear logic of find. We find recursivelly here, but what is type in this case. Acc to logic all the 
// hierarhy level should be same type. What is sence of that?
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

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(NodeType aType, const string& aName, TNodeAttr aAttr, const string& aAttrVal)
{
    CAE_ChromoNode::Iterator res = End();
    for (CAE_ChromoNode::Iterator it = Begin(); it != End(); it++) {
	const CAE_ChromoNode& node = (*it);
	if ((node.Type() == aType)  && (aName.compare(node.Name()) == 0) && node.AttrExists(aAttr) && (aAttrVal.compare(node.Attr(aAttr)) == 0)) {
	    res = it;  break;
	}
    }
    return res;
};



//*********************************************************
// CAE_Object
//*********************************************************

FAPWS_API CAE_Object::CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv):
    CAE_EBase(aInstName, aMan), iChromX(NULL), iEnv(aEnv), iMut(NULL), iChromo(NULL), iChromoIface(*this),
    iCtrl(*this), iOpv(NULL)
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
    iMut->Init(ENt_Object);
    iChromo = iEnv->Provider()->CreateChromo();
    iChromo->Init(ENt_Object);
    CAE_ChromoNode croot = iChromo->Root();
    croot.SetAttr(ENa_Id, iInstName);
    croot.SetAttr(ENa_Type, "none");
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

void  CAE_Object::RegisterCompL(CAE_EBase* aComp) 
{ 
    CAE_Object* comp = aComp->GetFbObj(comp);
    if (comp != NULL) {
	_FAP_ASSERT(iComps.count(aComp->InstName()) == 0);
	iComps.insert(pair<string, CAE_Object*>(comp->InstName(), comp));
    }
    else {
	CAE_StateBase* state = aComp->GetFbObj(state);
	_FAP_ASSERT(state != NULL);
	_FAP_ASSERT(iStates.count(aComp->InstName()) == 0);
	iStates.insert(pair<string, CAE_StateBase*>(state->InstName(), state));
    }
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
		    if (!p1->Extend(p2) && !p2->Extend(p1))
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
	iComps.clear();
	iStates.clear();
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

// TODO [YB] How control state is activated on chromo change? Object doesn't have something like connection in state
// to know that control state is "connected" to it. Answer: Object should not have connection. Control state itself has
// this mechanism. So if someone needs to be "connected" to object chromo then it should be connected to control state.
void CAE_Object::Confirm()
{
    // Confirm the elements
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++)
    {
	CAE_Object* obj = it->second;
	if (obj->IsUpdated() && !obj->IsQuiet())
	{
	    obj->Confirm();
	    obj->ResetUpdated();
	}
    }
    for (map<string, CAE_StateBase*>::iterator it = iStates.begin(); it != iStates.end(); it++)
    {
	CAE_StateBase* state = it->second;
	if (state->IsUpdated())
	{
	    state->Confirm();
	    state->ResetUpdated();
	}
    }

    // Mutate if required
    if (iMut != NULL) {
	CAE_ChromoNode mut = iMut->Root();
	if (mut.Begin() != mut.End())
	{
	    // Log muta spec
	    TInt logdata = GetLogSpecData(KBaseLe_Updated);
	    if (logdata != KBaseDa_None) {
		Logger()->WriteFormat("Updated object [%s.%s.%s]:", MansName(1), MansName(0), iInstName);
		iMut->Root().Dump(Logger());
	    }
	    Mutate();
	    // Reactivate all the components
	    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++) {
		CAE_EBase* elem = it->second;
		elem->SetActive();
	    }
	    for (map<string, CAE_StateBase*>::iterator it = iStates.begin(); it != iStates.end(); it++) {
		CAE_EBase* elem = it->second;
		elem->SetActive();
	    }
	}
    }
};

void CAE_Object::SetMutation(const CAE_ChromoNode& aMuta)
{
    iMut->Set(aMuta);
}

// TODO [YB] To consider creating from chromo but not via mut
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
	// Mutate object 
	obj->SetMutation(aNode);
	obj->Mutate();
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
		TTransInfo tinfo((nodetxt == stelem.End()) ? "" : (*nodetxt).Content());
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
	    // Extention is one way only!
	    // TODO [YB] Do we need some ref from entended cp to extender. Look at usecase
	    // UC_CONN_06 USER removes internal sub-system, SYSTEM disextend connection points
	    if (!p1->Extend(p2))
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


void CAE_Object::ChangeChromoAttr(const CAE_ChromoNode& aSpec, CAE_ChromoNode& aCurr)
{
    NodeType ntype = aSpec.AttrNtype(ENa_Type);
    string nname = aSpec.Name();
    TNodeAttr mattr = aSpec.AttrNatype(ENa_MutChgAttr);
    string mattrs = aSpec.Attr(ENa_MutChgAttr);
    string mval = aSpec.Attr(ENa_MutChgVal);
    if (aCurr.AttrExists(mattr)) {
	aCurr.SetAttr(mattr, mval);
	// TODO [YB] To update dependent nodes (connections etc.)
    }
    else {
	Logger()->WriteFormat("ERROR: Changing [%s] - attr [%s] not found", nname.c_str(), mattrs.c_str());
    }
}

void CAE_Object::ChangeAttr(CAE_EBase* aNode, const CAE_ChromoNode& aSpec, const CAE_ChromoNode& aCurr)
{
    NodeType ntype = aSpec.AttrNtype(ENa_Type);
    string nname = aSpec.Name();
    TNodeAttr mattr = aSpec.AttrNatype(ENa_MutChgAttr);
    string mattrs = aSpec.Attr(ENa_MutChgAttr);
    string mval = aSpec.Attr(ENa_MutChgVal);
    if (ntype == ENt_State)
    {
	map<string, CAE_StateBase*>::iterator it = iStates.find(nname);
	if (it != iStates.end()) {
	    CAE_StateBase* state = it->second;
	    if (mattr == ENa_StInit) {
		state->SetFromStr(mval.c_str());
		state->Confirm();
	    }
	    else if (mattr == ENa_Id) {
		iStates[nname] = NULL;
		iStates.erase(it);
		state->SetName(mval.c_str());
		iStates[mval] = state;
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
		    TBool res = ext->Disextend(cpair);
		    if (res) {
			CAE_ConnPointBase* npair = GetConn(mval.c_str());
			if (npair != NULL) {
			    res = ext->Extend(npair);
			    if (!res) {
				Logger()->WriteFormat("ERROR: Changing extention [%s] - re-extention to [%s] failed", nname.c_str(), 
					cpair_name.c_str());
			    }
			}
			else {
			    Logger()->WriteFormat("ERROR: Changing extention [%s] - new pair [%s] not found", nname.c_str(), mval.c_str());
			}
		    }
		    else {
			Logger()->WriteFormat("ERROR: Changing extention [%s] - disextention from [%s] failed", nname.c_str(), cpair_name.c_str());
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
    else if (ntype == ENt_Object)
    {
	map<string, CAE_Object*>::iterator it = iComps.find(nname);
	if (it != iComps.end()) {
	    CAE_Object* obj = it->second;
	    if (mattr == ENa_Id) {
		iComps[nname] = NULL;
		iComps.erase(it);
		obj->SetName(mval.c_str());
		iComps[mval] = obj;
	    }
	    else {
		Logger()->WriteFormat("ERROR: Changing object [%s] - changing attr [%s] not supported", obj->InstName(), mattrs.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Changing object [%s] - not found", nname.c_str());
	}
    }
    else if (ntype == ENt_Stinp)
    {
	CAE_StateBase* state = aNode->GetFbObj(state);
	map<string, CAE_ConnPointBase*>& inputs = state->Inputs();
	if (state != NULL) {
	    map<string, CAE_ConnPointBase*>::iterator it = inputs.find(nname);
	    if (it != inputs.end()) {
		CAE_ConnPointBase* cp = it->second;
		if (mattr == ENa_Id) {
		    inputs[nname] = NULL;
		    inputs.erase(it);
		    cp->SetName(mval.c_str());
		    inputs[mval] = cp;
		}
		else {
		    Logger()->WriteFormat("ERROR: Changing node [%s] - attr [%s] not found", aNode->InstName(), nname.c_str());
		}

	    }
	}
    }
    else if (ntype == ENt_Trans)
    {
	CAE_StateBase* state = aNode->GetFbObj(state);
	if (state != NULL) {
	    state->SetTrans(TTransInfo(mval));
	}
	else {
	    Logger()->WriteFormat("ERROR: Changing trans [%s] - node type is not supported", aNode->InstName());
	}

    }
    else {
	Logger()->WriteFormat("ERROR: Changing [%s] - changing node type [%d] not supported", nname.c_str(), ntype);
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
    else if (type == ENt_State) {
	CAE_ChromoNode& chr = iChromo->Root();
	CAE_ChromoNode::Iterator elemit = chr.Find(type, name);
	if (elemit != chr.End()) {
	    if (iStates.count(name) != 0) {
		iStates.erase(name);
	    }
	    else {
		Logger()->WriteFormat("ERROR: Deleting state from  [%s] - cannot find state [%s]", InstName(), name.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Deleting state from  [%s] - cannot find state [%s]", InstName(), name.c_str());
	}

    }
    else {
	Logger()->WriteFormat("ERROR: Deleting element from  [%s] - unsupported element type for deletion", InstName());
    }
}

void CAE_Object::Mutate()
{
    DoMutation();
    // Clear mutation
    CAE_ChromoNode& root = iMut->Root();
    for (CAE_ChromoNode::Iterator mit = root.Begin(); mit != root.End();)
    {
	CAE_ChromoNode node = *mit;
	mit++; // It is required because removing node by iterator breakes iterator itself
	root.RmChild(node);
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
    CAE_ChromoNode& mroot = iMut->Root();
    CAE_ChromoNode& chrroot = iChromo->Root();
    for (CAE_ChromoNode::Iterator rit = mroot.Begin(); rit != mroot.End(); rit++)
    {
	CAE_ChromoNode rno = (*rit);
	NodeType rnotype = rno.Type();
	if (rnotype == ENt_Object) {
	    AddObject(rno);
	    if (iComps.count(rno.Name()) > 0) {
		CAE_Object* comp = iComps[rno.Name()];
		chrroot.AddChild(comp->iChromo->Root(), EFalse);
	    }
	}
	else if (rnotype == ENt_Stinp) {
	    AddConn(rno, iInputs);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Soutp) {
	    AddConn(rno, iOutputs);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_State) {
	    AddState(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Conn) {
	    AddConn(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Cext) {
	    AddExt(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Cextc) {
	    AddExtc(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Trans) {
	    AddTrans(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Logspec) {
	    AddLogspec(rno);
	    chrroot.AddChild(rno);
	}
	else if (rnotype == ENt_Mut) {
	    TBool emnode = rno.AttrExists(ENa_MutNode);
	    string mnoden = rno.Attr(ENa_MutNode);
	    TBool selfnode = (!emnode || (mnoden.compare("self") == 0));
	    CAE_EBase* node = this;
	    CAE_ChromoNode mnode = chrroot;
	    if (!selfnode) {
		node = FindByName(mnoden.c_str());
		CAE_ChromoNode::Iterator mnodeit = chrroot.Find(ENt_Object, mnoden);
		if (mnodeit != chrroot.End()) {
		    mnode = *mnodeit;
		}
		_FAP_ASSERT(node == NULL || mnodeit != chrroot.End());
	    }
	    if (node != NULL)
	    {
		// Handling mutations
		for (CAE_ChromoNode::Iterator mit = rno.Begin(); mit != rno.End(); mit++)
		{
		    CAE_ChromoNode mno = (*mit);
		    if (mno.Type() == ENt_MutAdd) 
		    {
			for (CAE_ChromoNode::Iterator mait = mno.Begin(); mait != mno.End(); mait++)
			{
			    CAE_ChromoNode mano = (*mait);
			    if (mano.Type() == ENt_Object) {
				AddObject(mano);
			    }
			    else if (mano.Type() == ENt_Stinp) {
				CAE_Object* obj = node->GetFbObj(obj);
				if (obj != NULL) {
				    AddConn(mano, obj->iInputs);
				}
				else {
				    CAE_StateBase* state = node->GetFbObj(state);
				    if (state != NULL) {
					CreateStateInp(mano, state);
				    }
				    else {
					Logger()->WriteFormat("ERROR: Mutating node [%s] - adding inp not supported", node->InstName());
				    }
				}
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
			    // Change chromo
			    chrroot.AddChild(mano);
			}
		    }
		    else if (mno.Type() == ENt_MutRm) 
		    {
			for (CAE_ChromoNode::Iterator mrmit = mno.Begin(); mrmit != mno.End(); mrmit++)
			{
			    CAE_ChromoNode mrno = *mrmit;
			    if (mrno.Type() == ENt_Node) {
				NodeType type = mrno.AttrNtype(ENa_Type);
				string name = mrno.Name();
				CAE_ChromoNode::Iterator remit = chrroot.End();
				if (mrno.AttrExists(ENa_MutChgAttr)) {
				    TNodeAttr mattr = mrno.AttrNatype(ENa_MutChgAttr);
				    string attval = mrno.Attr(ENa_MutChgVal);
				    remit = chrroot.Find(type, name, mattr, attval);
				}
				else {
				    remit = chrroot.Find(type, name);
				}
				if (remit != chrroot.End()) {
				    RemoveElem(mrno);
				    // Change chromo
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
			    // Change runtime model
			    ChangeAttr(node, mno, *curr);
			    // Change chromo
			    CAE_ChromoNode currn = *curr;
			    ChangeChromoAttr(mno, currn);
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
	    }
	    else
	    {
		Logger()->WriteFormat("ERROR: Mutating object [%s] - unknown node [%s] to be mutated", InstName(), mnoden.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Mutating object [%s] - unknown mutation type [%d]", InstName(), rnotype);
	}
    }
}

void CAE_Object::Update()
{
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++)
    {
	CAE_EBase* elem = it->second;
	if (elem->IsActive() && !elem->IsQuiet())
	{
	    elem->SetUpdated();
	    elem->ResetActive();
	    elem->Update();
	}
    }
    for (map<string, CAE_StateBase*>::iterator it = iStates.begin(); it != iStates.end(); it++)
    {
	CAE_EBase* elem = it->second;
	if (elem->IsActive() && !elem->IsQuiet())
	{
	    elem->SetUpdated();
	    elem->ResetActive();
	    elem->Update();
	}
    }

}


void CAE_Object::UnregisterComp(CAE_EBase* aComp)
{
    CAE_Object* obj = aComp->GetFbObj(obj);
    if (obj != NULL) {
	map<string, CAE_Object*>::iterator it = iComps.find(obj->InstName());
	_FAP_ASSERT(it != iComps.end());
	iComps.erase(it);
    }
    else {
	CAE_StateBase* state = aComp->GetFbObj(state);
	_FAP_ASSERT(state != NULL);
	map<string, CAE_StateBase*>::iterator it = iStates.find(obj->InstName());
	_FAP_ASSERT(it != iStates.end());
	iStates.erase(it);
    }
}

CAE_Object* CAE_Object::GetComp(const char* aName, TBool aGlob)
{
    CAE_Object* res = iComps[aName];
    if (res == NULL && aGlob) {
	for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++) {
	    CAE_Object* comp = it->second;
	    res = comp->GetComp(aName, ETrue);
	}
    }
    return res;
}

// TODO [YB] To move FindByName to CAE_EBase
FAPWS_API CAE_EBase* CAE_Object::FindByName(const string& aName)
{
    CAE_EBase* res = NULL;
    size_t pos = aName.find(KNameSeparator);
    string tname = aName.substr(0, pos);
    NodeType type = ENt_Unknown;
    string name;
    iChromo->Root().ParseTname(tname, type, name);
    if (type == ENt_Object) {
	map<string, CAE_Object*>::iterator it = iComps.find(name);
	if (it != iComps.end()) {
	    res = it->second;
	}
    }
    else if (type == ENt_State) {
	map<string, CAE_StateBase*>::iterator it = iStates.find(name);
	if (it != iStates.end()) {
	    res = it->second;
	}
    }
    if ((res != NULL) &&  (pos != string::npos)) {
	CAE_Object* obj = res->GetFbObj(obj);
	if (obj != NULL) {
	    res = obj->FindByName(aName.substr(pos + 1)); 
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
    CAE_ChromoNode root = iChromo->Root();
    heir->SetMutation(root);
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

void CAE_Object::AddView(MAE_View* aView)
{
    _FAP_ASSERT (iViews[aView->Name()] == 0);
    iViews[aView->Name()] = aView;
    iOpv = new CPV_Opvi(aView);
    iOpv->SetObj(&iCtrl);
}

void CAE_Object::SetBaseViewProxy(MAE_Opv* aProxy, TBool aAsRoot)
{
    _FAP_ASSERT (iOpv == 0);
    iOpv = aProxy;
    iOpv->SetObj(&iCtrl);
    if (aAsRoot) {
	iOpv->SetRoot(&iCtrl);
    }
}

void CAE_Object::RemoveBaseViewProxy(MAE_Opv* aProxy)
{
    _FAP_ASSERT (iOpv == aProxy);
    iOpv->UnsetObj(&iCtrl);
    iOpv = NULL;
}

void CAE_Object::OnHeaderPress(const MAE_View* aView)
{
    // Return view to super-system
    MAE_View* view = iViews[aView->Name()];
    iViews[view->Name()] = NULL;
    //view->Wnd()->Clear();
    //ResetView();
    CAE_Object* obj = iMan;
    obj->AddView(view);
    //obj->OnExpose(view->Wnd(), CAV_Rect());
}

void CAE_Object::OnCompHeaderPress(const MAE_View* aView, const string& aName)
{
    // Return view to super-system
    MAE_View* view = iViews[aView->Name()];
    iViews[view->Name()] = NULL;
    //view->Wnd()->Clear();
    //ResetView();
    CAE_EBase* comp = FindByName(aName.c_str());
    CAE_Object* obj = comp->GetFbObj(obj);
    obj->AddView(view);
    //obj->OnExpose(view->Wnd(), CAV_Rect());
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


