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

#define CAE_TRANS_DEF(name, class) void name(CAE_State* aState);\
static void name##_S_(CAE_Object* aObject, CAE_State* aState) {((class*) aObject)->name(aState);};

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



// Adaptive automata programming constants

// Chromosome structure for the objects type A is as following:
// Chromosome := structure descriptor (SD). 
// SD := elem_num, (SD, ... SD) | (byte_block, ..., byte_block) If the bytes only then SD- simple SD
// The type of SD (complex or simple/byte/) is coded in the elem_num, see KAdp_ObjChromLen_SSD

// Length code structure: the most signed bite contains sign of that the element is SD, otherwise - simple bytes set
const TInt KAdp_ObjChromLen_SSD = 0x80; // Sign of that element id SD


// Chromosome of CAE_Object 
// KAdp_ObjStateVectLen Elements - state codons
// Each state codon is 4 byte length: State_Type, Input_1, Input_2, Op_Type
// State_Type (1 byte): access type (7,6 bits), state type (5..0 bits)
// So it's possible to have 64 states in the state vector
// Transition info:
// Element is 3 bytes length- Input1, Input2, operation_code
// Input1, Input2 value 0xff means "No input"

// Constants for codons are marked as following: 
// _Ind - index of codon in the chromosome
// _Len - length of codon in the chromosome

// State byte block length
const TInt KAdp_ObjStInfoLen = 4;

// Chromosome structure constants
// Chromosome length 
const TInt KAdp_ObjChromLen_Ind = 0;
const TInt KAdp_ObjChromLen_Len = 1;
// Object chromosome max length
const TInt KAdp_ObjCromMaxLen = 256;
// Type of state is coded as following:
const TUint8 KAdp_ObjStType_Reg = 0x00;  // Regular state, not accessed out of object
const TUint8 KAdp_ObjStType_Inp = 0x80;  // Input
const TUint8 KAdp_ObjStType_Out = 0xc0;  // Output
const TUint8 KAdp_ObjStType_NU =  0x40;  // Not used
const TUint8 KAdp_ObjStType_AccessMsk = 0xc0;  // Mask of access type
const TUint8 KAdp_ObjStType_TypeMsk = 0x3f;  // Mask of type

// Codes of states types
enum TChrStateType
{
	ECState_None =		0x00,
	ECState_Uint8 =		0x01,
	ECState_Uint32 =	0x02
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


class CAE_Object;
class CAE_State;

typedef void (*TTransFun)(CAE_Object* aObject, CAE_State* aState);
typedef char *(*TLogFormatFun)(CAE_State* aState, TBool aCurr);


// Log recorder interface
class MCAE_LogRec
{
public:
	virtual void WriteRecord(const char* aText) = 0;
	virtual void WriteFormat(const char* aFmt,...) = 0;
	virtual void Flush() = 0;
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
};

// Logging spec
struct TLogSpecBase
{
    TLogSpecBase(TInt aEvent, TInt aData): iEvent(aEvent), iData(aData) {};
    TInt  iEvent;  // Event of logging - TLeBase
    TInt  iData;   // Data to log TLdBase
};


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


/** Base class for FAP elements - updatable, managed, logged
 */
class CAE_EBase: public CAE_Base
{
public:
	CAE_EBase(const char* aInstName, CAE_Object* aMan);
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
	void SetName(const char *aName);
	void SetType(const char *aType);
	void SetMan(CAE_Object *aMan) { _FAP_ASSERT (iMan == NULL || iMan == aMan); iMan = aMan;};
	const char* InstName() const { return iInstName;};
	const char* TypeName() const { return iTypeName;};
	const char* MansName(TInt aLevel) const;
	void AddLogSpec(TInt aEvent, TInt aData);
	void SetQuiet(TBool aQuiet = ETrue) { iQuiet = aQuiet; };
	static inline const char *Type(); 
protected:
	// From CAE_Base
	virtual void *DoGetFbObj(const char *aName);
	TInt GetLogSpecData(TInt aEvent) const;
protected:
	char* iInstName;
	/* Name of ancestor */
	char* iTypeName;
	TBool	iUpdated, iActive;
	/* Element is not "moving" - cannot be activated */
	TBool iQuiet; 
	CAE_Object* iMan;
	vector<TLogSpecBase>* iLogSpec;
};

inline const char *CAE_EBase::Type() { return "State";} 

// TODO [YB] Actually only CAE_ConnPoint is used. Do we need iface CAE_ConnPointBase?
// Base class for connection points
class CAE_ConnPointBase: public CAE_Base
{
    public:
	static TBool Connect(CAE_ConnPointBase *aP1, CAE_ConnPointBase *aP2) { return aP1->Connect(aP2) && aP2->Connect(aP1);};
	virtual ~CAE_ConnPointBase() {};
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint) = 0;
	virtual void Disconnect(CAE_ConnPointBase *aConnPoint) = 0;
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin) = 0;
	virtual CAE_Base* GetSrcPin(const char* aName) = 0;
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint) = 0;
	virtual void Disextend(CAE_ConnPointBase *aConnPoint) = 0;
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
    private:
	// Pins
	map<string, CAE_Base*> iPins;
	const map<string, templ_elem>& iTempl;
};

