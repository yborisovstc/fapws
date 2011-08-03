#ifndef __FAP_BASE_H
#define __FAP_BASE_H

//************************************************************
// Finite automata programming base, level 1 prototype  
// Yuri Borisov  21/06/05              Initial version
// Yuri Borisov  11/07/05  FAP_CR_001  Added link based on reconfiguring of inputs/outputs
// Yuri Borisov  13/07/05  FAP_CR_004  Corrected activity processing
// Yuri Borisov  13/07/05  FAP_CR_005  Added Object activity processing
// Yuri Borisov  15/07/05  FAP_CR_006  Added callback object for input transition 
// Yuri Borisov  15/07/05  FAP_CR_???  Added dinamic naming 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
// Yuri Borisov  20/07/05  FAP_CR_010  Added support of asyncronous services
// Yuri Borisov  22/07/05  FAP_CR_014  Updated activating in the CAE_StateBase::SetNew
// Yuri Borisov  25/07/05  FAP_CR_016  Update CAE_Base class to move observer from state and object
// Yuri Borisov  25/07/05  FAP_CR_017  Added operator = to the templated state
// Yuri Borisov  03/21/05  FAPW_CR_001 Added binary authomata implementation
// Yuri Borisov  13/08/06  FAP_CR_011  Adding Adaptive Programming Feature: operations
// Yuri Borisov  18-Nov-08 FAPW_CR_003 Added type identification for objects
// Yuri Borisov  15-Dec-08 FAPW_CR_011 Corrected to make compatible with fapwstst1
//*************************************************************

// Notes:
// 1 - Use current inputs state values in transition function
//		New state value = Trans_fun(current value of input states)		 

#include <stdarg.h>
#include <string.h>
#include "fapplat.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

#define _LIT(name, cnt) const char* name = cnt;
#define _LIT8(name, cnt) const char* name = cnt;

// Standard Command values
#define KCommand_On 1
#define KCommand_Off 0

#define KRequestPending -1

#define CAE_TRANS_DEF(name, class) void name(CAE_StateBase* aState);\
static void name##_S_(CAE_Object* aObject, CAE_StateBase* aState) {((class*) aObject)->name(aState);};

#define CAE_TRANS(name) name##_S_

// Type of specification CAE element
enum TCaeElemType
{
    ECae_Unknown = 0,
    ECae_Object = 1,
    ECae_State = 2,
    ECae_Conn = 3,	// Connection
    ECae_Logspec = 4,	// Logging specification
    ECae_Dep = 5,	// Dependency
    ECae_Logdata = 6,	// Logging data
    ECae_Stinp = 7,	// State/system input
    ECae_State_Mut = 8,	// State mutation
    ECae_Soutp = 9,	// State/system output
    ECae_CpSource = 10,	// Connection point source
    ECae_CpDest = 11,	// Connection point destination
    ECae_Cext = 12,	// Connecting extention
    ECae_Cextc = 13,	// Custom Connecting extention
    ECae_CextcSrc = 14,	// Custom Connecting extention src spec
    ECae_CextcDest = 15	// Custom Connecting extention dest spec
};

// CAE elements mutation type
enum TCaeMut
{
    ECaeMut_None = 0,
    ECaeMut_Add = 1,	// Add 
    ECaeMut_Del = 2,	// Delete 
    ECaeMut_Change = 3	// Change 
};


enum TNodeType
{
    ENt_Unknown = 0,
    ENt_Object = 1,
    ENt_State = 2,
    ENt_Conn = 3,	   // Connection
    ENt_Logspec = 4,  // Logging specification
    ENt_Dep = 5,      // Dependency
    ENt_Logdata = 6,  // Logging data
    ENt_Stinp = 7,    // State/system input
    ENt_Mut = 8,      // Mutation
    ENt_Soutp = 9,    // State/system output
    ENt_CpSource = 10,// Connection point source
    ENt_CpDest = 11,  // Connection point destination
    ENt_Cext = 12,    // Connecting extention
    ENt_Cextc = 13,   // Custom Connecting extention
    ENt_CextcSrc = 14,// Custom Connecting extention src spec
    ENt_CextcDest = 15,// Custom Connecting extention dest spec
    ENt_Env = 16,     // Environment
    ENt_MutAdd = 17,  // Mutation - addition
    ENt_MutRm = 18,   // Mutation - removal
    ENt_MutChange = 19, // Mutation - change
    ENt_Robject = 20, // Root of object
    ENt_Trans = 23, // Transtion function of state
    ENt_MutAddStInp = 24,  // Mutation - addition of state input
    ENt_MutChangeCont = 25, // Mutation - change of content
    ENt_MutMove = 26, // Mutation - move node
    ENt_Rm = 27,   // Mutation - removal
    ENt_Change = 28, // Change node attribute
};

enum TNodeAttr
{
    ENa_Unknown = 0,
    ENa_Id = 1,
    ENa_Type = 2,
    ENa_ObjQuiet = 3,
    ENa_Transf = 4,
    ENa_StInit = 5,
    ENa_Logevent = 6,
    ENa_StLen = 7,
    ENa_ConnPair = 9,
    ENa_MutNode = 10,
    ENa_MutChgAttr = 11,
    ENa_MutChgVal = 12,
};

// Logging events
enum TLeBase 
{
    KBaseLe_None = 0,
    KBaseLe_Updated = 1, // On element is updated
    KBaseLe_Creation = 2, // On element creation
    KBaseLe_Any = 3,	// Any time
    KBaseLe_Trans = 4, // Transitioning 
    KBaseLe_END__ = KBaseLe_Any
};

// Logging data
enum TLdBase
{
    KBaseDa_None = 0,
    KBaseDa_Name = 1,
    KBaseDa_New = 2,	// New value of state
    KBaseDa_Curr = 4,	// Current value of state
    KBaseDa_Dep = 8,	// Dependencies of state (current value)
    KBaseDa_Trex = 16,	// Transition expressions
};



// Codes of states operation
enum TChrStateUint8OpType
{
	ECStateUint8Op_None =		0x00,  // No operation
	// Unary
	ECStateUint8Op_UnarySt =	0x01,  // Unary start
	ECStateUint8Op_Copy =		0x01,  // Simple transition
	ECStateUint8Op_UnaryEnd =	0x01,  // Unary end
	// Binary 
	ECStateUint8Op_BinSt =		0x02,  // Binary start
	ECStateUint8Op_Add =		0x02,  // Uint8 + Uint8
	ECStateUint8Op_Sub =		0x03,  // Uint8 + Uint8
	ECStateUint8Op_Mpl =		0x04,  // Uint8 + Uint8
	ECStateUint8Op_Cmp =		0x05,  // Uint8 + Uint8
	ECStateUint8Op_BinEnd =		0x05,  // Binary End
	ECStateUint8Op_Max__ =		0x05,  // Maximum code of operation
};

void Panic(TInt aRes);

class CAE_Base;
class CAE_NBase;

