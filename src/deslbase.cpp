

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deslbase.h"
#include "fapbase.h"

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv): iEnv(aEnv)
{
}

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType): iEnv(aEnv)
{
    SetType(aType);
}

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType, const string& aData): iEnv(aEnv)
{
    SetType(aType);
    AddData(aData);
}

CSL_ExprBase::CSL_ExprBase(const CSL_ExprBase& aExp): iEnv(aExp.iEnv), iType(aExp.iType), iData(aExp.iData)
{
}

CSL_ExprBase::~CSL_ExprBase()
{
}

/*
void CSL_ExprBase::ApplyA(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    string scont = *(iContents.begin());
    CSL_ExprBase* cont = iEnv.GetExpr(scont);
    // Get first term from args and evaluate it
    string argn = *aArgr;
    CSL_ExprBase* arg = iEnv.GetExpr(argn);
    CSL_ExprBase* avalres = NULL;
    if (arg != NULL) {
	// Known term
	arg->ApplyA(aArgs, ++aArgr, avalres);
    }
    else {
	// Data
	avalres = new CSL_ExprBase(iEnv, "", argn);
    }
    cont->Apply(aArgs, aArgr, *avalres, aRes);
}
*/

void CSL_ExprBase::EvalArgs(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    if (aArgr != aArgs.end()) {
	string argn = *aArgr;
	CSL_ExprBase* expr = aEnv.GetExpr(argn);
	aArgr++;
	if (expr != NULL) {
	    // Known term
	    if (aArgr != aArgs.end()) {
		expr->ApplyA(aArgs, aArgr, aRes);
	    }
	    else {
		aRes = expr;
	    }
	}
	else {
	    // Data
	    aRes = new CSL_ExprBase(aEnv, "", argn);
	}
    }
}

void CSL_ExprBase::ApplyA(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    // Get first term from args and evaluate it
    if (aArgr != aArgs.end()) {
	CSL_ExprBase* evalres = NULL;
	EvalArgs(iEnv, aArgs, aArgr, evalres);
	Apply(aArgs, aArgr, *evalres, aRes);
    }
    else {
	// TODO YB Panic
    }
}

void CSL_ExprBase::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
}

void CSL_ExprBase::SetType(const string& aType)
{
    size_t pb, pe = 0;
    while (pe != string::npos) {
	pb = aType.find_first_not_of(' ', pe);
	pe = aType.find(' ', pb);
	if (pe > pb) {
	   string elem = aType.substr(pb, pe-pb);
	   iType.push_back(elem);
	}
    }
}

void CSL_EfInpInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    string name = aArg.Data();
    CAE_StateBase* state = iEnv.Context();
    CAE_StateBase* sinp = state->Input(name.c_str());
    string ival = sinp->ValStr();
    aRes = new CSL_ExprBase(iEnv, "TInt", ival);
}

void CSL_EfInpt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    string name = aArg.Data();
    CAE_StateBase* state = iEnv.Context();
    aRes = new CSL_ExprBase(iEnv, "[TInt]");

    CAE_StateBase::mult_point_inp_iterator beg =  state->MpInput_begin(name.c_str());
    CAE_StateBase::mult_point_inp_iterator end =  state->MpInput_end(name.c_str());
    for (CAE_StateBase::mult_point_inp_iterator it = beg; it != end; it++) {
	CAE_State& sinp = *it;
	string ival = sinp.ValStr();
	aRes->AddData(ival);
    }
}


void CSL_EfSet::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    string data = aArg.Data();
    CAE_StateBase* state = iEnv.Context();
    state->SetFromStr(data);
}

void CSL_EfAddInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    CSL_EfAddInt_1 int1(iEnv, aArg.Data());
    int1.ApplyA(aArgs, aArgr, aRes);
}

void CSL_EfAddInt_1::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    const string& data = aArg.Data();
    int x,y, res;
    sscanf(Data().c_str(), "%d", &x);
    sscanf(data.c_str(), "%d", &y);
    res = x + y;
    string sres;
    char buf[10];
    memset(buf, 0, 10);
    sprintf(buf, "%d ", res);
    aRes = new CSL_ExprBase(iEnv, "TInt", buf);
}