// Multilink Connection point. Contains multiple dests
class CAE_ConnPoint: public CAE_ConnPointBase
{
    public:
	CAE_ConnPoint(const map<string, CAE_ConnSlot::templ_elem>& aSrcsTempl, const map<string, CAE_ConnSlot::templ_elem>& aDestsTempl);
	virtual ~CAE_ConnPoint();
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual void Disconnect(CAE_ConnPointBase *aConnPoint);
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint) {return EFalse;};
	virtual void Disextend(CAE_ConnPointBase *aConnPoint);
	vector<CAE_ConnSlot*>& Dests() {return iDests; };
	CAE_ConnSlot* Slot(TInt aInd) {return iDests.at(aInd); };
	CAE_ConnSlot* Srcs() {return iSrcs;};
	map<string, string>& DestsTempl() {return iDestsTempl;};
	map<string, string>& SrcsTempl() {return iSrcsTempl;};
	static const char *Type() {return "ConnPoint";}; 
    protected:
	TBool ConnectConnPoint(CAE_ConnPoint *aConnPoint);
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
	CAE_ConnPointExt(): iRef(NULL) {};
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin) { return EFalse;};
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual void Disconnect(CAE_ConnPointBase *aConnPoint);
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint);
	virtual void Disextend(CAE_ConnPointBase *aConnPoint) {};
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
	CAE_ConnPointExtC() {};
	virtual TBool Connect(CAE_ConnPointBase *aConnPoint);
	virtual TBool ConnectPin(const char* aPin, CAE_ConnPointBase *aPair, const char* aPairPin);
	virtual CAE_Base* GetSrcPin(const char* aName);
	virtual void Disconnect(CAE_ConnPointBase *aConnPoint) {};
	virtual TBool Extend(CAE_ConnPointBase *aConnPoint);
	virtual void Disextend(CAE_ConnPointBase *aConnPoint) {};
	SlotTempl& Templ() { return iSlotTempl;};
	vector<Slot>& Slots() { return iSlots; };
	static const char *Type() {return "ConnPointExtC";}; 
    protected:
	virtual void *DoGetFbObj(const char *aName);
    private:
	SlotTempl iSlotTempl;
	vector<Slot> iSlots;
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
class TTransInfo
{
public:
	TTransInfo(): iFun(NULL), iOpInd(0), iCbo(NULL), iId(NULL) {}
	TTransInfo(TTransFun aFun, const char* aId = NULL, CAE_Object* aCbo = NULL):
	    iFun(aFun), iCbo(aCbo), iOpInd(0), iId(aId) {}
	TTransInfo(TUint32 aOpInd): iFun(NULL), iOpInd(aOpInd) {}
public:
	TTransFun iFun;
	CAE_Object* iCbo; // Keep Cbo in order to be compatible with legacy projects
	TUint32 iOpInd;
	const char *iId;	// Unique identificator of Transition
};

template <class T> class CAE_TState;