class DesUri
{
    public:
	enum TQueryOpr {
	    EQop_Unknown,
	    EQop_And
	};
	typedef pair<TNodeType, string> TElem;
	typedef pair<TNodeAttr, string> TQueryCnd;
	typedef pair<TQueryOpr, TQueryCnd> TQueryElem;
	typedef vector<TElem>::const_iterator const_elem_iter;
    public:
	DesUri(const string& aUri);
	DesUri();
	DesUri(CAE_NBase* aElem, CAE_NBase* aBase);
	DesUri(TNodeType aType, const string& aName);
	const vector<TElem>& Elems() const {return iElems;};
	const vector<TQueryElem>& QueryElems() const { return iQueryElems;};
	string GetUri(vector<TElem>::const_iterator aStart) const;
	string GetUri() const { return GetUri(iElems.begin());};
	TNodeType GetType() const;
	string GetName() const;
	void AppendElem(TNodeType aType, const string& aName);
	void PrependElem(TNodeType aType, const string& aName);
	void PrependElem(CAE_NBase* aElem, TBool aRec = ETrue, CAE_NBase* aBase = NULL);
	void AppendQueryElem(TQueryOpr aOpr, TNodeAttr aAttr, const string& aValue);
	static const string& NodeAttrName(TNodeAttr aAttr);
	static const string& NodeTypeName(TNodeType aType);
	static TNodeAttr NodeAttr(const string& aAttrName);
	static const string& LeventName(TLeBase aEvent);
	static TLeBase Levent(const string& aName);
	static const string& LdataName(TLdBase aData);
	static TLdBase Ldata(const string& aName);
    protected:
	static void Construct();
    private:
	void Parse();
    private:
	static map<string, TNodeType> iEbNameToNType;
	static map<TLeBase, string> iLeventNames;
	static map<string, TLeBase> iLevents;
	static map<TLdBase, string> iLdataNames;
	static map<string, TLdBase> iLdata;
	string iUri;
	string iScheme;
	vector<TElem> iElems;
	vector<TQueryElem> iQueryElems;
};



class CAE_Object;
class CAE_StateBase;
class CAE_State;

typedef void (*TTransFun)(CAE_Object* aObject, CAE_StateBase* aState);
typedef char *(*TLogFormatFun)(CAE_State* aState, TBool aCurr);


// Log recorder interface
class MCAE_LogRec
{
public:
	virtual void WriteRecord(const char* aText) = 0;
	virtual void WriteFormat(const char* aFmt,...) = 0;
	virtual void Flush() = 0;
};
#if 0
// Logging spec
struct TLogSpecBase
{
    TLogSpecBase(TInt aEvent, TInt aData): iEvent(aEvent), iData(aData) {};
    TInt  iEvent;  // Event of logging - TLeBase
    TInt  iData;   // Data to log TLdBase
};
#endif

/** Base class for FAP all elements
*/
class CAE_Base
{
    public:
	CAE_Base() {};
	virtual ~CAE_Base() {};
	template <class T> T* GetFbObj(T* aInst) {return aInst = static_cast<T*>(DoGetFbObj(aInst->Type())); };
	void* GetFbObj(const char *aType) {return DoGetFbObj(aType); };
    protected:
	virtual void *DoGetFbObj(const char *aName) = 0;
};

// TODO [YB] Consider to redesign basing on tree model: to have tree node as base class
// The node can contain the refs to childs and parent. Shall be integrated with DES Uri
// Possible problem here is that there is no full relation between chromo nodes and runtime nodes
// For instance ENt_Cext, ENt_Conn. To consider if it makes sense to have such nodes in runtime

class CAE_ChromoNode;
class CAE_NBase: public CAE_Base
{
    public:
	static const char *Type() { return "NBase";}; 
	CAE_NBase(TNodeType aNodeType, const string& aInstName, CAE_NBase* aOwner = NULL): 
	    iNodeType(aNodeType), iInstName(aInstName), iOwner(aOwner) {};
	CAE_NBase(TNodeType aNodeType, CAE_NBase* aOwner = NULL): iNodeType(aNodeType), iOwner(aOwner) {};
	TNodeType NodeType() const { return iNodeType;};
	const string& Name() const { return iInstName;};
	void SetName(const string& aName) { iInstName = aName;};
	CAE_NBase* GetNode(const string& aUri) { DesUri uri(aUri); return GetNode(uri, uri.Elems().begin());};
	CAE_NBase* GetNode(const DesUri& aUri) {GetNode(aUri, aUri.Elems().begin());};
	void RmNode(const string& aUri) {DesUri uri(aUri); RmNode(uri);};
    public:
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase) { return NULL;};
	virtual void RmNode(const DesUri& aUri) {};
	virtual TBool ChangeAttr(TNodeAttr aAttr, const string& aVal);
	virtual TBool ChangeCont(const string& aVal) { return EFalse;};
	virtual CAE_NBase* Owner() { return iOwner;};
	virtual TBool IsMetQuery(const DesUri& aUri) { return EFalse;};
	virtual TBool MoveNode(const DesUri& aNodeUri, const DesUri& aDestUri) { return EFalse;};
	virtual TBool AddNode(const CAE_ChromoNode& aSpec) { return EFalse;};
	// Owners apis
	virtual void OnElemDeleting(CAE_NBase* aElem) {};
	virtual void OnElemAdding(CAE_NBase* aElem) {};
	virtual void OnElemChanging(CAE_NBase* aElem, TNodeAttr aAttr = ENa_Unknown) {};
    protected:
	virtual void *DoGetFbObj(const char *aName);
    protected:
	TNodeType iNodeType;
	string iInstName;
	CAE_NBase* iOwner;
};

// Logging spec
class TLogSpecBase: public CAE_NBase
{
    public:
	TLogSpecBase(TLeBase aEvent, TInt aData, CAE_NBase* aOwner): CAE_NBase(ENt_Logspec, DesUri::LeventName(aEvent), aOwner), 
	iEvent(aEvent), iData(aData) {};
	virtual ~TLogSpecBase();
    public:
	TLeBase  iEvent;  // Event of logging - TLeBase
	TInt  iData;   // Data to log TLdBase
};

class CAE_NotifierBase
{
    public: 
	void SetObs(void* aObs);
	void RemoveObs(void* aObs);
	TInt Size() { return iObs.size();};
    protected:
	vector<void*> iObs;
};

template <class T> 
class CAE_Notifier: public CAE_NotifierBase
{
    public:
	void SetObserver(CAE_Base* aObs) { SetObs(aObs->GetFbObj(T::Type()));};
	void RemoveObserver(CAE_Base* aObs) { RemoveObs(aObs->GetFbObj(T::Type()));};
	T* At(TInt aId) { return (T*) iObs.at(aId);};
};

class CAE_EBase;
class MAE_EbaseObserver
{
    public:
	static const char* Type() { return "EBaseObserver";} 
       	virtual void OnActivated(CAE_EBase* aObj) = 0;
};

/** Base class for FAP elements - updatable, managed, logged
*/
// TODO [YB] To move inputs and outputs to CAE_EBase
class CAE_EBase: public CAE_NBase
{
    friend class DesUri;
    private:
	class ENotifier: public CAE_Notifier<MAE_EbaseObserver>
    {
	public:
	    void OnActivated(CAE_EBase* aObj) { for (TInt i = 0; i < Size(); i++) At(i)->OnActivated(aObj);} ;
    };

    public:
	CAE_EBase(TNodeType aNodeType, const string& aInstName, CAE_Object* aMan);
	virtual ~CAE_EBase();
	virtual void Confirm() = 0;
	virtual void Update() = 0;
	TBool IsQuiet() { return iQuiet;};
	TBool IsActive() { return iActive;};
	void SetActive();
	void ResetActive() { iActive = EFalse;};
	void SetUpdated(); 
	void ResetUpdated() {iUpdated = EFalse;};
	TBool IsUpdated() const { return iUpdated; };
	TBool IsQuiet() const { return iQuiet; };
	void SetType(const char *aType);
	void SetMan(CAE_Object *aMan) { _FAP_ASSERT (iMan == NULL || iMan == aMan); iMan = aMan;};
	const char* TypeName() const { return iTypeName;};
	const char* MansName(TInt aLevel) const;
	void AddLogSpec(TLeBase aEvent, TInt aData);
	void RemLogSpec();
	void SetQuiet(TBool aQuiet = ETrue) { iQuiet = aQuiet; };
	static inline const char *Type(); 
	inline MCAE_LogRec* Logger();
	TInt GetLogSpecData(TLeBase aEvent) const;
	const map<string, TLogSpecBase*>& LogSpec() const { return iLogSpec;};
	CAE_Notifier<MAE_EbaseObserver>& Notifier() { return iNotifier;};
	// From CAE_NBase
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase);
	virtual CAE_NBase* Owner() { return (CAE_NBase*) iMan;};
	virtual void OnElemDeleting(CAE_NBase* aElem);
	virtual TBool AddNode(const CAE_ChromoNode& aSpec);
