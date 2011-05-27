
// Finite automata programming extension, level 1 prototype 
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "fapfact.h"
#include "fapbase.h"
#include "fapstext.h"
#include "faplogger.h"
#include "panics.h"
#include "tadesl.h"

// XML CAE spec parameters
// Element types
const char* KCaeElTypeObject = "object";
const char* KCaeElTypeState = "state";
const char* KCaeElTypeStateMut = "state_mut";
const char* KCaeElTypeConn = "conn";
const char* KCaeElTypeLogspec = "logspec";
const char* KCaeElTypeLogdata = "logdata";
const char* KCaeElTypeDep = "dep";
const char* KCaeElTypeStinp = "inp";
const char* KCaeElTypeSoutp = "out";
const char* KCaeElTypeCpsource = "src";
const char* KCaeElTypeCpdest = "dest";
const char* KCaeElTypeCext = "ext";
const char* KCaeElTypeCextc = "extc";
const char* KCaeElTypeCextcSrc = "srcext";
const char* KCaeElTypeCextcDest = "dstext";

// Base states registered by default
const TStateInfo KSinfo_State = TStateInfo("State", (TStateFactFun) CAE_State::NewL );
const TStateInfo KSinfo_StBool = TStateInfo("StBool", (TStateFactFun) CAE_TState<TBool>::NewL );
const TStateInfo KSinfo_StInt = TStateInfo("StInt", (TStateFactFun) CAE_TState<TInt>::NewL );
const TStateInfo KSinfo_StUint8 = TStateInfo("StUint8", (TStateFactFun) CAE_TState<TUint8>::NewL );
const TStateInfo KSinfo_StUint32 = TStateInfo("StUint32", (TStateFactFun) CAE_TState<TUint32>::NewL );
const TStateInfo KSinfo_Point = TStateInfo("StPoint", (TStateFactFun) CAE_TState<CF_TdPoint>::NewL );
const TStateInfo KSinfo_PointF = TStateInfo("StPointF", (TStateFactFun) CAE_TState<CF_TdPointF>::NewL );
const TStateInfo KSinfo_VectF = TStateInfo("StVectF", (TStateFactFun) CAE_TState<CF_TdVectF>::NewL );
const TStateInfo KSinfo_Rect = TStateInfo("StRect", (TStateFactFun) CAE_TState<CF_Rect>::NewL );
const TStateInfo KSinfo_Contr = TStateInfo("StContr", (TStateFactFun) CAE_StateCtr::New );

static const TStateInfo* sinfos[] = {&KSinfo_State, &KSinfo_StBool, &KSinfo_StInt, &KSinfo_StUint8, &KSinfo_StUint32, 
    &KSinfo_Point, &KSinfo_PointF, &KSinfo_VectF, &KSinfo_Rect, &KSinfo_Contr, 
    NULL};

//*********************************************************
// Model of XML based chromo
//*********************************************************

static map<string, NodeType> KNodeTypes;
static map<NodeType, string> KNodeTypesNames;
static map<TNodeAttr, string> KNodeAttrsNames;
static map<string, TNodeAttr> KNodeAttrs;