// Base class for state with transition function
// Transition function should be implemented as static function
// There can be two kinds of state with transition function:
// #1 Regular state. It has the transition function with only one argument- the pointer to state itself
// #2 Special state. It has the transition function with two arguments- the pointer to object and to the state
// Special state transition function can access to object data. So it should be used in the rare cases 
// TODO [YB] To add inputs type info (maybe checking in transf will be enough?)
class CAE_State: public CAE_EBase
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
	CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans);
	static CAE_State* NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans);  
	virtual ~CAE_State();
	virtual void Confirm();
	virtual void Update();
	TInt Len() const {return iLen; };
	void AddInputL(const char *aName);
	void SetInputL(const char *aName, CAE_State* aState);
	void AddInputL(const char *aName, CAE_State* aState);
	void Set(void* aNew);
	void SetFromStr(const char *aStr);
	const void* Value() const { return iCurr;}
	CAE_State* Input(const char* aName);
	map<string, CAE_ConnPointBase*>& Inputs() {return iInputs;};
	mult_point_inp_iterator MpInput_begin(const char *aName);
	mult_point_inp_iterator MpInput_end(const char *aName);
	CAE_ConnPointBase* Output() {return iOutput;};
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
	static inline const char *Type(); 
	// Set transition procedure. It may be callback function aFun or defined state operation
	// In this case aFun, aCbo should be set to NULL
	// The method verifies operation attempted and set the most passed operation if incorrect
	// Returns true if given operation was set correctly 
	virtual TBool SetTrans(TTransInfo aTinfo);
	CAE_State& Inp(const char* aName) { return *((CAE_State *) Input(aName));};
	inline TTransInfo GetTrans();
	template <class T> inline operator CAE_TState<T>* ();
	// TODO YB operator doesn't called
	template <class T> inline operator CAE_TState<T>& ();
	template <class T> inline operator const T& ();
	virtual TOperationInfo OperationInfo(TUint8 aId) const;
protected:
	// From CAE_Base
	virtual void *DoGetFbObj(const char *aName);
	void ConstructL();
private:
	virtual void DoTrans();
	virtual void DoOperation();
	static char *FmtData(void *aData, int aLen);
	void LogUpdate(TInt aLogData);
	void LogTrans(TInt aLogData);
	inline MCAE_LogRec *Logger();
public:
	void	*iCurr, *iNew;
	// Transition info
	TTransInfo iTrans;
protected:
	TInt iLen;
	map<string, CAE_ConnPointBase*> iInputs;
	CAE_ConnPointBase* iOutput;
};

inline const char *CAE_State::Type() { return "State";} 

inline TTransInfo CAE_State::GetTrans() { return iTrans; };


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
	// [YB] Interpret is obsolete. Use CAE_State casting operator instead
	inline static CAE_TState* Interpret(CAE_State* aPtr); 
	FAPWS_API virtual TBool SetTrans(TTransInfo aTinfo);
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
private:
	FAPWS_API virtual void DoOperation();
};


template <class T>
inline CAE_State::operator CAE_TState<T>* () {
    return (strcmp(TypeName(), CAE_TState<T>::Type()) == 0)?static_cast<CAE_TState<T>*>(this):NULL; 
}

template <class T>
inline CAE_State::operator CAE_TState<T>& () {
    return *((CAE_TState<T>*) *this);
}

template <class T> 
inline CAE_State::operator const T& () {
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

enum NodeType
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
    ENa_PinType = 8,
    ENa_ConnPair = 9,
    ENa_MutNode = 10,
    ENa_MutChgAttr = 11,
    ENa_MutChgVal = 12,
};

// TODO [YB] Consider avoiding iHandle from Node
class CAE_ChromoNode;

