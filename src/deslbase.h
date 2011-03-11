#ifndef __FAP_DESLBASE_H
#define __FAP_DESLBASE_H

#include <vector>
#include <map>
#include <string>
#include "fapbase.h"

using namespace std;

// Simple functional language - bae

class MSL_Expr
{
    public:
	virtual void Apply(vector<string>::iterator aArg) = 0;
	virtual void SetType(const string& aType) = 0;
	virtual void SetContents(const string& aData) = 0;
};

class CSL_ExprBase;
class CAE_StateBase;

// Interface of Environment of expresstion 
class MSL_ExprEnv
{
    public:
	virtual CSL_ExprBase* GetExpr(const string& aTerm) = 0;
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
	CSL_ExprBase(MSL_ExprEnv& aEnv);
	CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType);
	CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType, const string& aData);
	CSL_ExprBase(const CSL_ExprBase& aExp);
	virtual ~CSL_ExprBase();
	static void EvalArgs(MSL_ExprEnv& aEnv, const string& aReqType, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes);
	void ApplyArgs(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes);
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual CSL_ExprBase* Clone() { return NULL;};
	void SetType(const string& aType);
	void AcceptArg(CSL_ExprBase& aArg);
	const vector<string>& Data() {return iData;};
	const string& SData() {return iData.at(0);};
	const vector<string>& Type() {return iType;};
	const string& RType() {return iType.at(0);};
	const string& AType() {return *(iType.rbegin());};
	void AddData(const string& aData) { iData.push_back(aData);};
	static TBool IsTuple(const string& aType) { return aType[0] == '[';};
	static string TupleElemType(const string& aType) { return aType.substr(1, aType.length() - 2);};
    protected:
	MCAE_LogRec *Logger() { return iEnv.Logger();};
    protected:
	MSL_ExprEnv& iEnv;
	vector<string> iType;
	vector<string> iData;
	vector<CSL_ExprBase*> iArgs;
};

class CSL_EfLtInt: public CSL_ExprBase
{
    public:
	CSL_EfLtInt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TBool TInt TInt") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual CSL_ExprBase* Clone() { return new CSL_EfLtInt(*this);};
};

class CSL_EfGtInt: public CSL_ExprBase
{
    public:
	CSL_EfGtInt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TBool TInt TInt") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual CSL_ExprBase* Clone() { return new CSL_EfGtInt(*this);};
};



class CSL_EfInp: public CSL_ExprBase
{
    public:
	CSL_EfInp(MSL_ExprEnv& aEnv, const string& aType): CSL_ExprBase(aEnv, aType) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfSet: public CSL_ExprBase
{
    public:
	CSL_EfSet(MSL_ExprEnv& aEnv, const string& aType): CSL_ExprBase(aEnv, aType) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfAddVectF: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TVectF TVectF TVectF") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfTInt: public CSL_ExprBase
{
    public:
	CSL_EfTInt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TInt") {};
	CSL_EfTInt(MSL_ExprEnv& aEnv, const string& aData): CSL_ExprBase(aEnv, "TInt", aData) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	static string ToStr(const TInt& aData);
	static void FromStr(TInt& aData, const string& aStr);
};

class CSL_EfTBool: public CSL_ExprBase
{
    public:
	CSL_EfTBool(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TBool") {};
	CSL_EfTBool(MSL_ExprEnv& aEnv, const string& aData): CSL_ExprBase(aEnv, "TBool", aData) {};
	CSL_EfTBool(MSL_ExprEnv& aEnv, TBool& aData): CSL_ExprBase(aEnv, "TBool") { iData.push_back(ToStr(aData));};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	static string ToStr(const TBool& aData);
	static void FromStr(TBool& aData, const string& aStr);
};



class CF_TdVectF;
class CSL_EfVectF: public CSL_ExprBase
{
    public:
	CSL_EfVectF(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TVectF") {};
	CSL_EfVectF(MSL_ExprEnv& aEnv, const string& aData): CSL_ExprBase(aEnv, "TVectF", aData) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	static string ToStr(const CF_TdVectF& aData);
	static void FromStr(CF_TdVectF& aData, const string& aStr);
};


class CSL_EfAddVectF1: public CSL_ExprBase
{
    public:
	CSL_EfAddVectF1(MSL_ExprEnv& aEnv, const string& aData): CSL_ExprBase(aEnv, "TVectF TVectF", aData) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfIf: public CSL_ExprBase
{
    public:
	CSL_EfIf(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv) {};
	CSL_EfIf(MSL_ExprEnv& aEnv, const string& aType): CSL_ExprBase(aEnv, aType) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual CSL_ExprBase* Clone() { return new CSL_EfIf(*this);};
};



class CSL_EfAddInt: public CSL_ExprBase
{
    public:
	CSL_EfAddInt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TInt TInt TInt") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};


class CSL_EfAddInt_1: public CSL_ExprBase
{
    public:
	CSL_EfAddInt_1(MSL_ExprEnv& aEnv, CSL_ExprBase& aArg): CSL_ExprBase(aEnv, "TInt TInt") { iArgs.push_back(&aArg);};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};


class CSL_EfFilter: public CSL_ExprBase
{
    public:
	CSL_EfFilter(MSL_ExprEnv& aEnv, const string& aElemType): 
	    CSL_ExprBase(aEnv, "[" + aElemType + "] [" + aElemType + "] " + "(TBool " + aElemType + ")") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual CSL_ExprBase* Clone() { return new CSL_EfFilter(*this);};
};


class CAE_StateBase;
class CSL_Interpr: public MSL_ExprEnv
{
    public: 
	CSL_Interpr(MCAE_LogRec* aLogger);
	virtual ~CSL_Interpr();
	void Interpret(const string& aProg, CAE_StateBase* aState);
	virtual CSL_ExprBase* GetExpr(const string& aTerm);
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
