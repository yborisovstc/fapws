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

// Length of bite automata word 
const int KBaWordLen = 32;

void Panic(TInt aRes)
{
	_IND_PANIC(KFapPanic, aRes);
}

// CAE_Base

CAE_Base::CAE_Base(const char* aInstName, CAE_Object* aMan):
	iInstName(NULL), iMan(aMan), iUpdated(ETrue), iActive(ETrue), iLogSpec(NULL)
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

FAPWS_API CAE_Base::~CAE_Base()
{
    if (iMan != NULL)
	iMan->UnregisterComp(this);
    if (iInstName != NULL)
	free(iInstName);
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


// CAE_StateBase

CAE_StateBase::CAE_StateBase(const char* aInstName, TInt aLen, CAE_Object* aMan, StateType aStateType, TLogFormatFun aLogFormFun): 
	CAE_Base(aInstName, aMan), iLen(aLen), iStateType(aStateType), iFlags(0x00), iLogFormFun(aLogFormFun)
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
	iInputsList = new vector<CAE_StateBase*>;
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
	for (i = 0; i < iInputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iInputsList->at(i);
		state->RemoveOutput(this);
	}
	for (i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		state->RemoveInput(this);
	}

	delete iInputsList;
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

char *CAE_StateBase::GetFmtData(TBool aCurr)
{
    char* buf = NULL;
    if (iLogFormFun != NULL)
    {
	buf = iLogFormFun(this, aCurr);
    }
    else
    {	
	buf = (char *) malloc(iLen*4);
	memset(buf, 0, iLen*4);
	char fmtsmb[10] = "";
	void *data = aCurr ? iCurr : iNew;
	for (TInt i=0; i < iLen; i++)
	{
	    int symb = ((TUint8*) data)[i];
	    fmtsmb[0] = 0;
	    sprintf(fmtsmb, "%02x ", symb);
	    strcat(buf, fmtsmb);
	}
    }
    return buf;
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
    char *buf_cur = GetFmtData(ETrue);
    char *buf_new = GetFmtData(EFalse);
    if (aLogData & KBaseDa_New && aLogData & KBaseDa_Curr)
	Logger()->WriteFormat("Updated state [%s]: %s <= %s", iInstName, buf_new, buf_cur);
    else if (aLogData & KBaseDa_New)
	Logger()->WriteFormat("Updated state [%s]: %s", iInstName, buf_new);
    free (buf_cur);
    free (buf_new);
    // Loggin inputs
    if (aLogData & KBaseDa_Dep)
    {
	for (int i = 0; i < iInputsList->size(); i++)
	{
	    CAE_StateBase* state = (CAE_StateBase*) iInputsList->at(i);
	    char* buf = state->GetFmtData(ETrue);
	    if (Logger())
		Logger()->WriteFormat(">>>> Name: %s, val: %s", state->InstName(), buf);
	    free(buf);	
	}
    }
}

FAPWS_API void CAE_StateBase::Update()
{
    void* nthis = this;
    TInt logdata = KBaseDa_None;
    if (iStateType != EType_Input || iStateType == EType_Input && iInputsList->size())
    {
	DoTrans();
	TInt logdata = GetLogSpecData(KBaseLe_Updated);
	if (logdata != KBaseDa_None)
	    LogUpdate(logdata);
    }
}

FAPWS_API void CAE_StateBase::Set(void* aNew) 
{
    if (iStateType != EType_Output || iStateType == EType_Output && iOutputsList->size())
    {
	if (memcmp(aNew, iNew, iLen))
	    SetUpdated();
	memcpy(iNew, aNew, iLen); 
    }
};

FAPWS_API void* CAE_StateBase::DoGetObject(TInt aUid)
{
	if (aUid == EObUid)
		return this;
	else
		return NULL;
}

void CAE_StateBase::RefreshOutputs()
{
	for (TInt j = 0; j < iInputsList->size(); j++)
	{
		Input(j)->AddOutputL(this);
	}
}

FAPWS_API CAE_StateBase* CAE_StateBase::Input(const char* aInstName)
{
	CAE_StateBase* res = NULL;
	for (TInt i = 0; i < iInputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iInputsList->at(i);
		if (state->InstName() == aInstName || strcmp(state->InstName(), aInstName) == 0)
		{
			res = state; break;
		}
	}
	return res;
}


FAPWS_API void CAE_StateBase::AddInputL(CAE_StateBase* aState) 
{
	for (TInt i = 0; i < iInputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iInputsList->at(i);
		_FAP_ASSERT (state != aState);
	}
	iInputsList->push_back(aState);
	aState->AddOutputL(this);
};

void CAE_StateBase::AddOutputL(CAE_StateBase* aState) 
{
	for (TInt i = 0; i < iOutputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iOutputsList->at(i);
		_FAP_ASSERT (state != aState);
	}
	iOutputsList->push_back(aState);
};

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
	for (TInt i = 0; i < iInputsList->size(); i++)
	{
		CAE_StateBase* state = (CAE_StateBase*) iInputsList->at(i);
		if (state == aState)
		{
			iInputsList->erase(iInputsList->begin() + i);
			found = ETrue;
			break;
		}
	}
	_FAP_ASSERT(found);
}

