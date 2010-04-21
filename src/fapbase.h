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

// Type of CAE element
enum TCaeElemType
{
    ECae_Unknown = 0,
    ECae_Object = 1,
    ECae_State = 2,
    ECae_Conn = 3,	// Connection
    ECae_Logspec = 4,	// Logging specification
    ECae_Dep = 5,	// Dependency
    ECae_Logdata = 6	// Logging data
};

// Type UID of class mask 
const TInt KObUid_BaseTypeMask = 0xffff0000;
// Type UID of variation of class mask 
const TInt KObUid_ModifTypeMask = 0x0000ffff;

const TInt KObUid_CAE_StateBase =	0x000010000;
const TInt KObUid_CAE_Object =		0x000020000;
const TInt KObUid_CAE_ObjectBa =	0x000030000;
const TInt KObUid_CAE_State =		0x000040000;

// Variations UIDs
const TInt KObUid_CAE_Var_NotSpecified =	0x00000000;
const TInt KObUid_CAE_Var_StateInt =	0x00000001;
const TInt KObUid_CAE_Var_StateUint =	0x00000002;
const TInt KObUid_CAE_Var_StateFloat =	0x00000003;
const TInt KObUid_CAE_Var_StateUint8 =	0x00000004; 
const TInt KObUid_CAE_Var_StateBool =	0x00000005; 
const TInt KObUid_CAE_Var_State_Ext =	0x00001000;   // Extensions

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
class CAE_StateBase;

typedef void (*TTransFun)(CAE_Object* aObject, CAE_State* aState);
typedef char *(*TLogFormatFun)(CAE_StateBase* aState, TBool aCurr);

// Object provider interface. That's like the similar in the Symbian OS but 
// static method is used to obtain type UID. That should allow to provide templated classed instance
class MObjectProvider
{
public:
	FAPWS_API virtual void* DoGetObject(TInt aTypeUid) = 0;
	template <class T> T* GetObject(T* aInst) {return aInst = static_cast<T*>(DoGetObject(aInst->ObjectUid())); };
};

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
    KBaseLe_Any = 2,	// Any time
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


class CAE_Base: public MObjectProvider
{
public:
	enum
	{
		KAbase_NameLen = 12
	};
public:
	CAE_Base(const char* aInstName, CAE_Object* aMan);
	FAPWS_API virtual ~CAE_Base();
	virtual void Confirm() = 0;
	virtual void Update() = 0;
	void SetActive();
	void SetUpdated();
	void SetName(const char *aName);
	const char* InstName() const { return iInstName;};
	void AddLogSpec(TInt aEvent, TInt aData);
protected:
	TInt GetLogSpecData(TInt aEvent) const;
public:
	char* iInstName;
	TBool	iUpdated, iActive;
	CAE_Object* iMan;
	vector<TLogSpecBase>* iLogSpec;
};


class CAE_StateBase: public CAE_Base
{
	friend class CAE_Object;
public:
	enum TObUid
	{
		EObUid = KObUid_CAE_StateBase
	};
	enum StateType
	{
		EType_Unknown = 0,
		EType_Input = 1,
		EType_Reg = 2,
		EType_Output = 3
	};
public:
	CAE_StateBase(const char* aInstName, TInt aLen, CAE_Object* aMan, StateType aType= CAE_StateBase::EType_Reg);
	static inline TInt ObjectUid(); 
	FAPWS_API virtual ~CAE_StateBase();
	FAPWS_API virtual void Confirm();
	FAPWS_API virtual void Update();
	TInt Len() const {return iLen; };
	FAPWS_API void AddInputL(CAE_StateBase* aState);
	FAPWS_API void Set(void* aNew);
	FAPWS_API void SetFromStr(const char *aStr);
	const void* Value() const { return iCurr;}
	void RefreshOutputs();
	void RefreshOutput(CAE_StateBase* aState);
	FAPWS_API CAE_StateBase* Input(TInt aInd);
	FAPWS_API CAE_StateBase* Input(const char* aInstName);
	FAPWS_API CAE_StateBase* Output(TInt aInd);
	FAPWS_API void Reset();
	TBool IsInput() { return iStateType == EType_Input;}
	TBool IsOutput() { return iStateType == EType_Output;}
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
	// From MObjectProvider
	FAPWS_API virtual void* DoGetObject(TInt aUid);
protected:
	FAPWS_API void ConstructL();
private:
	virtual void DoTrans() = 0;
	void AddOutputL(CAE_StateBase* aState);
	void RemoveOutput(CAE_StateBase* aState);
	void RemoveInput(CAE_StateBase* aState);
	const char *AccessType() const;
	static char *FmtData(void *aData, int aLen);
	void LogUpdate(TInt aLogData);
	inline MCAE_LogRec *Logger();
public:
	void	*iCurr, *iNew;
protected:
	TInt iLen;
protected:
	StateType iStateType;
	TUint8		iFlags;
	vector<CAE_StateBase*>* iInputsList;
	vector<CAE_StateBase*>* iOutputsList;
};

