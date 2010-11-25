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

// CAE_Base

CAE_Base::CAE_Base(const char* aInstName, CAE_Object* aMan):
	iInstName(NULL), iTypeName(NULL), iMan(aMan), iUpdated(ETrue), iActive(ETrue), iQuiet(EFalse), iLogSpec(NULL)
{
    if (aInstName != NULL) 
	iInstName = strdup(aInstName);
};


void CAE_Base::SetActive() 
{ 
	iActive= ETrue; if (iMan) iMan->SetActive();
};

void CAE_Base::SetUpdated()
{ 
	iUpdated= ETrue; if (iMan) iMan->SetUpdated();
};

void CAE_Base::SetName(const char *aName)
{
    if (iInstName != NULL)
	free(iInstName);
    if (aName != NULL) 
	iInstName = strdup(aName);
}

void CAE_Base::SetType(const char *aName)
{
    if (iTypeName != NULL)
	free(iTypeName);
    if (aName != NULL) 
	iTypeName = strdup(aName);
}

FAPWS_API CAE_Base::~CAE_Base()
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

void CAE_Base::AddLogSpec(TInt aEvent, TInt aData)
{
    if (iLogSpec == NULL)
    {
	iLogSpec = new vector<TLogSpecBase>;
    }
    _FAP_ASSERT (iLogSpec != NULL);
    iLogSpec->push_back(TLogSpecBase(aEvent, aData));
}

TInt CAE_Base::GetLogSpecData(TInt aEvent) const
{	
    TInt res = KBaseDa_None;
    for (TInt i = 0; iLogSpec && i < iLogSpec->size(); i++)
    {
	const TLogSpecBase& spec = iLogSpec->at(i);
	if (spec.iEvent == aEvent)
	    res = spec.iData; break;
    }
    return res;
}

const char* CAE_Base::MansName(TInt aLevel) const 
{ 
    CAE_Base *man = iMan; 
    for (TInt i=aLevel; i > 0 && man != NULL && man->iMan != NULL; i--) man = man->iMan;
    return (man == NULL)? "": man->InstName();
};

// CAE_StateBase

CAE_StateBase::CAE_StateBase(const char* aInstName, TInt aLen, CAE_Object* aMan, StateType aStateType): 
	CAE_Base(aInstName, aMan), iLen(aLen), iStateType(aStateType), iFlags(0x00)
{
    _FAP_ASSERT (iMan != NULL);
}
	
FAPWS_API void CAE_StateBase::ConstructL()
{
	iCurr = (void*) new  TUint8[iLen];
	iNew = (void*) new  TUint8[iLen];
	memset(iCurr, 0x00, iLen);
	memset(iNew, 0x00, iLen);
	// TODO [YB] Do we need to assert iMan?
	if (iMan)
	    iMan->RegisterCompL(this);
	iOutputsList = new vector<CAE_StateBase*>;
	if (Logger())
	    Logger()->WriteFormat("State created:: Name: %s, Len: %d, Access: %s", iInstName, iLen, AccessType());
}	

FAPWS_API CAE_StateBase::~CAE_StateBase()
{
	free(iCurr);
	free(iNew);
	TInt i = 0;
	// Remove self from inputs and outputs
	for (map<string, CAE_StateBase*>::iterator i = iInpList.begin(); i != iInpList.end(); i++)
	{
		CAE_StateBase* state = i->second;
		if (state != NULL)
		    state->RemoveOutput(this);
	}
	for (i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		state->RemoveInput(this);
	}

	delete iOutputsList;
}

const char *CAE_StateBase::AccessType() const
{
    return (iStateType == CAE_StateBase::EType_Input) ? "Inp" : ((iStateType == CAE_StateBase::EType_Reg) ? "Reg" : "Out");
}

FAPWS_API void CAE_StateBase::Confirm()
{
    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
    {
	void* nthis = this;
	if (memcmp(iCurr, iNew, iLen))
	{
	    for (TInt i = 0; i < iOutputsList->size(); i++)
		((CAE_StateBase*) iOutputsList->at(i))->SetActive();
	}
	memcpy(iCurr, iNew, iLen); // iCurr << iNew
    }
}

char* CAE_StateBase::DataToStr(TBool aCurr) const
{
    return FmtData(aCurr ? iCurr : iNew, iLen);
}

void CAE_StateBase::DataFromStr(const char* aStr, void *aData) const
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