class MAE_ChromoMdl
{
    public:
	virtual NodeType GetType(const void* aHandle) = 0;
	virtual void* Next(const void* aHandle, NodeType aType = ENt_Unknown) = 0;
	virtual void* GetFirstChild(const void* aHandle, NodeType aType = ENt_Unknown) = 0;
	virtual void* GetLastChild(const void* aHandle, NodeType aType = ENt_Unknown) = 0;
	virtual char* GetAttr(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual TBool AttrExists(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual TNodeAttr GetAttrNat(const void* aHandle, TNodeAttr aAttr) = 0;
	virtual void* AddChild(void* aParent, NodeType aType) = 0;
	virtual void* AddChild(void* aParent, const void* aHandle) = 0;
	virtual void SetAttr(void* aNode, TNodeAttr aType, const char* aVal) = 0;
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
	Iterator Find(NodeType aNodeType) { return Iterator(iMdl, iMdl.GetFirstChild(iHandle, aNodeType)); };
	Const_Iterator Find(NodeType aNodeType) const { return Const_Iterator(iMdl, iMdl.GetFirstChild(iHandle, aNodeType)); };
    public:
	NodeType Type() { return iMdl.GetType(iHandle); };
	NodeType Type() const { return iMdl.GetType(iHandle); };
	const string Name() { return Attr(ENa_Id);};
	const string Name() const { return Attr(ENa_Id);};
	const string Attr(TNodeAttr aAttr);
	const string Attr(TNodeAttr aAttr) const;
	TInt AttrInt(TNodeAttr aAttr) const;
	TBool AttrExists(TNodeAttr aAttr) const { return iMdl.AttrExists(iHandle, aAttr);};
	TBool AttrBool(TNodeAttr aAttr) const;
	TNodeAttr AttrNatype(TNodeAttr aAttr) const { return iMdl.GetAttrNat(iHandle, aAttr); };
	const void* Handle() { return iHandle;};
	const void* Handle() const { return iHandle;};
	CAE_ChromoMdlBase& Mdl() const { return iMdl;};
	CAE_ChromoNode AddChild(NodeType aType) { return CAE_ChromoNode(iMdl, iMdl.AddChild(iHandle, aType)); };
	CAE_ChromoNode AddChild(const CAE_ChromoNode& aNode) { return CAE_ChromoNode(iMdl, iMdl.AddChild(iHandle, aNode.Handle())); };
	void SetAttr(TNodeAttr aType, const string& aVal) { iMdl.SetAttr(iHandle, aType, aVal.c_str()); };
    private :
	CAE_ChromoMdlBase& iMdl;
	void* iHandle;
};

// Chromosome
class MAE_Chromo 
{
    public:
	virtual CAE_ChromoNode& Root() = 0;
	virtual void Set(const char *aFileName) = 0;
	virtual void Set(const CAE_ChromoNode& aRoot) = 0;
	virtual void Init(NodeType aRootType) = 0;
	virtual void Reset() = 0;
};

class CAE_ChromoBase: public MAE_Chromo
{
    public:
	virtual ~CAE_ChromoBase() {};
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

// Provider of CAE elements
class MAE_Provider
{
    public:
	virtual CAE_State* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const = 0;
	virtual CAE_State* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const = 0;
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
};

// FAP environment interface
class MAE_Env
{
    public:
	virtual MAE_Provider *Provider() const = 0;
	// Getting Cromosome manager
	virtual MAE_ChroMan *Chman() const = 0;
	virtual MCAE_LogRec *Logger() = 0;
};

// Base class for object. Object is container and owner of its component and states
// The component of object can be other objects
// TODO [YB] Consider restricting access to object elements. Currently any internal object can be accessed
// TODO [YB] To reconsider approach with "quiet" elements. This approach is not effective enough 
class CAE_Object: public CAE_EBase
{
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
	static inline const char *Type(); 
	inline MCAE_LogRec *Logger();
	virtual ~CAE_Object();
	static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, const TUint8* aChrom = NULL, MAE_Env* aEnv = NULL);
	// Variant for Chrom XML. In order for somooth transition to XML chrom.
	static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, const void *aChrom = NULL, MAE_Env* aEnv = NULL);
	static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv = NULL);
	void RegisterCompL(CAE_EBase* aComp);
	void UnregisterComp(CAE_EBase* aComp);
	virtual void Update();
	virtual void Confirm();
	void LinkL(CAE_State* aInp, CAE_State* aOut, TTransFun aTrans = NULL);
	CAE_Object* GetComp(const char* aName, TBool aGlob = EFalse);
	TInt CountCompWithType(const char *aType = NULL);
	// TODO [YB] To restrict an access to components
	CAE_Object* GetNextCompByFapType(const char *aType, int* aCtx = NULL) const;
	CAE_Object* GetNextCompByType(const char *aType, int* aCtx = NULL) const;
	// TODO [YB] Implement an access to comp via control iface. FindByName not need this case.
	CAE_EBase* FindByName(const char* aName);
	CAE_ConnPointBase* GetConn(const char *aName);
	CAE_ConnPointBase* GetInpN(const char *aName);
	CAE_ConnPointBase* GetOutpN(const char *aName);
	CAE_State* GetOutpState(const char* aName);
	CAE_State* GetInpState(const char* aName);
	map<string, CAE_ConnPointBase*>& Inputs() {return iInputs;};
	map<string, CAE_ConnPointBase*>& Outputs() {return iOutputs;};
	// Create new inheritor of self. 
	CAE_Object* CreateNewL(const void* aSpec, const char *aName, CAE_Object *aMan);
	CAE_Object* CreateHeir(const char *aName, CAE_Object *aMan);
	void SetMutation(const CAE_ChromoNode& aMuta);
	void DoMutation();
	MAE_Chromo& Mut() { return *iMut;};
protected:
	virtual void *DoGetFbObj(const char *aName);
	CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv = NULL);
	void Construct();
	void ConstructL(const void* aChrom = NULL);
	void ConstructFromChromXL(const void* aChrom);
	void SetChromosome(TChromOper aOper = EChromOper_Copy, const void* aChrom1 = NULL, const char* aChrom2 = NULL);
