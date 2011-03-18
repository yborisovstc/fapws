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
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) = 0;
	virtual CAE_StateBase* Context() = 0;
	virtual string ContextType() = 0;
	virtual string DataType(const CAE_StateBase& aState) = 0;
	virtual MCAE_LogRec *Logger() = 0;
};

class CSL_ExprBase
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
	virtual CSL_ExprBase* Clone() { return NULL;};
	void SetType(const string& aType);
	void AcceptArg(CSL_ExprBase& aArg);
	void AddArg(CSL_ExprBase& aArg);
	const string& Data() {return iData;};
	const vector<string>& Type() {return iType;};
	const string& RType() {return iType.at(0);};
	const string& AType() {return *(iType.rbegin());};
	void SetData(const string& aData) { iData = aData;};
	static TBool IsTuple(const string& aType) { return aType[0] == '[';};
	static string TupleElemType(const string& aType) { return aType.substr(1, aType.length() - 2);};
	string TypeLn(vector<string>::const_iterator& aTop) const;
	string TypeLn() const { vector<string>::const_iterator top = iType.end() - 1; return TypeLn(top);};
	TBool IsTypeOf(const string& aType) const;
	const vector<CSL_ExprBase*>& Args() { return iArgs;};
	static void ParseSet(const string& aData, vector<string>& aRes);
    protected:
	vector<string> iType;
	string iData;
	vector<CSL_ExprBase*> iArgs;
};

class CSL_FunBase: public CSL_ExprBase
{
    public:
	class Env: public MSL_ExprEnv
    {
	public:
	    Env(CSL_FunBase& aFun, MSL_ExprEnv& aEnv): iFun(aFun), iEnv(aEnv) {};
	public:
	virtual CSL_ExprBase* GetExpr(const string& aName, const string& aRtype);
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) { iEnv.SetExpr(aName, aExpr); };
	virtual CAE_StateBase* Context() { return iEnv.Context();};
	virtual string ContextType() { return iEnv.ContextType();};
	virtual string DataType(const CAE_StateBase& aState) { return iEnv.DataType(aState);};
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

class CSL_EfLtInt: public CSL_ExprBase
{
    public:
	CSL_EfLtInt(): CSL_ExprBase("TBool TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfLtInt(*this);};
};

class CSL_EfGtInt: public CSL_ExprBase
{
    public:
	CSL_EfGtInt(): CSL_ExprBase("TBool TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfGtInt(*this);};
};



class CSL_EfInp: public CSL_ExprBase
{
    public:
	CSL_EfInp(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};

class CSL_EfSet: public CSL_ExprBase
{
    public:
	CSL_EfSet(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};

class CSL_EfAddVectF: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF(): CSL_ExprBase("TVectF TVectF TVectF") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};

class CSL_EfTInt: public CSL_ExprBase
{
    public:
	CSL_EfTInt(): CSL_ExprBase("TInt") {};
	CSL_EfTInt(const string& aData): CSL_ExprBase("TInt", aData) {};
	CSL_EfTInt(TInt aData): CSL_ExprBase("TInt", ToStr(aData)) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const TInt& aData);
	static void FromStr(TInt& aData, const string& aStr);
};

class CSL_EfTBool: public CSL_ExprBase
{
    public:
	CSL_EfTBool(): CSL_ExprBase("TBool") {};
	CSL_EfTBool(const string& aData): CSL_ExprBase("TBool", aData) {};
	CSL_EfTBool(TBool& aData): CSL_ExprBase("TBool", ToStr(aData)) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const TBool& aData);
	static void FromStr(TBool& aData, const string& aStr);
};



class CF_TdVectF;
class CSL_EfVectF: public CSL_ExprBase
{
    public:
	CSL_EfVectF(): CSL_ExprBase("TVectF") {};
	CSL_EfVectF(const string& aData): CSL_ExprBase("TVectF", aData) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	static string ToStr(const CF_TdVectF& aData);
	static void FromStr(CF_TdVectF& aData, const string& aStr);
};


class CSL_EfAddVectF1: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF1(const string& aData): CSL_ExprBase("TVectF TVectF", aData) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};

class CSL_EfIf: public CSL_ExprBase
{
    public:
	CSL_EfIf(): CSL_ExprBase() {};
	CSL_EfIf(const string& aType): CSL_ExprBase(aType) {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfIf(*this);};
};



class CSL_EfAddInt: public CSL_ExprBase
{
    public:
	CSL_EfAddInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};


class CSL_EfAddInt_1: public CSL_ExprBase
{
    public:
	CSL_EfAddInt_1(CSL_ExprBase& aArg): CSL_ExprBase("TInt TInt") { iArgs.push_back(&aArg);};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
};

class CSL_EfSubInt: public CSL_ExprBase
{
    public:
	CSL_EfSubInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfSubInt(*this);};
};

class CSL_EfDivInt: public CSL_ExprBase
{
    public:
	CSL_EfDivInt(): CSL_ExprBase("TInt TInt TInt") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfDivInt(*this);};
};



class CSL_EfFilter: public CSL_ExprBase
{
    public:
	CSL_EfFilter(const string& aElemType): 
	    CSL_ExprBase("[" + aElemType + "] [" + aElemType + "] " + "(TBool " + aElemType + ")") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfFilter(*this);};
};

class CSL_EfCount: public CSL_ExprBase
{
    public:
	CSL_EfCount(const string& aElemType): CSL_ExprBase("TInt [" + aElemType + "]") {};
	virtual void Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType);
	virtual CSL_ExprBase* Clone() { return new CSL_EfCount(*this);};
};


class CAE_StateBase;
class CSL_Interpr: public MSL_ExprEnv
{
    public: 
	CSL_Interpr(MCAE_LogRec* aLogger);
	virtual ~CSL_Interpr();
	void EvalTrans(MAE_TransContext* aContext, CAE_StateBase* aState, const string& aTrans);
	const map<string, CSL_ExprBase*>& Exprs() { return iExprs;};
	virtual CSL_ExprBase* GetExpr(const string& aName, const string& aRtype);
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr);
	virtual CAE_StateBase* Context();
	virtual string ContextType();
	virtual string DataType(const CAE_StateBase& aState);
	virtual MCAE_LogRec *Logger() { return iLogger;};
    private:
	void ParseSet(const string& aData, vector<string>& aRes);
	static string GetStateDataType(const string& aStateType);
    private:
	map<string, CSL_ExprBase*> iExprs;
	CSL_ExprBase* iRootExpr;
	CAE_StateBase* iState;
	MCAE_LogRec* iLogger;
};

#endif // __FAP_DESLBASE_H