inline TInt CAE_StateBase::ObjectUid() { return KObUid_CAE_StateBase;} 


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

// State transition transformation
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

// Base class for state with transition function
// Transition function should be implemented as static function
// There can be two kinds of state with transition function:
// #1 Regular state. It has the transition function with only one argument- the pointer to state itself
// #2 Special state. It has the transition function with two arguments- the pointer to object and to the state
// Special state transition function can access to object data. So it should be used in the rare cases 
// TODO YB: Consider to combine CAE_State with CAE_StateBase
class CAE_State: public CAE_StateBase
{
public:
	enum TObUid
	{
		EObUid = KObUid_CAE_State
	};
public:
	static inline TInt ObjectUid(); 
public:
	FAPWS_API CAE_State(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, 
		StateType aType= CAE_StateBase::EType_Reg, TInt aDataTypeUid = KObUid_CAE_Var_NotSpecified);
	FAPWS_API static CAE_State* NewL(const char* aInstName, TInt aLen, CAE_Object* aMan,  TTransInfo aTrans, 
		StateType aType= CAE_StateBase::EType_Reg, TInt aDataTypeUid = KObUid_CAE_Var_NotSpecified);  
	// Set transition procedure. It may be callback function aFun or defined state operation
	// In this case aFun, aCbo should be set to NULL
	// The method verifies operation attempted and set the most passed operation if incorrect
	// Returns true if given operation was set correctly 
	FAPWS_API virtual TBool SetTrans(TTransInfo aTinfo);
	inline TTransInfo GetTrans();
	FAPWS_API virtual TOperationInfo OperationInfo(TUint8 aId) const;
	// From MObjectProvider
	FAPWS_API virtual void* DoGetObject(TInt aUid);
	inline TBool IsDataType(TInt aDataType) const;

private:
	FAPWS_API virtual void DoTrans();
	FAPWS_API virtual void DoOperation();
public:
	// Transition info
	TTransInfo iTrans;
protected:
	// UID of type of data. This is variant of object type. 
	// TODO YB Migrate to string based data type
	TInt iDataTypeUid;
};

inline TTransInfo CAE_State::GetTrans() { return iTrans; };

inline TBool CAE_State::IsDataType(TInt aDataType) const { return iDataTypeUid == aDataType; }

inline TInt CAE_State::ObjectUid() { return EObUid;} 


template <class T>
class CAE_TState: public CAE_State
{
public:
	static inline TInt ObjectUid() { return CAE_State::ObjectUid() | DataTypeUid();}; 
public:
	CAE_TState(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, StateType aType= CAE_StateBase::EType_Reg):
	  CAE_State(aInstName, sizeof(T), aMan, aTrans, aType, KObUid_CAE_Var_NotSpecified) {iDataTypeUid = DataTypeUid();};
	static CAE_TState* NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, StateType aType= CAE_StateBase::EType_Reg);
	T& operator~ () { return *((T*)iCurr); };
	const T& Value() { return *((T*) CAE_StateBase::Value()); }
	const T& operator() () { return Value(); }
	T& operator! () { return *((T*)iNew); };
	CAE_TState<T>& operator= (T aVal) { Set(&aVal); return *this;};
	static inline TInt DataTypeUid();
	inline static CAE_TState* Interpret(CAE_State* aPtr); 
	FAPWS_API virtual TBool SetTrans(TTransInfo aTinfo);
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
private:
	FAPWS_API virtual void DoOperation();
};