char *CAE_StateBase::FmtData(void *aData, int aLen)
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

void CAE_StateBase::LogUpdate(TInt aLogData)
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
	for (map<string, CAE_StateBase*>::iterator i = iInpList.begin(); i != iInpList.end(); i++)
	{
	    CAE_StateBase* state = i->second;
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
	for (map<string, CAE_StateBase*>::iterator i = iInpList.begin(); i != iInpList.end(); i++)
	{
	    CAE_StateBase* state = i->second;
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

FAPWS_API void CAE_StateBase::Update()
{
    void* nthis = this;
    TInt logdata = KBaseDa_None;
    if (iStateType != EType_Input || iStateType == EType_Input && iInpList.size())
    {
	TInt logdata = GetLogSpecData(KBaseLe_Trans);
	if (logdata != KBaseDa_None)
	    LogTrans(logdata);
	DoTrans();
    }
}

FAPWS_API void CAE_StateBase::Set(void* aNew) 
{
    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
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

FAPWS_API void CAE_StateBase::SetFromStr(const char *aStr)
{
    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
    {
	void *data = malloc(iLen);
	memset(data, 0, iLen);
	DataFromStr(aStr, data);
	Set(data);
	free(data);
    }
}

CAE_Base *CAE_StateBase::DoGetFbObj(const char *aName)
{
    if (strcmp(aName, Type()) == 0)
	return this;
    else
	return NULL;
}

void CAE_StateBase::RefreshOutputs()
{
    for (map<string, CAE_StateBase*>::iterator i = iInpList.begin(); i != iInpList.end(); i++) {
	i->second->AddOutputL(this);
    }
}

FAPWS_API CAE_StateBase* CAE_StateBase::Input(const char* aName)
{
    map<string, CAE_StateBase*>::iterator it = iInpList.find(aName);
    if (it == iInpList.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), aName);
    }
    if (it->second == NULL) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), InstName(), aName);
    }
    return (it == iInpList.end()) ? NULL : it->second;
}

FAPWS_API CAE_StateBase* CAE_StateBase::Input(const char* aName, TInt aExt)
{
    char *name = strdup(aName);
    strcat(name, ".");
    char ext[40] = "";
    sprintf(ext, "%d", aExt);
    strcat(name, ext);
    map<string, CAE_StateBase*>::iterator it = iInpList.find(name);
    // TODO YB To consider the way of logging config for avoiding spam
    /*
    if (it == iInpList.end()) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), name);
    }
    if (it->second == NULL) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not connected", MansName(1), MansName(0), InstName(), name);
    }
    */
    free(name);
    return (it == iInpList.end()) ? NULL : it->second;
}

void CAE_StateBase::AddOutputL(CAE_StateBase* aState) 
{
	for (TInt i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		_FAP_ASSERT (state != aState);
	}
	iOutputsList->push_back(aState);
};

FAPWS_API void CAE_StateBase::AddInputL(const char* aName) 
{
    TBool exists = iInpList.find(aName) != iInpList.end();
    if (exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already exists", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(!exists);
    }
    iInpList.insert(pair<string, CAE_StateBase*>(aName, NULL));
}
/*
FAPWS_API void CAE_StateBase::SetInputL(const char *aName, CAE_StateBase* aState)
{
    TBool exists = iInpList.find(aName) != iInpList.end();
    if (!exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(exists);
    }
    iInpList[aName] = aState;
    aState->AddOutputL(this);
}
*/

FAPWS_API void CAE_StateBase::SetInputL(const char *aName, CAE_StateBase* aState)
{
    // Check if the name is the name of extended input 
    char *name = strdup(aName);
    char *extp = strchr(name, '.');
    if (extp != NULL) {
	// Extended, form the name template for search
	*(extp + 1) = '*'; *(extp + 2) = 0;
    }

    // Check if the input is specified for the state
    TBool exists = iInpList.find(name) != iInpList.end();
    if (!exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] not exists", MansName(1), MansName(0), InstName(), name);
	_FAP_ASSERT(name);
    }
    // Check if the input is not connected yet
    if (iInpList[aName] != NULL) {
	Logger()->WriteFormat("State [%s.%s.%s]: Inp [%s] already connected", MansName(1), MansName(0), InstName(), aName);
	_FAP_ASSERT(EFalse);
    }
    iInpList[aName] = aState;
    aState->AddOutputL(this);
    free(name);
}

