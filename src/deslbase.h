#ifndef __FAP_DESLBASE_H
#define __FAP_DESLBASE_H

#include <vector>
#include <map>
#include <string>

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
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr) = 0;
	virtual CAE_StateBase* Context() = 0;
};

class CSL_ExprBase
{
    public:
	CSL_ExprBase(MSL_ExprEnv& aEnv);
	CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType);
	CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType, const string& aData);
	CSL_ExprBase(const CSL_ExprBase& aExp);
	virtual ~CSL_ExprBase();
	static void EvalArgs(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes);
	void ApplyA(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes);
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
	virtual void SetType(const string& aType);
	virtual void AddData(const string& aData) {iData.push_back(aData);};
	const vector<string>& AData() {return iData;};
	const string& Data() {return iData.at(0);};
    protected:
	MSL_ExprEnv& iEnv;
	vector<string> iType;
	vector<string> iData;
};

class CSL_EfInpInt: public CSL_ExprBase
{
    public:
	CSL_EfInpInt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TInt TString") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfInpt: public CSL_ExprBase
{
    public:
	CSL_EfInpt(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CSL_EfSet: public CSL_ExprBase
{
    public:
	CSL_EfSet(MSL_ExprEnv& aEnv): CSL_ExprBase(aEnv, "TInt") {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
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
	CSL_EfAddInt_1(MSL_ExprEnv& aEnv, const string& aData): CSL_ExprBase(aEnv, "TInt TInt", aData) {};
	virtual void Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes);
};

class CAE_StateBase;
class CSL_Interpr: public MSL_ExprEnv
{
    public: 
	CSL_Interpr();
	virtual ~CSL_Interpr();
	void Interpret(const string& aProg, CAE_StateBase* aState);
	virtual CSL_ExprBase* GetExpr(const string& aTerm);
	virtual void SetExpr(const string& aName, CSL_ExprBase* aExpr);
	virtual CAE_StateBase* Context();
    private:
	void SetArgs(const string& aData, vector<string>& aArgs);
    private:
	map<string, CSL_ExprBase*> iExprs;
	CSL_ExprBase* iRootExpr;
	CAE_StateBase* iState;
};

#endif // __FAP_DESLBASE_H