protected:
	void AddLogSpec(const CAE_ChromoNode& aSpec);
	// From CAE_Base
	virtual void *DoGetFbObj(const char *aName);
protected:
	/* Name of ancestor */
	char* iTypeName;
	TBool	iUpdated, iActive;
	/* Element is not "moving" - cannot be activated */
	TBool iQuiet; 
	CAE_Object* iMan;
	map<string, TLogSpecBase*> iLogSpec;
	ENotifier iNotifier;
};

inline const char *CAE_EBase::Type() { return "EBase";} 


// TODO [YB] Actually only CAE_ConnPoint is used. Do we need iface CAE_ConnPointBase?
// Base class for connection points
// TODO [YB] To enhance connection/extension model
class CAE_ConnPointBase: public CAE_NBase
{
    public:
	static const char *Type() { return "ConnPointBase";} 
	CAE_ConnPointBase(TNodeType aNodeType, const string& aName, CAE_EBase* aMan): CAE_NBase(aNodeType, aName), iMan(aMan) {};
	static TBool Connect(CAE_ConnPointBase *aP1, CAE_ConnPointBase *aP2) { return aP1->Connect(aP2) && aP2->Connect(aP1);};
	virtual ~CAE_ConnPointBase();
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint) = 0;
	virtual TBool Disconnect(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse) = 0;
	virtual TBool Disconnect() = 0;
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin) = 0;
	virtual void DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin) = 0;
	virtual CAE_Base* GetSrcPin(const char* aName) = 0;
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint) = 0;
	virtual TBool Disextend(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse) = 0;
	virtual TBool Disextend();
	// Extention is directed relation, so use separate methods for "slave"
	// TODO [YB] Consider of necessity of separate methods for extention slave
	virtual TBool SetExtended(CAE_ConnPointBase *aConnPoint) = 0;
	virtual TBool SetDisextended(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse) = 0;
	virtual TBool SetDisextended();
	const vector<CAE_ConnPointBase*>& Conns() { return iConns; };
	const vector<CAE_ConnPointBase*>& Conns() const { return iConns; };
	const vector<CAE_ConnPointBase*>& Exts() const { return iExts; };
	const CAE_EBase& Man() const { return *iMan;};
	CAE_EBase& Man() { return *iMan;};
	// From CAE_NBase
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase) { return NULL;};
	virtual void RmNode(const DesUri& aUri) {};
	virtual CAE_NBase* Owner() { return iMan;};
    protected:
	// From CAE_Base
	virtual void *DoGetFbObj(const char *aName);
    protected:
	// TODO [YB] To migrate from vector to map
	vector<CAE_ConnPointBase*> iConns;
	// References to extending cp only, not extended
	vector<CAE_ConnPointBase*> iExts;
	CAE_EBase* iMan;
};


class CAE_ConnPoint;

// Connection slot. Container of source pins, destination pins, and ref to connecred slot.
class CAE_ConnSlot
{
    public: 
	typedef string templ_elem; 
	typedef map<string, templ_elem> Template;
    public:
	CAE_ConnSlot(const map<string, templ_elem>& aTempl);
	~CAE_ConnSlot() {};
	CAE_Base* Pin(const char *aName);
	const CAE_Base* Pin(const char *aName) const;
	map<string, CAE_Base*>& Pins() { return iPins;};
	TBool SetPin(const char* aName, CAE_Base *aSrc);
	TBool Set(CAE_ConnSlot& aSrcs);
	TBool operator==(const CAE_ConnSlot& aSrcs);
	TBool IsEmpty() const;
    private:
	// Pins
	map<string, CAE_Base*> iPins;
	const map<string, templ_elem>& iTempl;
};

