#ifndef __FAP_DESLBASE_H
#define __FAP_DESLBASE_H

#include <vector>
#include <map>
#include <string>
#include "fapbase.h"

using namespace std;

// Simple functional language - bae

class CSL_ExprBase;
class CAE_StateBase;

// Interface of Environment of expresstion 
class MSL_ExprEnv
{
    public:
	virtual CSL_ExprBase* GetExpr(const string& aTerm, const string& aRtype) = 0;
	virtual multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype, 
		multimap<string, CSL_ExprBase*>::iterator& aEnd) = 0;
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) = 0;
	virtual CAE_EBase* Context() = 0;
	virtual string ContextType() = 0;
	virtual string DataType(CAE_StateBase& aState) = 0;
	virtual MCAE_LogRec *Logger() = 0;
	virtual void GetConstructors(vector<string>& aRes) {};
};

class CSL_ExprBase: public CAE_Base
{
    public:
	CSL_ExprBase();
	CSL_ExprBase(const string& aType);
	CSL_ExprBase(const string& aType, const string& aData);
	CSL_ExprBase(const CSL_ExprBase& aExp);
	virtual ~CSL_ExprBase();
	static void EvalArgs(MSL_ExprEnv& aEnv, const string& aReqType, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes);
	void ApplyArgs(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes, const string& aReqType);
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_ExprBase(*this);};
	virtual string ToString() { return iData; };
	TBool operator ==(const CSL_ExprBase& aExpr);
	void SetType(const string& aType);
	void SetName(const string& aName) { iName = aName;};
	void AcceptArg(CSL_ExprBase& aArg);
	void AcceptArg() { iType.pop_back();};
	void AddArg(CSL_ExprBase& aArg);
	const string& Data() {return iData;};
	const string& Name() {return iName;};
	const vector<string>& EType() {return iType;};
	const string& RType() {return iType.at(0);};
	const string AType();
	void SetData(const string& aData) { iData = aData;};
	static TBool IsTuple(const string& aType) { return aType[0] == '[';};
	static string TupleElemType(const string& aType) { return aType.substr(1, aType.length() - 2);};
	string TypeLn(vector<string>::const_iterator& aTop) const;
	string TypeLn() const { vector<string>::const_iterator top = iType.end() - 1; return TypeLn(top);};
	TBool IsTypeOf(const string& aType) const;
	const vector<CSL_ExprBase*>& Args() { return iArgs;};
	static void ParseSet(const string& aData, vector<string>& aRes);
	static TBool ParseTerms(const string& aData, vector<string>& aRes);
	static TBool ParseTerms(const string& aData, size_t& aPos, vector<string>& aRes);
	static TBool ParseTerm(const string& aData, size_t& aPos, vector<string>& aRes);
	static inline const char *Type(); 
	// From CAE_Base
	virtual void *DoGetFbObj(const char *aName);
    protected:
	string iName;
	vector<string> iType;
	string iData;
	vector<CSL_ExprBase*> iArgs;
};

inline const char *CSL_ExprBase::Type() { return "ExprBase";} 

class CSL_FunBase: public CSL_ExprBase
{
    public:
	class Env: public MSL_ExprEnv
    {
	public:
	    Env(CSL_FunBase& aFun, MSL_ExprEnv& aEnv): iFun(aFun), iEnv(aEnv) {};
	public:
	virtual CSL_ExprBase* GetExpr(const string& aName, const string& aRtype);
	virtual	multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype, 
		multimap<string, CSL_ExprBase*>::iterator& aEnd);
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) { iEnv.SetExpr(aName, aExpr); };
	virtual CAE_EBase* Context() { return iEnv.Context();};
	virtual string ContextType() { return iEnv.ContextType();};
	virtual string DataType(CAE_StateBase& aState) { return iEnv.DataType(aState);};
	virtual MCAE_LogRec *Logger() { return iEnv.Logger();};
	private: 
	MSL_ExprEnv& iEnv;
	CSL_FunBase& iFun;
    };
    public:
	CSL_FunBase(const string& aType, const string& aArgs, const string& aContents);
	CSL_FunBase(const CSL_FunBase& aFun): CSL_ExprBase(aFun), iArgsInfo(aFun.iArgsInfo) {};
	void SetAgrsInfo(const string& aData);
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, 
		CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_FunBase(*this);};
    protected:
	map<string, int> iArgsInfo;
};

class CSL_EfStruct: public CSL_ExprBase
{
    public:
	typedef pair<string, int> felem;
    public:
	CSL_EfStruct(): CSL_ExprBase("Struct String") {};
	CSL_EfStruct(const CSL_EfStruct& aExpr): CSL_ExprBase(aExpr), iFields(aExpr.iFields) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, 
		CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfStruct(*this);};
	CSL_ExprBase* GetField(const string& aName) { return iArgs[iFields[aName].second]; };
	virtual string ToString();
    protected:
	map<string, felem> iFields;
};