CSL_Interpr::CSL_Interpr()
{
    iRootExpr = new CSL_ExprBase(*this);
    // Register embedded function
    SetExpr("_add_int", new CSL_EfAddInt(*this));
    SetExpr("_inp", new CSL_EfInpInt(*this));
    SetExpr("_set", new CSL_EfSet(*this));
}

CSL_Interpr::~CSL_Interpr()
{
    delete iRootExpr;
}

void CSL_Interpr::Interpret(const string& aProg, CAE_StateBase* aState)
{
   iState = aState;
   size_t pb = 0, pe = 0;
   do {
       pb = aProg.find_first_not_of("\n\t ", pe);
       pe = aProg.find('\n', pb);
       if (pe == string::npos) break;
       string line = aProg.substr(pb, pe - pb);
       // Select keyword
       size_t kwpe = line.find(' ');
       string kw = line.substr(0, kwpe);
       if (kw.compare("let") == 0) {
	   // Definition
	   size_t nmpb, nmpe;
	   nmpb = line.find_first_not_of(' ', kwpe + 1);
	   nmpe = line.find(' ', nmpb);
	   string name = line.substr(nmpb, nmpe-nmpb);
	   size_t qpb, qpe;
	   qpb = line.find_first_not_of(' ', nmpe + 1);
	   qpe = line.find(' ', qpb);
	   string qual = line.substr(qpb, qpe-qpb);
	   if (qual.compare(":=") == 0) {
	       // Assignment
	       CSL_ExprBase* nexp = GetExpr(name);
	       if (nexp == NULL) {
		   size_t tpb, tpe;
		   tpb = line.find_first_not_of(' ', qpe + 1);
		   tpe = line.find('\n', tpb);
		   string data = line.substr(tpb, tpe-tpb);
		   vector<string> args;
		   SetArgs(data, args);
		   vector<string>::iterator argsrest = args.begin();
		   CSL_ExprBase* res = NULL;
		   CSL_ExprBase::EvalArgs(*this, args, argsrest, res);
		   SetExpr(name, res);
	       }
	       else {
		   // TODO [YB] Error "term was already defined
	       }
	   }
	   else if (qual.compare("::") == 0) {
	       // Type definition
	       CSL_ExprBase* exp = GetExpr(name);
	       if (exp != NULL) {
		   size_t tpb, tpe;
		   tpb = line.find_first_not_of(' ', qpe + 1);
		   tpe = line.find('\n', tpb);
		   string type = line.substr(tpb, tpe-tpb);
		   exp->SetType(type);
	       }
	       else {
		   // TODO [YB] Error "term was already defined
	       }
	   }
       }
       else {
	   // Expression
	   CSL_ExprBase* exp = GetExpr(kw);
	   size_t tpb, tpe;
	   tpb = line.find_first_not_of(' ', kwpe + 1);
	   tpe = line.find('\n', tpb);
	   string data = line.substr(tpb, tpe-tpb);
	   vector<string> args;
	   SetArgs(data, args);
	   vector<string>::iterator argsrest = args.begin();
	   CSL_ExprBase* res = new CSL_ExprBase(*this);
	   exp->ApplyA(args, argsrest, res);
       }
   } while (pe != string::npos);
}

CSL_ExprBase* CSL_Interpr::GetExpr(const string& aName) 
{
    CSL_ExprBase* res = iExprs[aName];
    return res;
}

void CSL_Interpr::SetExpr(const string& aName, CSL_ExprBase* aExpr)
{
    iExprs[aName] = aExpr;
}

void CSL_Interpr::SetArgs(const string& aData, vector<string>& aArgs)
{
    size_t pb, pe = 0;
    while (pe != string::npos) {
	pb = aData.find_first_not_of(' ', pe);
	pe = aData.find(' ', pb);
	if (pe > pb) {
	   string elem = aData.substr(pb, pe-pb);
	   aArgs.push_back(elem);
	}
    }
}

CAE_StateBase* CSL_Interpr::Context()
{
    return iState;
}

