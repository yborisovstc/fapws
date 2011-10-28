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
#include "fapfact.h"

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

void Panic(TInt aRes)
{
	_IND_PANIC(KFapPanic, aRes);
}


// Des URI
// Uri conforms to RFC 3986 
// Query syntax: (attr_name '=' attr_value) *( ('&' | '|')  (attr_name '=' attr_value))  

map<string, TNodeType> DesUri::iEbNameToNType;
map<TLeBase, string> DesUri::iLeventNames;
map<string, TLeBase> DesUri::iLevents;
map<TLdBase, string> DesUri::iLdataNames;
map<string, TLdBase> DesUri::iLdata;

extern map<string, TNodeType> KNodeTypes;
extern map<TNodeType, string> KNodeTypesNames;
extern map<TNodeAttr, string> KNodeAttrsNames;
extern map<string, TNodeAttr> KNodeAttrs;

const string& DesUri::NodeAttrName(TNodeAttr aAttr)
{
    return KNodeAttrsNames[aAttr];
}

const string& DesUri::NodeTypeName(TNodeType aType)
{
    return KNodeTypesNames[aType];
}

TNodeAttr DesUri::NodeAttr(const string& aAttrName)
{
    return KNodeAttrs.count(aAttrName) > 0 ? KNodeAttrs[aAttrName] : ENa_Unknown;
}

DesUri::DesUri(const string& aUri): iUri(aUri)
{
    Construct();
    Parse();
}

DesUri::DesUri(): iUri()
{
    Construct();
}

DesUri::DesUri(CAE_NBase* aElem, CAE_NBase* aBase)
{
    Construct();
    PrependElem(aElem, ETrue, aBase);
}

DesUri::DesUri(TNodeType aType, const string& aName)
{
    Construct();
    AppendElem(aType, aName);
}

void DesUri::Construct()
{
    if (iEbNameToNType.size() == 0)
    {
	iEbNameToNType[CAE_Object::Type()] = ENt_Object;
    }
    if (iLeventNames.size() == 0)
    {
	iLeventNames[KBaseLe_None] = "none";
	iLeventNames[KBaseLe_Creation] = "cre";
	iLeventNames[KBaseLe_Updated] = "upd";
	iLeventNames[KBaseLe_Trans] = "tran";
	iLeventNames[KBaseLe_Any] = "any";

	for (map<TLeBase, string>::const_iterator it = iLeventNames.begin(); it != iLeventNames.end(); it++) {
	    iLevents[it->second] = it->first;
	}
    }
    if (iLdataNames.size() == 0)
    {
	iLdataNames[KBaseDa_None] = "none";
	iLdataNames[KBaseDa_Curr] = "cur";
	iLdataNames[KBaseDa_New] = "new";
	iLdataNames[KBaseDa_Dep] = "dep";
	iLdataNames[KBaseDa_Trex] = "trex";

	for (map<TLdBase, string>::const_iterator it = iLdataNames.begin(); it != iLdataNames.end(); it++) {
	    iLdata[it->second] = it->first;
	}
    }
}

const string& DesUri::LeventName(TLeBase aEvent) 
{ 
    if (iLeventNames.size() == 0) Construct(); 
    return iLeventNames[aEvent];
};

TLeBase DesUri::Levent(const string& aName)
{ 
    if (iLevents.size() == 0) Construct(); 
    return iLevents.count(aName) == 0 ? KBaseLe_None : iLevents[aName];
};

const string& DesUri::LdataName(TLdBase aData) 
{ 
    if (iLdataNames.size() == 0) Construct(); 
    return iLdataNames[aData];
};

TLdBase DesUri::Ldata(const string& aName)
{ 
    if (iLdata.size() == 0) Construct(); 
    return iLdata.count(aName) == 0 ? KBaseDa_None : iLdata[aName];
};

void DesUri::Parse()
{
    TBool fin = EFalse;
    size_t query_beg = iUri.find_first_of('?', 0);
    string hier = iUri.substr(0, query_beg);
    size_t elem_beg = 0;
    size_t elem_end = hier.find_first_of('/', 0);
    string elem = hier.substr(0, elem_end);
    // Hier
    while (!elem.empty()) {
	size_t type_end = elem.find_first_of(':');
	size_t name_beg = 0;
	TNodeType type;
	if (type_end == string::npos) {
	    type = ENt_Object;
	}
	else {
	    string tname = elem.substr(0, type_end);
	    type = KNodeTypes[tname];
	    name_beg = type_end + 1;
	}
	string name = elem.substr(name_beg);
	iElems.push_back(TElem(type, name));
	if (elem_end == string::npos) {
	    elem.clear();
	} 
	else {
	    elem_beg = elem_end + 1;
	    elem_end = hier.find_first_of('/', elem_beg);
	    elem = hier.substr(elem_beg, elem_end == string::npos ? elem_end : elem_end - elem_beg);
	}
    }
    // Query
    if (query_beg != string::npos) {
	// Just one condition for now
	string query = iUri.substr(query_beg+1);
	size_t cond_beg = 0;
	size_t cond_end = query.find_first_of("&", cond_beg);
	string cond = query.substr(cond_beg, cond_end);
	TQueryOpr op = EQop_Unknown;
	while (cond_beg != string::npos && !cond.empty()) {
	    size_t attr_beg = 0;
	    size_t attr_end = cond.find_first_of('=', 0);
	    size_t val_beg = attr_end + 1;
	    size_t val_end = string::npos;
	    string attrs = cond.substr(attr_beg, attr_end);
	    string val = cond.substr(val_beg, val_end);
	    TNodeAttr attr = KNodeAttrs.count(attrs) > 0 ? KNodeAttrs[attrs] : ENa_Unknown;
	    AppendQueryElem(op, attr, val);
	    char ops = cond_end == string::npos ? ' ' : query.at(cond_end);
	    op = EQop_Unknown;
	    if (ops == '&') 
		op = EQop_And;
	    cond_beg = cond_end == string::npos ? string::npos : cond_end + 1;
	    if (cond_beg != string::npos) {
		cond_end = query.find_first_of("&", cond_beg);
		cond = query.substr(cond_beg, cond_end);
	    }
	}
    }
}

string DesUri::GetUri(vector<TElem>::const_iterator aStart) const
{
    string res;
    // Hier
    for (vector<DesUri::TElem>::const_iterator it = aStart; it != iElems.end(); it++) {
	DesUri::TElem elem = *it;
	if (elem.first != ENt_Object) {
	    res.append(KNodeTypesNames[elem.first] + ":");
	}
	res.append(elem.second);
	if (it + 1 != iElems.end()) {
	    res.append("/");
	}
    }
    // Query
    if (!iQueryElems.empty()) {
	res.append("?");
	for (vector<TQueryElem>::const_iterator it = iQueryElems.begin(); it != iQueryElems.end(); it++) {
	    TQueryOpr opr = it->first;
	    if (opr ==  EQop_And) {
		res.append("&");
	    }
	    res.append(KNodeAttrsNames[it->second.first]);
	    res.append("=");
	    res.append(it->second.second);
	}
    }
    return res;
}

TNodeType DesUri::GetType() const
{
    TInt size = iElems.size();
    return size == 0 ? ENt_Unknown : iElems.at(size -1).first;
}

string DesUri::GetName() const
{
    TInt size = iElems.size();
    return size == 0 ? string() : iElems.at(size -1).second;
}

void DesUri::AppendElem(TNodeType aType, const string& aName)
{
    iElems.push_back(TElem(aType, aName));
}

void DesUri::PrependElem(TNodeType aType, const string& aName)
{
    iElems.insert(iElems.begin(), TElem(aType, aName));
}

// TODO [YB] To redesign the run-time model basing on treee. To have method of prepending uri in tree node base class
void DesUri::PrependElem(CAE_NBase* aElem, TBool aRec, CAE_NBase* aBase)
{
    if (aElem != aBase) {
	CAE_NBase* base = aElem->Owner();
	PrependElem(aElem->NodeType(), aElem->Name());
	if (aRec && base != NULL && base != aBase) {
	    PrependElem(base, aRec, aBase);
	}
    }
}

void DesUri::AppendQueryElem(TQueryOpr aOpr, TNodeAttr aAttr, const string& aValue)
{
    iQueryElems.push_back(TQueryElem(aOpr, TQueryCnd(aAttr, aValue)));
}

// Node base

void *CAE_NBase::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : NULL;
}

TBool CAE_NBase::ChangeAttr(TNodeAttr aAttr, const string& aVal) 
{ 
    TBool res = EFalse;
    if (aAttr == ENa_Id) {
	// Re-register in owner
	// TODO [YB] Consider nodifying of attr change
	if (Owner() != NULL) {
	    Owner()->OnElemDeleting(this);
	    iInstName = aVal;
	    Owner()->OnElemAdding(this);
	    res = ETrue;
	}
    }
    return res;
};

// Log spec

TLogSpecBase::~TLogSpecBase()
{
    Owner()->OnElemDeleting(this);
}

// Notifier base