// Multilink Connection point. Contains multiple dests
class CAE_ConnPoint: public CAE_ConnPointBase
{
    public:
	//CAE_ConnPoint(const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl);
	CAE_ConnPoint(TNodeType aNodeType, const string aName, CAE_EBase* aMan, const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, 
		const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl);
	virtual ~CAE_ConnPoint();
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual void DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual TBool Disconnect(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disconnect();
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetExtended(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetDisextended(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disextend(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disextend() { return CAE_ConnPointBase::Disextend();};
	virtual TBool SetDisextended() { return CAE_ConnPointBase::SetDisextended();};
	vector<CAE_ConnSlot*>& Dests() {return iDests; };
	CAE_ConnSlot* Slot(TInt aInd) {return iDests.at(aInd); };
	CAE_ConnSlot* Srcs() {return iSrcs;};
	map<string, string>& DestsTempl() {return iDestsTempl;};
	map<string, string>& SrcsTempl() {return iSrcsTempl;};
	static const char *Type() {return "ConnPoint";}; 
    protected:
	TBool ConnectConnPoint(CAE_ConnPoint *aConnPoint);
	TBool DisconnectConnPoint(CAE_ConnPoint *aConnPoint);
	virtual void *DoGetFbObj(const char *aName);
    private:
	// Sources
	CAE_ConnSlot* iSrcs;
	// Connection slots
	vector<CAE_ConnSlot*> iDests;
	// Destinations template
	map<string, CAE_ConnSlot::templ_elem> iDestsTempl;
	// Sources template
	map<string, CAE_ConnSlot::templ_elem> iSrcsTempl;
};

// Simple extender. Just contains the reference to connector
class CAE_ConnPointExt: public CAE_ConnPointBase
{
    public:
	CAE_ConnPointExt(TNodeType aNodeType, const string aName, CAE_EBase* aMan): CAE_ConnPointBase(aNodeType, aName, aMan), iRef(NULL) {};
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin) { return EFalse;};
	virtual void DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual TBool  Disconnect(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disconnect();
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetExtended(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetDisextended(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disextend(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	void Set(CAE_ConnPointBase *aConnPoint);
	void Unset();
	CAE_ConnPointBase* Ref() {return iRef;};
	static const char *Type() {return "ConnPointExt";}; 
    protected:
	virtual void *DoGetFbObj(const char *aName);
    private:
	CAE_ConnPointBase* iRef;
};

// 
// Commutating extender. Allows pin commutation for srcs and dest
// TODO [YB] Reconsider, why not to use the set of simple ext instead
class CAE_ConnPointExtC: public CAE_ConnPointBase
{
    public:
	class SlotTempl
	{
	    public:
	       	typedef string slot_templ_elem;
	    public:
		map<string, slot_templ_elem>& Srcs() { return iSrcs;};
		map<string, slot_templ_elem>& Dests() { return iDests;};
		const map<string, slot_templ_elem>& Srcs(TInt a=0) const { return iSrcs;};
		const map<string, slot_templ_elem>& Dests(TInt a=0) const { return iDests;};
		TInt SrcCnt(const char* aName);
		TInt DestCnt(const char* aName);
	    private:
		// Element: [bus pin name]
		map<string, slot_templ_elem> iSrcs;
		map<string, slot_templ_elem> iDests;
	};

	class Slot
	{
	    public:
		typedef pair<CAE_ConnPointBase*, string> slot_elem;
		Slot() {};
		Slot(const SlotTempl& aTempl);
		map<string, pair<CAE_ConnPointBase*, string> >& Srcs() { return iSrcs;};
		map<string, pair<CAE_ConnPointBase*, string> >& Dests() { return iDests;};
	    private:
		// Element: [ref to connector, pin name]
		map<string, slot_elem> iSrcs;
		map<string, slot_elem> iDests;
	};

    public:
	CAE_ConnPointExtC(TNodeType aNodeType, const string aName, CAE_EBase* aMan): CAE_ConnPointBase(aNodeType, aName, aMan) {};
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual void DisconnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual TBool Disconnect(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disconnect();
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetExtended(CAE_ConnPointBase *aConnPoint);
	virtual TBool SetDisextended(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse);
	virtual TBool Disextend(CAE_ConnPointBase *aConnPoint, TBool aOneSide = EFalse) {};
	SlotTempl& Templ() { return iSlotTempl;};
	vector<Slot>& Slots() { return iSlots; };
	static const char *Type() {return "ConnPointExtC";}; 
    protected:
	virtual void *DoGetFbObj(const char *aName);
    private:
	SlotTempl iSlotTempl;
	vector<Slot> iSlots;
};

// Extention

class CAE_ConnBase: public CAE_NBase
{
    public:
	CAE_ConnBase(TNodeType aNodeType, CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner);
	virtual ~CAE_ConnBase();
	TBool IsValid() const { return iIsValid;};
	// From CAE_NBase
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase) { return NULL;};
	virtual void RmNode(const DesUri& aUri) {};
	virtual TBool IsMetQuery(const DesUri& aUri);
    protected:
	virtual TBool Connect() = 0;
	virtual TBool Disconnect() = 0;
    protected:
	CAE_ConnPointBase* iPoint;
	CAE_ConnPointBase* iPair;
	TBool iIsValid;
};

class CAE_Ext: public CAE_ConnBase
{
    public:
	CAE_Ext(CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner);
	virtual ~CAE_Ext();
    protected:
	virtual TBool Connect();
	virtual TBool Disconnect();
};

class CAE_Conn: public CAE_ConnBase
{
    public:
	CAE_Conn(CAE_ConnPointBase* aPoint, CAE_ConnPointBase* aPair, CAE_NBase* aOwner);
	virtual ~CAE_Conn();
    protected:
	virtual TBool Connect();
	virtual TBool Disconnect();
};


// Parameters of operation
struct TOperationInfo
{
	TUint8 iId; // Index of operation, zero based
	TUint8 iMaxId; // Maximum Index of operation
	TUint32 iType1; // Type of the first operand
	TUint32 iType2; // Type of the second operand, 0 if operation is unary
};

// Formatter of CAE element

class CAE_Formatter
{
    public:
	CAE_Formatter(int aUid, TLogFormatFun aFun);
	~CAE_Formatter();
    public:
	TLogFormatFun iFun;
	// UID of state data for wich formatter is applicable
	int iStateDataUid;
};

// State transition spec 
// TODO YB iId represents constant Id. Consider universal solution
#if 0
class TTransInfo
{
public:
	TTransInfo(): iFun(NULL), iOpInd(0) {}
	TTransInfo(TTransFun aFun, const char* aId = NULL): iFun(aFun), iOpInd(0), iId(aId) {}
	TTransInfo(TUint32 aOpInd): iFun(NULL), iOpInd(aOpInd) {}
	TTransInfo(const string& aETrans);
	static void FormatEtrans(const string& aTrans, string& aRes);
public:
	TTransFun iFun;
	string iETrans; // Embedded trans
	TUint32 iOpInd;
	const char* iId;
};
#endif

class TTransInfo: public CAE_NBase
{
public:
	TTransInfo(CAE_NBase* aOwner = NULL): CAE_NBase(ENt_Trans, aOwner), iFun(NULL), iOpInd(0) {}
	TTransInfo(TTransFun aFun, const string& aId): CAE_NBase(ENt_Trans, aId), iFun(aFun), iOpInd(0) {}
	TTransInfo(TTransFun aFun): CAE_NBase(ENt_Trans), iFun(aFun), iOpInd(0) {}
	TTransInfo(TUint32 aOpInd): CAE_NBase(ENt_Trans), iFun(NULL), iOpInd(aOpInd) {}
	TTransInfo(const string& aETrans, CAE_NBase* aOwner = NULL);
	static void FormatEtrans(const string& aTrans, string& aRes);
	// From CAE_NBase
	virtual TBool ChangeCont(const string& aVal);
public:
	TTransFun iFun;
	string iETrans; // Embedded trans
	TUint32 iOpInd;
};

class CSL_ExprBase;
class CAE_StateBase;;
// Context of transition
class MAE_TransContext
{
    public:
	const char* Type() { return "TransContext";} 
	virtual CSL_ExprBase* GetExpr(const string& aTerm, const string& aRtype, MAE_TransContext* aRequestor = NULL) = 0;
	virtual multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype,
		multimap<string, CSL_ExprBase*>::iterator& aEnd, MAE_TransContext* aRequestor = NULL) = 0;
};


template <class T> class CAE_TState;

// Base class for state with transition function
// Transition function should be implemented as static function
// There can be two kinds of state with transition function:
// #1 Regular state. It has the transition function with only one argument- the pointer to state itself
// #2 Special state. It has the transition function with two arguments- the pointer to object and to the state
// Special state transition function can access to object data. So it should be used in the rare cases 
// TODO [YB] To add inputs type info (maybe checking in transf will be enough?)
class CAE_StateBase: public CAE_EBase, public MAE_TransContext
{
    friend class CAE_Object;
public:
	// State input iterator for multiple point input
	class mult_point_inp_iterator: public iterator<input_iterator_tag, CAE_State>
    {
	public:
	    mult_point_inp_iterator(vector<CAE_ConnSlot*>& aDests, TInt aInd): iDests(aDests), iInd(aInd) {};
	    mult_point_inp_iterator(const mult_point_inp_iterator& aIt): iDests(aIt.iDests), iInd(aIt.iInd) {};
	    mult_point_inp_iterator& operator++() { iInd++; return *this; };
	    mult_point_inp_iterator operator++(int) { mult_point_inp_iterator tmp(*this); operator++(); return tmp; };
	    TBool operator==(const mult_point_inp_iterator& aIt) { return (iInd == aIt.iInd); };
	    TBool operator!=(const mult_point_inp_iterator& aIt) { return (iInd != aIt.iInd); };
	    CAE_State& operator*() { CAE_State* st = iDests.at(iInd)->Pin("_1")->GetFbObj(st); return *st;};
	    CAE_State& State(const char* aName) { CAE_State* st = iDests.at(iInd)->Pin(aName)->GetFbObj(st); return *st;};
	public:
	    vector<CAE_ConnSlot*>& iDests;
	    TInt iInd;
    };
public:
	CAE_StateBase(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);
	virtual ~CAE_StateBase();
	virtual void Update();
	void Set(void* aData);
	void SetFromStr(const char *aStr);
	void AddInputL(const char *aName);
	void SetInputL(const char *aName, CAE_StateBase* aState);
	void AddInputL(const char *aName, CAE_StateBase* aState);
	// TODO [YB] To make templated "Input" to return iface required
	CAE_StateBase* Input(const char* aName);
	CAE_StateBase& Inp(const char* aName) { return *(Input(aName));};
	map<string, CAE_ConnPointBase*>& Inputs() {return iInputs;};
	const map<string, CAE_ConnPointBase*>& Inputs() const {return iInputs;};
	mult_point_inp_iterator MpInput_begin(const char *aName);
	mult_point_inp_iterator MpInput_end(const char *aName);
	CAE_ConnPointBase* Output() {return iOutput;};
	const CAE_ConnPointBase* Output() const {return iOutput;};
	static inline const char *Type(); 
	// Set transition procedure. It may be callback function aFun or defined state operation
	// In this case aFun, aCbo should be set to NULL
	// The method verifies operation attempted and set the most passed operation if incorrect
	// Returns true if given operation was set correctly 
	virtual TBool SetTrans(TTransInfo aTinfo);
	inline const TTransInfo& GetTrans();
	virtual TOperationInfo OperationInfo(TUint8 aId) const;
	template <class T> inline operator CAE_TState<T>* ();
	// TODO YB operator doesn't called
	template <class T> inline operator CAE_TState<T>& ();
	template <class T> inline operator const T& ();
	// From CAE_NBase
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase);
	virtual void RmNode(const DesUri& aUri);
	virtual TBool ChangeAttr(TNodeAttr aAttr, const string& aVal);
	virtual void OnElemDeleting(CAE_NBase* aElem);
	virtual void OnElemAdding(CAE_NBase* aElem);
	virtual TBool AddNode(const CAE_ChromoNode& aSpec);
public:
	const string ValStr() const;
	void SetFromStr(const string& aStr) { DoSetFromStr(aStr.c_str());};
	inline MCAE_LogRec *Logger();
protected:
	virtual void *DoGetFbObj(const char *aName);
	void ConstructL();
	void AddInp(const CAE_ChromoNode& aSpec);
	virtual char* DataToStr(TBool aCurr) const = 0;
	virtual void DataFromStr(const char* aStr, void *aData) const = 0;
	virtual void DoSet(void* aData) = 0;
	virtual void DoSetFromStr(const char *aStr) = 0;
	virtual void DoTrans();
	virtual void DoOperation();
	void LogUpdate(TInt aLogData);
	void LogTrans(TInt aLogData);
	virtual CSL_ExprBase* GetExpr(const string& aTerm, const string& aRtype, MAE_TransContext* aRequestor = NULL);
	virtual multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype, 
		multimap<string, CSL_ExprBase*>::iterator& aEnd, MAE_TransContext* aRequestor = NULL);
protected:
	// Transition info
	TTransInfo iTrans;
	map<string, CAE_ConnPointBase*> iInputs;
	CAE_ConnPointBase* iOutput;
};

inline const char *CAE_StateBase::Type() { return "StateBase";} 

inline const TTransInfo& CAE_StateBase::GetTrans() { return iTrans; };



// State handling owned data
// TODO [YB] To remove two-phase instantiation in states
class CAE_State: public CAE_StateBase
{
    friend class CAE_Object;
public:
	CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans);
	static CAE_State* NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans);  
	virtual ~CAE_State();
	virtual void Confirm();
	TInt Len() const {return iLen; };
	const void* Value() const { return iCurr;}
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
	static inline const char *Type(); 
//	CAE_State& Inp(const char* aName) { return *((CAE_State *) Input(aName));};
protected:
	virtual void DoSet(void* aNew);
	virtual void DoSetFromStr(const char *aStr);
	virtual void *DoGetFbObj(const char *aName);
	void ConstructL();
private:
	virtual void DoOperation();
	static char *FmtData(void *aData, int aLen);
public:
	void	*iCurr, *iNew;
protected:
	TInt iLen;
};

inline const char *CAE_State::Type() { return "State";} 



// TODO [YB] To add operator ==
template <class T>
class CAE_TState: public CAE_State
{
public:
	CAE_TState(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans):
	  CAE_State(aInstName, sizeof(T), aMan, aTrans) { iTypeName = strdup(Type());};
	static CAE_TState* NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);
	T& operator~ () { return *((T*)iCurr); };
	const T& Value() { return *((T*) CAE_State::Value()); }
	// TODO [YB] to remove ! operator. No need of getting new values at all
	T& operator! () { return *((T*)iNew); };
	CAE_TState<T>& operator= (T aVal) { Set(&aVal); return *this;};
	static inline TInt DataTypeUid();
	//static inline const char *Type();
	static const char *Type();
	// TODO [YB] Interpret is obsolete. Use CAE_State casting operator instead
	inline static CAE_TState* Interpret(CAE_State* aPtr); 
	FAPWS_API virtual TBool SetTrans(TTransInfo aTinfo);
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
private:
	FAPWS_API virtual void DoOperation();
};

/*
template <class T>
inline CAE_StateBase::operator CAE_TState<T>* () {
    return (strcmp(TypeName(), CAE_TState<T>::Type()) == 0)?static_cast<CAE_TState<T>*>(this):NULL; 
}
*/

template <class T>
inline CAE_StateBase::operator CAE_TState<T>* () {
    CAE_TState<T>* res = GetFbObj(res); return res;
}

template <class T>
inline CAE_StateBase::operator CAE_TState<T>& () {
    return *((CAE_TState<T>*) *this);
}

template <class T> 
inline CAE_StateBase::operator const T& () {
    return ~*((CAE_TState<T>*) *this);
}

template <class T>
inline CAE_TState<T>* CAE_TState<T>::Interpret(CAE_State* aPtr) { 
    return (strcmp(aPtr->TypeName(), Type()) == 0)?static_cast<CAE_TState<T>*>(aPtr):NULL; 
}; 

template <class T>
CAE_TState<T>* CAE_TState<T>::NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans)
{ 
	CAE_TState* self = new CAE_TState(aInstName, aMan, aTrans);
	self->ConstructL();
	return self;
}

