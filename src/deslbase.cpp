

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deslbase.h"
#include "fapbase.h"
#include "fapstext.h"

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv): iEnv(aEnv)
{
}

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType): iEnv(aEnv)
{
    SetType(aType);
}

CSL_ExprBase::CSL_ExprBase(MSL_ExprEnv& aEnv, const string& aType, const string& aData): iEnv(aEnv), iData(aData)
{
    SetType(aType);
}

CSL_ExprBase::CSL_ExprBase(const CSL_ExprBase& aExp): iEnv(aExp.iEnv), iType(aExp.iType), iData(aExp.iData), iArgs(aExp.iArgs)
{
}

CSL_ExprBase::~CSL_ExprBase()
{
}

void CSL_ExprBase::EvalArgs(MSL_ExprEnv& aEnv, const string& aReqType, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    if (aArgr != aArgs.end()) {
	string argn = *aArgr;
	CSL_ExprBase* expr = aEnv.GetExpr(argn, aReqType);
	aArgr++;
	if (expr != NULL) {
	    // Known term
	    if (!expr->IsTypeOf(aReqType) && (aArgr != aArgs.end())) {
		expr->ApplyArgs(aArgs, aArgr, aRes);
	    }
	    else {
		aRes = expr;
	    }
	}
	else {
	    // Data
	    // TODO [YB] To check if the type is correct, or to use constructor
	    aRes = new CSL_ExprBase(aEnv, aReqType, argn);
	}
    }
}

void CSL_ExprBase::ApplyArgs(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    // Get first term from args and evaluate it
    if (aArgr != aArgs.end()) {
	CSL_ExprBase* evalres = NULL;
	EvalArgs(iEnv, AType(), aArgs, aArgr, evalres);
	Apply(aArgs, aArgr, *evalres, aRes);
    }
    else {
	// TODO YB Panic
    }
}

void CSL_ExprBase::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase* next = Clone();
	next->AcceptArg(aArg);
	next->ApplyArgs(aArgs, aArgr, aRes);
	if (aRes == NULL) {
	    aRes = next;
	}
	else {
	    delete next;
	}
    }
}

void CSL_ExprBase::SetType(const string& aType)
{
    size_t pb, pe = 0;
    while (pe != string::npos) {
	pb = aType.find_first_not_of(' ', pe);
	char em = ' ';
	if (aType[pb] == '(') {
	    em = ')';
	    pb++;
	}
	pe = aType.find(em, pb);
	if (pe > pb) {
	   string elem = aType.substr(pb, pe-pb);
	   iType.push_back(elem);
	}
	if (pe != string::npos) 
	    pe++;
    }
}

void CSL_ExprBase::AcceptArg(CSL_ExprBase& aArg)
{
    iArgs.push_back(&aArg);
    iType.pop_back();
}

void CSL_ExprBase::AddArg(CSL_ExprBase& aArg)
{
    iArgs.push_back(&aArg);
}

string CSL_ExprBase::TypeLn(vector<string>::const_iterator& aTop) const
{
    string res;
    for (vector<string>::const_iterator it = iType.begin(); it <= aTop; it++) {
	res += *it; 
	if (it < aTop)
	    res += " ";
    }
    return res;
}

TBool CSL_ExprBase::IsTypeOf(const string& aType) const
{
    return (aType.compare(TypeLn()) == 0);
}

void CSL_EfInp::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    string name = aArg.Data();
    CAE_StateBase* state = iEnv.Context();
    if (IsTuple(RType())) {
	string etype = TupleElemType(RType());
	CAE_StateBase::mult_point_inp_iterator beg =  state->MpInput_begin(name.c_str());
	CAE_StateBase::mult_point_inp_iterator end =  state->MpInput_end(name.c_str());
	for (CAE_StateBase::mult_point_inp_iterator it = beg; it != end; it++) {
	    CAE_State& sinp = *it;
	    if (etype.compare(iEnv.DataType(sinp)) == 0) { 
		if (aRes == NULL) {
		    aRes = new CSL_ExprBase(iEnv, RType());
		}
		string ival = sinp.ValStr();
		CSL_ExprBase* elem = new CSL_ExprBase(iEnv, etype, ival);
		aRes->AddArg(*elem);
	    }
	}
    }
    else {
	CAE_StateBase* sinp = state->Input(name.c_str());
	if (RType().compare(iEnv.DataType(*sinp)) == 0) { 
	    string ival = sinp->ValStr();
	    aRes = new CSL_ExprBase(iEnv, RType(), ival);
	}
    }
}

void CSL_EfLtInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TInt x,y;
	CSL_EfTInt::FromStr(x,iArgs[0]->Data());
	CSL_EfTInt::FromStr(y,aArg.Data());
	TBool res = x < y;
	aRes = new CSL_EfTBool(iEnv, res);
    }
}