class CAE_ChromoMdlX: public CAE_ChromoMdlBase
{
    public:
	CAE_ChromoMdlX();
    public:
	virtual NodeType GetType(const string& aId);
	virtual NodeType GetType(const void* aHandle);
	virtual string GetTypeId(NodeType aType);
	virtual void* Next(const void* aHandle, NodeType aType = ENt_Unknown);
	virtual void* NextText(const void* aHandle);
	virtual void* Prev(const void* aHandle, NodeType aType = ENt_Unknown);
	virtual void* GetFirstChild(const void* aHandle, NodeType aType = ENt_Unknown);
	virtual void* GetLastChild(const void* aHandle, NodeType aType = ENt_Unknown);
	virtual void* GetFirstTextChild(const void* aHandle);
	virtual char *GetAttr(const void* aHandle, TNodeAttr aAttr);
	virtual char* GetContent(const void* aHandle);
	virtual TBool AttrExists(const void* aHandle, TNodeAttr aAttr);
	virtual TNodeAttr GetAttrNat(const void* aHandle, TNodeAttr aAttr);
	virtual NodeType GetAttrNt(const void* aHandle, TNodeAttr aAttr);
	virtual void* AddChild(void* aParent, NodeType aNode);
	virtual void* AddChild(void* aParent, const void* aHandle);
	virtual void RmChild(void* aParent, void* aChild);
	virtual void SetAttr(void* aNode, TNodeAttr aType, const char* aVal);
	virtual void SetAttr(void* aNode, TNodeAttr aType, NodeType aVal);
	virtual void SetAttr(void* aNode, TNodeAttr aType, TNodeAttr aVal);
	virtual void Dump(void* aNode, MCAE_LogRec* aLogRec);
	virtual void Save(const string& aFileName) const;
    public:
	int GetAttrInt(void *aHandle, const char *aName);
	void* Set(const char* aFileName);
	void* Set(CAE_ChromoMdlX& aMdl, const void* aHandle);
	xmlDoc* Doc() { return iDoc;};
	static inline const char *Type() { return "ChromoMdlX";}; 
	virtual void* Init(NodeType aRootType);
	void Reset();
    protected:
	virtual void *DoGetFbObj(const char *aName);
    private:
	xmlDoc *iDoc;	// XML document
	TBool iDocOwned;
};

CAE_ChromoMdlX::CAE_ChromoMdlX(): iDoc(NULL), iDocOwned(EFalse)
{
    if (KNodeTypes.size() == 0)
    {
	KNodeTypes["iobject"] = ENt_Object;
	KNodeTypes["object"] = ENt_Robject;
	KNodeTypes["state"] = ENt_State;
	KNodeTypes["conn"] = ENt_Conn;
	KNodeTypes["logspec"] = ENt_Logspec;
	KNodeTypes["dep"] = ENt_Dep;
	KNodeTypes["logdata"] = ENt_Logdata;
	KNodeTypes["inp"] = ENt_Stinp;
	KNodeTypes["mut"] = ENt_Mut;
	KNodeTypes["out"] = ENt_Soutp;
	KNodeTypes["src"] = ENt_CpSource;
	KNodeTypes["dest"] = ENt_CpDest;
	KNodeTypes["ext"] = ENt_Cext;
	KNodeTypes["extc"] = ENt_Cextc;
	KNodeTypes["srcext"] = ENt_CextcSrc;
	KNodeTypes["dstext"] = ENt_CextcDest;
	KNodeTypes["caeenv"] = ENt_Env;
	KNodeTypes["add"] = ENt_MutAdd;
	KNodeTypes["rm"] = ENt_MutRm;
	KNodeTypes["change"] = ENt_MutChange;
	KNodeTypes["node"] = ENt_Node;
	KNodeTypes["chnode"] = ENt_ChNode;
	KNodeTypes["trans"] = ENt_Trans;
	KNodeTypes["state_inp"] = ENt_MutAddStInp;

	for (map<string, NodeType>::const_iterator it = KNodeTypes.begin(); it != KNodeTypes.end(); it++) {
	    KNodeTypesNames[it->second] = it->first;
	}
    }
    if (KNodeAttrsNames.size() == 0)
    {
	KNodeAttrsNames[ENa_Id] = "id";
	KNodeAttrsNames[ENa_Type] = "type";
	KNodeAttrsNames[ENa_ObjQuiet] = "quiet";
	KNodeAttrsNames[ENa_Transf] = "transf";
	KNodeAttrsNames[ENa_StInit] = "init";
	KNodeAttrsNames[ENa_StLen] = "len";
	KNodeAttrsNames[ENa_Logevent] = "event";
	KNodeAttrsNames[ENa_PinType] = "type";
	KNodeAttrsNames[ENa_ConnPair] = "pair";
	KNodeAttrsNames[ENa_MutNode] = "node";
	KNodeAttrsNames[ENa_MutChgAttr] = "attr";
	KNodeAttrsNames[ENa_MutChgVal] = "val";

	for (map<TNodeAttr, string>::const_iterator it = KNodeAttrsNames.begin(); it != KNodeAttrsNames.end(); it++) {
	    KNodeAttrs[it->second] = it->first;
	}
    }
};