// TODO [YB] It's possible to simply add extended input - not consistent with SetInput
FAPWS_API void CAE_StateBase::AddInputL(const char *aName, CAE_StateBase* aState) 
{
    AddInputL(aName);
    SetInputL(aName, aState);
}

FAPWS_API void CAE_StateBase::AddExtInputL(const char *aName, CAE_StateBase* aState)
{
    // Check if extended name registered
    char *name = (char *) malloc(strlen(aName) + 40);
    strcpy(name, aName);
    strcat(name, ".*");
    TBool exists = iInpList.find(name) != iInpList.end();
    if (!exists) {
	Logger()->WriteFormat("State [%s.%s.%s]: Ext inp [%s] not exists", MansName(1), MansName(0), InstName(), name);
	_FAP_ASSERT(EFalse);
    }
    TInt ind = GetExInpLastInd(aName);
    char sind[40] = "";
    sprintf(sind, "%d", ++ind);
    strcpy(name, aName);
    strcat(name, ".");
    strcat(name, sind);
    SetInputL(name, aState);
    free(name);
}

TInt CAE_StateBase::GetExInpLastInd(const char *aName)
{
    TInt ind = -1;
    map<string, CAE_StateBase*>::iterator it = iInpList.end();
    for (--it ; it != iInpList.begin(); it--) {
	if (it->first.find(aName) == 0) {
	    char *cstr = strdup(it->first.c_str());
	    char *ptr = strchr(cstr, '.');
	    if (ptr != NULL && strlen(ptr) > 1 && ptr[1] != '*') {
		sscanf(ptr+1, "%d", &ind);
		break;
	    }
	    free(cstr);
	} 
    }
    return ind;
}
    
void CAE_StateBase::RefreshOutput(CAE_StateBase* aState)
{
	// Verify there is not the same state in the output list
	for (TInt i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		_FAP_ASSERT (state != aState);
	}
	iOutputsList->push_back(aState);
}

void CAE_StateBase::RemoveOutput(CAE_StateBase* aState)
{
	TBool found = EFalse;
	for (TInt i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		if (state == aState)
		{
			iOutputsList->erase(iOutputsList->begin() + i);
			found = ETrue;
			break;
		}
	}
	_FAP_ASSERT(found);
}

void CAE_StateBase::RemoveInput(CAE_StateBase* aState)
{
    TBool found = EFalse;
    for (map<string, CAE_StateBase*>::iterator i = iInpList.begin(); i != iInpList.end(); i++) {
	if (i->second == aState) {
	    i->second = NULL;
	    found = ETrue; break;
	}
    }
    _FAP_ASSERT(found);
}

FAPWS_API CAE_StateBase* CAE_StateBase::Input(TInt aInd) 
{ 
    CAE_StateBase* res = NULL;
    map<string, CAE_StateBase*>::iterator i;
    if (aInd < iInpList.size()) {
	for (TInt ind = 0; ind < aInd; ind++)
	    i++;
	res = i->second;
    }
    return res;
}

FAPWS_API CAE_StateBase* CAE_StateBase::Output(TInt aInd) 
{ 
	CAE_StateBase* res = NULL;
	if (aInd < iOutputsList->size())
		res = (CAE_StateBase*) iOutputsList->at(aInd);
	return res;
}

FAPWS_API void CAE_StateBase::Reset()
{
	memset(iCurr, 0x00, iLen);
	memset(iNew, 0x00, iLen);
}


// CAE_State

const char* KCAE_StateName = "State";

FAPWS_API CAE_State::CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, StateType aType):
    CAE_StateBase(aInstName, aLen, aMan, aType), iTrans(aTrans)
{
};

FAPWS_API CAE_State* CAE_State::NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, StateType aType)
{
	CAE_State* self = new CAE_State(aInstName, aLen, aMan, aTrans, aType);
	self->ConstructL();
	return self;
}

FAPWS_API TBool CAE_State::SetTrans(TTransInfo aTinfo) 
{ 
	iTrans = aTinfo; 
	return ETrue;
};


FAPWS_API void CAE_State::DoTrans() 
{
	if (iTrans.iFun != NULL) 
		iTrans.iFun((iTrans.iCbo != NULL) ? iTrans.iCbo : iMan, this);
	else if (iTrans.iOpInd != 0)
		DoOperation();
	// TODO [YB] To consider if this case is still needed
	else if (iInpList.size())
	{
	    CAE_StateBase *inp = iInpList.begin()->second;
	    if (inp != NULL) {
		_FAP_ASSERT(iLen == inp->Len());
		Set(inp->iCurr);
	    }
//	    memcpy(iNew, inp->iCurr, iLen);
	}
// [YB] Default transition denied (ref FAP_REQ_STA_01, bug#133)
//	else
//		memcpy(iNew, iCurr, iLen);
};

