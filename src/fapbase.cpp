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

#include <stdlib.h>
#include <stdio.h>
#include "fapplat.h"
#include "panics.h"
#include "fapbase.h"


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

CAE_ConnPoint::CAE_ConnPoint(const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl): 
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
	// Check the latest slot, if the pin is not set, then set them, otherwise create new slot
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

CAE_Base* CAE_ConnPoint::GetSrcPin(const char* aName)
{
    return iSrcs->Pin(aName);
}

void CAE_ConnPoint::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
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
    }
    return res;
}

void CAE_ConnPointExt::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
    if (iRef != NULL) {
	iRef->Disconnect(aConnPoint);
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

TBool CAE_ConnPointExtC::ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    TBool res = ETrue;
    // Process all the slots
    for (vector<Slot>::iterator is = iSlots.begin(); is != iSlots.end() && res; is++) {
	// Redirect to connected pin
	map<string, Slot::slot_elem>::iterator id = (*is).Dests().find(aPin);
	CAE_ConnPointBase* cp = id->second.first;
	string& pin_name = id->second.second;
	res = cp->ConnectPin(pin_name.c_str(), aPair, aPairPin);
    }
    return res;
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
    return EFalse;
}

void *CAE_ConnPointExtC::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : NULL;
}

CAE_ConnPointExtC::Slot::Slot(const SlotTempl& aTempl)
{
    Slot slot;
    // Copy Dest and Srcs from template
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = aTempl.Dests().begin(); it != aTempl.Dests().end(); it++) {
	slot.Dests()[it->first] = slot_elem(NULL, "");
    }
    for (map<string, SlotTempl::slot_templ_elem>::const_iterator it = aTempl.Srcs().begin(); it != aTempl.Srcs().end(); it++) {
	slot.Srcs()[it->first] = slot_elem(NULL, "");
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
	iActive= ETrue; if (iMan) iMan->SetActive();
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

// CAE_State
	
void CAE_State::ConstructL()
{
	iCurr = (void*) new  TUint8[iLen];
	iNew = (void*) new  TUint8[iLen];
	memset(iCurr, 0x00, iLen);
	memset(iNew, 0x00, iLen);
	// TODO [YB] Do we need to assert iMan?
	if (iMan)
	    iMan->RegisterCompL(this);
	CAE_ConnSlot::Template dt, st;
	dt["_1"]  = "State";
	st["_1"]  = "State";
	CAE_ConnPoint *outp = new CAE_ConnPoint(st, dt); 
	iOutput = outp;
	outp->Srcs()->SetPin("_1", this);
	if (Logger())
	    Logger()->WriteFormat("State created:: Name: %s, Len: %d", iInstName, iLen);
}	

CAE_State::~CAE_State()
{
    free(iCurr);
    free(iNew);
    TInt i = 0;
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

CAE_State::mult_point_inp_iterator CAE_State::MpInput_begin(const char *aName) 
{ 
    CAE_ConnPoint *cp = iInputs.find(aName)->second->GetFbObj(cp); 
    return mult_point_inp_iterator(cp->Dests(), 0);
};

CAE_State::mult_point_inp_iterator CAE_State::MpInput_end(const char *aName) 
{ 
    CAE_ConnPoint *cp = iInputs.find(aName)->second->GetFbObj(cp); 
    return mult_point_inp_iterator(cp->Dests(), cp->Dests().size());
}

void CAE_State::Confirm()
{
    if (memcmp(iCurr, iNew, iLen))
    {
	CAE_ConnPoint *outp = iOutput->GetFbObj(outp);
	_FAP_ASSERT(outp != NULL);
	for (TInt i = 0; i < outp->Dests().size(); i++) {
	    CAE_State* sout =  outp->Slot(i)->Pin("_1")->GetFbObj(sout);
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

void CAE_State::LogUpdate(TInt aLogData)
{
    if (Logger() == NULL) return;
    char *buf_cur = DataToStr(ETrue);
    char *buf_new = DataToStr(EFalse);
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
	    CAE_State* state = (*it).second->GetFbObj(state);
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

void CAE_State::LogTrans(TInt aLogData)
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
	    CAE_State* state = (*it).second->GetFbObj(state);
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

void CAE_State::Update()
{
    TInt logdata = GetLogSpecData(KBaseLe_Trans);
    if (logdata != KBaseDa_None)
	LogTrans(logdata);
    DoTrans();
}

void CAE_State::Set(void* aNew) 
{
    // [YB] Why cannot set output when emply outp list? 
//    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
    {
	if (memcmp(aNew, iNew, iLen))
	{
	    SetUpdated();
	    memcpy(iNew, aNew, iLen); 
	    TInt logdata = GetLogSpecData(KBaseLe_Updated);
	    if (logdata != KBaseDa_None)
		LogUpdate(logdata);
	}
    }
};

void CAE_State::SetFromStr(const char *aStr)
{
    // [YB] Why cannot set output when emply outp list? 
//    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
    {
	void *data = malloc(iLen);
	memset(data, 0, iLen);
	DataFromStr(aStr, data);
	Set(data);
	free(data);
    }
}

CAE_State* CAE_State::Input(const char* aName)
{
    CAE_State *res = NULL;
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

void CAE_State::AddInputL(const char* aName) 
{
    TBool exists = iInputs.find(aName) != iInputs.end();
    if (exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already exists", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(!exists);
    }
    CAE_ConnSlot::Template dt, st;
    st["_1"]  = "State";
    dt["_1"]  = "State";
    CAE_ConnPoint *cpoint = new CAE_ConnPoint(st, dt);
    cpoint->Srcs()->SetPin("_1", this);
    pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = iInputs.insert(pair<string, CAE_ConnPoint*>(aName, cpoint));
    bool bres = res.second;
}


void CAE_State::SetInputL(const char *aName, CAE_State* aState)
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
void CAE_State::AddInputL(const char *aName, CAE_State* aState) 
{
    AddInputL(aName);
    SetInputL(aName, aState);
}

const char* KCAE_StateName = "State";

CAE_State::CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_EBase(aInstName, aMan), iLen(aLen), iTrans(aTrans)
{
    _FAP_ASSERT (iMan != NULL);
}

CAE_State* CAE_State::NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans)
{
	CAE_State* self = new CAE_State(aInstName, aLen, aMan, aTrans);
	self->ConstructL();
	return self;
}

TBool CAE_State::SetTrans(TTransInfo aTinfo) 
{ 
	iTrans = aTinfo; 
	return ETrue;
};


void CAE_State::DoTrans() 
{
    if (iTrans.iFun != NULL) 
	iTrans.iFun((iTrans.iCbo != NULL) ? iTrans.iCbo : iMan, this);
    else if (iTrans.iOpInd != 0)
	DoOperation();
};

// The base class has the trivial implementation of operation
void CAE_State::DoOperation()
{
}


FAPWS_API TOperationInfo CAE_State::OperationInfo(TUint8 aId) const
{
	TOperationInfo nfo = {aId, 0, 0, 0};
	return nfo;
}

// From CAE_Base
void *CAE_State::DoGetFbObj(const char *aName)
{
    if ((iTypeName != NULL) && (strcmp(iTypeName, aName) == 0) || (strcmp(aName, Type()) == 0))
	return this;
    else
	return CAE_EBase::DoGetFbObj(aName);
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
// CAE_Object
//*********************************************************

FAPWS_API CAE_Object::CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv):
	CAE_EBase(aInstName, aMan), iChromX(NULL), iEnv(aEnv) 
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

FAPWS_API CAE_Object* CAE_Object::NewL(const char* aInstName, CAE_Object* aMan, const void* aChrom, MAE_Env* aEnv)
{
	CAE_Object* self = new CAE_Object(aInstName, aMan, aEnv);
	self->SetChromosome(EChromOper_Copy, aChrom);
	// TODO [YB] To add chrom mutation
	self->ConstructL();
	return self;
}

FAPWS_API void CAE_Object::SetChromosome(TChromOper aOper, const void* aChrom1, const char* aChrom2)
{
    if (aChrom1 != NULL)
    {
	iEnv->Chman()->CopySpec(aChrom1, &iChromX);	
    }
}


// On construction phase object is created from given chromosome
// If chromosome isn't given and provider exists then chromosome is created randomly
// There is restrictions of mutation implemented on construction phase:
// If there is the locus with the unallowed allele then it's considered as mutation
// and the allele is generated randomly in the legal value interval   

// TODO [YB] Do we need ConstructL? It's just wrapper of ConstructFromChromXL
FAPWS_API void CAE_Object::ConstructL(const void* aChrom)
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

FAPWS_API void CAE_Object::ChangeChrom(const void* aChrom)
{
    MAE_ChroMan *chman = iEnv->Chman();
    _FAP_ASSERT(chman != NULL);
    char *sparent = chman->GetStrAttr((void*) aChrom, KXTransAttr_Type);
//    iEnv->Root()->FindName(sparent);
}

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
	    CAE_State *state = NULL;
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
		state = (CAE_State *) FindByName(name);
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
	    CAE_State* state = (CAE_State *) FindByName(name);
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

void CAE_Object::CreateStateInp(void* aSpecNode, CAE_State *aState) 
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
	CAE_ConnPoint *cpoint = new CAE_ConnPoint(st, dt);
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
	    CAE_ConnPointExtC *cpoint = new CAE_ConnPointExtC();
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
	    cp = new CAE_ConnPointExt();
	}
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aConns.insert(pair<string, CAE_ConnPointBase*>(name, cp));
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
    else return KBaseDa_None;
}

FAPWS_API void CAE_Object::Confirm()
{
    for (TInt i = 0; i < iCompReg.size(); i++ )
    {
	CAE_EBase* obj = (CAE_EBase*) iCompReg.at(i);
	if (obj->IsUpdated() && !obj->IsQuiet())
	{
	    obj->Confirm();
	    obj->ResetUpdated();
	}
    }
};

FAPWS_API void CAE_Object::DoMutation()
{
    // TODO [YB] to implement
}

FAPWS_API void CAE_Object::Update()
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
	    if (strcmp(comp->TypeName(), aType) == 0) {
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
	if ((strcmp(comp->TypeName(), aType) == 0) && (elem = comp->GetFbObj(elem)) != NULL) {
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
	CAE_State *state = elem->GetFbObj(state);
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
	CAE_State *state = elem->GetFbObj(state);
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


FAPWS_API void CAE_Object::LinkL(CAE_State* aInp, CAE_State* aOut, TTransFun aTrans)
{
    _FAP_ASSERT(aTrans || (aInp->Len() == aOut->Len())); // Panic(EFapPan_IncorrLinksSize)); 
    aInp->AddInputL("_Inp_1");
    aInp->SetInputL("_Inp_1", aOut);
	aInp->SetTrans(TTransInfo(aTrans));
}

FAPWS_API CAE_State* CAE_Object::GetStateByName(const char *aName)
{
    CAE_State* res = NULL;
    for (TInt i = 0; i < iCompReg.size(); i++)
    {
	CAE_EBase* comp = (CAE_EBase*) iCompReg.at(i);
	CAE_State* state = (CAE_State*) comp->GetFbObj(state);
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

CAE_State* CAE_Object::GetOutpState(const char* aName)
{
    CAE_State *res = NULL;
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

CAE_State* CAE_Object::GetInpState(const char* aName)
{
    CAE_State *res = NULL;
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

void CAE_ObjectBa::UpdateInput(CAE_State* aState)
{
	TInt data = iInp->Value();
	*iStInp = data;
}

void CAE_ObjectBa::UpdateInpReset(CAE_State* aState)
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

void CAE_StateASrv::UpdateSetActive(CAE_State* aState)
{
	if (iInpSetActive->Value() > 0)
	{
		iReqMon->iStatus = KRequestPending;
		iReqMon->SetActive();
	}
}

void CAE_StateASrv::UpdateCancel(CAE_State* aState)
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