template <class T>
inline CAE_TState<T>* CAE_TState<T>::Interpret(CAE_State* aPtr) 
{ 
    return aPtr->IsDataType(DataTypeUid())?static_cast<CAE_TState<T>*>(aPtr):NULL; 
}; 

template <class T>
CAE_TState<T>* CAE_TState<T>::NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, StateType aType)
{ 
	CAE_TState* self = new CAE_TState(aInstName, aMan, aTrans, aType);
	self->ConstructL();
	return self;
}

//!! <<Comment Yuri Borisov -- Why do we need these static methods if there is already CAE_State::iDataTypeUid>>
//!! <<Is  iDataTypeUid redundant?>>
template<> inline TInt CAE_TState<TUint8>::DataTypeUid() {return KObUid_CAE_Var_StateUint8;}

template<> inline TInt CAE_TState<TInt>::DataTypeUid() {return KObUid_CAE_Var_StateInt;}

template<> inline TInt CAE_TState<TUint32>::DataTypeUid() {return KObUid_CAE_Var_StateUint;}

template<> inline TInt CAE_TState<TBool>::DataTypeUid() {return KObUid_CAE_Var_StateBool;}

// Chromosome manager

class MAE_ChroMan
{
    public:
	virtual void CopySpec(const void *aSrc, void **aDest) = 0;
	virtual void *GetChild(void *aSpec) = 0;
	virtual void *GetNext(void *aChildSpec) = 0;
	virtual char *GetType(void *aSpec) = 0;
	virtual char *GetName(void *aSpec) = 0;
	virtual CAE_StateBase::StateType GetAccessType(void *aSpec) = 0;
	virtual int GetLen(void *aSpec) = 0;
	virtual TCaeElemType FapType(void *aElement) = 0;
	virtual char *GetStrAttr(void *aSpec, const char *aName) = 0;
	virtual int GetAttrInt(void *aSpec, const char *aName) = 0;
};

// Provider of CAE elements
class MAE_Provider
{
public:
	virtual CAE_State* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan, 
		CAE_StateBase::StateType aType= CAE_StateBase::EType_Reg) const = 0;
	virtual CAE_State* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan, 
		CAE_StateBase::StateType aType= CAE_StateBase::EType_Reg) const = 0;
	virtual CAE_Base* CreateObjectL(TUint32 aTypeUid) const  = 0;
	virtual CAE_Base* CreateObjectL(const char *aName) const  = 0;
	virtual const TTransInfo* GetTransf(const char *aName) const  = 0;
	virtual void RegisterTransf(const TTransInfo *aName) = 0;
	virtual const CAE_Formatter* GetFormatter(int aUid) const  = 0;
	virtual void RegisterFormatter(CAE_Formatter *aForm) = 0;
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
class CAE_Object: public CAE_Base
{
public:
	enum TObUid
	{
		EObUid = KObUid_CAE_Object
	};
	enum TObSubtypeUid
	{
		EObStypeUid = KObUid_CAE_Var_NotSpecified
	};
	// Operation under chromosome
	enum TChromOper
	{
		EChromOper_Copy = 1,
		EChromOper_Rand = 2,
		EChromOper_Mutate = 3,
		EChromOper_Combine = 4 // Combine then mutate
	};
public:
	static inline TInt ObjectUid(); 
	inline MCAE_LogRec *Logger();
	FAPWS_API virtual ~CAE_Object();
	FAPWS_API static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, const TUint8* aChrom = NULL, MAE_Env* aEnv = NULL);
	// Variant for Chrom XML. In order for somooth transition to XML chrom.
	FAPWS_API static CAE_Object* NewL(const char* aInstName, CAE_Object* aMan, const void *aChrom = NULL, MAE_Env* aEnv = NULL);
	inline void RegisterCompL(CAE_Base* aComp);
	void UnregisterComp(CAE_Base* aComp);
	FAPWS_API virtual void Update();
	FAPWS_API virtual void Confirm();
	FAPWS_API virtual void* DoGetObject(TInt aUid);
	FAPWS_API void LinkL(CAE_State* aInp, CAE_StateBase* aOut, TTransFun aTrans = NULL);
	FAPWS_API void SetActive();
	FAPWS_API void SetUpdated();
	FAPWS_API CAE_Object* GetComp(const char* aName) const;
	FAPWS_API TInt CountCompWithType(TInt aUid = 0) const;
	FAPWS_API CAE_Object* GetNextCompByType(TInt aUid, int* aCtx = NULL) const;
	FAPWS_API CAE_Base* FindName(const char* aName) const;
	FAPWS_API CAE_State* GetInput(TUint32 aInd);
	FAPWS_API CAE_State* GetInput(const char *aName);
	FAPWS_API CAE_State* GetOutput(TUint32 aInd);
	FAPWS_API void DoMutation();
	FAPWS_API void Reset();
	FAPWS_API TBool IsTheSame(CAE_Object* aObj) const;
	// Create new inheritor of self. 
	FAPWS_API CAE_Object* CreateNewL(const char* aInstName, TChromOper aOper, const CAE_Object* aParent2 = NULL);
