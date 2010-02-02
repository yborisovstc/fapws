//************************************************************
// Finite automata programming extension, level 1 prototype 
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "fapfact.h"
#include "faplogger.h"
#include "panics.h"

//*********************************************************
// Base class of provider implementation
//*********************************************************


FAPWS_API CAE_ProviderBase::CAE_ProviderBase()
{
}


FAPWS_API CAE_ProviderBase::~CAE_ProviderBase()
{
}

//*********************************************************
// General provider
//*********************************************************

FAPWS_API CAE_ProviderGen::CAE_ProviderGen()
{
}

FAPWS_API CAE_ProviderGen::~CAE_ProviderGen()
{
}

FAPWS_API CAE_Base* CAE_ProviderGen::CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const
{
    CAE_Base* res = NULL;
    TUint8 rtypeid = aTypeUid & KAdp_ObjStType_TypeMsk;
    TUint8 catype = aTypeUid & KAdp_ObjStType_AccessMsk;
    CAE_StateBase::StateType satype = CAE_StateBase::EType_Reg;
    if (catype == KAdp_ObjStType_Inp)
	satype = CAE_StateBase::EType_Input;
    else if (catype == KAdp_ObjStType_Out)
	satype = CAE_StateBase::EType_Output;
    switch (rtypeid) 
    {
	case ECState_Uint8:
	    res = 	CAE_TState<TUint8>::NewL(aInstName, aMan,  TTransInfo(), satype);
	    break;
	case ECState_Uint32:
	    res = 	CAE_TState<TUint32>::NewL(aInstName, aMan,  TTransInfo(), satype);
	    break;
	default:
	    break;
    }
    return res;
}

FAPWS_API CAE_Base* CAE_ProviderGen::CreateObjectL(TUint32 aTypeUid) const
{
    CAE_Base* res = NULL;
    return res;
}

FAPWS_API  CAE_Base* CAE_ProviderGen::CreateObjectL(const char *aName) const
{
    CAE_Base* res = NULL;
    return res;
}

//*********************************************************
// Factory of element of object
//*********************************************************


FAPWS_API CAE_Fact::CAE_Fact(): 
    iProviders(NULL), iChroman(NULL)
{
}

FAPWS_API CAE_Fact::~CAE_Fact()
{
    if (iProviders != NULL)
    {
	TInt count = iProviders->size();
	for (TInt i = 0; i < count; i++)
	{
	    CAE_ProviderBase* elem = GetProviderAt(i);
	    if (elem != NULL)
	    {
		delete elem;
	    }
	}
	delete iProviders;
	iProviders = NULL;
    }
    if (iChroman != NULL)
    {
	delete iChroman;
	iChroman = NULL;
    }
}

FAPWS_API void CAE_Fact::ConstructL()
{
    iProviders = new vector<CAE_ProviderBase*>;
    CAE_ProviderBase* baseprov = new CAE_ProviderGen();
    AddProviderL(baseprov);
}

FAPWS_API CAE_Fact* CAE_Fact::NewL()
{
    CAE_Fact* self = new CAE_Fact();
    self->ConstructL();
    return self;
}

CAE_ProviderBase* CAE_Fact::GetProviderAt(TInt aInd) const
{
    _FAP_ASSERT(aInd >= 0 && aInd <= iProviders->size());
    return  static_cast<CAE_ProviderBase*>(iProviders->at(aInd));
}

FAPWS_API void CAE_Fact::AddProviderL(CAE_ProviderBase* aProv)
{
    _FAP_ASSERT(aProv != NULL);
    iProviders->push_back(aProv);

}


FAPWS_API CAE_Base* CAE_Fact::CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const
{
    CAE_Base* res = NULL;
    TInt count = iProviders->size();
    for (TInt i = 0; i < count; i++)
    {
	CAE_ProviderBase* prov = GetProviderAt(i);
	_FAP_ASSERT(prov != NULL);
	res = prov->CreateStateL(aTypeUid, aInstName, aMan);
	if (res != NULL)
	    break;
    }
    return res;
}

FAPWS_API CAE_Base* CAE_Fact::CreateObjectL(TUint32 aTypeUid) const
{
    CAE_Base* res = NULL;
    return res;
}

FAPWS_API CAE_Base* CAE_Fact::CreateObjectL(const char *aName) const
{
    CAE_Base* res = NULL;
    if (iChroman != NULL)
    {
    }
    return res;
}

FAPWS_API MAE_ChroMan* CAE_Fact::Chman() const 
{ 
    return iChroman;
}

FAPWS_API void CAE_Fact::AddChmanXml(const char *aXmlFileName)
{
    // Create Chromosome manager. There is only one for now.
    // So don't provide the functionality of selecting of manager
    // TODO [YB] Consider adding selection of manager
    _FAP_ASSERT(iChroman == NULL);
    iChroman = CAE_ChroManX::New(aXmlFileName);
}

//*********************************************************
// Chromosome manager for XML based chromosome
//*********************************************************

CAE_ChroManX *CAE_ChroManX::New(const char *aFileName)
{
    CAE_ChroManX *self = new CAE_ChroManX();
    self->Construct(aFileName);
    return self;
}

CAE_ChroManX::~CAE_ChroManX()
{
}

void CAE_ChroManX::Copy(const void *aSrc, void *aDest)
{
}

CAE_ChroManX::CAE_ChroManX(): iDoc(NULL)
{
}

void CAE_ChroManX::Construct(const char *aFileName)
{
    iDoc = xmlReadFile(aFileName, NULL, 0);
}