void *CAE_ChromoMdlX::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this : NULL;
}

void* CAE_ChromoMdlX::Init(NodeType aRootType)
{
    iDocOwned = ETrue;
    iDoc = xmlNewDoc((const xmlChar*) "1.0");
    string sroottype = KNodeTypesNames[aRootType];
    xmlNodePtr root = xmlNewNode(NULL, (const xmlChar *) sroottype.c_str());
    xmlDocSetRootElement(iDoc, root);
    return root;
}

void CAE_ChromoMdlX::Reset()
{
    if (iDocOwned) {
	xmlFreeDoc(iDoc);
    }
    iDoc = NULL;
}

void* CAE_ChromoMdlX::Set(const char *aFileName)
{
    xmlNode *sEnv = NULL; // Node of environment element
    xmlNode *sRoot = NULL; // Node of root element
    // Read and parse the CAE spec file
    iDoc = xmlReadFile(aFileName, NULL, XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID);
    _FAP_ASSERT(iDoc != NULL);
    // Get the node of environment, not used for now
//    sEnv = (xmlNodePtr) GetFirstChild((void *) iDoc, ENt_Env);
//    _FAP_ASSERT(sEnv != NULL);
//    sRoot = (xmlNodePtr) GetFirstChild((void *) sEnv, ENt_Object);
    sRoot = (xmlNodePtr) GetFirstChild((void *) iDoc, ENt_Object);
    iDocOwned = EFalse;
    return sRoot;
}

void* CAE_ChromoMdlX::Set(CAE_ChromoMdlX& aMdl, const void* aNode)
{
    iDoc = aMdl.Doc();
    xmlNodePtr node = xmlDocCopyNode((xmlNodePtr) aNode, iDoc, 1);
    return node;
}

NodeType CAE_ChromoMdlX::GetType(const string& aId)
{
    return (KNodeTypes.count(aId) == 0) ? ENt_Unknown: KNodeTypes[aId];
}

string CAE_ChromoMdlX::GetTypeId(NodeType aType)
{
    return (aType == ENt_Unknown) ? "" : KNodeTypesNames[aType];
}

NodeType CAE_ChromoMdlX::GetType(const void* aHandle)
{
    xmlNodePtr node = (xmlNodePtr) aHandle;
    const char* type_name = (const char*) node->name;
    return (KNodeTypes.count(type_name) == 0) ? ENt_Unknown: KNodeTypes[type_name];
}

void *CAE_ChromoMdlX::GetFirstChild(const void *aHandle, NodeType aType)
{
    xmlNodePtr node = (xmlNodePtr) aHandle;
    _FAP_ASSERT(node != NULL);
    xmlNodePtr res = node->children;
    if (res != NULL) {
	NodeType type = GetType((void*) res);
	if ((res->type != XML_ELEMENT_NODE) || ((aType != ENt_Unknown) ? (type != aType) : (type == ENt_Unknown)))
	    res = (xmlNodePtr) Next(res, aType);
    }
    return res;
}

void *CAE_ChromoMdlX::GetLastChild(const void *aHandle, NodeType aType)
{
    xmlNodePtr node = (xmlNodePtr) aHandle;
    _FAP_ASSERT(node != NULL);
    xmlNodePtr res = node->last;
    if (res != NULL) {
	NodeType type = GetType((void*) res);
	if ((res->type != XML_ELEMENT_NODE) || ((aType != ENt_Unknown) ? (type != aType) : (type == ENt_Unknown)))
	    res = (xmlNodePtr) Prev(res, aType);
    }
    return res;
}