// The base class has the trivial implementation of operation
FAPWS_API void CAE_State::DoOperation()
{
}


FAPWS_API TOperationInfo CAE_State::OperationInfo(TUint8 aId) const
{
	TOperationInfo nfo = {aId, 0, 0, 0};
	return nfo;
}

// From CAE_Base
CAE_Base *CAE_State::DoGetFbObj(const char *aName)
{
    if ((iTypeName != NULL) && (strcmp(iTypeName, aName) == 0) || (strcmp(aName, Type()) == 0))
	return this;
    else
	return CAE_StateBase::DoGetFbObj(aName);
}

//*********************************************************
// CAE_TState - typified state
//*********************************************************


template<> FAPWS_API TBool CAE_TState<TUint8>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	TBool err = EFalse;
	if (aTinfo.iOpInd != 0)
	{
		TUint32 opind = aTinfo.iOpInd;
		// Get operand types
		CAE_StateBase* sinp1 = NULL;
		CAE_StateBase* sinp2 = NULL;
		TInt ninp = iInpList.size();
		if (ninp > 0)
		{
			CAE_StateBase* sinp1 = Input(0);
			//CAE_TState<TUint8>* tsinp1 = sinp1->GetObject(tsinp1);
			CAE_TState<TUint8>* tsinp1 = sinp1->GetFbObj(tsinp1);
			if (tsinp1 == NULL)
				err = ETrue;
		}
		if (!err && ninp > 1)
		{
			CAE_StateBase* sinp2 = Input(1);
			CAE_TState<TUint8>* tsinp2 = sinp2->GetFbObj(tsinp2);
			if (tsinp2 == NULL)
				err = ETrue;
		}
		if (!err)
		{
			if (ninp == 0)
			{
				opind = ECStateUint8Op_None;
			}
			else if (ninp == 1)
			{
				if (opind < ECStateUint8Op_UnarySt || opind > ECStateUint8Op_UnaryEnd)
				{
					opind = ECStateUint8Op_UnarySt + ((ECStateUint8Op_UnaryEnd-ECStateUint8Op_UnarySt+1)*rand())/RAND_MAX;
				}
			}
			else if (ninp == 2)
			{
				if (opind < ECStateUint8Op_BinSt || opind > ECStateUint8Op_BinEnd)
				{
					opind = ECStateUint8Op_BinSt + ((ECStateUint8Op_BinEnd-ECStateUint8Op_BinSt+1)*rand())/RAND_MAX;
				}
			}
		}
		aTinfo.iOpInd = opind;
	}
	if (!err)
		CAE_State::SetTrans(aTinfo);
	else
		res = !err;
	return res;
}

template<> FAPWS_API void CAE_TState<TUint8>::DoOperation()
{
	CAE_StateBase* sinp1 = Input(0);
	_FAP_ASSERT(sinp1 != NULL);
	CAE_TState<TUint8>* tsinp1 = sinp1->GetFbObj(tsinp1);
	_FAP_ASSERT(tsinp1 != NULL);
	CAE_StateBase* sinp2 = Input(1);
	CAE_TState<TUint8>* tsinp2 = NULL;
	if (sinp2 != NULL)
		tsinp2 = sinp2->GetFbObj(tsinp2);
	_FAP_ASSERT((iTrans.iOpInd < ECStateUint8Op_BinSt) || tsinp2 != NULL);
	switch (iTrans.iOpInd)
	{
	case ECStateUint8Op_Copy:
		*this = tsinp1->Value();
		break;
	case ECStateUint8Op_Add:
		*this = ~*tsinp1 + ~*tsinp2;
		break;
	case ECStateUint8Op_Sub:
		*this = ~*tsinp1 - ~*tsinp2;
		break;
	case ECStateUint8Op_Mpl:
		*this = ~*tsinp1 * ~*tsinp2;
		break;
	case ECStateUint8Op_Cmp:
		*this = ~*tsinp1 > ~*tsinp2 ? 1:0;
		break;
	}
}

template<> FAPWS_API TBool CAE_TState<TUint32>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	CAE_State::SetTrans(aTinfo);
	return res;
}

