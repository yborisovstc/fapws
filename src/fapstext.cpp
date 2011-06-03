
// FAP States extentions

#include <stdlib.h>
#include <stdio.h>
#include "panics.h"
#include "fapstext.h"
#include "deslbase.h"

template<>const char *CAE_TState<TUint8>::Type() {return "StUint8"; }

template<>const char *CAE_TState<TInt>::Type() {return "StInt"; }

template<>const char *CAE_TState<TUint32>::Type() {return "StUint32"; }

template<>const char *CAE_TState<TBool>::Type() {return "StBool"; }


template<> TBool CAE_TState<TUint8>::SetTrans(TTransInfo aTinfo)
{
	TBool res = ETrue;
	TBool err = EFalse;
	if (aTinfo.iOpInd != 0)
	{
		TUint32 opind = aTinfo.iOpInd;
		// Get operand types
		CAE_State* sinp1 = NULL;
		CAE_State* sinp2 = NULL;
		TInt ninp = iInputs.size();
		if (ninp > 0)
		{
			CAE_StateBase* sinp1 = Input("_0");
			//CAE_TState<TUint8>* tsinp1 = sinp1->GetObject(tsinp1);
			CAE_TState<TUint8>* tsinp1 = sinp1->GetFbObj(tsinp1);
			if (tsinp1 == NULL)
				err = ETrue;
		}
		if (!err && ninp > 1)
		{
			CAE_StateBase* sinp2 = Input("_1");
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

template<> void CAE_TState<TUint8>::DoOperation()
{
	CAE_StateBase* sinp1 = Input("_0");
	_FAP_ASSERT(sinp1 != NULL);
	CAE_TState<TUint8>* tsinp1 = sinp1->GetFbObj(tsinp1);
	_FAP_ASSERT(tsinp1 != NULL);
	CAE_StateBase* sinp2 = Input("_1");
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
    CAE_State::DataFromStr(aStr, aData);
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
    sscanf(aStr, "%u", (unsigned int *) aData);
}

template<> char *CAE_TState<TUint32>::DataToStr(TBool aCurr) const
{
    TUint32 data = *((TUint32 *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%u ", (unsigned int ) data);
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
    TBool* data = (TBool*) aData;
    *data = EFalse;
    if (strcmp(aStr, "True") == 0 || strcmp(aStr, "true") == 0 || strcmp(aStr, "1") == 0)
	*data = ETrue;
}

template<> char *CAE_TState<TBool>::DataToStr(TBool aCurr) const
{
    TBool data = *((TBool *) (aCurr ? iCurr : iNew));
    char* buf = (char *) malloc(10);
    memset(buf, 0, 10);
    sprintf(buf, "%s ", data ? "True" : "False");
    return buf;
}





template<> const char* CAE_TState<CF_TdPoint>::Type() {return "StPoint";};

template<> const char* CAE_TState<CF_TdPointF>::Type() {return "StPointF";};

template<> const char* CAE_TState<CF_TdVectF>::Type() {return "StVectF";};

template<> const char* CAE_TState<CF_Rect>::Type() {return "StRect";};



_TEMPLATE_ TBool CAE_TState<CF_Rect>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_Rect>::DoOperation()
{
}

_TEMPLATE_ char* CAE_TState<CF_Rect>::DataToStr(TBool aCurr) const
{
    int buflen = Len();
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_Rect* data = (CF_Rect*) (aCurr? iCurr: iNew);
    sprintf(buf, "(%i,%i):(%i,%i)", data->iLeftUpper.iX, data->iLeftUpper.iY, data->iRightLower.iX, data->iRightLower.iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_Rect>::DataFromStr(const char* aStr, void *aData) const
{
}

_TEMPLATE_ TBool CAE_TState<CF_TdPoint>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ char* CAE_TState<CF_TdPoint>::DataToStr(TBool aCurr) const
{
    int buflen = 40;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdPoint* data = (CF_TdPoint*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%d,%d)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdPoint>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdPoint* data = (CF_TdPoint*) aData;
    sscanf(aStr, "(%d,%d)", &(data->iX), &(data->iY));
}

_TEMPLATE_ void CAE_TState<CF_TdPoint>::DoOperation()
{
}

_TEMPLATE_ TBool CAE_TState<CF_TdPointF>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_TdPointF>::DoOperation()
{
}

_TEMPLATE_ char* CAE_TState<CF_TdPointF>::DataToStr(TBool aCurr) const
{
    int buflen = 80;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdPointF* data = (CF_TdPointF*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%6.3f,%6.3f)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdPointF>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdPointF* data = (CF_TdPointF*) aData;
    sscanf(aStr, "(%f,%f)", &(data->iX), &(data->iY));
}

_TEMPLATE_ TBool CAE_TState<CF_TdVectF>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ char* CAE_TState<CF_TdVectF>::DataToStr(TBool aCurr) const
{
    int buflen = 80;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    CF_TdVectF* data = (CF_TdVectF*) (aCurr ? iCurr : iNew);
    sprintf(buf, "(%6.3f,%6.3f)", data->iX, data->iY);
    return buf;
}

_TEMPLATE_ void CAE_TState<CF_TdVectF>::DataFromStr(const char* aStr, void *aData) const
{
    CF_TdVectF* data = (CF_TdVectF*) aData;
    sscanf(aStr, "(%f,%f)", &(data->iX), &(data->iY));
}

_TEMPLATE_ void CAE_TState<CF_TdVectF>::DoOperation()
{
}


// User defined state, based on DEST expression
//
CAE_StateEx::CAE_StateEx(const string& aType, const string& aInstName, CAE_Object* aMan):
    CAE_StateBase(aInstName.c_str(), aMan, TTransInfo()), iCurr(NULL), iNew(NULL)
{
    iTypeName = strdup(aType.c_str());
    CAE_StateBase::ConstructL();
    CSL_ExprBase* expr = iMan->CreateDataExpr(aType);
    if (expr != NULL) {
	iCurr = expr->Clone();
	iNew = expr->Clone();
    }
    else {
	Logger()->WriteFormat("ERROR: Creating state [%s] of type [%s]: type constr not found", InstName(), TypeName());
    }
}

CAE_StateEx::~CAE_StateEx()
{
    if (iCurr != NULL) {
	delete iCurr; iCurr = NULL;
    }
    if (iNew != NULL) {
	delete iNew; iNew = NULL;
    }
}

char* CAE_StateEx::DataToStr(TBool aCurr) const
{
    return strdup((aCurr ? iCurr : iNew)->ToString().c_str());
}

void CAE_StateEx::DataFromStr(const char* aStr, void *aData) const
{
    _FAP_ASSERT(EFalse); // Not supported
}

void CAE_StateEx::DoSet(void* aData)
{
    CAE_EBase* data = (CAE_EBase*) aData;
    CSL_ExprBase* expr = data->GetFbObj(expr);
    _FAP_ASSERT(expr != NULL);
    delete iNew;
    iNew = expr->Clone();
    TInt logdata = GetLogSpecData(KBaseLe_Updated);
    if (logdata != KBaseDa_None)
	LogUpdate(logdata);
}

void CAE_StateEx::DoSetFromStr(const char *aStr)
{
    iMan->DoTrans(this, aStr);
}

void CAE_StateEx::Confirm()
{
    if (iCurr != NULL && iNew != NULL && !(*iCurr == *iNew)) {
	CAE_ConnPoint *outp = iOutput->GetFbObj(outp);
	_FAP_ASSERT(outp != NULL);
	for (TInt i = 0; i < outp->Dests().size(); i++) {
	    CAE_StateBase* sout =  outp->Slot(i)->Pin("_1")->GetFbObj(sout);
	    _FAP_ASSERT(sout != NULL);
	    sout->SetActive();
	}
	delete iCurr;
	iCurr = iNew->Clone();
    }
}

// From CAE_Base
void *CAE_StateEx::DoGetFbObj(const char *aName)
{
    if (strcmp(aName, Type()) == 0)
	return this;
    else
	return CAE_StateBase::DoGetFbObj(aName);
}