// State factory function
typedef CAE_State* (*TStateFactFun)(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);

// State info
// It is for registration of custom state into provider
class TStateInfo
{
    public:
	TStateInfo(const char* aType, TStateFactFun aFactFun, TInt aLen = -1): iType(aType), iFactFun(aFactFun), iLen(aLen) {};
    public:
	const char *iType;
	TInt iLen;
	TStateFactFun iFactFun;
};

// Base class for State that handles data not owned but referenced
// Actually the state deals with iface that provide APIs for update the data. 
// It is assumed that data owner is responcible for confirmation within system tick
class CAE_StateRef: public CAE_StateBase
{
    public:
	CAE_StateRef(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);
	static inline const char *Type(); 
	CAE_Base* Ref() { return iRef; };
    protected:
	virtual void *DoGetFbObj(const char *aName);
    protected:
	CAE_Base* iRef;
};

inline const char *CAE_StateRef::Type() { return "StateRef";} 

// State - controller of object
// The state handle the object's  iface for mutition and chromo
class CAE_StateCtr: public CAE_StateRef
{
    public:
	CAE_StateCtr(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);
	static CAE_StateCtr* New(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans);
	static inline const char *Type(); 
	virtual void Confirm();
    protected:
	virtual void *DoGetFbObj(const char *aName);
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
	virtual void DoSet(void* aNew);
	virtual void DoSetFromStr(const char *aStr);
};

inline const char *CAE_StateCtr::Type() { return "StateCtr";} 


// TODO [YB] Consider avoiding iHandle from Node
class CAE_ChromoNode;