void CAE_NotifierBase::SetObs(void* aObs)
{
    _FAP_ASSERT(aObs != NULL);
    iObs.push_back(aObs);
}

void CAE_NotifierBase::RemoveObs(void* aObs)
{
    for (vector<void*>::iterator it = iObs.begin(); it != iObs.end(); it++) {
	if (*it == aObs) {
	    iObs.erase(it); return;
	}
    }
    _FAP_ASSERT(0);
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

// Connection point base

void CAE_ConnPointBase::AddConn(CAE_ConnBase* aConn)
{
    for (vector<CAE_ConnBase*>::iterator it = iConns.begin(); it != iConns.end(); it++) {
	_FAP_ASSERT(aConn != *it);
    }
    iConns.push_back(aConn);
}

void CAE_ConnPointBase::RmConn(CAE_ConnBase* aConn)
{
    for (vector<CAE_ConnBase*>::iterator it = iConns.begin(); it != iConns.end(); it++) {
	if (aConn == *it) {
	    iConns.erase(it);
	    break;
	}
    }
}

void *CAE_ConnPointBase::DoGetFbObj(const char *aName)
{
    void* res = NULL;
    if (strcmp(aName, Type()) == 0) {
	res = this;
    }
    else {
    }
    return res;
}

CAE_ConnPointBase::~CAE_ConnPointBase()
{
    Owner()->OnElemDeleting(this);
    for (vector<CAE_ConnBase*>::iterator it = iConns.begin(); it != iConns.end(); it++) {
	(*it)->OnCpDelete(this);
    }
    iConns.clear();
}

TBool CAE_ConnPointBase::Connect(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Point() == this || aConn->Pair() == this);
    TBool res = EFalse;
    res = Connect(aConn->Point() == this ? aConn->Pair() : aConn->Point());
    if (res) {
	AddConn(aConn);
    }
    return res;
}

TBool CAE_ConnPointBase::Extend(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Point() == this);
    TBool res = EFalse;
    res = Extend(aConn->Pair());
    if (res) {
	AddConn(aConn);
    }
    return res;
}

TBool CAE_ConnPointBase::SetExtended(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Pair() == this);
    TBool res = EFalse;
    res = SetExtended(aConn->Point());
    if (res) {
	AddConn(aConn);
    }
    return res;
}

TBool CAE_ConnPointBase::Disconnect(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Point() == this || aConn->Pair() == this);
    TBool res = EFalse;
    res = Disconnect(aConn->Point() == this ? aConn->Pair() : aConn->Point());
    RmConn(aConn);
    return res;
}

TBool CAE_ConnPointBase::Disextend(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Point() == this);
    TBool res = EFalse;
    res = Disextend(aConn->Pair());
    RmConn(aConn);
    return res;
}

TBool CAE_ConnPointBase::SetDisextended(CAE_ConnBase* aConn)
{
    _FAP_ASSERT(aConn->Pair() == this);
    TBool res = EFalse;
    res = SetDisextended(aConn->Point());
    RmConn(aConn);
    return res;
}
/*
CAE_ConnPoint::CAE_ConnPoint(const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl): 
    iSrcsTempl(aSrcsTempl), iDestsTempl(aDestsTempl) 
{
    iSrcs = new CAE_ConnSlot(iSrcsTempl);
}
*/

CAE_ConnPoint::CAE_ConnPoint(TNodeType aNodeType, const string aName, CAE_EBase* aMan, const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, 
	const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl): CAE_ConnPointBase(aNodeType, aName, aMan),
    iSrcsTempl(aSrcsTempl), iDestsTempl(aDestsTempl) 
{
    iSrcs = new CAE_ConnSlot(iSrcsTempl);
}

CAE_ConnPoint::~CAE_ConnPoint() 
{
}

void *CAE_ConnPoint::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : CAE_ConnPointBase::DoGetFbObj(aName);
}

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
    TBool res = ETrue;
    // Try to get connection
    CAE_ConnPoint *pair = aConnPoint->GetFbObj(pair); 
    if (pair != NULL ) {
	res = ConnectConnPoint(pair);
    }
    else {
	CAE_ConnPointExt *pex = aConnPoint->GetFbObj(pex); 
	if (pex !=  NULL)
	{
	    for (vector<CAE_ConnPointBase*>::iterator it = pex->Ref().begin(); it != pex->Ref().end() && res; it++) {
		res = Connect(*it);
	    }
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

TBool CAE_ConnPoint::DisconnectConnPoint(CAE_ConnPoint *aConnPoint)
{
    TBool res = ETrue;
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
    return res;
}

TBool CAE_ConnPoint::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = EFalse;
    CAE_ConnPoint *pair = aConnPoint->GetFbObj(pair); 
    if (pair != NULL ) {
	// Disconnect conn point
	res = DisconnectConnPoint(pair);
    }
    return res;
}

TBool CAE_ConnPoint::Extend(CAE_ConnPointBase *aConnPoint) 
{
    _FAP_ASSERT(0);
    return EFalse;
}

TBool CAE_ConnPoint::SetExtended(CAE_ConnPointBase *aConnPoint)
{
    return ETrue;
}

TBool CAE_ConnPoint::SetDisextended(CAE_ConnPointBase *aConnPoint)
{
    return ETrue;
}

TBool CAE_ConnPoint::Disextend(CAE_ConnPointBase *aConnPoint)
{
    _FAP_ASSERT(0);
    return EFalse;
}

void *CAE_ConnPointExt::DoGetFbObj(const char *aName)
{
    void* res = NULL;
    if (strcmp(aName, Type()) == 0) 
	res = this;
    else { 
	res = CAE_ConnPointBase::DoGetFbObj(aName);
	if (res == NULL && iRef.size() == 1)
	    res = iRef.at(0)->GetFbObj(aName);
    }
    return res;
}

TBool CAE_ConnPointExt::Connect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = ETrue;
    for (vector<CAE_ConnPointBase*>::iterator it = iRef.begin(); it != iRef.end() && res; it++) {
	res = (*it)->Connect(aConnPoint);
    }
    return res;
}

TBool CAE_ConnPointExt::Disconnect(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = ETrue;
    for (vector<CAE_ConnPointBase*>::iterator it = iRef.begin(); it != iRef.end() && res; it++) {
	res = (*it)->Disconnect(aConnPoint);
    }
    return res;
}

TBool CAE_ConnPointExt::Extend(CAE_ConnPointBase *aConnPoint) 
{
    TBool res = EFalse;
    iRef.push_back(aConnPoint);
    res = ETrue;
    return res;
};

CAE_Base* CAE_ConnPointExt::GetSrcPin(const char* aName)
{
    CAE_Base* res = NULL;
    if (iRef.size() == 1) {
	CAE_ConnPointBase* cp = iRef.at(0);
       	res = cp->GetSrcPin(aName);
    }
    return res;
}

void CAE_ConnPointExt::DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin)
{
    for (vector<CAE_ConnPointBase*>::iterator it = iRef.begin(); it != iRef.end(); it++) {
	(*it)->DisconnectPin(aPin, aPair, aPairPin);
    }
}

TBool CAE_ConnPointExt::SetExtended(CAE_ConnPointBase *aConnPoint)
{
    return ETrue;
}

TBool CAE_ConnPointExt::Disextend(CAE_ConnPointBase *aConnPoint)
{
    TBool res = EFalse;
    for (vector<CAE_ConnPointBase*>::iterator it = iRef.begin(); it != iRef.end(); it++) {
	if ((*it) == aConnPoint) { 
	    iRef.erase(it);
	    res = ETrue;
	    break;
	}
    }
    return res;
}