void* CAE_ChromoMdlX::GetFirstTextChild(const void* aHandle)
{
    xmlNodePtr node = (xmlNodePtr) aHandle;
    _FAP_ASSERT(node != NULL);
    xmlNodePtr res = node->children;
    if (res != NULL) {
	if (res->type != XML_TEXT_NODE)
	    res = (xmlNodePtr) NextText(res);
    }
    return res;
}

void *CAE_ChromoMdlX::Next(const void *aHandle, NodeType aType)
{
    _FAP_ASSERT(aHandle!= NULL);
    xmlNodePtr res = ((xmlNodePtr) aHandle)->next;
    if (res != NULL) {
	NodeType type = GetType((void*) res);
	while ((res != NULL) && ((res->type != XML_ELEMENT_NODE) || ((aType != ENt_Unknown) ? (type != aType) : (type == ENt_Unknown)))) {
	    res = res->next;
	    if (res != NULL)
		type = GetType((void*) res);
	}
    }
    return res;
}

void* CAE_ChromoMdlX::NextText(const void* aHandle)
{
    _FAP_ASSERT(aHandle!= NULL);
    xmlNodePtr res = ((xmlNodePtr) aHandle)->next;
    if (res != NULL) {
	while ((res != NULL) && (res->type != XML_TEXT_NODE)) {
	    res = res->next;
	}
    }
    return res;
}

void *CAE_ChromoMdlX::Prev(const void *aHandle, NodeType aType)
{
    _FAP_ASSERT(aHandle!= NULL);
    xmlNodePtr res = ((xmlNodePtr) aHandle)->prev;
    NodeType type = GetType((void*) res);
    while ((res != NULL) && ((res->type != XML_ELEMENT_NODE) || ((aType != ENt_Unknown) ? (type != aType) : (type == ENt_Unknown))))
       	res = res->prev;
    return res;
}

char *CAE_ChromoMdlX::GetAttr(const void* aHandle, TNodeAttr aAttr)
{
    _FAP_ASSERT(aHandle != NULL);
    xmlNodePtr node = (xmlNodePtr) aHandle;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) KNodeAttrsNames[aAttr].c_str());
    return (char *) attr;
}

TBool CAE_ChromoMdlX::AttrExists(const void* aHandle, TNodeAttr aAttr)
{
    _FAP_ASSERT(aHandle != NULL);
    TBool res = EFalse;
    xmlNodePtr node = (xmlNodePtr) aHandle;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) KNodeAttrsNames[aAttr].c_str());
    res = (attr != NULL);
    free (attr);
    return res;
}

char* CAE_ChromoMdlX::GetContent(const void* aHandle)
{
    _FAP_ASSERT(aHandle != NULL);
    xmlNodePtr node = (xmlNodePtr) aHandle;
    xmlChar *cont = xmlNodeGetContent(node);
    return (char *) cont;
}

int CAE_ChromoMdlX::GetAttrInt(void *aHandle, const char *aName)
{
    int res = -1;
    _FAP_ASSERT(aHandle!= NULL);
    xmlNodePtr node = (xmlNodePtr) aHandle;
    xmlChar *attr = xmlGetProp(node, (const xmlChar *) aName);
    _FAP_ASSERT(attr != NULL);
    res = atoi((const char *) attr);
    free(attr);
    return res;
}

TNodeAttr CAE_ChromoMdlX::GetAttrNat(const void* aHandle, TNodeAttr aAttr)
{
    TNodeAttr res = ENa_Unknown;
    char* stattr = GetAttr(aHandle, aAttr);
    if (KNodeAttrs.count(stattr) > 0) {
	res = KNodeAttrs[stattr];
    }
    free(stattr);
    return res;
}

NodeType CAE_ChromoMdlX::GetAttrNt(const void* aHandle, TNodeAttr aAttr)
{
    NodeType res = ENt_Unknown;
    char* str = GetAttr(aHandle, aAttr);
    if (str != NULL && KNodeTypes.count(str) > 0) {
	res = KNodeTypes[str];
    }
    free(str);
    return res;

}