class MAE_ChromoMdl
{
    public:
	virtual TNodeType GetType(const void* aHandle) = 0;
	virtual TNodeType GetType(const string& aId) = 0;
	virtual void* Parent(const void* aHandle) = 0;
	virtual void* Next(const void* aHandle, TNodeType aType = ENt_Unknown) = 0;
	virtual void* NextText(const void* aHandle) = 0;
	virtual void* GetFirstChild(const void* aHandle, TNodeType aType = ENt_Unknown) = 0;
	virtual void* GetLastChild(const void* aHandle, TNodeType aType = ENt_Unknown) = 0;
	virtual void* GetFirstTextChild(const void* aHandle) = 0;
	virtual char* GetAttr(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual char* GetContent(const void* aHandle) = 0;
	virtual void  SetContent(const void* aHandle, const string& aContent) = 0;
	virtual TBool AttrExists(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual TNodeAttr GetAttrNat(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual TNodeType GetAttrNt(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual void* AddChild(void* aParent, TNodeType aType) = 0;
	virtual void* AddChild(void* aParent, const void* aHandle, TBool aCopy = ETrue) = 0;
	virtual void* AddChildDef(void* aParent, const void* aHandle, TBool aCopy = ETrue) = 0;
	virtual void* AddNext(const void* aPrev, const void* aHandle, TBool aCopy = ETrue) = 0;
	virtual void* AddNext(const void* aPrev, TNodeType aNode) = 0;
	virtual void RmChild(void* aParent, void* aChild) = 0;
	virtual void Rm(void* aHandle) = 0;
	virtual void MoveNextTo(void* aHandle, void* aDest) = 0;
	virtual void SetAttr(void* aNode, TNodeAttr aType, const char* aVal) = 0;
	virtual void SetAttr(void* aNode, TNodeAttr aType, TNodeType aVal) = 0;
	virtual void SetAttr(void* aNode, TNodeAttr aType, TNodeAttr aVal) = 0;
	virtual void Dump(void* aNode, MCAE_LogRec* aLogRec) = 0;
	virtual void Save(const string& aFileName) const = 0;
	virtual void* Find(const void* aHandle, const string& aUri) = 0;
};

class CAE_ChromoMdlBase: public CAE_Base, public MAE_ChromoMdl
{
};

// Node of chromo spec
class CAE_ChromoNode
{
    public:
	class Iterator: public iterator<input_iterator_tag, CAE_ChromoNode>
    {
	friend class CAE_ChromoNode;
	public:
	Iterator(const CAE_ChromoNode& aNode): iMdl(aNode.iMdl), iHandle(aNode.iHandle) {};
	Iterator(CAE_ChromoMdlBase& aMdl, void* aHandle): iMdl(aMdl), iHandle(aHandle) {};
	Iterator(const Iterator& aIt): iMdl(aIt.iMdl), iHandle(aIt.iHandle) {};
	Iterator& operator=(const Iterator& aIt) { iMdl = aIt.iMdl; iHandle = aIt.iHandle; return *this; };
	Iterator& operator++() { iHandle = iMdl.Next(iHandle); return *this; };
	Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; };
	TBool operator==(const Iterator& aIt) { return (iHandle == aIt.iHandle); };
	TBool operator!=(const Iterator& aIt) { return !(iHandle == aIt.iHandle); };
	CAE_ChromoNode operator*() { return CAE_ChromoNode(iMdl, iHandle);};
	public:
	CAE_ChromoMdlBase& iMdl;
	void* iHandle; // NULL point to the past-of-the-end element
    };
    public:
	class Const_Iterator: public iterator<input_iterator_tag, CAE_ChromoNode>
    {
	friend class CAE_ChromoNode;
	public:
	Const_Iterator(const CAE_ChromoNode& aNode): iMdl(aNode.iMdl), iHandle(aNode.iHandle) {};
	Const_Iterator(CAE_ChromoMdlBase& aMdl, void* aHandle): iMdl(aMdl), iHandle(aHandle) {};
	Const_Iterator(const Const_Iterator& aIt): iMdl(aIt.iMdl), iHandle(aIt.iHandle) {};
	Const_Iterator& operator=(const Const_Iterator& aIt) { iMdl = aIt.iMdl; iHandle = aIt.iHandle; return *this; };
	Const_Iterator& operator++() { iHandle = iMdl.Next(iHandle); return *this; };
	Const_Iterator operator++(int) { Const_Iterator tmp(*this); operator++(); return tmp; };
	TBool operator==(const Const_Iterator& aIt) { return (iHandle == aIt.iHandle); };
	TBool operator!=(const Const_Iterator& aIt) { return !(iHandle == aIt.iHandle); };
	CAE_ChromoNode operator*() { return CAE_ChromoNode(iMdl, iHandle);};
	public:
	CAE_ChromoMdlBase& iMdl;
	void* iHandle; // NULL point to the past-of-the-end element
    };

    public:
	CAE_ChromoNode(CAE_ChromoMdlBase& aMdl, void* aHandle): iMdl(aMdl), iHandle(aHandle) {};
	CAE_ChromoNode(const CAE_ChromoNode& aNode): iMdl(aNode.iMdl), iHandle(aNode.iHandle) {};
	CAE_ChromoNode& operator=(const CAE_ChromoNode& aNode) { iMdl = aNode.iMdl; iHandle = aNode.iHandle; return *this; };
	Iterator Begin() { return Iterator(iMdl, iMdl.GetFirstChild(iHandle)); };
	Const_Iterator Begin() const { return Const_Iterator(iMdl, iMdl.GetFirstChild(iHandle)); };
	Iterator End() { return Iterator(iMdl, NULL); };
	Const_Iterator End() const { return Const_Iterator(iMdl, NULL); };
	Iterator Find(TNodeType aNodeType) { return Iterator(iMdl, iMdl.GetFirstChild(iHandle, aNodeType)); };
	Const_Iterator Find(TNodeType aNodeType) const { return Const_Iterator(iMdl, iMdl.GetFirstChild(iHandle, aNodeType)); };
	Iterator FindText() { return Iterator(iMdl, iMdl.GetFirstTextChild(iHandle)); };
	Const_Iterator FindText() const { return Const_Iterator(iMdl, iMdl.GetFirstTextChild(iHandle)); };
	void Reset() { iHandle = NULL;};
    public:
	TNodeType Type() { return iMdl.GetType(iHandle); };
	TNodeType Type() const { return iMdl.GetType(iHandle); };
	const string Name() { return Attr(ENa_Id);};
	const string Name() const { return Attr(ENa_Id);};
	const string Attr(TNodeAttr aAttr);
	const string Attr(TNodeAttr aAttr) const;
	const string Content() { return iMdl.GetContent(iHandle);};
	void  SetContent(const string& aContent) { return iMdl.SetContent(iHandle, aContent);};
	TInt AttrInt(TNodeAttr aAttr) const;
	TBool AttrExists(TNodeAttr aAttr) const { return iMdl.AttrExists(iHandle, aAttr);};
	TBool AttrBool(TNodeAttr aAttr) const;
	TNodeAttr AttrNatype(TNodeAttr aAttr) const { return iMdl.GetAttrNat(iHandle, aAttr); };
	TNodeType AttrNtype(TNodeAttr aAttr) const { return iMdl.GetAttrNt(iHandle, aAttr); };
	const void* Handle() { return iHandle;};
	const void* Handle() const { return iHandle;};
	CAE_ChromoMdlBase& Mdl() const { return iMdl;};
	CAE_ChromoNode AddChild(TNodeType aType) { return CAE_ChromoNode(iMdl, iMdl.AddChild(iHandle, aType)); };
	CAE_ChromoNode AddChild(const CAE_ChromoNode& aNode, TBool aCopy = ETrue) { return 
	    CAE_ChromoNode(iMdl, iMdl.AddChild(iHandle, aNode.Handle(), aCopy)); };
	CAE_ChromoNode AddChildDef(const CAE_ChromoNode& aNode, TBool aCopy = ETrue) { return 
	    CAE_ChromoNode(iMdl, iMdl.AddChildDef(iHandle, aNode.Handle(), aCopy)); };
	CAE_ChromoNode AddNext(const CAE_ChromoNode& aPrev, const CAE_ChromoNode& aNode, TBool aCopy = ETrue) { return 
	    CAE_ChromoNode(iMdl, iMdl.AddNext(aPrev.Handle(), aNode.Handle(), aCopy)); };
	CAE_ChromoNode AddNext(TNodeType aType) { return CAE_ChromoNode(iMdl, iMdl.AddNext(iHandle, aType));};
	// Be careful while removing node got from iterator. Iterator is not cleaned thus it returns wrong node on ++
	void RmChild(const CAE_ChromoNode& aChild) { iMdl.RmChild(iHandle, aChild.iHandle); };
	void Rm() { iMdl.Rm(iHandle); };
	void SetAttr(TNodeAttr aType, const string& aVal) { iMdl.SetAttr(iHandle, aType, aVal.c_str()); };
	void SetAttr(TNodeAttr aType, TNodeType aVal) { iMdl.SetAttr(iHandle, aType, aVal); };
	void SetAttr(TNodeAttr aType, TNodeAttr aVal) { iMdl.SetAttr(iHandle, aType, aVal); };
	CAE_ChromoNode::Iterator Parent();
	CAE_ChromoNode::Iterator Find(const string& aName);
	CAE_ChromoNode::Iterator Find(TNodeType aType, const string& aName);
	CAE_ChromoNode::Const_Iterator Find(TNodeType aType, const string& aName) const;
	CAE_ChromoNode::Iterator Find(TNodeType aType, TNodeAttr aAttr, const string& aAttrVal, TBool aPathVal = EFalse);
	CAE_ChromoNode::Iterator Find(TNodeType aType, const string& aName, TNodeAttr aAttr, const string& aAttrVal);
	void Dump(MCAE_LogRec* aLogRec) const { iMdl.Dump(iHandle, aLogRec);};
	void ParseTname(const string& aTname, TNodeType& aType, string& aName);
	string GetName(const string& aTname);
	void MoveNextTo(Iterator& aDest) { iMdl.MoveNextTo(iHandle, aDest.iHandle);};
    private :
	CAE_ChromoMdlBase& iMdl;
	void* iHandle;
};


// Chromosome
class MAE_Chromo 
{
    public:
	virtual CAE_ChromoNode& Root() = 0;
	virtual const CAE_ChromoNode& Root() const= 0;
	virtual void Set(const char *aFileName) = 0;
	virtual TBool Set(const string& aUri) = 0;
	virtual void Set(const CAE_ChromoNode& aRoot) = 0;
	virtual void Init(TNodeType aRootType) = 0;
	virtual void Reset() = 0;
	virtual void Save(const string& aFileName) const = 0;
    public:
	// TODO [YB] To move the utils from iface to CAE_ChromoBase or sec utils class
	static string GetTypeId(TNodeType aType);
	static string GetAttrId(TNodeAttr aType);
	static string GetTName(TNodeType aType, const string& aName);
	static void ParseTname(const string& aTname, TNodeType& aType, string& aName);
	static TBool ComparePath(const string& aS1, const string& aS2);
	static TBool ReplacePathElem(string& aPath, const string& aPathTempl, const string& aNewElem);
};

class CAE_ChromoBase: public MAE_Chromo
{
    public:
	virtual ~CAE_ChromoBase() {};
    public:
	static void GetUriScheme(const string& aUri, string& aScheme);
	static void GetPath(const string& aUri, string& aPath);
	static void GetFrag(const string& aUri, string& aFrag);
};

// Chromosome manager
class MAE_ChroMan
{
    public:
	virtual void CopySpec(const void *aSrc, void **aDest) = 0;
	virtual void *GetChild(void *aSpec) = 0;
	virtual void *GetNext(void *aChildSpec) = 0;
	virtual char *GetType(void *aSpec) = 0;
	virtual char *GetName(void *aSpec) = 0;
	virtual int GetLen(void *aSpec) = 0;
	virtual TCaeElemType FapType(void *aElement) = 0;
	virtual TCaeMut MutType(void *aElement) = 0;
	virtual char *GetStrAttr(void *aSpec, const char *aName) = 0;
	virtual int GetAttrInt(void *aSpec, const char *aName) = 0;
};

// Transitions
//
// Executable agent for FAP transtions
class MAE_TranEx
{
    public:
	virtual void EvalTrans(MAE_TransContext* aContext, CAE_EBase* aExpContext, const string& aTrans) = 0;
	virtual const multimap<string, CSL_ExprBase*>& Exprs() = 0;
	virtual CSL_ExprBase* GetExpr(const string& aTerm, const string& aRtype) = 0;
};

// Executable agent base 
class CAE_TranExBase: public CAE_Base, public MAE_TranEx
{
    public:
	CAE_TranExBase(MCAE_LogRec* aLogger): iLogger(aLogger) {};
    protected:
	MCAE_LogRec* iLogger;
};


class MAE_Env;
class CAE_Env;
class MAE_Opv;
class MCAE_LogRec;
// Base class for object. Object is container and owner of its component and states
// The component of object can be other objects
// TODO [YB] Consider restricting access to object elements. Currently any internal object can be accessed
// TODO [YB] To reconsider approach with "quiet" elements. This approach is not effective enough 
class CAE_Object: public CAE_EBase, public MAE_TransContext
{
    friend class CAE_Env;
public:
	// Operation under chromosome
	enum TChromOper
	{
		EChromOper_Copy = 1,
		EChromOper_Rand = 2,
		EChromOper_Mutate = 3,
		EChromOper_Combine = 4 // Combine then mutate
	};
public:
	class ChromoPx: public CAE_Base
    {
	public:
	    ChromoPx(CAE_Object& aMaster): iMaster(aMaster) {};
	    MAE_Chromo& Mut() { return iMaster.Mut();};
	    const MAE_Chromo& Chr() { return iMaster.Chromo();};
	    void SetUpdated() { iMaster.SetUpdated(); };
	    static const char *Type() { return "ObjChromo";}; 
	protected:
	    virtual void *DoGetFbObj(const char *aName);
	private:
	    CAE_Object& iMaster;
    };

	// Object's controller
	// TODO [YB] To consider if we need controller at all. The access control can be done via root. 
	class Ctrl
	{
	    public:
		Ctrl(CAE_Object& aOwner): iOwner(aOwner) {}
		map<string, CAE_Object*>& Comps() { return iOwner.iComps;};
		vector<CAE_Object*>& CompsOrd() { return iOwner.iCompsOrd;};
		map<string, CAE_StateBase*>& States() { return iOwner.iStates;};
		const TTransInfo& Trans() const { return iOwner.iTransSrc;};
		CAE_Object& Object() { return iOwner;}
		CAE_EBase* FindByName(const string& aName) { return iOwner.FindByName(aName);};
		CAE_Object* Mangr() { return iOwner.iMan;};
		CAE_Object::Ctrl& GetCtrl(CAE_Object* aObj) { return aObj->iCtrl;};
	    public:
		CAE_Object& iOwner;
	};

	friend class Crtl;

public:
	static inline const char *Type(); 
	inline MCAE_LogRec *Logger();
	virtual ~CAE_Object();
	static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, const TUint8* aChrom = NULL, MAE_Env* aEnv = NULL);
	static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv = NULL);
	// TODO [YB] -DONE- CAE_Object cannot register/unregister just base class because the reristry are for heir of EBase
	// Object uses type resolver to understand what is to be reg/unreg. Thats why it is impossible to unger 
	// from EBase destructor (resolver is based on virt funct so isnt working there). To conside specialize reg/unreg. 
	// TODO [YB] To migrate to NBase apis for adding/deleting element
	void RegisterCompL(CAE_EBase* aComp);
	void UnregisterComp(CAE_Object* aComp);
	virtual void Update();
	virtual void Confirm();
	CAE_Object* GetComp(const char* aName, CAE_Object* aRequestor = NULL);
	CAE_Object* GetComp(const string& aUri);
	// TODO [YB] Implement an access to comp via control iface. FindByName not need this case.
	CAE_EBase* FindByName(const string& aName);
	CAE_ConnPointBase* GetConn(const char *aName);
	CAE_ConnPointBase* GetInpN(const char *aName);
	CAE_ConnPointBase* GetOutpN(const char *aName);
	CAE_StateBase* GetOutpState(const char* aName);
	CAE_StateBase* GetInpState(const char* aName);
	map<string, CAE_ConnPointBase*>& Inputs() {return iInputs;};
	const map<string, CAE_ConnPointBase*>& Inputs() const {return iInputs;};
	map<string, CAE_ConnPointBase*>& Outputs() {return iOutputs;};
	const map<string, CAE_ConnPointBase*>& Outputs() const {return iOutputs;};
	// Create new inheritor of self. 
	CAE_Object* CreateHeir(const char *aName, CAE_Object *aMan);
	void SetMutation(const CAE_ChromoNode& aMuta);
	MAE_Chromo& Mut() { return *iMut;};
	const MAE_Chromo& Chromo() { return *iChromo;};
	CAE_Object::ChromoPx* ChromoIface() { return &iChromoIface;};
	void Mutate(TBool aRunTimeOnly = EFalse);
	void DoTrans(CAE_StateBase* aState);
	void DoTrans(CAE_StateBase* aState, const string& aInit);
	virtual CSL_ExprBase* GetExpr(const string& aTerm, const string& aRtype, MAE_TransContext* aRequestor = NULL);
	virtual multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype, multimap<string,
	       	CSL_ExprBase*>::iterator& aEnd, MAE_TransContext* aRequestor = NULL);
	CSL_ExprBase* CreateDataExpr(const string& aType);
	// TODO [YB] To migrate from proxy to observer
	void SetBaseViewProxy(MAE_Opv* aProxy, TBool aAsRoot = EFalse);
	void RemoveBaseViewProxy(MAE_Opv* aProxy);
	void AppendCompList(vector<string>& aList, const CAE_Object* aRequestor) const;
	void SetEbaseObsRec(CAE_Base* aObs);
	void Deactivate();
	void Activate();
	CAE_Object* FindMutableMangr();
	// From CAE_NBase
	virtual CAE_NBase* GetNode(const DesUri& aUri, DesUri::const_elem_iter aPathBase);
	virtual TBool ChangeAttr(TNodeAttr aAttr, const string& aVal);
	virtual void RmNode(const DesUri& aUri);
	virtual TBool MoveNode(const DesUri& aNodeUri, const DesUri& aDestUri);
	virtual void OnElemDeleting(CAE_NBase* aElem);
	virtual void OnElemAdding(CAE_NBase* aElem);
	virtual void OnElemChanging(CAE_NBase* aElem, TNodeAttr aAttr = ENa_Unknown);
protected:
	virtual void *DoGetFbObj(const char *aName);
	CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv = NULL);
	void Construct();
	virtual void DoMutation_v1(CAE_ChromoNode& aMutSpec, TBool aRunTime);
	CAE_Object* FindDeattachedMangr();