protected:
	FAPWS_API CAE_Object(const char* aInstName, CAE_Object* aMan, MAE_Env* aEnv = NULL);
	FAPWS_API void ConstructL();
	FAPWS_API void ConstructFromChromL();
	FAPWS_API void ConstructFromChromXL();
	FAPWS_API void SetChromosome(TChromOper aOper = EChromOper_Copy, const TUint8* aChrom1 = NULL, const TUint8* aChrom2 = NULL);
	// Version for XML-Chromosome
	FAPWS_API void SetChromosome(TChromOper aOper = EChromOper_Copy, const void* aChrom1 = NULL, const char* aChrom2 = NULL);
private:
	FAPWS_API CAE_State* GetStateByInd(TUint32 aInd);
	FAPWS_API CAE_State* GetStateByName(const char *aName);
	// Calculates the length of chromosome
	TInt ChromLen(const TUint8* aChrom) const;
	// Returns true if given index points to chromosome control tag
	TBool IsChromControlTag(TInt aInd, TInt aTagInd = 0) const;
public:
	TUint32 iFitness; 
	TUint32 iFitness1; 
protected:
	TInt iVariantUid;  // UID variant of object type.
private:
	vector<CAE_Base*>* iCompReg;
	TUint8*	iChrom;		// Chromosome, owned
	void* iChromX;		// Chromosome, XML based
	MAE_Env* iEnv;  // FAP Environment, not owned
};

inline TInt CAE_Object::ObjectUid() { return EObUid | EObStypeUid;} 

inline MCAE_LogRec *CAE_Object::Logger() {return iEnv ? iEnv->Logger(): NULL; }

inline void  CAE_Object::RegisterCompL(CAE_Base* aComp) 
{ 
    iCompReg->push_back(aComp); 
    aComp->iMan = this;
    if (aComp->iUpdated) SetUpdated();
};
	
// Bit based authomata 
class CAE_ObjectBa: public CAE_Object
{
public:
	enum TObUid
	{
		EObUid = KObUid_CAE_ObjectBa
	};
public:
	FAPWS_API virtual ~CAE_ObjectBa();
	FAPWS_API static CAE_ObjectBa* NewL(const char* aInstName, CAE_Object* aMan, TInt aLen, TInt aInpInfo, TInt aOutInfo);
	FAPWS_API void SetTrans(const TUint32* aTrans);
	FAPWS_API virtual void Update();
	FAPWS_API virtual void Confirm();
	CAE_StateBase*  Input() {return iInp;}
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
	FAPWS_API static CAE_Link* NewL(const char* aInstName, CAE_Object* aReg, CAE_StateBase* aInp, CAE_StateBase* aOut);
	virtual ~CAE_Link();
private:
	CAE_Link(const char* aInstName, CAE_Object* aMan);
	void ConstructL(CAE_StateBase* aInp, CAE_StateBase* aOut);
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


inline MCAE_LogRec *CAE_StateBase::Logger() { return iMan ? iMan->Logger(): NULL;}

#endif // __FAP_BASE_H
