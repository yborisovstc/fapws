
// Finite automata programming extension, level 1 prototype 
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "fapfact.h"
#include "faplogger.h"
#include "panics.h"

// XML CAE spec parameters
const char* KCaeElTypeObject = "object";
const char* KCaeElTypeState = "state";
const char* KCaeElTypeTransf = "transf";
const char* KCaeElTypeLogspec = "logspec";
const char* KCaeElTypeLogdata = "logdata";
const char* KCaeElTypeDep = "dep";

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
    iTransfs = new vector<const TTransInfo*>;
    iFormatters = new vector<CAE_Formatter*>;
    // Register formatterw
    RegisterFormatter(CAE_TState<TUint32>::DataTypeUid(), CAE_TState<TUint32>::LogFormFun);
    RegisterFormatter(CAE_TState<TBool>::DataTypeUid(), CAE_TState<TBool>::LogFormFun);
}

FAPWS_API CAE_ProviderGen::~CAE_ProviderGen()
{
    if (iTransfs != NULL)
    {
    /* YB no need as trans info is constant ref for now
	TInt count = iTransfs->size();
	for (TInt i = 0; i < count; i++)
	{
	    TTransInfo* elem = static_cast<TTransInfo*>(iTransfs->at(i));
	    if (elem != NULL)
	    {
		delete elem;
	    }
	}
	*/
	delete iTransfs;
	iTransfs = NULL;
    }
    if (iFormatters != NULL)
    {
	TInt count = iFormatters->size();
	for (TInt i = 0; i < count; i++)
	{
	    CAE_Formatter* elem = static_cast<CAE_Formatter*>(iFormatters->at(i));
	    if (elem != NULL)
	    {
		delete elem;
	    }
	}
	delete iFormatters;
	iFormatters = NULL;
    }

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

FAPWS_API CAE_Base* CAE_ProviderGen::CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const
{
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

const TTransInfo* CAE_ProviderGen::GetTransf(const char *aName) const
{
    _FAP_ASSERT(aName != NULL);
    const TTransInfo *res = NULL;
    int count = iTransfs->size();
    for (int i = 0; i < count; i++)
    {
	const TTransInfo* trans =   static_cast<const TTransInfo*>(iTransfs->at(i));
	if (strcmp(trans->iId, aName) == 0)
	{
	    res = trans;
	    break;
	}
    }
    return res;
}

void CAE_ProviderGen::RegisterTransf(const TTransInfo *aTrans)
{
    _FAP_ASSERT(aTrans != NULL);
    iTransfs->push_back(aTrans);
}

const CAE_Formatter* CAE_ProviderGen::GetFormatter(int aUid) const
{
    const CAE_Formatter *res = NULL;
    int count = iFormatters->size();
    for (int i = 0; i < count; i++)
    {
	const CAE_Formatter* elem =   static_cast<const CAE_Formatter*>(iFormatters->at(i));
	if (elem->iStateDataUid == aUid)
	{
	    res = elem;
	    break;
	}
    }
    return res;

}

void CAE_ProviderGen::RegisterFormatter(CAE_Formatter *aForm)
{
    _FAP_ASSERT(aForm != NULL);
    iFormatters->push_back(aForm);
}

void CAE_ProviderGen::RegisterFormatter(int aUid, TLogFormatFun aFun)
{
    RegisterFormatter(new CAE_Formatter(aUid, aFun));
}

//*********************************************************
// Chromosome manager for XML based chromosome
//*********************************************************

// Name of CAE environment XML element (root)
const char *KXmlEnvElemName = "caeenv";

// Chromosome manager for XML based chromosome
class CAE_ChroManX: public CAE_ChroManBase
{
    public:
	static CAE_ChroManX *New(const char *aFileName);
	virtual ~CAE_ChroManX();
	// Copies spec of CAE element
	virtual void CopySpec(const void *aSrc, void **aDest);
	virtual void *GetChild(void *aSpec);
	virtual void *GetNext(void *aChildSpec);
	virtual char *GetType(void *aSpec);
	virtual char *GetName(void *aSpec);
	virtual int GetLen(void *aSpec);
	virtual CAE_StateBase::StateType GetAccessType(void *aSpec);
	virtual TCaeElemType FapType(void *aElement);
	virtual char *GetStrAttr(void *aSpec, const char *aName);
	virtual int GetAttrInt(void *aSpec, const char *aName);
    protected:
	void Construct(const char *aFileName);
	CAE_ChroManX();
    private:
	static xmlNodePtr GetNode(xmlNodePtr aNode, const char *aName, bool aChild = NULL);
    private:
	xmlDoc *iDoc;	// XML document
	xmlNode *iEnv; // Node of environment element
};



CAE_ChroManX *CAE_ChroManX::New(const char *aFileName)
{
    CAE_ChroManX *self = new CAE_ChroManX();
    self->Construct(aFileName);
    return self;
}

CAE_ChroManX::~CAE_ChroManX()
{
}

void CAE_ChroManX::CopySpec(const void *aSrc, void **aDest)
{
    xmlNodePtr node = xmlDocCopyNode((xmlNodePtr) aSrc, iDoc, 1);
    *aDest = node;
}

CAE_ChroManX::CAE_ChroManX(): iDoc(NULL)
{
}

void CAE_ChroManX::Construct(const char *aFileName)
{
    // Read and parse the CAE spec file
    iDoc = xmlReadFile(aFileName, NULL, 0);
    _FAP_ASSERT(iDoc != NULL);
    // Get the node of environment 
    iEnv = GetNode(iDoc->children, KXmlEnvElemName);
    _FAP_ASSERT(iEnv != NULL);
}

// Searches for node (siblig or child) by name
//
xmlNodePtr CAE_ChroManX::GetNode(xmlNodePtr aNode, const char *aName, bool aChild)
{
    xmlNodePtr res = NULL;
    xmlNodePtr iter = aChild ? aNode->children : aNode;
    for (; iter != NULL; iter = iter->next)
    {
	if (strcmp((char*) iter->name, aName) == 0)
	{
	    res = iter;
	    break;
	}
    }
    return res;
}

void *CAE_ChroManX::GetChild(void *aSpec)
{
    xmlNodePtr node = aSpec ? (xmlNodePtr) aSpec : iEnv;
    _FAP_ASSERT(node != NULL);
    xmlNodePtr res = node->children;
    if ((res != NULL) && ((res->type != XML_ELEMENT_NODE) || (FapType(res) == ECae_Unknown)))
       	res = (xmlNodePtr) GetNext(res);
    return res;
}

void *CAE_ChroManX::GetNext(void *aChildSpec)
{
    _FAP_ASSERT(aChildSpec!= NULL);
    xmlNodePtr res = ((xmlNodePtr) aChildSpec)->next;
    while ((res != NULL) && ((res->type != XML_ELEMENT_NODE) || (FapType(res) == ECae_Unknown)))
       	res = res->next;
    return res;
}

TCaeElemType CAE_ChroManX::FapType(void *aElement)
{
    TCaeElemType res = ECae_Unknown;
    xmlNodePtr node = (xmlNodePtr) aElement;
    const char* type = (const char*) node->name;
    if (strcmp(type, KCaeElTypeObject) == 0)
	res = ECae_Object;
    else if (strcmp(type, KCaeElTypeState) == 0)
	res = ECae_State;
    else if (strcmp(type, KCaeElTypeTransf) == 0)
	res = ECae_Transf;
    else if (strcmp(type, KCaeElTypeDep) == 0)
	res = ECae_Dep;
    else if (strcmp(type, KCaeElTypeLogspec) == 0)
	res = ECae_Logspec;
    else if (strcmp(type, KCaeElTypeLogdata) == 0)
	res = ECae_Logdata;
    return res;
}

char *CAE_ChroManX::GetType(void *aSpec)
{
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) "type");
    return (char *) attr;
}