void* CAE_ChromoMdlX::AddChild(void* aParent, NodeType aNode)
{
    string name = KNodeTypesNames[aNode];
    xmlNodePtr node = xmlNewNode(NULL, (const xmlChar*) name.c_str());
    return xmlAddChild((xmlNodePtr) aParent, node);
}

void* CAE_ChromoMdlX::AddChild(void* aParent, const void* aHandle)
{
    xmlNodePtr node = xmlCopyNode((xmlNodePtr) aHandle, 1);
    return xmlAddChild((xmlNodePtr) aParent, node);
}

void CAE_ChromoMdlX::SetAttr(void* aNode, TNodeAttr aType, const char* aVal)
{
    string name = KNodeAttrsNames[aType];
    if (AttrExists(aNode, aType)) {
	xmlSetProp((xmlNodePtr) aNode, (const xmlChar*) name.c_str(), (const xmlChar*) aVal);
    }
    else {
	xmlNewProp((xmlNodePtr) aNode, (const xmlChar*) name.c_str(), (const xmlChar*) aVal);
    }
}

void CAE_ChromoMdlX::SetAttr(void* aNode, TNodeAttr aType, NodeType aVal)
{
    SetAttr(aNode, aType, KNodeTypesNames[aVal].c_str());
}

void CAE_ChromoMdlX::SetAttr(void* aNode, TNodeAttr aType, TNodeAttr aVal)
{
    SetAttr(aNode, aType, KNodeAttrsNames[aVal].c_str());
}

void CAE_ChromoMdlX::RmChild(void* aParent, void* aChild)
{
    xmlNodePtr child = (xmlNodePtr) aChild;
    xmlUnlinkNode(child);
    xmlFreeNode(child);
}

void CAE_ChromoMdlX::Dump(void* aNode, MCAE_LogRec* aLogRec)
{
    xmlBufferPtr bufp = xmlBufferCreate();	
    int	res = xmlNodeDump(bufp, iDoc, (xmlNodePtr) aNode, 0, 0);
    aLogRec->WriteFormat("%s", xmlBufferContent(bufp));
}

void CAE_ChromoMdlX::Save(const string& aFileName) const
{
    int res = xmlSaveFile(aFileName.c_str(), iDoc);
}


//*********************************************************
// XML based chromo
//*********************************************************

class CAE_ChromoX: public CAE_ChromoBase
{
    public:
	CAE_ChromoX();
	CAE_ChromoX(const char *aFileName);
	virtual ~CAE_ChromoX();
    public:
	virtual CAE_ChromoNode& Root();
	virtual const CAE_ChromoNode& Root() const;
	virtual void Set(const char *aFileName);
	virtual void Set(const CAE_ChromoNode& aRoot);
	virtual void Init(NodeType aRootType);
	virtual void Reset();
	virtual void Save(const string& aFileName) const;
    private:
	CAE_ChromoMdlX iMdl;
	CAE_ChromoNode iRootNode;
};


CAE_ChromoX::CAE_ChromoX(): iRootNode(iMdl, NULL)
{
}

void CAE_ChromoX::Set(const char *aFileName)
{
    void *root = iMdl.Set(aFileName);
    iRootNode = CAE_ChromoNode(iMdl, root);
}

void CAE_ChromoX::Set(const CAE_ChromoNode& aRoot)
{
    CAE_ChromoMdlX *mdl = aRoot.Mdl().GetFbObj(mdl);
    _FAP_ASSERT(mdl != NULL);
    void *root = iMdl.Set(*mdl, aRoot.Handle());
    iRootNode = CAE_ChromoNode(iMdl, root);
}

CAE_ChromoX::~CAE_ChromoX()
{
}

CAE_ChromoNode& CAE_ChromoX::Root()
{
    return iRootNode;
}

const CAE_ChromoNode& CAE_ChromoX::Root() const
{
    return iRootNode;
}

void CAE_ChromoX::Reset()
{
    iMdl.Reset();
}