// Generic vector. Arguments of constructor are: (ElemType Size)
class CSL_EfVectG: public CSL_ExprBase
{
    public:
	typedef pair<string, int> felem;
    public:
	CSL_EfVectG(): CSL_ExprBase("TVectG -"), iSize(0) {};
	CSL_EfVectG(const string& aElemType, TInt aSize);
	CSL_EfVectG(const CSL_EfVectG& aExpr): CSL_ExprBase(aExpr), iSize(aExpr.iSize), iElemType(aExpr.iElemType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, 
		CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfVectG(*this);};
	virtual string ToString();
    public:
	string iElemType;
	TInt iSize;
};

class CSL_EfFld: public CSL_ExprBase
{
    public:
	CSL_EfFld(): CSL_ExprBase("* String Struct") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfFld(*this);};
};


class CSL_EfAnd: public CSL_ExprBase
{
    public:
	CSL_EfAnd(): CSL_ExprBase("TBool TBool TBool") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAnd(*this);};
};

class CSL_EfEqInt: public CSL_ExprBase
{
    public:
	CSL_EfEqInt(): CSL_ExprBase("TBool TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfEqInt(*this);};
};

class CSL_EfLtInt: public CSL_ExprBase
{
    public:
	CSL_EfLtInt(): CSL_ExprBase("TBool TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfLtInt(*this);};
};

class CSL_EfGtInt: public CSL_ExprBase
{
    public:
	CSL_EfGtInt(): CSL_ExprBase("TBool TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfGtInt(*this);};
};


class CSL_EfLtFloat: public CSL_ExprBase
{
    public:
	CSL_EfLtFloat(): CSL_ExprBase("TBool Float Float") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfLtFloat(*this);};
};

class CSL_EfLeFloat: public CSL_ExprBase
{
    public:
	CSL_EfLeFloat(): CSL_ExprBase("TBool Float Float") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfLeFloat(*this);};
};


class CSL_EfInp: public CSL_ExprBase
{
    public:
	CSL_EfInp(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfInp(*this);};
};


class CSL_EfSet: public CSL_ExprBase
{
    public:
	CSL_EfSet(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfSet(*this);};
};

class CSL_EfAddVectF: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF(): CSL_ExprBase("TVectF TVectF TVectF") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddVectF(*this);};
};

// Addition of generic vector
class CSL_EfSmulVectG: public CSL_ExprBase
{
    public:
	CSL_EfSmulVectG(): CSL_ExprBase("Float TVectG TVectG") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfSmulVectG(*this);};
};

// Scalar multiplication of generic vector
class CSL_EfAddVectG: public CSL_ExprBase
{
    public:
	CSL_EfAddVectG(): CSL_ExprBase("TVectG TVectG TVectG") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddVectG(*this);};
};

class CSL_EfTBool: public CSL_ExprBase
{
    public:
	CSL_EfTBool(): CSL_ExprBase("TBool -") {};
	CSL_EfTBool(TBool aData): CSL_ExprBase("TBool", ToStr(aData)) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const TBool& aData);
	static void FromStr(TBool& aData, const string& aStr);
	virtual CSL_ExprBase* Clone() { return new CSL_EfTBool(*this);};
};

class CSL_EfTInt: public CSL_ExprBase
{
    public:
	CSL_EfTInt(): CSL_ExprBase("TInt -") {};
	CSL_EfTInt(TInt aData): CSL_ExprBase("TInt", ToStr(aData)) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const TInt& aData);
	static void FromStr(TInt& aData, const string& aStr);
	virtual CSL_ExprBase* Clone() { return new CSL_EfTInt(*this);};
};

class CSL_EfFloat: public CSL_ExprBase
{
    public:
	CSL_EfFloat(): CSL_ExprBase("Float -") {};
	CSL_EfFloat(float aData): CSL_ExprBase("Float", ToStr(aData)) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const float& aData);
	static void FromStr(float& aData, const string& aStr);
	virtual CSL_ExprBase* Clone() { return new CSL_EfFloat(*this);};
};

class CSL_EfString: public CSL_ExprBase
{
    public:
	CSL_EfString(): CSL_ExprBase("String -") {};
	CSL_EfString(const string& aData): CSL_ExprBase("String -", aData) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfString(*this);};
};