char *CAE_ChroManX::GetName(void *aSpec)
{
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) "id");
    return (char *) attr;
}

int CAE_ChroManX::GetLen(void *aSpec)
{
    int res = -1;
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) "len");
    if (attr != NULL)
    {
	res = atoi((const char *) attr);
    }
    return res;
}

CAE_StateBase::StateType CAE_ChroManX::GetAccessType(void *aSpec)
{
    CAE_StateBase::StateType res = CAE_StateBase::EType_Unknown;
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    const char *attr = (const char *) xmlGetProp(node, (const xmlChar *) "access");
    if (attr != NULL)
    {
	if (strcmp(attr, "Reg") == 0)
	    res = CAE_StateBase::EType_Reg;
	else if (strcmp(attr,"Inp") == 0)
	    res = CAE_StateBase::EType_Input;
	else if (strcmp(attr,"Out") == 0)
	    res = CAE_StateBase::EType_Output;
    }
    return res;
}

CAE_ChroManBase *CAE_ChroManXFact::CreateChroManX(const char *aFileName)
{
    return CAE_ChroManX::New(aFileName);
}

char *CAE_ChroManX::GetStrAttr(void *aSpec, const char *aName)
{
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) aName);
    return (char *) attr;
}

int CAE_ChroManX::GetAttrInt(void *aSpec, const char *aName)
{
    int res = -1;
    _FAP_ASSERT(aSpec!= NULL);
    xmlNodePtr node = (xmlNodePtr) aSpec;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) aName);
    if (attr != NULL)
    {
	res = atoi((const char *) attr);
    }
    return res;
}


//*********************************************************
// Factory of element of object
//*********************************************************


FAPWS_API CAE_Fact::CAE_Fact(): 
    iProviders(NULL)
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


FAPWS_API CAE_Base* CAE_Fact::CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const
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
    return res;
}

const TTransInfo* CAE_Fact::GetTransf(const char *aName) const
{
    const TTransInfo* res = NULL;
    for (TInt i = 0; i < iProviders->size(); i++)
    {
	CAE_ProviderBase* prov = GetProviderAt(i);
	_FAP_ASSERT(prov != NULL);
	res = prov->GetTransf(aName);
	if (res != NULL)
	    break;
    }
    return res;
}

void CAE_Fact::RegisterTransf(const TTransInfo *aTrans)
{
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->RegisterTransf(aTrans);
    }
}

const CAE_Formatter* CAE_Fact::GetFormatter(int aUid) const
{
    const CAE_Formatter* res = NULL;
    for (TInt i = 0; i < iProviders->size(); i++)
    {
	CAE_ProviderBase* prov = GetProviderAt(i);
	_FAP_ASSERT(prov != NULL);
	res = prov->GetFormatter(aUid);
	if (res != NULL)
	    break;
    }
    return res;
}

void CAE_Fact::RegisterFormatter(CAE_Formatter *aForm)
{
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->RegisterFormatter(aForm);
    }
}