TBool CAE_ConnPointExt::SetDisextended(CAE_ConnPointBase *aConnPoint)
{
    TBool res = EFalse;
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

TBool CAE_ConnPointExtC::Disconnect(CAE_ConnPointBase *aConnPoint)
{
    TBool res = ETrue;
    // Disconnect all the bus pins
    for (map<string, SlotTempl::slot_templ_elem>::iterator id = Templ().Dests().begin(); id != Templ().Dests().end(); id++) {
	DisconnectPin(id->first.c_str(), aConnPoint, id->first.c_str());
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

TBool CAE_ConnPointExtC::SetExtended(CAE_ConnPointBase *aConnPoint)
{
    return ETrue;
}

TBool CAE_ConnPointExtC::SetDisextended(CAE_ConnPointBase *aConnPoint)
{
    _FAP_ASSERT(0);
    return EFalse;
}

void *CAE_ConnPointExtC::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : CAE_ConnPointBase::DoGetFbObj(aName);
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

// Connection base

CAE_ConnBase::CAE_ConnBase(TNodeType aNodeType, CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner): 
    CAE_NBase(aNodeType, aPoint->Name(), aOwner), 
    iPoint(aPoint), iPair(aPair), iIsValid(EFalse)
{
}

CAE_ConnBase::~CAE_ConnBase()
{
    Owner()->OnElemDeleting(this);
}

TBool CAE_ConnBase::IsMetQuery(const DesUri& aUri)
{
    TBool res = ETrue;
    const vector<DesUri::TQueryElem>& qr = aUri.QueryElems();
    for (vector<DesUri::TQueryElem>::const_iterator it = qr.begin(); it != qr.end(); it++) {
	DesUri::TQueryOpr op = it->first;
	TBool lres = EFalse;
	TNodeAttr attr = it->second.first;
	const string& attrname = it->second.second;
	if (attr == ENa_Id) {
	    DesUri puri(iPoint, Owner());
	    lres = attrname.compare(puri.GetUri()) == 0;
	}
	else if (attr == ENa_ConnPair) {
	    DesUri pairuri(iPair, Owner());
	    lres = attrname.compare(pairuri.GetUri()) == 0;
	}
	if (op == DesUri::EQop_Unknown) {
	    res = lres;
	}
	else if (op == DesUri::EQop_And) {
	    res = res && lres;
	}
    }
    return res;
}

// Extention 

CAE_Ext::CAE_Ext(CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner): 
    CAE_ConnBase(ENt_Cext, aPoint, aPair, aOwner) 
{
    iIsValid =  Connect();
}

CAE_Ext::~CAE_Ext()
{
    if (iIsValid) {
	Disconnect();
	iIsValid = EFalse;
    }
}

TBool CAE_Ext::Connect()
{
    _FAP_ASSERT(!iIsValid);
    TBool res = EFalse;
    //res = (iPoint->Extend(iPair) && iPair->SetExtended(iPoint));
    res = (iPoint->Extend(this) && iPair->SetExtended(this));
#if 0
    if (res) {
	iPoint->AddConn(this);
	iPair->AddConn(this);
    }
#endif
    return res;
}

TBool CAE_Ext::Disconnect()
{
    TBool res = EFalse;
    if (iIsValid) {
	res = iPoint->Disextend(this) && iPair->SetDisextended(this);
	iIsValid = EFalse;
    }
    return res;
}

void CAE_Ext::OnCpDelete(CAE_ConnPointBase* aCp)
{
    _FAP_ASSERT(aCp == iPoint || aCp == iPair);
    if (aCp == iPoint) {
	iPair->SetDisextended(this);
    }
    else {
	iPoint->Disextend(this);
    }
    iIsValid = EFalse;
    delete this;
}


// Connection 

CAE_Conn::CAE_Conn(CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner): 
    CAE_ConnBase(ENt_Conn, aPoint, aPair, aOwner) 
{
    iIsValid =  Connect();
}

CAE_Conn::~CAE_Conn()
{
    if (iIsValid) {
	Disconnect();
	iIsValid = EFalse;
    }
}

TBool CAE_Conn::Connect()
{
    _FAP_ASSERT(!iIsValid);
    return (iPoint->Connect(this) && iPair->Connect(this));
}

TBool CAE_Conn::Disconnect()
{
    TBool res = EFalse;
    if (iIsValid) {
	res = iPoint->Disconnect(this) && iPair->Disconnect(this);
	iIsValid = EFalse;
    }
    return res;
}

void CAE_Conn::OnCpDelete(CAE_ConnPointBase* aCp)
{
    _FAP_ASSERT(aCp == iPoint || aCp == iPair);
    if (aCp == iPoint) {
	iPair->Disconnect(this);
    }
    else {
	iPoint->Disconnect(this);
    }
    iIsValid = EFalse;
    delete this;
}



// CAE_EBase

CAE_EBase::CAE_EBase(TNodeType aNodeType, const string& aInstName, CAE_Object* aMan): CAE_NBase(aNodeType, aInstName), 
	iTypeName(NULL), iMan(aMan), iUpdated(ETrue), iActive(ETrue), iQuiet(EFalse)
{
};


void CAE_EBase::SetActive() 
{ 
    if (!iActive) {
	iActive= ETrue; 
	iNotifier.OnActivated(this);
	if (iMan && !iMan->IsActive()) iMan->SetActive();
    }
};

void CAE_EBase::SetUpdated()
{ 
	iUpdated= ETrue; if (iMan) iMan->SetUpdated();
};

void CAE_EBase::SetType(const char *aName)
{
    if (iTypeName != NULL)
	free(iTypeName);
    if (aName != NULL) 
	iTypeName = strdup(aName);
}

FAPWS_API CAE_EBase::~CAE_EBase()
{
    if (iTypeName != NULL)
	free(iTypeName);
    // TODO [YB] To reimplement this hack
    for (map<string, TLogSpecBase*>::reverse_iterator it = iLogSpec.rbegin(); it != iLogSpec.rend(); ) {
	map<string, TLogSpecBase*>::reverse_iterator nit = it;
	delete (it->second);
	it = nit;
    }
    iLogSpec.clear();
}

void CAE_EBase::AddLogSpec(TLeBase aEvent, TInt aData)
{
    TLogSpecBase* logspec = new TLogSpecBase(aEvent, aData, this);
    iLogSpec[logspec->Name()] = logspec;
}

void CAE_EBase::RemLogSpec()
{
	iLogSpec.clear();
}

TInt CAE_EBase::GetLogSpecData(TLeBase aEvent) const
{	
    const string& ename = DesUri::LeventName(aEvent);
    map<string, TLogSpecBase*>::const_iterator it = iLogSpec.find(ename);
    return it != iLogSpec.end() ? it->second->iData: KBaseDa_None;
}

const char* CAE_EBase::MansName(TInt aLevel) const 
{ 
    CAE_EBase *man = iMan; 
    for (TInt i=aLevel; i > 0 && man != NULL && man->iMan != NULL; i--) man = man->iMan;
    return (man == NULL)? "": man->Name().c_str();
}

// From CAE_Base
void *CAE_EBase::DoGetFbObj(const char *aName)
{
    if ((iTypeName != NULL) && (strcmp(iTypeName, aName) == 0) || (strcmp(aName, Type()) == 0))
	return this;
    else
	return NULL;
}

CAE_NBase* CAE_EBase::GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase)
{
    CAE_NBase* res = NULL;
    vector<DesUri::TElem>::const_iterator it = aPathBase;
    DesUri::TElem elem = *it;
    if (elem.first == ENt_Object) {
	if (elem.second.compare("..") == 0) {
	    res = iMan->GetNode(aUri, ++it);
	}
    }
    else if (elem.first == ENt_Logspec) {
	res = iLogSpec.count(elem.second) > 0 ? iLogSpec[elem.second] : NULL;
    }
    if (res != NULL && ++it != aUri.Elems().end()) {
	res = res->GetNode(aUri, it);
    }
    return res;
}

void CAE_EBase::OnElemDeleting(CAE_NBase* aElem)
{
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_Logspec) {
	map<string, TLogSpecBase*>::iterator it = iLogSpec.find(aElem->Name());
	if (it != iLogSpec.end()) {
	    iLogSpec.erase(it); 
	}
    }
}

TBool CAE_EBase::AddNode(const CAE_ChromoNode& aSpec)
{
    TBool res = ETrue;
    TNodeType ntype = aSpec.Type();
    if (ntype == ENt_Logspec) {
	AddLogSpec(aSpec);
    }
    else {
	res = CAE_NBase::AddNode(aSpec);
    }
    return res;
}

void CAE_EBase::AddLogSpec(const CAE_ChromoNode& aSpec)
{
    // Set logspec 
    string aevent = aSpec.Attr(ENa_Logevent);
    TInt event = DesUri::Levent(aevent);
    // Get logging data
    TInt ldata = 0;
    for (CAE_ChromoNode::Const_Iterator lselemit = aSpec.Begin(); lselemit != aSpec.End(); lselemit++)
    {
	CAE_ChromoNode lselem = *lselemit;
	if (lselem.Type() == ENt_Logdata)
	{
	    string adata = lselem.Name();
	    TInt data = DesUri::Ldata(adata);
	    ldata |= data;
	}
	else
	{
	    string aname = lselem.Name();
	    Logger()->WriteFormat("ERROR: Unknown type [%s]", aname.c_str());
	}
    }
    AddLogSpec((TLeBase) event, ldata);
}

// Trans inofo

TTransInfo::TTransInfo(const string& aETrans, CAE_NBase* aOwner): CAE_NBase(ENt_Trans, aOwner), iFun(NULL), iOpInd(0)
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

TBool TTransInfo::ChangeCont(const string& aVal)
{
    TBool res = EFalse;
    iETrans.erase();
    FormatEtrans(aVal, iETrans);
    if (Owner() != NULL) {
	Owner()->OnElemChanging(this);
    }
    res = ETrue;
    return res;
}

// ******************************************************************************
// CAE_StateBase - state base
// ******************************************************************************


CAE_StateBase::CAE_StateBase(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans):
    CAE_EBase(ENt_State, aInstName, aMan), iTrans(aTrans)
{
    _FAP_ASSERT (iMan != NULL);
}


CAE_StateBase::~CAE_StateBase()
{
//    if (iMan != NULL)
//	iMan->UnregisterState(this);
    if (Owner() != NULL)
	Owner()->OnElemDeleting(this);
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

void CAE_StateBase::OnElemDeleting(CAE_NBase* aElem)
{
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_Stinp) {
	map<string, CAE_ConnPointBase*>::iterator it = iInputs.find(aElem->Name());
	_FAP_ASSERT(it != iInputs.end());
	iInputs.erase(it);
    }
    else {
	CAE_EBase::OnElemDeleting(aElem);
    }
}

void CAE_StateBase::OnElemAdding(CAE_NBase* aElem)
{
    _FAP_ASSERT(aElem != NULL);
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_Stinp) {
	CAE_ConnPointBase* elem = aElem->GetFbObj(elem);
	_FAP_ASSERT(elem != NULL && iInputs.count(elem->Name()) == 0);
	iInputs.insert(pair<string, CAE_ConnPointBase*>(elem->Name(), elem));
    }
    else {
	CAE_EBase::OnElemAdding(aElem);
    }
}