void CSL_EfGtInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TInt x,y;
	CSL_EfTInt::FromStr(x,iArgs[0]->Data());
	CSL_EfTInt::FromStr(y,aArg.Data());
	TBool res = x > y;
	aRes = new CSL_EfTBool(iEnv, res);
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
    CSL_EfAddInt_1 int1(iEnv, aArg);
    int1.ApplyArgs(aArgs, aArgr, aRes);
}

void CSL_EfAddInt_1::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    TInt x,y, res;
    CSL_EfTInt::FromStr(x, iArgs[0]->Data());
    CSL_EfTInt::FromStr(y, aArg.Data());
    res = x + y;
    aRes = new CSL_ExprBase(iEnv, "TInt", CSL_EfTInt::ToStr(res));
}

void CSL_EfSubInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TInt x,y, res;
	CSL_EfTInt::FromStr(x, iArgs[0]->Data());
	CSL_EfTInt::FromStr(y, aArg.Data());
	res = x - y;
	aRes = new CSL_ExprBase(iEnv, "TInt", CSL_EfTInt::ToStr(res));
    }
}

void CSL_EfDivInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TInt x,y, res = 0;
	CSL_EfTInt::FromStr(x, iArgs[0]->Data());
	CSL_EfTInt::FromStr(y, aArg.Data());
	if (y != 0)
	    res = x / y;
	aRes = new CSL_ExprBase(iEnv, "TInt", CSL_EfTInt::ToStr(res));
    }
}


void CSL_EfTInt::FromStr(TInt& aData, const string& aStr)
{
    sscanf(aStr.c_str(), "%d", &aData);
}

string CSL_EfTInt::ToStr(const TInt& aData)
{
    int buflen = 10;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    sprintf(buf, "%d", aData);
    string res(buf);
    free(buf);
    return res;
}

void CSL_EfTInt::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    aRes = new CSL_ExprBase(iEnv, "TInt", aArg.Data());
}

void CSL_EfTBool::FromStr(TBool& aData, const string& aStr)
{
    if (aStr.compare("True") == 0) 
	aData = ETrue;
    else if (aStr.compare("False") == 0) 
	aData = EFalse;
    else {
	// TODO [YB] To handle
	aData = EFalse;
    }
}

string CSL_EfTBool::ToStr(const TBool& aData)
{
    int buflen = 10;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    sprintf(buf, "%s", aData ? "True" : "False");
    string res(buf);
    free(buf);
    return res;
}

void CSL_EfTBool::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    aRes = new CSL_ExprBase(iEnv, "TInt", aArg.Data());
}

void CSL_EfVectF::FromStr(CF_TdVectF& aData, const string& aStr)
{
    sscanf(aStr.c_str(), "(%f,%f)", &(aData.iX), &(aData.iY));
}

string CSL_EfVectF::ToStr(const CF_TdVectF& aData)
{
    int buflen = 80;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    sprintf(buf, "(%6.3f,%6.3f)", aData.iX, aData.iY);
    string res(buf);
    free(buf);
    return res;
}

void CSL_EfVectF::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    aRes = new CSL_ExprBase(iEnv, "TVectF", aArg.Data());
}

void CSL_EfAddVectF::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    CSL_EfAddVectF1 f1(iEnv, aArg.Data());
    f1.ApplyArgs(aArgs, aArgr, aRes);
}

void CSL_EfAddVectF1::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    CF_TdVectF x,y, res;
    CSL_EfVectF::FromStr(x, Data());
    CSL_EfVectF::FromStr(y, aArg.Data());
    res = x + y;
    string sres = CSL_EfVectF::ToStr(res);
    aRes = new CSL_ExprBase(iEnv, "TVectF", sres);
}

void CSL_EfIf::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TBool cond;
	CSL_EfTBool::FromStr(cond, iArgs[0]->Data());
	aRes = cond ? iArgs[1]: &aArg;
    }
}

void CSL_EfFilter::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aArgs, aArgr, aArg, aRes);
    }
    else {
	TBool cond;
	string elemtype = TupleElemType(RType());
	CSL_ExprBase* filter = iArgs[0];
	aRes = new CSL_ExprBase(iEnv, RType());
	for (vector<CSL_ExprBase*>::const_iterator eit = aArg.Args().begin(); eit != aArg.Args().end(); eit++) {
	    CSL_ExprBase* elem = *eit; 
	    vector<string> args;
	    vector<string>::iterator argr = args.end();
	    CSL_ExprBase* fres;
	    filter->Apply(args, argr, *elem, fres);
	    TBool passed;
	    CSL_EfTBool::FromStr(passed, fres->Data());
	    if (passed) {
		aRes->AddArg(*(elem->Clone()));
	    }
	}
    }
}

void CSL_EfCount::Apply(vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes)
{
    TInt count = aArg.Args().size();
    aRes = new CSL_EfTInt(iEnv, count);
}


static map<string, string> KStateDataTypes;