void CAE_ChromoX::Init(NodeType aRootType)
{
    void *root = iMdl.Init(aRootType);
    iRootNode = CAE_ChromoNode(iMdl, root);
}

void CAE_ChromoX::Save(const string& aFileName) const
{
    iMdl.Save(aFileName);
}


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
    iStateInfos = new vector<const TStateInfo*>;
    iFormatters = new vector<CAE_Formatter*>;
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
    if (iStateInfos != NULL) {
	delete iStateInfos;
	iStateInfos = NULL;
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

CAE_StateBase* CAE_ProviderGen::CreateStateL(const char *aType, const char* aInstName, CAE_Object* aMan) const
{
    CAE_StateBase* res = NULL;
    const TStateInfo *info = GetStateInfo(aType);
    if (info != NULL) {
	res = info->iFactFun(aInstName, aMan, TTransInfo());
    }
    else {
	res = new CAE_StateEx(aType, aInstName, aMan);

    }
    return res;

}

CAE_EBase* CAE_ProviderGen::CreateObjectL(TUint32 aTypeUid) const
{
    CAE_EBase* res = NULL;
    return res;
}

FAPWS_API  CAE_EBase* CAE_ProviderGen::CreateObjectL(const char *aName) const
{
    CAE_EBase* res = NULL;
    return res;
}

const TStateInfo* CAE_ProviderGen::GetStateInfo(const char *aType) const
{
    const TStateInfo *res = NULL;
    int count = iStateInfos->size();
    for (int i = 0; i < count; i++)
    {
	const TStateInfo* info =   static_cast<const TStateInfo*>(iStateInfos->at(i));
	if (strcmp(info->iType, aType) == 0)
	{
	    res = info;
	    break;
	}
    }
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

void CAE_ProviderGen::RegisterTransfs(const TTransInfo **aTransfs)
{
    _FAP_ASSERT(aTransfs != NULL);
    for (int i = 0; aTransfs[i] != NULL; i++) {
	iTransfs->push_back(aTransfs[i]);
    }
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

void CAE_ProviderGen::RegisterState(const TStateInfo *aInfo)
{
    _FAP_ASSERT(aInfo != NULL);
    iStateInfos->push_back(aInfo);
}

void CAE_ProviderGen::RegisterStates(const TStateInfo **aInfos)
{
    _FAP_ASSERT(aInfos != NULL);
    for (int i = 0; aInfos[i] != NULL; i++) {
	iStateInfos->push_back(aInfos[i]);
    }
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

CAE_ChromoBase* CAE_ProviderGen::CreateChromo() const
{
    return new CAE_ChromoX();
}

CAE_TranExBase* CAE_ProviderGen::CreateTranEx(MCAE_LogRec* aLogger) const
{
    return new CAE_TaDesl(aLogger);
}

MAE_Opv* CAE_ProviderGen::CreateViewProxy()
{
    return NULL;
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
	virtual TCaeElemType FapType(void *aElement);
	virtual TCaeMut MutType(void *aElement);
	virtual char *GetStrAttr(void *aSpec, const char *aName);
	virtual int GetAttrInt(void *aSpec, const char *aName);
    protected:
	void Construct(const char *aFileName);
	CAE_ChroManX();
    private:
	static xmlNodePtr GetNode(xmlNodePtr aNode, const char *aName, xmlElementType aType = XML_ELEMENT_NODE, bool aChild = false);
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
    iDoc = xmlReadFile(aFileName, NULL, XML_PARSE_DTDLOAD | XML_PARSE_DTDVALID);
    _FAP_ASSERT(iDoc != NULL);
    // Get the node of environment 
    iEnv = GetNode(iDoc->children, KXmlEnvElemName);
    _FAP_ASSERT(iEnv != NULL);
}

// Searches for node (siblig or child) by name
//
xmlNodePtr CAE_ChroManX::GetNode(xmlNodePtr aNode, const char *aName, xmlElementType aType, bool aChild)
{
    xmlNodePtr res = NULL;
    xmlNodePtr iter = aChild ? aNode->children : aNode;
    for (; iter != NULL; iter = iter->next)
    {
	if ((iter->type == aType) && strcmp((char*) iter->name, aName) == 0)
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
    else if (strcmp(type, KCaeElTypeStateMut) == 0)
	res = ECae_State_Mut;
    else if (strcmp(type, KCaeElTypeConn) == 0)
	res = ECae_Conn;
    else if (strcmp(type, KCaeElTypeDep) == 0)
	res = ECae_Dep;
    else if (strcmp(type, KCaeElTypeLogspec) == 0)
	res = ECae_Logspec;
    else if (strcmp(type, KCaeElTypeLogdata) == 0)
	res = ECae_Logdata;
    else if (strcmp(type, KCaeElTypeStinp) == 0)
	res = ECae_Stinp;
    else if (strcmp(type, KCaeElTypeSoutp) == 0)
	res = ECae_Soutp;
    else if (strcmp(type, KCaeElTypeCpsource) == 0)
	res = ECae_CpSource;
    else if (strcmp(type, KCaeElTypeCpdest) == 0)
	res = ECae_CpDest;
    else if (strcmp(type, KCaeElTypeCext) == 0)
	res = ECae_Cext;
    else if (strcmp(type, KCaeElTypeCextc) == 0)
	res = ECae_Cextc;
    else if (strcmp(type, KCaeElTypeCextcSrc) == 0)
	res = ECae_CextcSrc;
    else if (strcmp(type, KCaeElTypeCextcDest) == 0)
	res = ECae_CextcDest;
    return res;
}

TCaeMut CAE_ChroManX::MutType(void *aElement)
{
    TCaeMut res = ECaeMut_None;
    xmlNodePtr node = (xmlNodePtr) aElement;
    char *mut = (char *) xmlGetProp(node, (const xmlChar *) "mut");
    if (mut == NULL) 
	res = ECaeMut_Add;
    else if (strcmp(mut, "Add") == 0)
	res = ECaeMut_Add;
    else if (strcmp(mut, "Del") == 0)
	res = ECaeMut_Del;
    else if (strcmp(mut, "Change") == 0)
	res = ECaeMut_Change;
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
    baseprov->RegisterStates(sinfos);
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

CAE_StateBase* CAE_Fact::CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const
{
    CAE_StateBase* res = NULL;

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

FAPWS_API CAE_EBase* CAE_Fact::CreateObjectL(TUint32 aTypeUid) const
{
    CAE_EBase* res = NULL;
    return res;
}

FAPWS_API CAE_EBase* CAE_Fact::CreateObjectL(const char *aName) const
{
    CAE_EBase* res = NULL;
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

void CAE_Fact::RegisterState(const TStateInfo *aInfo) {
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->RegisterState(aInfo);
    }
}

void CAE_Fact::RegisterStates(const TStateInfo **aInfos) {
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->RegisterStates(aInfos);
    }
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

void CAE_Fact::RegisterTransfs(const TTransInfo **aTrans)
{
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->RegisterTransfs(aTrans);
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

CAE_ChromoBase* CAE_Fact::CreateChromo() const
{
    if (iProviders->size() > 0)
    {
	// Register in general provider
	CAE_ProviderBase* prov = GetProviderAt(0);
	prov->CreateChromo();
    };
}

CAE_TranExBase* CAE_Fact::CreateTranEx(MCAE_LogRec* aLogger) const
{
    GetProviderAt(0)->CreateTranEx(aLogger);
}

MAE_Opv* CAE_Fact::CreateViewProxy()
{
    MAE_Opv* res = NULL;
    for (TInt i = 0; i < iProviders->size(); i++)
    {
	CAE_ProviderBase* prov = GetProviderAt(i);
	_FAP_ASSERT(prov != NULL);
	res = prov->CreateViewProxy();
	if (res != NULL)
	    break;
    }
    return res;
}