void CAE_StateBase::ConstructL()
{
    if (iMan)
	iMan->RegisterCompL(this);
    CAE_ConnSlot::Template dt, st;
    dt["_1"]  = "State";
    st["_1"]  = "State";
    CAE_ConnPoint *outp = new CAE_ConnPoint(ENt_Soutp, "output", this, st, dt); 
    iOutput = outp;
    outp->Srcs()->SetPin("_1", this);
    if (Logger())
	Logger()->WriteFormat("State created:: Name: %s", Name().c_str());
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
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s <= %s", MansName(1), MansName(0), Name().c_str(), buf_new, buf_cur);
    else if (aLogData & KBaseDa_New)
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s", MansName(1), MansName(0), Name().c_str(), buf_new);
    // Loggin inputs
    if (aLogData & KBaseDa_Dep)
    {
	Logger()->WriteFormat("Updated state [%s.%s.%s]: %s <<<<< ", MansName(1), MansName(0), Name().c_str(), buf_new);
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
	Logger()->WriteFormat("Transf state [%s.%s.%s]: ?? <= %s", MansName(1), MansName(0), Name().c_str(), buf_cur);
    // Loggin inputs
    if (aLogData & KBaseDa_Dep)
    {
	Logger()->WriteFormat("Transf state [%s.%s.%s]: ?? <= %s via ", MansName(1), MansName(0), Name().c_str(), buf_cur);
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
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), Name().c_str(), aName);
    }
    else {
	CAE_ConnPoint *inp = it->second->GetFbObj(inp);
	if (inp->Dests().size() > 0 ) {
	    CAE_Base* refb = inp->Slot(0)->Pins().begin()->second;
	    res = (refb != NULL) ? refb->GetFbObj(res) : NULL;
	}
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), Name().c_str(), aName);
	}
    }
    return res;
}

// TODO [YB] Obsolete?
void CAE_StateBase::AddInputL(const char* aName) 
{
    TBool exists = iInputs.find(aName) != iInputs.end();
    if (exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already exists", MansName(1), MansName(0), Name().c_str(), aName);
	_FAP_ASSERT(!exists);
    }
    CAE_ConnSlot::Template dt, st;
    st["_1"]  = "State";
    dt["_1"]  = "State";
    CAE_ConnPoint *cpoint = new CAE_ConnPoint(ENt_Stinp, aName, this, st, dt);
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
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), Name().c_str(), name);
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
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already connected", MansName(1), MansName(0), Name().c_str(), aName);
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

void CAE_StateBase::SetInit(const string& aInit)
{
    iInit = aInit;
}

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

CSL_ExprBase* CAE_StateBase::GetExpr(const string& aTerm, const string& aRtype, MAE_TransContext* aRequestor) 
{ 
    return iMan->GetExpr(aTerm, aRtype);
};

multimap<string, CSL_ExprBase*>::iterator CAE_StateBase::GetExprs(const string& aName, const string& aRtype, multimap<string,
       	CSL_ExprBase*>::iterator& aEnd, MAE_TransContext* aRequestor)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    if (iMan != NULL)
	res = iMan->GetExprs(aName, aRtype, aEnd);
    else {
	_FAP_ASSERT(EFalse);
    }
    return res;
}

void CAE_StateBase::AddInp(const CAE_ChromoNode& aSpec) 
{
    TBool r_dest_spec = EFalse;
    const string sname = aSpec.Name();
    const char *name = sname.c_str();
    if (sname.empty())
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", Name().c_str(), name);
    else {
	if (Inputs().count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), Name().c_str(), name);
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
		const string stype = elem.Attr(ENa_Type);
		const char *type = stype.c_str(); 
		// Create default templ if type isnt specified
		dt[name] = type ? type: "State";
		r_dest_spec = ETrue;
	    }
	}
	if (!r_dest_spec) {
	    dt["_1"] = "State";
	}
	CAE_ConnPoint *cpoint = new CAE_ConnPoint(ENt_Stinp, name, this, st, dt);
	cpoint->Srcs()->SetPin("_1", this);
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = Inputs().insert(pair<string, CAE_ConnPointBase*>(name, cpoint));
    }
}

CAE_NBase* CAE_StateBase::GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase)
{
    CAE_NBase* res = NULL;
    vector<DesUri::TElem>::const_iterator it = aPathBase;
    DesUri::TElem elem = *it;
    if (elem.first == ENt_Object) {
	if (elem.second.compare("..") == 0) {
	    res = iMan->GetNode(aUri, ++it);
	}
    }
    else if (elem.first == ENt_Stinp) {
	res = iInputs.count(elem.second) > 0 ? iInputs[elem.second] : NULL;
    }
    else if (elem.first == ENt_Soutp) {
	res = iOutput;
    }
    else if (elem.first == ENt_Trans) {
	res = &iTrans;
    }
    else  {
	res = CAE_EBase::GetNode(aUri, aPathBase);
    }
    if (res != NULL && ++it != aUri.Elems().end()) {
	res = res->GetNode(aUri, it);
    }
    return res;
}

void CAE_StateBase::RmNode(const DesUri& aUri)
{
}

TBool CAE_StateBase::ChangeAttr(TNodeAttr aAttr, const string& aVal) 
{ 
    TBool res = EFalse;
    if (aAttr == ENa_StInit) {
	SetInit(aVal);
	// [YB] TODO To consider if it is correct to set value on Init mutation
	SetFromStr(aVal.c_str());
	Confirm();
	res = ETrue;
    }
    else {
	res = CAE_EBase::ChangeAttr(aAttr, aVal);
    }
    return res;
};

TBool CAE_StateBase::AddNode(const CAE_ChromoNode& aSpec)
{
    TBool res = ETrue;
    TNodeType ntype = aSpec.Type();
    if (ntype == ENt_Stinp) {
	AddInp(aSpec);
    }
    else {
	res = CAE_EBase::AddNode(aSpec);
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

void CAE_ChromoNode::ParseTname(const string& aTname, TNodeType& aType, string& aName)
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

CAE_ChromoNode::Iterator CAE_ChromoNode::Parent()
{
    void* parent = iMdl.Parent(iHandle);
    return  (parent == NULL) ?  End() : Iterator(CAE_ChromoNode(iMdl, parent));
}

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(const string& aName) 
{ 
    Iterator res = End();
    TNodeType type = ENt_Unknown;
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
CAE_ChromoNode::Iterator CAE_ChromoNode::Find(TNodeType aType, const string& aName) 
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

CAE_ChromoNode::Const_Iterator CAE_ChromoNode::Find(TNodeType aType, const string& aName) const
{ 
    CAE_ChromoNode::Const_Iterator res = End();
    for (CAE_ChromoNode::Const_Iterator it = Begin(); it != End(); it++) {
	if (((*it).Type() == aType) && (aName.compare((*it).Name()) == 0)) {
	    res = it;  break;
	}
    }
    return res;
};

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(TNodeType aType, TNodeAttr aAttr, const string& aAttrVal, TBool aPathVal)
{
    CAE_ChromoNode::Iterator res = End();
    for (CAE_ChromoNode::Iterator it = Begin(); it != End(); it++) {
	if (((*it).Type() == aType) && (*it).AttrExists(aAttr)) {
	    if (aPathVal && MAE_Chromo::ComparePath(aAttrVal, (*it).Attr(aAttr)) || !aPathVal && (aAttrVal.compare((*it).Attr(aAttr)) == 0)) {
		res = it;  break;
	    }
	}
    }
    return res;
};

CAE_ChromoNode::Iterator CAE_ChromoNode::Find(TNodeType aType, const string& aName, TNodeAttr aAttr, const string& aAttrVal)
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
    CAE_EBase(ENt_Object, aInstName, aMan), iEnv(aEnv), iMut(NULL), iChromo(NULL), iChromoIface(*this),
    iCtrl(*this), iOpv(NULL), iTransSrc(this)
{
    if (iMan != NULL) 
	iMan->RegisterCompL(this);
}

void *CAE_Object::DoGetFbObj(const char *aName)
{
    void *res = NULL;
    if (strcmp(aName, Type()) == 0) {
	res = this;
    }
    if (strcmp(aName, MAE_TransContext::Type()) == 0) {
	res = (MAE_TransContext*) this;
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

void  CAE_Object::RegisterCompL(CAE_EBase* aComp) 
{ 
    CAE_Object* comp = aComp->GetFbObj(comp);
    if (comp != NULL) {
	_FAP_ASSERT(iComps.count(aComp->Name()) == 0);
	iComps.insert(pair<string, CAE_Object*>(comp->Name(), comp));
	iCompsOrd.push_back(comp);
    }
    else {
	CAE_StateBase* state = aComp->GetFbObj(state);
	_FAP_ASSERT(state != NULL);
	_FAP_ASSERT(iStates.count(aComp->Name()) == 0);
	iStates.insert(pair<string, CAE_StateBase*>(state->Name(), state));
    }
    aComp->SetMan(this);
    if (aComp->IsUpdated()) SetUpdated();
};

void CAE_Object::Deactivate()
{
    ResetActive();
    for (map<string, CAE_StateBase*>::iterator it = iStates.begin(); it != iStates.end(); it++) {
	it->second->ResetActive();
    }
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++) {
	it->second->Deactivate();
    }
}

void CAE_Object::Activate()
{
    SetActive();
    for (map<string, CAE_StateBase*>::iterator it = iStates.begin(); it != iStates.end(); it++) {
	it->second->SetActive();
    }
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++) {
	it->second->Activate();
    }
}

void CAE_Object::CreateStateInp(const CAE_ChromoNode& aSpec, CAE_StateBase *aState) 
{
    TBool r_dest_spec = EFalse;
    const string sname = aSpec.Name();
    const char *name = sname.c_str();
    if (sname.empty())
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", Name().c_str(), name);
    else {
	if (aState->Inputs().count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), Name().c_str(), name);
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
		const string stype = elem.Attr(ENa_Type);
		const char *type = stype.c_str(); 
		// Create default templ if type isnt specified
		dt[name] = type ? type: "State";
		r_dest_spec = ETrue;
	    }
	}
	if (!r_dest_spec) {
	    dt["_1"] = "State";
	}
	CAE_ConnPoint *cpoint = new CAE_ConnPoint(ENt_Stinp, name, aState, st, dt);
	cpoint->Srcs()->SetPin("_1", aState);
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aState->Inputs().insert(pair<string, CAE_ConnPointBase*>(name, cpoint));
    }
}