private:
	CAE_State* GetStateByName(const char *aName);
	// Calculates the length of chromosome
	TInt ChromLen(const TUint8* aChrom) const;
	// Get logspec event from string
	static TInt LsEventFromStr(const char *aStr);
	// Get logspec data from string
	static TInt LsDataFromStr(const char *aStr);
	void CreateStateInp(void* aSpecNode, CAE_State *aState);
	void CreateStateInp(const CAE_ChromoNode& aSpecNode, CAE_State *aState);
	void CreateConn(void* aSpecNode, map<string, CAE_ConnPointBase*>& aConns);
	void AddConn(const CAE_ChromoNode& aSpecNode, map<string, CAE_ConnPointBase*>& aConns);
	void AddObject(const CAE_ChromoNode& aNode);
	void AddState(const CAE_ChromoNode& aSpec);
	void AddConn(const CAE_ChromoNode& aSpec);
	void AddExt(const CAE_ChromoNode& aSpec);
	void AddExtc(const CAE_ChromoNode& aSpec);
	void ChangeAttr(CAE_EBase& aElem, const CAE_ChromoNode& aSpec);
private:
	// TODO [YB] To migrate to map
	vector<CAE_EBase*> iCompReg;
	void* iChromX;		// Chromosome, XML based
	MAE_Env* iEnv;  // FAP Environment, not owned
	map<string, CAE_ConnPointBase*> iInputs;
	map<string, CAE_ConnPointBase*> iOutputs;
	CAE_ChromoBase *iMut;
	CAE_ChromoBase *iChromo;
};

inline const char *CAE_Object::Type() { return "Object";} 


inline MCAE_LogRec *CAE_Object::Logger() {return iEnv ? iEnv->Logger(): NULL; }
	
// Bit based authomata 
class CAE_ObjectBa: public CAE_Object
{
public:
	FAPWS_API virtual ~CAE_ObjectBa();
	FAPWS_API static CAE_ObjectBa* NewL(const char* aInstName, CAE_Object* aMan, TInt aLen, TInt aInpInfo, TInt aOutInfo);
	FAPWS_API void SetTrans(const TUint32* aTrans);
	FAPWS_API virtual void Update();
	FAPWS_API virtual void Confirm();
	CAE_State*  Input() {return iInp;}
	FAPWS_API CAE_ObjectBa* CreateNew(const char* aInstName);
	void DoMutationOfTrans();
protected:
	FAPWS_API CAE_ObjectBa(const char* aInstName, CAE_Object* aMan, TInt aLen, TInt aInpInfo, TInt aOutInfo);
	FAPWS_API void ConstructL();
	CAE_TRANS_DEF(UpdateInput, CAE_ObjectBa);
	CAE_TRANS_DEF(UpdateInpReset, CAE_ObjectBa);
	void SetTransRandomly();
public:
	CAE_TState<TInt>*	iInp;
	CAE_TState<TUint8>*	iInpReset;
	CAE_TState<TInt>*	iOut;
	// Fitness rate. It's usable for GA
	CAE_TState<TInt>*	iStFitness; 
private:
	CAE_TState<TUint8>*	iStInpReset;
	CAE_TState<TInt>*	iStInp;
	TInt iLen;
	TUint32* iNew;
	TUint32* iCurr;
	TUint32* iTrans;
	TInt iInpInfo; // The bit number in the bit mask
	TInt iOutInfo; // The bit number in the bit mask
};