FAPWS_API CAE_StateBase* CAE_StateBase::Input(TInt aInd) 
{ 
	CAE_StateBase* res = NULL;
	if (aInd < iInputsList->size())
		res = (CAE_StateBase*) iInputsList->at(aInd);
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

FAPWS_API CAE_State::CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, StateType aType, 
	TInt aDataTypeUid, TLogFormatFun aLogFormFun):
    CAE_StateBase(aInstName, aLen, aMan, aType, aLogFormFun), iTrans(aTrans)
{
	iDataTypeUid = aDataTypeUid;
};

FAPWS_API CAE_State* CAE_State::NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, StateType aType, 
	TInt aDataTypeUid, TLogFormatFun aLogFormFun)
{
	CAE_State* self = new CAE_State(aInstName, aLen, aMan, aTrans, aType, aDataTypeUid, aLogFormFun);
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
	else if (iInputsList->size())
	{
		_FAP_ASSERT(iLen == ((CAE_StateBase*) iInputsList->at(0))->Len());
		memcpy(iNew, ((CAE_StateBase*) iInputsList->at(0))->iCurr, iLen);
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

// From MObjectProvider
FAPWS_API void* CAE_State::DoGetObject(TInt aUid)
{
	// The method returns the state if there is non-varianted request (ModifType == 0) or exact request
	TInt baseuid = aUid & KObUid_BaseTypeMask;
	TInt modifuid = aUid & KObUid_ModifTypeMask;
	TInt oid = ObjectUid();
	if (baseuid == (oid & KObUid_BaseTypeMask) && (modifuid == 0 || IsDataType(modifuid)))
		return this;
	else
		return NULL;
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
		TInt ninp = iInputsList->size();
		if (ninp > 0)
		{
			CAE_StateBase* sinp1 = Input(0);
			CAE_TState<TUint8>* tsinp1 = sinp1->GetObject(tsinp1);
			if (tsinp1 == NULL)
				err = ETrue;
		}
		if (!err && ninp > 1)
		{
			CAE_StateBase* sinp2 = Input(1);
			CAE_TState<TUint8>* tsinp2 = sinp2->GetObject(tsinp2);
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
	CAE_TState<TUint8>* tsinp1 = sinp1->GetObject(tsinp1);
	_FAP_ASSERT(tsinp1 != NULL);
	CAE_StateBase* sinp2 = Input(1);
	CAE_TState<TUint8>* tsinp2 = NULL;
	if (sinp2 != NULL)
		tsinp2 = sinp2->GetObject(tsinp2);
	_FAP_ASSERT((iTrans.iOpInd < ECStateUint8Op_BinSt) || tsinp2 != NULL);
	switch (iTrans.iOpInd)
	{
	case ECStateUint8Op_Copy:
		*this = (*tsinp1)();
		break;
	case ECStateUint8Op_Add:
		*this = (*tsinp1)() + (*tsinp2)();
		break;
	case ECStateUint8Op_Sub:
		*this = (*tsinp1)() - (*tsinp2)();
		break;
	case ECStateUint8Op_Mpl:
		*this = (*tsinp1)() * (*tsinp2)();
		break;
	case ECStateUint8Op_Cmp:
		*this = ((*tsinp1)() > (*tsinp2)())?1:0;
		break;
	}
}

template<> FAPWS_API TBool CAE_TState<TUint32>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	CAE_State::SetTrans(aTinfo);
	return res;
}


template<> FAPWS_API void CAE_TState<TUint32>::DoOperation()
{
}

template<> FAPWS_API char *CAE_TState<TUint32>::LogFormFun(CAE_StateBase* aState, TBool aCurr)
{
    CAE_State *state = aState->GetObject(state); 
    if (!state->IsDataType(DataTypeUid()))
	return NULL;
    TUint32 data = *((TUint32 *) (aCurr  ? aState->iCurr : aState->iNew));
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


template<> FAPWS_API TBool CAE_TState<TBool>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	CAE_State::SetTrans(aTinfo);
	return res;
}

template<> FAPWS_API void CAE_TState<TBool>::DoOperation()
{
}

template<> FAPWS_API char *CAE_TState<TBool>::LogFormFun(CAE_StateBase* aState, TBool aCurr)
{
    CAE_State *state = aState->GetObject(state); 
    if (!state->IsDataType(DataTypeUid()))
	return NULL;
    TBool data = *((TBool *) (aCurr  ? aState->iCurr : aState->iNew));
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
	CAE_Base(aInstName, aMan), iVariantUid(EObStypeUid), iCompReg(NULL), iChrom(NULL), iChromX(NULL), iEnv(aEnv), 
	iFitness(0), iFitness1(0)
{
}

// From MObjectProvider
// CAE_Object  supports providing the derivatives using VariantUid
// If derivative needs some special providing, it needs to implement specific DoGetObject()
FAPWS_API void* CAE_Object::DoGetObject(TInt aUid)
{
	// The method returns the object if there is non-varianted request (ModifType == 0) or exact request
	TInt baseuid = aUid & KObUid_BaseTypeMask;
	TInt modifuid = aUid & KObUid_ModifTypeMask;
	TInt oid = ObjectUid();
	if (baseuid == (oid & KObUid_BaseTypeMask) && (modifuid == 0 || modifuid == iVariantUid))
		return this;
	else
		return NULL;
}


FAPWS_API CAE_Object* CAE_Object::NewL(const char* aInstName, CAE_Object* aMan, const TUint8* aChrom, MAE_Env* aEnv)
{
	CAE_Object* self = new CAE_Object(aInstName, aMan, aEnv);
	self->SetChromosome(EChromOper_Copy, aChrom);
	self->ConstructL();
	return self;
}

FAPWS_API CAE_Object* CAE_Object::NewL(const char* aInstName, CAE_Object* aMan, const void* aChrom, MAE_Env* aEnv)
{
	CAE_Object* self = new CAE_Object(aInstName, aMan, aEnv);
	self->SetChromosome(EChromOper_Copy, aChrom);
	self->ConstructL();
	return self;
}

FAPWS_API void CAE_Object::SetChromosome(TChromOper aOper, const TUint8* aChrom1, const TUint8* aChrom2)
{
	TInt chrlen = 0;
	if (aChrom1 != NULL)
	{
		chrlen = ChromLen(aChrom1);
		iChrom = new TUint8[chrlen];
		memcpy(iChrom, aChrom1, chrlen); 
	}
	if (aOper == EChromOper_Rand)
	{
		if (aChrom1 != NULL)
		{
			TInt chrlen = ChromLen(aChrom1);
			iChrom = new TUint8[chrlen];
			memcpy(iChrom, aChrom1, chrlen);
			for (TInt i = 0; i < chrlen; i++)
			{
				TUint8 srnd = (rand()*0xff)/RAND_MAX;
				if (!IsChromControlTag(i))
					iChrom[i] = srnd;
			}
		}
	}
	else if (aOper == EChromOper_Mutate)
	{
		if (aChrom1 != NULL)
		{
			DoMutation();
		}
	}
	else if (aOper == EChromOper_Combine)
	{
		if (aChrom1 != NULL && aChrom2 != NULL)
		{
			// Perform crossovering of chromosomes
			// Calculate randomly a point of crossing
			TInt srnd = 1 + (rand()*(chrlen-1))/RAND_MAX;
			memcpy(iChrom+srnd, aChrom2+srnd, chrlen - srnd);
			DoMutation();
		}
	}
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

FAPWS_API void CAE_Object::ConstructL()
{
	iCompReg = new vector<CAE_Base*>;
	if (iMan != NULL  && iCompReg != NULL) 
		iMan->RegisterCompL(this);
	// Create object from chromosome if exists
	if (iChrom != NULL)
	{
	    ConstructFromChromL();
	}
	else if (iChromX != NULL)
	{
	    ConstructFromChromXL();
	}
}

FAPWS_API void CAE_Object::ConstructFromChromL()
{
    TUint8* ploc = iChrom; // Current locus
    TUint8 vectlen = *ploc/KAdp_ObjStInfoLen;
    _FAP_ASSERT(vectlen <= KAdp_ObjCromMaxLen);
    ploc++; // States types
    // Create the states vector
    TInt i = 0;
    for (i = 0; i < vectlen; i++)
    {
	TUint8 sttype = *ploc;
	if (sttype != 0)
	{
	    // Create the state
	    if ((sttype&KAdp_ObjStType_AccessMsk) == KAdp_ObjStType_NU)
		sttype = KAdp_ObjStType_Reg | ECState_Uint8; 
	    else	
		sttype = (sttype&KAdp_ObjStType_AccessMsk) | ECState_Uint8; 
	    CAE_Base* state = iEnv->Provider()->CreateStateL(sttype, NULL, this);
	    _FAP_ASSERT(state != NULL);
	    *ploc = sttype;
	}
	ploc += KAdp_ObjStInfoLen;
    }
    // Create Transitions
    ploc = iChrom + KAdp_ObjChromLen_Len; // Return to the description of the first state
    for (i = 0; i < vectlen; i++)
    {
	// Avoid the state type
	ploc++;
	CAE_State* state = GetStateByInd(i);
	if (state != NULL)
	{
	    TUint8 sinp1ind = *ploc++;	// Input_1
	    if (state->IsInput()) sinp1ind = 0xff;
	    if (sinp1ind != 0xff)
	    {
		CAE_State* sinp = GetStateByInd(sinp1ind);
		if (sinp == NULL)
		{
		    sinp1ind = (vectlen*rand())/RAND_MAX;
		    sinp = GetStateByInd(sinp1ind);
		}
		if (sinp != NULL)
		    state->AddInputL(sinp);
	    }
	    *(ploc-1) = sinp1ind;
	    TUint8 sinp2ind = *ploc++;	// Input_2
	    if (state->IsInput()) sinp2ind = 0xff;
	    if (sinp2ind != 0xff)
	    {
		if (sinp2ind == sinp1ind)
		    sinp2ind = ((sinp1ind+1) == vectlen)?0:(sinp1ind+1);
		CAE_State* sinp = GetStateByInd(sinp2ind);
		if (sinp == NULL)
		{
		    sinp2ind = (vectlen*rand())/RAND_MAX;
		    if (sinp2ind == sinp1ind)
			sinp2ind = ((sinp1ind+1) == vectlen)?0:(sinp1ind+1);
		    sinp = GetStateByInd(sinp2ind);
		}
		if (sinp != NULL)
		    state->AddInputL(sinp);
	    }
	    *(ploc-1) = sinp2ind;
	    TUint8 opr = *ploc++;	// Operation code
	    _FAP_ASSERT(state != NULL);
	    state->SetTrans(TTransInfo(opr));
	    opr = state->GetTrans().iOpInd;
	    *(ploc-1) = opr;
	} // state != NULL
    }
}

FAPWS_API void CAE_Object::ConstructFromChromXL()
{
    MAE_ChroMan *chman = iEnv->Chman();
    MAE_Provider *prov = iEnv->Provider();
    _FAP_ASSERT(chman != NULL);
    char *name = chman->GetName(iChromX);
    if (name != NULL && strlen(name) != 0)
    {
	SetName(name);
    }
    Logger()->WriteFormat("Object constructing. Name: %s", InstName());
    // Pass thru children
    void *child = NULL;
    // Go thru children list in spec and create the children
    for (child = chman->GetChild(iChromX); child != NULL; child = chman->GetNext(child))
    {
	TCaeElemType ftype = chman->FapType(child);
	if (ftype == ECae_Object)
	{
	    CAE_Object::NewL(NULL, this, child, iEnv);
	}
	else if (ftype == ECae_State)
	{
	    int datatype = chman->GetAttrInt(child, KXTransAttr_Type); 
	    char *name = chman->GetName(child); 
	    int len = chman->GetLen(child); 
	    CAE_StateBase::StateType access = chman->GetAccessType(child);
	    const CAE_Formatter *form = prov->GetFormatter(datatype);
	    TLogFormatFun formfun = form ? form->iFun : NULL; 

	    CAE_State *state = CAE_State::NewL(name, len, this,  TTransInfo(), access, datatype, formfun);  
	    if (state == NULL)
		Logger()->WriteFormat("ERROR: Creating state [%s] failed", name);
	    else
	    {
		// Set logspec
		for (void *stelem = chman->GetChild(child); stelem != NULL; stelem = chman->GetNext(stelem))
		{
		    TCaeElemType stetype = chman->FapType(stelem);
		    if (stetype == ECae_Logspec)
		    {
			TInt event = chman->GetAttrInt(stelem, KXTransAttr_LogEvent);
			// Get logging data
			TInt ldata = 0;
			for (void *lselem = chman->GetChild(stelem); lselem != NULL; lselem = chman->GetNext(lselem))
			{
			    TCaeElemType lstype = chman->FapType(lselem);
			    if (lstype == ECae_Logdata)
			    {
				TInt data = chman->GetAttrInt(lselem, KXTransAttr_LogData);
				ldata |= data;
			    }
			    else
			    {
				Logger()->WriteFormat("ERROR: Unknown type [%s]", chman->GetName(lselem));
			    }
			}
			state->AddLogSpec(event, ldata);
		    }
		    else
		    {
			Logger()->WriteFormat("ERROR: Unknown type [%s]", chman->GetName(stelem));
		    }
		}
	    }
	}
	else if (ftype == ECae_Transf)
	{
	    void *dep = NULL;
	    char *name = chman->GetName(child); 
	    const TTransInfo *trans = prov->GetTransf(name);
	    if (trans == NULL)
		Logger()->WriteFormat("ERROR: Transition [%s] not found", name);
	    _FAP_ASSERT(trans != NULL);
	    char *state_name = chman->GetStrAttr(child, KXTransAttr_State);
	    CAE_State* state = GetStateByName(state_name);
	    if (state != NULL)
	    {
		state->SetTrans(*trans);
	    }
	    // Set dependencies
	    for (dep = chman->GetChild(child); dep != NULL; dep = chman->GetNext(dep))
	    {
		char *dep_name = chman->GetStrAttr(dep,"id"); 
		if (dep_name != NULL)
		{
		    CAE_State* dstate = GetStateByName(dep_name);
		    if (dstate != NULL)
		    {
			state->AddInputL(dstate);
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
	if (iChrom != NULL)
	{
		delete iChrom;
		iChrom = NULL;
	}
	iEnv = NULL; // Not owned
};

FAPWS_API TBool CAE_Object::IsTheSame(CAE_Object* aObj) const
{
	TBool res = EFalse;
	TInt chrlen = ChromLen(iChrom);
	if (memcmp(iChrom, aObj->iChrom, chrlen) == 0)
		res = ETrue;
	return res;
}


FAPWS_API void CAE_Object::Confirm()
{
	for (TInt i = 0; i < iCompReg->size(); i++ )
	{
		CAE_Base* obj = (CAE_Base*) iCompReg->at(i);
		if (obj->iUpdated)
		{
			obj->Confirm();
			obj->iUpdated = EFalse;
		}
	}
};

FAPWS_API void CAE_Object::DoMutation()
{
	TInt chrlen = ChromLen(iChrom);
	TInt srnd1 = rand();
	TInt srnd2 = rand();
	TInt wordidx = (chrlen*srnd1)/RAND_MAX; // Index of word in the gene words array 
	TInt bitidx = (8*srnd2)/RAND_MAX; // Index of bit in the gene word 
	TUint32 mask = 1 << bitidx;
	if (!IsChromControlTag(wordidx))
		iChrom[wordidx] ^= mask;
}

TBool CAE_Object::IsChromControlTag(TInt aInd, TInt aTagInd) const
{
	TBool res = EFalse;
	if (aInd == aTagInd)
		res = ETrue;
	else
	{
		TUint8 num = iChrom[aTagInd] & ~KAdp_ObjChromLen_SSD;
		TUint8 ssd = iChrom[aTagInd] & KAdp_ObjChromLen_SSD;
		if (ssd)
		{
			aTagInd++; // Move to the first element 
			for (TInt i = 0; i < num; i++)
			{
				res |= IsChromControlTag(aInd, aTagInd);
				aTagInd += ChromLen(iChrom + aTagInd);
			}
		}
	}
	return res;
}


FAPWS_API void CAE_Object::Update()
{
	for (TInt i = 0; i < iCompReg->size(); i++ )
	{
		CAE_Base* obj = (CAE_Base*) iCompReg->at(i);
		if (obj->iActive)
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

FAPWS_API CAE_Object* CAE_Object::GetComp(const char* aName) const
{
	CAE_Object* res = NULL;
	for (TInt i = 0; i < iCompReg->size(); i++)
	{
		CAE_Object* comp = (CAE_Object*) iCompReg->at(i);
		if (strcmp(aName, comp->InstName()) == 0)
		{
			res = comp; 
			break;
		}
	}
	return res;
}

FAPWS_API TInt CAE_Object::CountCompWithType(TInt aUid) const
{
	TInt res = 0;
	if (aUid == 0)
	{
		res = iCompReg->size();
	}
	else
	{
		for (TInt i = 0; i < iCompReg->size(); i++)
		{
			CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
			if (((CAE_Object*) comp->DoGetObject(aUid)) != NULL)
			{
				res++;
			}
		}
	}
	return res;
}

FAPWS_API CAE_Object* CAE_Object::GetNextCompByType(TInt aUid, int* aCtx) const
{
	CAE_Object* res = NULL;
	for (TInt i = aCtx?*aCtx:0; i < iCompReg->size(); i++)
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
		if ((res = (CAE_Object*) comp->DoGetObject(aUid)) != NULL)
		{
			*aCtx = i+1;
			break;
		}
	}
	return res;
}


FAPWS_API CAE_Base* CAE_Object::FindName(const char* aName) const
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
				CAE_Object* obj = (CAE_Object*) comp->DoGetObject(CAE_Object::EObUid);
				if (obj && strcmp(name, obj->InstName()) == 0)
				{
					res = obj->FindName(tail+1); 
					break;
				}
			}
		}
	}
	return res;
}

FAPWS_API void CAE_Object::LinkL(CAE_State* aInp, CAE_StateBase* aOut, TTransFun aTrans)
{
	_FAP_ASSERT(aTrans || (aInp->Len() == aOut->Len())); // Panic(EFapPan_IncorrLinksSize)); 
	aInp->AddInputL(aOut);
	aInp->SetTrans(TTransInfo(aTrans));
}

FAPWS_API CAE_State* CAE_Object::GetInput(TUint32 aInd)
{
	CAE_State* res = NULL;
	TUint32 sInd = 0;
	for (TInt i = 0; i < iCompReg->size(); i++)
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(i);
		CAE_State* obj = (CAE_State*) comp->GetObject(obj);
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
		CAE_State* obj = (CAE_State*) comp->GetObject(obj);
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

FAPWS_API CAE_State* CAE_Object::GetStateByInd(TUint32 aInd)
{
	CAE_State* res = NULL;
	if (aInd < iCompReg->size())
	{
		CAE_Base* comp = (CAE_Base*) iCompReg->at(aInd);
		if (comp != NULL)
		{
			res = comp->GetObject(res);
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
	CAE_State* state = (CAE_State*) comp->GetObject(state);
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
		CAE_State* state = (CAE_State*) comp->GetObject(state);
		if (state != NULL)
		{
			state->Reset();
		}
	}
}

// The inheritor is created by clone the parent and mutate it for now

FAPWS_API CAE_Object* CAE_Object::CreateNewL(const char* aInstName, TChromOper aOper, const CAE_Object* aParent2)
{
	CAE_Object* sinheritor = new CAE_Object(aInstName, iMan, iEnv);
	sinheritor->SetChromosome(aOper, iChrom, aParent2?aParent2->iChrom:NULL);
	sinheritor->ConstructL();
	return sinheritor;
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
	iStInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("StInpReset", sizeof(TUint8), this, CAE_TRANS(UpdateInpReset));
	iInpReset = (CAE_TState<TUint8>*) CAE_State::NewL("InpReset", sizeof(TUint8), this, TTransInfo(), CAE_StateBase::EType_Reg);
	iStFitness = (CAE_TState<TInt>*) CAE_State::NewL("Fitness", sizeof(TInt), this, TTransInfo());
	iOut = (CAE_TState<TInt>*) CAE_State::NewL("Out", sizeof(TInt), this, TTransInfo(), CAE_StateBase::EType_Reg);
	iStInp->AddInputL(iInp);
	iStInpReset->AddInputL(iInpReset);
}

void CAE_ObjectBa::UpdateInput(CAE_State* aState)
{
	TInt data = (*iInp)();
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
		TInt data = (*iStInp)();
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
	iOut->AddInputL(iInp);
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
	iStSetActive->AddInputL(iInpSetActive); 
	iStCancel->AddInputL(iInpCancel);
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
	if ((*iInpCancel)() > 0)
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