void CAE_Object::CreateConn(void* aSpecNode, map<string, CAE_ConnPointBase*>& aConns, TBool aInp) 
{
    MAE_ChroMan *chman = iEnv->Chman();
    char *name = chman->GetStrAttr(aSpecNode, KXStateInpAttr_Id);
    if (name == NULL)
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", Name().c_str(), name);
    else {
	if (aConns.count(name) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(1), MansName(0), Name().c_str(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnPointBase* cp = NULL;
	if (chman->GetChild(aSpecNode) != NULL) {
	    // There are pins spec - Commutating extender
	    CAE_ConnPointExtC *cpoint = new CAE_ConnPointExtC(aInp? ENt_Stinp: ENt_Soutp, name, this);
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
	    cp = new CAE_ConnPointExt(aInp? ENt_Stinp: ENt_Soutp, name, this);
	}
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aConns.insert(pair<string, CAE_ConnPointBase*>(name, cp));
    }
}

void CAE_Object::AddConn(const CAE_ChromoNode& aSpecNode, map<string, CAE_ConnPointBase*>& aConns, TBool aInp) 
{
    const string sname = aSpecNode.Name();
    const char* name = sname.c_str();
    if (sname.empty())
	Logger()->WriteFormat("ERROR: Creating conn [%s,%s]: empty name", Name().c_str(), name);
    else {
	if (aConns.count(sname) > 0) {
	    Logger()->WriteFormat("ERROR: Conn [%s.%s.%s] already exists", MansName(0), Name().c_str(), name);
	    _FAP_ASSERT(EFalse);
	}
	CAE_ConnPointBase* cp = NULL;
	if ((aSpecNode.Find(ENt_CpSource) != aSpecNode.End()) || (aSpecNode.Find(ENt_CpDest) != aSpecNode.End()))  {
	    // There are pins spec - Commutating extender
	    CAE_ConnPointExtC *cpoint = new CAE_ConnPointExtC(aInp? ENt_Stinp: ENt_Soutp, sname, this);
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
	    cp = new CAE_ConnPointExt(aInp? ENt_Stinp: ENt_Soutp, sname, this);
	}
	pair<map<string, CAE_ConnPointBase*>::iterator, bool> res = aConns.insert(pair<string, CAE_ConnPointBase*>(sname, cp));
    }
}

FAPWS_API CAE_Object::~CAE_Object()
{
    if (iMan != NULL)
	iMan->UnregisterComp(this);
    // Use reverse because the nodes request unregistering within destructors
    for (map<string, CAE_StateBase*>::reverse_iterator it = iStates.rbegin(); it != iStates.rend(); ) {
	map<string, CAE_StateBase*>::reverse_iterator nit = it;
	delete it->second;
	it = nit;
    }
    iStates.clear();
    for (map<string, CAE_Object*>::reverse_iterator it = iComps.rbegin(); it != iComps.rend(); ) {
	map<string, CAE_Object*>::reverse_iterator nit = it;
	delete it->second;
	it = nit;
    }
    iComps.clear();
    iCompsOrd.clear();
    iEnv = NULL; // Not owned

    if (iMut != NULL) {
	delete iMut;
	iMut = NULL;
    }
    if (iChromo != NULL) {
	delete iChromo;
	iChromo = NULL;
    }
};

void CAE_Object::OnElemDeleting(CAE_NBase* aElem)
{
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_Cext) {
	for (vector<CAE_Ext*>::iterator it = iExts.begin(); it != iExts.end(); it++) {
	    if (*it == aElem) {
		iExts.erase(it); break;
	    }
	}
    }
    else if (nt == ENt_Conn) {
	for (vector<CAE_Conn*>::iterator it = iConns.begin(); it != iConns.end(); it++) {
	    if (*it == aElem) {
		iConns.erase(it); break;
	    }
	}
    }
    else if (nt == ENt_State) {
	map<string, CAE_StateBase*>::iterator it = iStates.find(aElem->Name());
	_FAP_ASSERT(it != iStates.end());
	iStates.erase(it);
    }
    else if (nt == ENt_Object) {
	map<string, CAE_Object*>::iterator it = iComps.find(aElem->Name());
	_FAP_ASSERT(it != iComps.end());
	iComps.erase(it);
    }
    else if (nt == ENt_Stinp) {
	map<string, CAE_ConnPointBase*>::iterator it = iInputs.find(aElem->Name());
	_FAP_ASSERT(it != iInputs.end());
	iInputs.erase(it);
    }
    else if (nt == ENt_Soutp) {
	map<string, CAE_ConnPointBase*>::iterator it = iOutputs.find(aElem->Name());
	_FAP_ASSERT(it != iOutputs.end());
	iOutputs.erase(it);
    }
    else {
	CAE_NBase::OnElemDeleting(aElem);
    }
}

void CAE_Object::OnElemAdding(CAE_NBase* aElem)
{
    _FAP_ASSERT(aElem != NULL);
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_State) {
	CAE_StateBase* elem = aElem->GetFbObj(elem);
	_FAP_ASSERT(elem != NULL && iStates.count(elem->Name()) == 0);
	iStates.insert(pair<string, CAE_StateBase*>(elem->Name(), elem));
    }
    else if (nt == ENt_Object) {
	CAE_Object* elem = aElem->GetFbObj(elem);
	_FAP_ASSERT(elem != NULL && iComps.count(elem->Name()) == 0);
	iComps.insert(pair<string, CAE_Object*>(elem->Name(), elem));
    }
    else if (nt == ENt_Stinp) {
	CAE_ConnPointBase* elem = aElem->GetFbObj(elem);
	_FAP_ASSERT(elem != NULL && iInputs.count(elem->Name()) == 0);
	iInputs.insert(pair<string, CAE_ConnPointBase*>(elem->Name(), elem));
    }
    else if (nt == ENt_Soutp) {
	CAE_ConnPointBase* elem = aElem->GetFbObj(elem);
	_FAP_ASSERT(elem != NULL && iOutputs.count(elem->Name()) == 0);
	iOutputs.insert(pair<string, CAE_ConnPointBase*>(elem->Name(), elem));
    }
    else {
	CAE_NBase::OnElemAdding(aElem);
    }
}

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
		Logger()->WriteFormat("Updated object [%s.%s.%s]:", MansName(1), MansName(0), Name().c_str());
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
CAE_Object* CAE_Object::AddObject(const CAE_ChromoNode& aNode)
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
	CAE_Object *parent = NULL;
	// Check the scheme
	string scheme;
	CAE_ChromoBase::GetUriScheme(sparent, scheme);
	TBool ext_parent = ETrue;
	string sparfull = sparent;
	if (scheme.empty()) {
	    // FAP scheme by default
	    parent = GetComp(sparent);
	    ext_parent = EFalse;
	}
	else {
	    // TODO [YB] To add seaching the module - it will allow to specify just file of spec wo full uri
	    CAE_ChromoBase *spec = iEnv->Provider()->CreateChromo();
	    TBool res = spec->Set(sparent);
	    if (res) {
		const CAE_ChromoNode& root = spec->Root();
		parent = AddObject(root);
		delete spec;
		// TODO [YB] Hack of adding root. Migrate to disabling implicit frag
		string frag;
		CAE_ChromoBase::GetFrag(sparent, frag);
		if (frag.empty()) {
		    sparfull.append(string("#") + parent->Name());
		}
	    }
	}
	if (parent == NULL) {
	    Logger()->WriteFormat("ERROR: Creating object [%s] - parent [%s] not found", sname.c_str(), sparent.c_str());
	}
	else {
	    // Create heir from the parent
	    obj = parent->CreateHeir(sname.c_str(), this);
	    obj->SetType(sparfull.c_str());
	    // TODO [YB] Seems to be just temporal solution. To consider using context instead.
	    CAE_ChromoNode hroot = obj->Chromo().Root();
	    hroot.SetAttr(ENa_Type, sparfull);
	    // Remove external parent from system
	    if (ext_parent) {
		delete parent;
	    }
	}
    }
    if (obj == NULL) {
	Logger()->WriteFormat("ERROR: Creating object [%s] - failed", sname.c_str());
    }
    else {
	obj->SetQuiet(squiet);
	if (squiet) {
	    CAE_ChromoNode hroot = obj->Chromo().Root();
	    hroot.SetAttr(ENa_ObjQuiet, "yes");
	    Logger()->WriteFormat("ATTENTION: Object [%s] created as quiet", sname.c_str());
	}
	// Mutate object 
	obj->SetMutation(aNode);
	obj->Mutate();
    }
    return obj;
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
	    Logger()->WriteFormat("ERROR: Transition [%s] not found", transf_name);
	_FAP_ASSERT(trans != NULL);
    }
    string ainit = aSpec.Attr(ENa_StInit);
    const char *init = ainit.c_str();
    if (iStates.count(name) == 0) {
	state = prov->CreateStateL(stype, name, this);  
    }
    else {
	Logger()->WriteFormat("ERROR: Creating state [%s]: state with such name already exists", name);
    }
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
		state->AddLogSpec((TLeBase) event, ldata);
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
	    state->iInit = ainit;
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