struct TCmd
{
	TUint8 iCmd, iId;
};

class CAE_Link: public CAE_Object
{
public:
	FAPWS_API static CAE_Link* NewL(const char* aInstName, CAE_Object* aReg, CAE_State* aInp, CAE_State* aOut);
	virtual ~CAE_Link();
private:
	CAE_Link(const char* aInstName, CAE_Object* aMan);
	void ConstructL(CAE_State* aInp, CAE_State* aOut);
	void UpdateOut();
	static void UpdateOutS(CAE_Object* aObject, CAE_State* aState) {((CAE_Link*) aObject)->UpdateOut();};
private:
	CAE_State*	iOut;
	CAE_State*	iInp;
};


class CActive;
class CAE_ReqMon;


class CAE_StateASrv: public CAE_Object
{
	friend class CAE_ReqMon;
public:
	FAPWS_API static CAE_StateASrv* NewL(const char* aInstName, CAE_Object* aMan, TInt aPriority);
	FAPWS_API virtual ~CAE_StateASrv();
protected:
	void ConstructL(TInt aPriority);
	CAE_StateASrv(const char* aInstName,CAE_Object* aMan);
	virtual void DoCancel() {};
	virtual void RunL();
private:
	CAE_TRANS_DEF(UpdateSetActive, CAE_StateASrv);
	CAE_TRANS_DEF(UpdateCancel, CAE_StateASrv);
public:
	CAE_TState<TInt>*	iOutStatusPtr;
	CAE_State*	iOutStatus;
	CAE_TState<TUint8>*	iInpSetActive;
	CAE_TState<TUint8>*	iInpCancel;
private:
	CAE_ReqMon* iReqMon;
	CAE_TState<TUint8>*	iStSetActive;
	CAE_TState<TUint8>*	iStCancel;
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

class CAE_ReqMon: public CActive
{
public:
	CAE_ReqMon(TInt aPriority, CAE_StateASrv* aObs): CActive(aPriority), iObs(aObs) {};
	virtual void DoCancel() {iObs->DoCancel();};
	virtual void RunL() {iObs->RunL();};
private:
	CAE_StateASrv* iObs;
};

class CAE_ReqMonAs;

class CAE_ObjectAs: public CAE_Object
{
	friend class CAE_ReqMonAs;
public:
	FAPWS_API static CAE_ObjectAs* NewL(const char* aInstName, CAE_Object* aMan, TInt aPriority);
	FAPWS_API virtual ~CAE_ObjectAs();
protected:
	void ConstructL(TInt aPriority);
	CAE_ObjectAs(const char* aInstName,CAE_Object* aMan);
	void DoCancel() {};
	void RunL();
protected:
	CAE_TState<TInt>*	iStAsStatus;
	CAE_ReqMonAs* iReqMon;
};


class CAE_ReqMonAs: public CActive
{
public:
	CAE_ReqMonAs(TInt aPriority, CAE_ObjectAs* aObs): CActive(aPriority), iObs(aObs) {};
	virtual void DoCancel() {iObs->DoCancel();};
	virtual void RunL() {iObs->RunL();};
	void SetActive() {CActive::SetActive();};
private:
	CAE_ObjectAs* iObs;
};


inline MCAE_LogRec *CAE_State::Logger() { return iMan ? iMan->Logger(): NULL;}

#endif // __FAP_BASE_H