template<> void CAE_TState<TUint8>::DataFromStr(const char* aStr, void *aData) const
{
    CAE_StateBase::DataFromStr(aStr, aData);
}

template<> char *CAE_TState<TUint8>::DataToStr(TBool aCurr) const
{
    TUint8 data = *((TUint8 *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%d ", data);
    return buf;
}

template<> FAPWS_API void CAE_TState<TUint32>::DoOperation()
{
}

template<> void CAE_TState<TUint32>::DataFromStr(const char* aStr, void *aData) const
{
    sscanf(aStr, "%d", (TUint32 *) aData);
}

template<> char *CAE_TState<TUint32>::DataToStr(TBool aCurr) const
{
    TUint32 data = *((TUint32 *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%d ", data);
    return buf;
}

template<> FAPWS_API TBool CAE_TState<TInt>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	CAE_State::SetTrans(aTinfo);
	return res;
}

template<> FAPWS_API void CAE_TState<TInt>::DoOperation()
{
}

template<> void CAE_TState<TInt>::DataFromStr(const char* aStr, void *aData) const
{
    sscanf(aStr, "%d", (TInt *) aData);
}

template<> char *CAE_TState<TInt>::DataToStr(TBool aCurr) const
{
    TInt data = *((TInt *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%d ", data);
    return buf;
}



template<> FAPWS_API TBool CAE_TState<TBool>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	CAE_State::SetTrans(aTinfo);
	return res;
}

template<> FAPWS_API void CAE_TState<TBool>::DoOperation()
{
}

template<> void CAE_TState<TBool>::DataFromStr(const char* aStr, void *aData) const
{
    CAE_StateBase::DataFromStr(aStr, aData);
}

// TODO [YB] Default method is called instead of this, investigate why
template<> char *CAE_TState<TBool>::DataToStr(TBool aCurr) const
{
    TBool data = *((TBool *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%s ", data ? "True" : "False");
    return buf;
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
	CAE_Base(aInstName, aMan), iCompReg(NULL), iChromX(NULL), iEnv(aEnv), 
	iFitness(0), iFitness1(0)
{
}

CAE_Base *CAE_Object::DoGetFbObj(const char *aName)
{
    CAE_Base *res = NULL;
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

FAPWS_API void CAE_Object::ConstructL(const void* aChrom)
{
    if (iCompReg == NULL) {
	iCompReg = new vector<CAE_Base*>;
	if (iMan != NULL  && iCompReg != NULL) 
	    iMan->RegisterCompL(this);
    }
    if (aChrom != NULL)
	ConstructFromChromXL(aChrom);
    else if (iChromX != NULL)
	ConstructFromChromXL(iChromX);
}

FAPWS_API void CAE_Object::ChangeChrom(const void* aChrom)
{
    MAE_ChroMan *chman = iEnv->Chman();
    _FAP_ASSERT(chman != NULL);
    char *sparent = chman->GetStrAttr((void*) aChrom, KXTransAttr_Type);
//    iEnv->Root()->FindName(sparent);
}

// TODO [YB] To support referencing to object in spec instead of direct definition
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
	    CAE_StateBase::StateType access = chman->GetAccessType(child);
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
		state = prov->CreateStateL(stype, name, this, access);  
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
			    state->AddInputL(sinpid);
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
	else if (ftype == ECae_Conn)
	{
	    void *dep = NULL;
	    char *state_name = chman->GetStrAttr(child, KXTransAttr_State);
	    CAE_State* state = (CAE_State *) FindByName(state_name);
	    if (state == NULL) {
		Logger()->WriteFormat("ERROR: Connecting state [%s]: connection not found", state_name);
	    }
	    else {
		for (dep = chman->GetChild(child); dep != NULL; dep = chman->GetNext(dep))
		{
		    char *inp_name = chman->GetStrAttr(dep,"inp"); 
		    char *dep_name = chman->GetStrAttr(dep,"conn"); 
		    if (dep_name != NULL)
		    {
			CAE_State* dstate = (CAE_State *) FindByName(dep_name);
			if (dstate != NULL)
			{
			    state->SetInputL(inp_name, dstate);
			}
			else {
			    Logger()->WriteFormat("ERROR: Creating state [%s]: connection [%s] not found", name, dep_name);
			}
		    }
		}
	    }
	}
	else
	{
	    // TODO Exception to be raised
	}
    }
}

FAPWS_API CAE_Object::~CAE_Object()
{
	TInt count = iCompReg->size();
	// Use the revers orders recalling that deleting object cause unregister it from parent
	for (TInt i = count-1; i >= 0; i-- )
	{
		CAE_Base* obj = (CAE_Base*) iCompReg->at(i);
		delete obj;
	}
	iCompReg->clear();
	delete iCompReg;
	iCompReg = NULL;
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
	for (TInt i = 0; i < iCompReg->size(); i++ )
	{
		CAE_Base* obj = (CAE_Base*) iCompReg->at(i);
		if (obj->iUpdated && !obj->iQuiet)
		{
			obj->Confirm();
			obj->iUpdated = EFalse;
		}
	}
};

FAPWS_API void CAE_Object::DoMutation()
{
    // TODO [YB] to implement
}

FAPWS_API void CAE_Object::Update()
{
	for (TInt i = 0; i < iCompReg->size(); i++ )
	{
		CAE_Base* obj = (CAE_Base*) iCompReg->at(i);
		if (obj->iActive && !obj->iQuiet)
		{
			obj->iUpdated = obj->iActive;
			obj->iActive = EFalse;
			obj->Update();
		}
	}
}

FAPWS_API void CAE_Object::SetActive() 
{
	iActive = ETrue; 
	if (iMan) 
		iMan->SetActive();
};

FAPWS_API void CAE_Object::SetUpdated() 
{
	iUpdated = ETrue; 
	if (iMan) 
		iMan->SetUpdated();
};


void CAE_Object::UnregisterComp(CAE_Base* aComp)
{
	for (TInt i = 0; i < iCompReg->size(); i++ )
		if (iCompReg->at(i) == aComp) {iCompReg->erase(iCompReg->begin() + i); break; };
}

FAPWS_API CAE_Object* CAE_Object::GetComp(const char* aName, TBool aGlob)
{
    CAE_Object* res = NULL;
    for (TInt i = 0; i < iCompReg->size(); i++) {
	CAE_Object* comp = (CAE_Object*) iCompReg->at(i);
	if (strcmp(aName, comp->InstName()) == 0) {
	    res = comp; break;
	}
    }
    if (res == NULL && aGlob) {
	for (TInt i = 0; i < iCompReg->size() && res == NULL; i++) {
	    CAE_Object* comp = (CAE_Object*) iCompReg->at(i);
	    res = comp->GetComp(aName, ETrue);
	}
    }
    return res;
}

FAPWS_API TInt CAE_Object::CountCompWithType(const char *aType)
{
    TInt res = 0;
    if (aType == NULL) {
	res = iCompReg->size();
    }
    else {
	for (TInt i = 0; i < iCompReg->size(); i++) {
	    CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
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
    for (TInt i = aCtx?*aCtx:0; i < iCompReg->size(); i++)
    {
	CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
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
    for (TInt i = aCtx?*aCtx:0; i < iCompReg->size(); i++) {
	CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
	if ((strcmp(comp->TypeName(), aType) == 0) && (elem = comp->GetFbObj(elem)) != NULL) {
	    res = elem;
	    *aCtx = i+1;
	    break;
	}
    }
    return res;
}


FAPWS_API CAE_Base* CAE_Object::FindByName(const char* aName)
{
	CAE_Base* res = NULL;
	char name[KNameMaxLen];
	const char* tail = strchr(aName, KNameSeparator);
	TInt namelen = tail?tail - aName:strlen(aName);
	if (namelen < KNameMaxLen)
	{
		strncpy(name, aName, namelen);
		name[namelen] = 0x00;
		
		if (tail == 0x00)
		{ // Simple name
			for (TInt i = 0; i < iCompReg->size(); i++)
			{
				CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
				if ((comp->InstName() != NULL) && (strcmp(name, comp->InstName()) == 0))
				{
					res = comp; break;
				}
			}
		}
		else
		{ // Full name
			for (TInt i = 0; i < iCompReg->size(); i++)
			{
				CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
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

FAPWS_API void CAE_Object::LinkL(CAE_State* aInp, CAE_StateBase* aOut, TTransFun aTrans)
{
	_FAP_ASSERT(aTrans || (aInp->Len() == aOut->Len())); // Panic(EFapPan_IncorrLinksSize)); 
	aInp->AddInputL("_Inp_1");
	aInp->SetInputL("_Inp_1", aOut);
	aInp->SetTrans(TTransInfo(aTrans));
}

FAPWS_API CAE_State* CAE_Object::GetInput(TUint32 aInd)
{
	CAE_State* res = NULL;
	TUint32 sInd = 0;
	for (TInt i = 0; i < iCompReg->size(); i++)
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
		CAE_State* obj = (CAE_State*) comp->GetFbObj(obj);
		if (obj != NULL && obj->IsInput())
		{
			if (sInd++ == aInd)
			{
				res = obj; 
				break;
			}
		}
	}
	return res;
}

FAPWS_API CAE_State* CAE_Object::GetInput(const char *aName)
{
    CAE_State* state = GetStateByName(aName);
    return (state != NULL && state->IsInput()) ? state : NULL;
}

FAPWS_API CAE_State* CAE_Object::GetOutput(TUint32 aInd)
{
	CAE_State* res = NULL;
	TUint32 sInd = 0;
	for (TInt i = 0; i < iCompReg->size(); i++)
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
		CAE_State* obj = (CAE_State*) comp->GetFbObj(obj);
		if (obj != NULL && obj->IsOutput())
		{
			if (sInd++ == aInd)
			{
				res = obj; 
				break;
			}
		}
	}
	return res;
}

FAPWS_API CAE_State* CAE_Object::GetOutput(const char *aName)
{
    CAE_State* state = GetStateByName(aName);
    return (state != NULL && state->IsOutput()) ? state : NULL;
}

FAPWS_API CAE_State* CAE_Object::GetStateByInd(TUint32 aInd)
{
	CAE_State* res = NULL;
	if (aInd < iCompReg->size())
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(aInd);
		if (comp != NULL)
		{
			res = comp->GetFbObj(res);
		}
	}
	return res;
}

FAPWS_API CAE_State* CAE_Object::GetStateByName(const char *aName)
{
    CAE_State* res = NULL;
    for (TInt i = 0; i < iCompReg->size(); i++)
    {
	CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
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

FAPWS_API void CAE_Object::Reset()
{
	for (TInt i = 0; i < iCompReg->size(); i++)
	{
		CAE_Base* comp = iCompReg->at(i);
		CAE_State* state = (CAE_State*) comp->GetFbObj(state);
		if (state != NULL)
		{
			state->Reset();
		}
	}
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
	iInp = (CAE_TState<TInt>*) CAE_State::NewL("Inp", sizeof(TInt), this, TTransInfo(), CAE_StateBase::EType_Input);
	iStInp = (CAE_TState<TInt>*) CAE_State::NewL("StInp", sizeof(TInt), this, CAE_TRANS(UpdateInput), CAE_StateBase::EType_Input);
	iStInp->AddInputL("_Inp");
	iStInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("StInpReset", sizeof(TUint8), this, CAE_TRANS(UpdateInpReset));
	iStInpReset->AddInputL("_InpReset");
	iInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("InpReset", sizeof(TUint8), this, TTransInfo(), CAE_StateBase::EType_Reg);
	iStFitness = (CAE_TState<TInt>*) CAE_State::NewL("Fitness", sizeof(TInt), this, TTransInfo());
	iOut = (CAE_TState<TInt>*) CAE_State::NewL("Out", sizeof(TInt), this, TTransInfo(), CAE_StateBase::EType_Reg);
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

FAPWS_API CAE_Link* CAE_Link::NewL(const char* aInstName, CAE_Object* aMan, CAE_StateBase* aInp, CAE_StateBase* aOut)
{
	CAE_Link* self = new CAE_Link(aInstName, aMan);
	self->ConstructL(aInp, aOut);
	return self;
};

void CAE_Link::ConstructL(CAE_StateBase* aInp, CAE_StateBase* aOut)
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
	iOutStatusPtr = (CAE_TState<TInt>*) CAE_State::NewL("OStatusPtr", sizeof(void*), this, TTransInfo(), CAE_State::EType_Output);
	iOutStatus = CAE_State::NewL("OStatus", sizeof(TInt), this, TTransInfo(), CAE_State::EType_Output);
	iInpSetActive = (CAE_TState<TUint8>*) CAE_State::NewL("ISetAct", sizeof(TUint8), this, TTransInfo(), CAE_State::EType_Input);
	iInpCancel = (CAE_TState<TUint8>*) CAE_State::NewL("ICancel", sizeof(TUint8), this, TTransInfo(), CAE_State::EType_Input);
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