void CAE_Object::AddConn_v1(const CAE_ChromoNode& aSpec)
{
    string sname = aSpec.Name();
    string spair = aSpec.Attr(ENa_ConnPair);
    const char *p1_name = sname.c_str(); 
    const char *p2_name = spair.c_str(); 
    CAE_NBase* np1 = CAE_NBase::GetNode(sname);
    CAE_NBase* np2 = CAE_NBase::GetNode(spair);
    if ((np1 != NULL) && (np2 != NULL))
    {
	CAE_ConnPointBase* p1 = np1->GetFbObj(p1);
	CAE_ConnPointBase* p2 = np2->GetFbObj(p2);
	if ((p1 != NULL) && (p2 != NULL)) {
	    CAE_Conn* conn = new CAE_Conn(p1, p2, this);
	    if (conn != NULL && conn->IsValid()) {
		iConns.push_back(conn);
	    }
	    else {
		Logger()->WriteFormat("ERROR: Connecting [%s] <- [%s]: failure", p1_name, p2_name);
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Connecting [%s] <- [%s]: point [%s] not found", p1_name, p2_name, (p1 == NULL) ? p1_name: p2_name);
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Connecting [%s] <- [%s]: node [%s] not found", p1_name, p2_name, (np1 == NULL) ? p1_name: p2_name);
    }
}

void CAE_Object::AddTrans(const CAE_ChromoNode& aSpec)
{
    CAE_ChromoNode::Const_Iterator iconnode = aSpec.FindText();
    SetTrans((*iconnode).Content());
    //iEnv->Tranex()->EvalTrans(this, this, iTransSrc);
    //iTrans = iEnv->Tranex()->Exprs();
}

// TODO [YB] To migrate to node base class method "RmNode"
void CAE_Object::RmNode(const DesUri& aUri)
{
    TNodeType type = aUri.GetType();
    string name = aUri.GetName();
    string suri = aUri.GetUri();
    CAE_NBase* node = CAE_NBase::GetNode(aUri);
    if (node != NULL) {
	delete node;
    }
    else {
	Logger()->WriteFormat("ERROR: Deleting [%s] - cannot find node", suri.c_str());
    }
}

void CAE_Object::MutAddNode(const CAE_ChromoNode& aSpec)
{
    TBool res = ETrue;
    string snode = aSpec.Attr(ENa_MutNode);
    DesUri unode(snode);
    CAE_NBase* node = CAE_NBase::GetNode(unode);
    if (node != NULL) {
	for (CAE_ChromoNode::Const_Iterator mit = aSpec.Begin(); mit != aSpec.End() && res; mit++)
	{
	    const CAE_ChromoNode& mno = (*mit);
	    res = node->AddNode(mno);
	}
	if (!res) {
	    Logger()->WriteFormat("ERROR: Adding node into [%s] - failure", snode.c_str());
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Adding node: cannot find [%s]", snode.c_str());
    }
}

void CAE_Object::ChangeAttr_v2(const CAE_ChromoNode& aSpec)
{
    string snode = aSpec.Attr(ENa_MutNode);
    string mattrs = aSpec.Attr(ENa_MutChgAttr);
    string mval = aSpec.Attr(ENa_MutChgVal);
    DesUri unode(snode);
    CAE_NBase* node = CAE_NBase::GetNode(unode);
    if (node != NULL) {
	TBool res = node->ChangeAttr(DesUri::NodeAttr(mattrs), mval);
	if (!res) {
	    Logger()->WriteFormat("ERROR: Changing [%s] - failure", snode.c_str());
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Changing [%s] - cannot find node", snode.c_str());
    }
}

void CAE_Object::ChangeCont_v2(const CAE_ChromoNode& aSpec)
{
    string snode = aSpec.Attr(ENa_MutNode);
    string mval = aSpec.Attr(ENa_MutChgVal);
    DesUri unode(snode);
    CAE_NBase* node = CAE_NBase::GetNode(unode);
    if (node != NULL) {
	TBool res = node->ChangeCont(mval);
	if (!res) {
	    Logger()->WriteFormat("ERROR: Changing [%s] - failure", snode.c_str());
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Changing [%s] - cannot find node", snode.c_str());
    }
}

void CAE_Object::AddExt_v1(const CAE_ChromoNode& aSpec)
{
    string sname = aSpec.Name();
    string spair = aSpec.Attr(ENa_ConnPair);
    const char *p1_name = sname.c_str(); 
    const char *p2_name = spair.c_str(); 
    CAE_NBase* np1 = CAE_NBase::GetNode(sname);
    CAE_NBase* np2 = CAE_NBase::GetNode(spair);
    if ((np1 != NULL) && (np2 != NULL))
    {
	CAE_ConnPointBase* p1 = np1->GetFbObj(p1);
	CAE_ConnPointBase* p2 = np2->GetFbObj(p2);
	if ((p1 != NULL) && (p2 != NULL)) {
	    CAE_Ext* ext = new CAE_Ext(p1, p2, this);
	    if (ext != NULL && ext->IsValid()) {
		iExts.push_back(ext);
	    }
	    else {
		Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: failure", p1_name, p2_name);
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: point [%s] not found", p1_name, p2_name, (p1 == NULL) ? p1_name: p2_name);
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Extending [%s] <- [%s]: node [%s] not found", p1_name, p2_name, (np1 == NULL) ? p1_name: p2_name);
    }
}

void CAE_Object::AddExtc_v1(const CAE_ChromoNode& aSpec)
{
    string stexname = aSpec.Name();
    const char *exname = stexname.c_str(); 
    if (!stexname.empty())
    {
	CAE_NBase* epn = CAE_NBase::GetNode(stexname);
	CAE_ConnPointExtC* ep = epn != NULL? epn->GetFbObj(ep): NULL;
	if (ep != NULL) {
	    // Create slog
	    CAE_ConnPointExtC::Slot slot(ep->Templ());
	    // Go thru custom extention elements
	    for (CAE_ChromoNode::Const_Iterator elemit = aSpec.Begin(); elemit != aSpec.End(); elemit++)
	    {
		CAE_ChromoNode elem = *elemit;
		TNodeType etype = elem.Type();
		if (etype == ENt_CextcSrc)
		{
		    string spin_name = elem.Name();
		    const char *pin_name = spin_name.c_str(); 
		    string spair_name = elem.Attr(ENa_ConnPair);
		    const char *pair_name = spair_name.c_str(); 
		    CAE_NBase* pairn = CAE_NBase::GetNode(spair_name);
		    CAE_ConnPointBase* pairb = pairn != NULL ? pairn->GetFbObj(pairb): NULL;
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
		    CAE_NBase* pairn = CAE_NBase::GetNode(spair_name);
		    CAE_ConnPointBase* pairb = pairn != NULL ? pairn->GetFbObj(pairb): NULL;
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

void CAE_Object::ChangeChromoCont(const CAE_ChromoNode& aSpec, CAE_ChromoNode& aCurr)
{
    string mval = aSpec.Attr(ENa_MutChgVal);
    aCurr.SetContent(mval);
}

TBool CAE_Object::MergeMutation(const CAE_ChromoNode& aSpec)
{
    TBool res = EFalse;
    CAE_ChromoNode& chrroot = iChromo->Root();
    TNodeType rnotype = aSpec.Type();
    if (rnotype == ENt_MutMove) {
	res = MergeMutMove(aSpec);
    }
    else {
	chrroot.AddChild(aSpec);
	res = ETrue;
    }
    return res;
}

TBool CAE_Object::MergeMutMove(const CAE_ChromoNode& aSpec)
{
    TBool res = EFalse;
    DesUri src(aSpec.Attr(ENa_Id));
    TNodeType srctype = src.Elems().at(0).first;
    string srcname = src.Elems().at(0).second;
    DesUri dest(aSpec.Attr(ENa_MutNode));
    string destname = dest.Elems().at(0).second;
    if (srctype == ENt_Object) {
	CAE_ChromoNode& croot = iChromo->Root();
	// Find the dest and src
	CAE_ChromoNode::Iterator nidest = croot.Find(ENt_Object, destname);
	CAE_ChromoNode::Iterator nisrc = croot.Find(ENt_Object, srcname);
	// Move node
	CAE_ChromoNode nsrc = *nisrc;
	nsrc.MoveNextTo(nidest);
	res = ETrue;
    }
    else {
	Logger()->WriteFormat("ERROR: Moving element [%s] - unsupported element type for moving", srcname.c_str());
    }
    return res;
}

void CAE_Object::MoveElem_v1(const CAE_ChromoNode& aSpec)
{
    string srcs = aSpec.Attr(ENa_Id);
    TBool dest_spec = aSpec.AttrExists(ENa_MutNode);
    string dests = dest_spec ? aSpec.Attr(ENa_MutNode): string();
    CAE_NBase* snode = CAE_NBase::GetNode(srcs);
    CAE_NBase* dnode = dest_spec ? CAE_NBase::GetNode(dests) : NULL;
    CAE_NBase* owner = snode->Owner();
    if (snode != NULL && (!dest_spec || dnode != NULL)) {
	if (owner != NULL && owner == dnode->Owner()) {
	    DesUri suri(snode, owner);
	    DesUri duri(dnode, owner);
	    TBool res = owner->MoveNode(suri, duri);
	    if (!res) {
		Logger()->WriteFormat("ERROR: Moving element [%s] - failure", srcs.c_str());
	    }
	}
	else {
	    Logger()->WriteFormat("ERROR: Moving element [%s] - node not owned", srcs.c_str());
	}
    }
    else {
	Logger()->WriteFormat("ERROR: Moving element [%s] - node not found", srcs.c_str());
    }
}

void CAE_Object::Mutate(TBool aRunTimeOnly)
{
    CAE_ChromoNode& root = iMut->Root();
    DoMutation_v1(root, aRunTimeOnly);
    // Clear mutation
    for (CAE_ChromoNode::Iterator mit = root.Begin(); mit != root.End();)
    {
	CAE_ChromoNode node = *mit;
	mit++; // It is required because removing node by iterator breakes iterator itself
	root.RmChild(node);
    }
    // Activate
    SetActive();
}

void CAE_Object::AddLogspec(CAE_EBase* aNode, const CAE_ChromoNode& aSpec)
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
    aNode->AddLogSpec((TLeBase) event, ldata);
}

void CAE_Object::DoMutation_v1(CAE_ChromoNode& aMutSpec, TBool aRunTime)
{
    CAE_ChromoNode& mroot = aMutSpec;
    CAE_ChromoNode& chrroot = iChromo->Root();
    for (CAE_ChromoNode::Iterator rit = mroot.Begin(); rit != mroot.End(); rit++)
    {
	CAE_ChromoNode rno = (*rit);
	TNodeType rnotype = rno.Type();
	if (rnotype == ENt_Object) {
	    AddObject(rno);
	    if (!aRunTime) {
		// Attach comp chromo
		if (iComps.count(rno.Name()) > 0) {
		    CAE_Object* comp = iComps[rno.Name()];
		    chrroot.AddChild(comp->iChromo->Root(), EFalse);
		}
	    }
	}
	else {
	    if (rnotype == ENt_Stinp) {
		AddConn(rno, iInputs, ETrue);
	    }
	    else if (rnotype == ENt_Soutp) {
		AddConn(rno, iOutputs, EFalse);
	    }
	    else if (rnotype == ENt_State) {
		AddState(rno);
	    }
	    else if (rnotype == ENt_Conn) {
		AddConn_v1(rno);
	    }
	    else if (rnotype == ENt_Cext) {
		AddExt_v1(rno);
	    }
	    else if (rnotype == ENt_Cextc) {
		AddExtc_v1(rno);
	    }
	    else if (rnotype == ENt_Trans) {
		AddTrans(rno);
	    }
	    else if (rnotype == ENt_Logspec) {
		AddLogspec(this, rno);
	    }
	    else if (rnotype == ENt_MutAdd) {
		MutAddNode(rno);
	    }
	    else if (rnotype == ENt_Rm) {
		string snode = rno.Attr(ENa_MutNode);
		DesUri unode(snode);
		RmNode(unode);
	    }
	    else if (rnotype == ENt_Change) {
		ChangeAttr_v2(rno);
	    }
	    else if (rnotype == ENt_MutChangeCont) {
		ChangeCont_v2(rno);
	    }
	    else if (rnotype == ENt_MutMove) 
	    {
		MoveElem_v1(rno);
	    }
	    else {
		Logger()->WriteFormat("ERROR: Mutating object [%s] - unknown mutation type [%d]", Name().c_str(), rnotype);
	    }
	    if (!aRunTime) {
		MergeMutation(rno);
	    }
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

void CAE_Object::UnregisterComp(CAE_Object* aComp)
{
    map<string, CAE_Object*>::iterator it = iComps.find(aComp->Name());
    _FAP_ASSERT(it != iComps.end());
    for (vector<CAE_Object*>::iterator oit = iCompsOrd.begin(); oit != iCompsOrd.end(); oit++) {
	if (*oit == it->second) {
	    iCompsOrd.erase(oit); break;
	}	
    }
    iComps.erase(it);
}

CAE_Object* CAE_Object::GetComp(const char* aName, CAE_Object* aRequestor)
{
    // Search local
    CAE_Object* res = iComps.count(aName) > 0 ? iComps[aName] : NULL;
    // Then down
    if (res == NULL && aRequestor != NULL) {
	for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end() && res == NULL; it++) {
	    CAE_Object* comp = it->second;
	    if (comp != aRequestor) {
		res = comp->GetComp(aName, this);
	    }
	}
    }
    // Then up
    if (res == NULL && iMan != aRequestor) {
	res = iMan->GetComp(aName, this);
    }
    return res;
}

void CAE_Object::AppendCompList(vector<string>& aList, const CAE_Object* aRequestor) const
{
    // Get local
    for (map<string, CAE_Object*>::const_iterator it = iComps.begin(); it != iComps.end(); it++) 
    {
	aList.push_back(it->first);
    }
    // Then down
    for (map<string, CAE_Object*>::const_iterator it = iComps.begin(); it != iComps.end(); it++) {
	CAE_Object* comp = it->second;
	if (comp != aRequestor) {
	    comp->AppendCompList(aList, this);
	}
    }
    // Then up
    if (iMan != NULL && iMan != aRequestor) {
	iMan->AppendCompList(aList, this);
    }
}

// TODO [YB] To move to NBase
CAE_NBase* CAE_Object::GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase)
{
    CAE_NBase* res = NULL;
    vector<DesUri::TElem>::const_iterator it = aPathBase;
    DesUri::TElem elem = *it;
    if (elem.first == ENt_Object) {
	if (elem.second.compare("..") == 0) {
	    res = iMan->GetNode(aUri, ++it);
	}
	else {
	    res = iComps.count(elem.second) > 0 ? iComps[elem.second] : NULL;
	}
    }
    else if (elem.first == ENt_State) {
	res = iStates.count(elem.second) > 0 ? iStates[elem.second] : NULL;
    }
    else if (elem.first == ENt_Stinp) {
	res = iInputs.count(elem.second) > 0 ? iInputs[elem.second] : NULL;
    }
    else if (elem.first == ENt_Soutp) {
	res = iOutputs.count(elem.second) > 0 ? iOutputs[elem.second] : NULL;
    }
    else if (elem.first == ENt_Cext) {
	for (vector<CAE_Ext*>::const_iterator cit = iExts.begin(); cit != iExts.end(); cit++) {
	    if ((*cit)->IsMetQuery(aUri)) {
		res = *cit; break;
	    }
	}
    }
    else if (elem.first == ENt_Conn) {
	for (vector<CAE_Conn*>::const_iterator cit = iConns.begin(); cit != iConns.end(); cit++) {
	    if ((*cit)->IsMetQuery(aUri)) {
		res = *cit; break;
	    }
	}
    }
    else if (elem.first == ENt_Trans) {
	res = &iTransSrc;
    }
    else  {
	res = CAE_EBase::GetNode(aUri, aPathBase);
    }
    if (res != NULL && ++it != aUri.Elems().end()) {
	res = res->GetNode(aUri, it);
    }
    return res;
}

CAE_Object* CAE_Object::GetComp(const string& aUri)
{
    CAE_Object* res = this;
    DesUri uri(aUri);
    vector<DesUri::TElem>::const_iterator it = uri.Elems().begin();
    DesUri::TElem elem = *it;
    if (elem.first == ENt_Object) {
	if (uri.Elems().size() == 1) {
	    // Simple name, search the comp
	    res = GetComp(elem.second.c_str(), this);
	}
	else {
	    // Path
	    if (elem.second.compare("..") == 0) {
		string nuri = uri.GetUri(++it);
		res = iMan->GetComp(nuri);
	    }
	    else {
		res = iComps[elem.second];
		if (res != NULL && ++it != uri.Elems().end()) {
		    res = res->GetComp(uri.GetUri(it));
		}
	    }
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
    TNodeType type = ENt_Unknown;
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

// Variant for delta chromo form
// TODO [YB] It will not work with grandparent. To support.
CAE_Object* CAE_Object::CreateHeir(const char *aName, CAE_Object *aMan)
{
    CAE_Object* heir = new CAE_Object(aName, aMan, iEnv);
    heir->SetType(Name().c_str());
    heir->Construct();
    // Set parent
    // TODO [YB] The context is missed here, just name set. To consider, ref discussion in md#sec_desg_chromo_full
    CAE_ChromoNode hroot = heir->Chromo().Root();
    hroot.SetAttr(ENa_Type, Name());
    // Mutate bare child with original parent chromo, mutate run-time only to have clean heir's chromo
    CAE_ChromoNode root = iChromo->Root();
    heir->SetMutation(root);
    heir->Mutate(ETrue);
    return heir;
}

CAE_StateBase* CAE_Object::GetOutpState(const char* aName)
{
    CAE_StateBase *res = NULL;
    map<string, CAE_ConnPointBase*>::iterator it = iOutputs.find(aName);
    if (it == iOutputs.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), Name().c_str(), aName);
    }
    else {
	CAE_ConnPoint *cp = it->second->GetFbObj(cp);
	res = cp->Srcs()->Pins().begin()->second->GetFbObj(res);
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), Name().c_str(), aName);
	}
    }
    return res;
}

CAE_StateBase* CAE_Object::GetInpState(const char* aName)
{
    CAE_StateBase *res = NULL;
    map<string, CAE_ConnPointBase*>::iterator it = iInputs.find(aName);
    if (it == iInputs.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), Name().c_str(), aName);
    }
    else {
	CAE_ConnPoint *cp = it->second->GetFbObj(cp);
	res = cp->Srcs()->Pins().begin()->second->GetFbObj(res);
	if (res == NULL) {
	    Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), Name().c_str(), aName);
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

CSL_ExprBase* CAE_Object::GetExpr(const string& aName, const string& aRtype, MAE_TransContext* aRequestor)
{
    CSL_ExprBase* res = NULL;
    // Look at self first
    string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
    multimap<string, CSL_ExprBase*>::iterator it = iTrans.find(key);
    if (it != iTrans.end()) {
	res = it->second;
    }
    // Then look at the self components
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end() && res == NULL; it++) {
	MAE_TransContext* comp = it->second->GetFbObj(comp);
	if (comp != aRequestor) {
	    res = comp->GetExpr(aName, aRtype, this);
	}
    }	
    // Finally asking man
    if (res == NULL && iMan != NULL) {
	MAE_TransContext* man = iMan->GetFbObj(man);
	if (man != aRequestor) {
	    res = man->GetExpr(aName, aRtype, this);
	}
    }
    return res;
}

multimap<string, CSL_ExprBase*>::iterator CAE_Object::GetExprs(const string& aName, const string& aRtype,
	multimap<string, CSL_ExprBase*>::iterator& aEnd, MAE_TransContext* aRequestor)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    // Look at emb terms first then for current
    string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
    res = iTrans.find(key); aEnd = iTrans.end();
    if (res == aEnd) {
	// Then look at the self components
	for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end() && res == aEnd; it++) {
	    MAE_TransContext* comp = it->second->GetFbObj(comp);
	    if (comp != aRequestor) {
		res = comp->GetExprs(aName, aRtype, aEnd, this);
	    }
	}	
	if (res == aEnd && iMan != NULL) {
	    MAE_TransContext* man = iMan->GetFbObj(man);
	    if (man != aRequestor) {
		res = man->GetExprs(aName, aRtype, aEnd, this);
	    }
	}
    }
    return res;
}

CSL_ExprBase* CAE_Object::CreateDataExpr(const string& aType)
{
    return iEnv->Tranex()->GetExpr(aType, "");
}

void CAE_Object::SetTrans(const string& aTrans)
{
    iTransSrc.iETrans.erase();
    TTransInfo::FormatEtrans(aTrans, iTransSrc.iETrans);
    iEnv->Tranex()->EvalTrans(this, this, iTransSrc.iETrans);
    iTrans = iEnv->Tranex()->Exprs();
}

void CAE_Object::SetBaseViewProxy(MAE_Opv* aProxy, TBool aAsRoot)
{
    _FAP_ASSERT (iOpv == 0);
    iOpv = aProxy;
}

void CAE_Object::RemoveBaseViewProxy(MAE_Opv* aProxy)
{
    _FAP_ASSERT (iOpv == aProxy);
    iOpv = NULL;
}

void CAE_Object::SetEbaseObsRec(CAE_Base* aObs)
{
    Notifier().SetObserver(aObs);
    for (map<string, CAE_Object*>::iterator it = iComps.begin(); it != iComps.end(); it++) {
	it->second->SetEbaseObsRec(aObs);
    }
}

// This method find the mgr that can be mutated to change the current node
// This is because for delta chromo the current node can belong to parent, so be "deattached" from
// heir's chromo. Ref to md#sec_desg_mut_exnode for more info
CAE_Object* CAE_Object::FindMutableMangr()
{
    CAE_Object* res = FindDeattachedMangr();
    if (res == NULL) {
	res = this;
    }
    return res;
}

CAE_Object* CAE_Object::FindDeattachedMangr()
{
    CAE_Object* res = NULL;
    if (iMan != NULL) {
	// Check if mgr chromo includes the current node
	const CAE_ChromoNode& mgroot = iMan->Chromo().Root();
	CAE_ChromoNode::Const_Iterator mnodeit = mgroot.Find(ENt_Object, Name());
	if (mnodeit == mgroot.End()) {
	    res = iMan;
	}
	else {
	    res = iMan->FindDeattachedMangr();
	}
    }
    return res;
}

TBool CAE_Object::ChangeAttr(TNodeAttr aAttr, const string& aVal) 
{ 
    TBool res = EFalse;
    if (aAttr == ENa_ObjQuiet) {
	SetQuiet(aVal.compare("yes") == 0);
	res = ETrue;
    }
    else {
	res = CAE_EBase::ChangeAttr(aAttr, aVal);
    }
    return res;
};

void CAE_Object::OnElemChanging(CAE_NBase* aElem, TNodeAttr aAttr)
{
    TNodeType nt = aElem->NodeType();
    if (nt == ENt_Trans) {
	if (aAttr == ENa_Unknown) {
	    // Trans content changed, re-evaluate
	    iEnv->Tranex()->EvalTrans(this, this, iTransSrc.iETrans);
	    iTrans = iEnv->Tranex()->Exprs();
	}
    }
}

TBool CAE_Object::MoveNode(const DesUri& aNodeUri, const DesUri& aDestUri)
{
    TBool res = EFalse;
    CAE_NBase* nsrc = CAE_NBase::GetNode(aNodeUri);
    CAE_NBase* ndest = CAE_NBase::GetNode(aDestUri);
    if (nsrc->NodeType() == ENt_Object && ndest->NodeType() == ENt_Object) {
	map<string, CAE_Object*>::iterator srcrtit = iComps.find(nsrc->Name());
	map<string, CAE_Object*>::iterator destrtit = iComps.find(ndest->Name());
	vector<CAE_Object*>::iterator sit = iCompsOrd.end();
	vector<CAE_Object*>::iterator dit = iCompsOrd.end();
	for (vector<CAE_Object*>::iterator it = iCompsOrd.begin(); it != iCompsOrd.end(); it++) {
	    if (*it == srcrtit->second) {
		sit = it; break;
	    }
	}
	for (vector<CAE_Object*>::iterator it = iCompsOrd.begin(); it != iCompsOrd.end(); it++) {
	    if (*it == destrtit->second) {
		dit = it; break;
	    }
	}
	iCompsOrd.erase(sit);
	iCompsOrd.insert(++dit, srcrtit->second);
	res = ETrue;
    }
    else {
	res = CAE_EBase::MoveNode(aNodeUri, aDestUri);
    }
    return res;
}