private:
	// Get logspec event from string
	static TInt LsEventFromStr(const char *aStr);
	// Get logspec data from string
	static TInt LsDataFromStr(const char *aStr);
	void CreateStateInp(const CAE_ChromoNode& aSpecNode, CAE_StateBase *aState);
	void CreateConn(void* aSpecNode, map<string, CAE_ConnPointBase*>& aConns, TBool aInp);
	void AddConn(const CAE_ChromoNode& aSpecNode, map<string, CAE_ConnPointBase*>& aConns, TBool aInp);
	CAE_Object* AddObject(const CAE_ChromoNode& aNode);
	void AddState(const CAE_ChromoNode& aSpec);
	void AddConn_v1(const CAE_ChromoNode& aSpec);
	void AddExt_v1(const CAE_ChromoNode& aSpec);
	void AddExtc_v1(const CAE_ChromoNode& aSpec);
	void AddTrans(const CAE_ChromoNode& aSpec);
	void AddLogspec(CAE_EBase* aNode, const CAE_ChromoNode& aSpec);
	void MoveElem_v1(const CAE_ChromoNode& aSpec);
	void MutAddNode(const CAE_ChromoNode& aSpec);
	void ChangeAttr_v2(const CAE_ChromoNode& aSpec);
	void ChangeCont_v2(const CAE_ChromoNode& aSpec);
	void ChangeChromoCont(const CAE_ChromoNode& aSpec, CAE_ChromoNode& aCurr);
	void SetTrans(const string& aTrans);
	TBool MergeMutation(const CAE_ChromoNode& aSpec);
	TBool MergeMutMove(const CAE_ChromoNode& aSpec);