class CF_TdVectF;
class CSL_EfVectF: public CSL_ExprBase
{
    public:
	CSL_EfVectF(): CSL_ExprBase("TVectF -") {};
	CSL_EfVectF(const string& aData): CSL_ExprBase("TVectF", aData) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const CF_TdVectF& aData);
	static void FromStr(CF_TdVectF& aData, const string& aStr);
	virtual CSL_ExprBase* Clone() { return new CSL_EfVectF(*this);};
};


class CSL_EfAddVectF1: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF1(const string& aData): CSL_ExprBase("TVectF TVectF", aData) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddVectF1(*this);};
};

class CSL_EfIf: public CSL_ExprBase
{
    public:
	CSL_EfIf(): CSL_ExprBase() {};
	CSL_EfIf(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfIf(*this);};
};


class CSL_EfAddInt: public CSL_ExprBase
{
    public:
	CSL_EfAddInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddInt(*this);};
};


class CSL_EfAddInt_1: public CSL_ExprBase
{
    public:
	CSL_EfAddInt_1(CSL_ExprBase& aArg): CSL_ExprBase("TInt TInt") { iArgs.push_back(&aArg);};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddInt_1(*this);};
};

class CSL_EfSubInt: public CSL_ExprBase
{
    public:
	CSL_EfSubInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfSubInt(*this);};
};

class CSL_EfMplInt: public CSL_ExprBase
{
    public:
	CSL_EfMplInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfMplInt(*this);};
};

class CSL_EfDivInt: public CSL_ExprBase
{
    public:
	CSL_EfDivInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfDivInt(*this);};
};

class CSL_EfRandInt: public CSL_ExprBase
{
    public:
	CSL_EfRandInt(): CSL_ExprBase("TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfRandInt(*this);};
};

class CSL_EfAddFloat: public CSL_ExprBase
{
    public:
	CSL_EfAddFloat(): CSL_ExprBase("Float Float Float") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfAddFloat(*this);};
};




class CSL_EfFilter: public CSL_ExprBase
{
    public:
	CSL_EfFilter(const string& aElemType): 
	    CSL_ExprBase("[" + aElemType + "] [" + aElemType + "] " + "(TBool " + aElemType + ")") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfFilter(*this);};
};

class CSL_EfCount: public CSL_ExprBase
{
    public:
	CSL_EfCount(const string& aElemType): CSL_ExprBase("TInt [" + aElemType + "]") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
		CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfCount(*this);};
};


class CAE_StateBase;
class CSL_Interpr: public MSL_ExprEnv
{
    public: 
	typedef pair<string, CSL_ExprBase*> exprsmapelem;

	class Logger: public MCAE_LogRec 
    {
	public:
	    Logger(CSL_Interpr& aMain): iMain(aMain) {};
	public:
	    virtual void WriteRecord(const char* aText) { iMain.iLogger->WriteRecord(aText);};
	    virtual void WriteFormat(const char* aFmt,...);
	    virtual void Flush() { iMain.iLogger->Flush();};
	private:
	    CSL_Interpr& iMain;
    };
	Logger iELogger;
    friend class Logger;
    public: 
	CSL_Interpr(MCAE_LogRec* aLogger);
	virtual ~CSL_Interpr();
	void EvalTrans(MAE_TransContext* aContext, CAE_EBase* aExpContext, const string& aTrans);
	const multimap<string, CSL_ExprBase*>& Exprs() { return iExprs;};
	virtual CSL_ExprBase* GetExpr(const string& aName, const string& aRtype);
	virtual multimap<string, CSL_ExprBase*>::iterator GetExprs(const string& aName, const string& aRtype, 
		multimap<string, CSL_ExprBase*>::iterator& aEnd);
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr, multimap<string, CSL_ExprBase*>& aExprs);
	virtual void SetExprEmb(const string& aName, CSL_ExprBase* aExpr) {SetExpr(aName, aExpr, iExprsEmb);};
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) {SetExpr(aName, aExpr, iExprs);};
	virtual void SetExprEmb(const string& aName, const string& aType, CSL_ExprBase* aExpr);
	virtual CAE_EBase* Context();
	virtual string ContextType();
	virtual string DataType(CAE_StateBase& aState);
	virtual MCAE_LogRec *Logger() { return &iELogger;};
	virtual void GetConstructors(vector<string>& aRes);
    private:
	static string GetStateDataType(const string& aStateType);
    private:
	multimap<string, CSL_ExprBase*> iExprsEmb; // Embedded terms
	multimap<string, CSL_ExprBase*> iExprs;
	CSL_ExprBase* iRootExpr;
	CAE_EBase* iExpContext;
	MCAE_LogRec* iLogger;
	MAE_TransContext* iContext;
	int iLine;
};

#endif // __FAP_DESLBASE_H