CSL_Interpr::CSL_Interpr(MCAE_LogRec* aLogger): iLogger(aLogger)
{
    iRootExpr = new CSL_ExprBase(*this);
    // Register embedded function
    SetExpr("VectF", new CSL_EfVectF(*this));
    SetExpr("add", new CSL_EfAddInt(*this));
    SetExpr("sub", new CSL_EfSubInt(*this));
    SetExpr("div", new CSL_EfDivInt(*this));
    SetExpr("add", new CSL_EfAddVectF(*this));
    SetExpr("inp", new CSL_EfInp(*this, "TInt TString"));
    SetExpr("inp", new CSL_EfInp(*this, "TVectF TString"));
    SetExpr("inp", new CSL_EfInp(*this, "[TInt] TString"));
    SetExpr("set", new CSL_EfSet(*this, "TInt TInt"));
    SetExpr("set", new CSL_EfSet(*this, "TVectF TVectF"));
    SetExpr("if", new CSL_EfIf(*this, "TInt TInt TInt TBool"));
    SetExpr("lt", new CSL_EfLtInt(*this));
    SetExpr("gt", new CSL_EfGtInt(*this));
    SetExpr("filter", new CSL_EfFilter(*this, "TInt"));
    SetExpr("count", new CSL_EfCount(*this, "TInt"));

    // Set datatypes map
    KStateDataTypes["StInt"] = "TInt";
    KStateDataTypes["StVectF"] = "TVectF";
}

CSL_Interpr::~CSL_Interpr()
{
    delete iRootExpr;
}

void CSL_Interpr::EvalTrans(MAE_TransContext* aContext, const string& aTrans)
{
   iState = aContext->GetState();
   size_t pb = 0, pe = 0;
   do {
       pb = aTrans.find_first_not_of("\n\t ", pe);
       pe = aTrans.find('\n', pb);
       if (pe == string::npos) break;
       string line = aTrans.substr(pb, pe - pb);
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
	   /*
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
		   string rtype;
		   CSL_ExprBase::EvalArgs(*this, rtype, args, argsrest, res);
		   SetExpr(name, res);
	       }
	       else {
		   // TODO [YB] Error "term was already defined
	       }
	   }
	   */
	   if (qual.compare("::") == 0) {
	       // Type definition
	       size_t tpb, tpe;
	       tpb = line.find_first_not_of(' ', qpe + 1);
	       tpe = line.find(":=", tpb);
	       string type = line.substr(tpb, tpe-tpb);
	       size_t epb, epe;
	       epb = tpe + 2;
	       epe = line.find('\n', epb);
	       string data = line.substr(epb, epe-epb);
	       vector<string> args;
	       ParseSet(data, args);
	       vector<string>::iterator argsrest = args.begin();
	       CSL_ExprBase* res = NULL;
	       vector<string> stype;
	       ParseSet(type, stype);
	       string rtype = stype.at(0);
	       CSL_ExprBase::EvalArgs(*this, rtype, args, argsrest, res);
	       SetExpr(name, res);
	       // TODO [YB] To check type
	   }
       }
       else {
	   // Expression
	   string state_dtype = GetStateDataType(iState->TypeName());
	   CSL_ExprBase* exp = GetExpr(kw, state_dtype);
	   size_t tpb, tpe;
	   tpb = line.find_first_not_of(' ', kwpe + 1);
	   tpe = line.find('\n', tpb);
	   string data = line.substr(tpb, tpe-tpb);
	   vector<string> args;
	   ParseSet(data, args);
	   vector<string>::iterator argsrest = args.begin();
	   CSL_ExprBase* res = new CSL_ExprBase(*this);
	   exp->ApplyArgs(args, argsrest, res);
       }
       const char* skw = kw.c_str();
   } while (pe != string::npos);
}

CSL_ExprBase* CSL_Interpr::GetExpr(const string& aName, const string& aRtype)
{
    CSL_ExprBase* res = aRtype.empty() ? iExprs[aName] : iExprs[aName + " " + aRtype];
    return res;
}

void CSL_Interpr::SetExpr(const string& aName, CSL_ExprBase* aExpr)
{
    iExprs[aName] = aExpr;
    for (vector<string>::const_iterator it = aExpr->Type().begin(); it != aExpr->Type().end(); it++) {
	iExprs[aName + " " + aExpr->TypeLn(it)] = aExpr;
    }
}

void CSL_Interpr::ParseSet(const string& aData, vector<string>& aRes)
{
    size_t pb, pe = 0;
    while (pe != string::npos) {
	pb = aData.find_first_not_of(' ', pe);
	pe = aData.find(' ', pb);
	if (pe > pb) {
	   string elem = aData.substr(pb, pe-pb);
	   aRes.push_back(elem);
	}
    }
}

CAE_StateBase* CSL_Interpr::Context()
{
    return iState;
}


string CSL_Interpr::GetStateDataType(const string& aStateType)
{
    return KStateDataTypes[aStateType];
}

string CSL_Interpr::ContextType() 
{ 
    return GetStateDataType(iState->TypeName());
};

string CSL_Interpr::DataType(const CAE_StateBase& aState)
{
    return GetStateDataType(aState.TypeName());
}