private:
	map<string, CAE_Object*> iComps;
	vector<CAE_Object*> iCompsOrd;
	map<string, CAE_StateBase*> iStates;
	MAE_Env* iEnv;  // FAP Environment, not owned
	map<string, CAE_ConnPointBase*> iInputs;
	map<string, CAE_ConnPointBase*> iOutputs;
	multimap<string, CSL_ExprBase*> iTrans; // Transition module
	vector<CAE_Ext*> iExts; // Extentions
	vector<CAE_Conn*> iConns; // Extentions
	CAE_ChromoBase *iMut;
	CAE_ChromoBase *iChromo;
	ChromoPx iChromoIface;
	TTransInfo iTransSrc;
	MAE_Opv* iOpv; // Proxy for base view
	Ctrl iCtrl; // Controll interface
};

inline const char *CAE_Object::Type() { return "Object";} 


// Object proxy for base view
class MAE_Opv
{
    public:
	virtual void SetRoot(CAE_Object::Ctrl* aObj) = 0;
	virtual void SetObj(CAE_Object::Ctrl* aObj) = 0;
	virtual void UnsetObj(CAE_Object::Ctrl* aObj) = 0;
	virtual void UnsetRoot(CAE_Object* aObj) = 0;
	virtual void Destroy() = 0;
};

	
// Provider of CAE elements
class MAE_Provider
{
    public:
	virtual CAE_StateBase* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const = 0;
	virtual CAE_EBase* CreateObjectL(TUint32 aTypeUid) const  = 0;
	virtual CAE_EBase* CreateObjectL(const char *aName) const  = 0;
	virtual const TTransInfo* GetTransf(const char *aName) const  = 0;
	virtual void RegisterState(const TStateInfo *aInfo) = 0;
	virtual void RegisterStates(const TStateInfo **aInfos) = 0;
	virtual void RegisterTransf(const TTransInfo *aName) = 0;
	virtual void RegisterTransfs(const TTransInfo **aNames) = 0;
	virtual const CAE_Formatter* GetFormatter(int aUid) const  = 0;
	virtual void RegisterFormatter(CAE_Formatter *aForm) = 0;
	virtual CAE_ChromoBase* CreateChromo() const = 0;
	virtual CAE_TranExBase* CreateTranEx(MCAE_LogRec* aLogger) const = 0;
	virtual MAE_Opv* CreateViewProxy() = 0;
};

// FAP environment interface
class MAE_Env
{
    public:
	virtual MAE_Provider *Provider() const = 0;
	virtual void RequestProvider(const string& aName) = 0;
	// Getting Cromosome manager
	virtual MAE_ChroMan *Chman() const = 0;
	virtual MCAE_LogRec *Logger() = 0;
	virtual MAE_TranEx *Tranex() = 0;
};

class CActive
{
public:
	CActive(TInt aPriority): iPriority(aPriority) {}
	void Cancel() { DoCancel();}
	virtual void DoCancel() = 0;
	virtual void RunL() = 0;
	void SetActive() { isActive = ETrue;}
public:
	TInt  iStatus;
private:
	TBool isActive;
	TInt iPriority;
};


inline MCAE_LogRec *CAE_EBase::Logger() { return iMan ? iMan->Logger(): NULL;}

inline MCAE_LogRec *CAE_StateBase::Logger() { return iMan ? iMan->Logger(): NULL;}

inline MCAE_LogRec *CAE_Object::Logger() {return iEnv ? iEnv->Logger(): NULL; }

#endif // __FAP_BASE_H
