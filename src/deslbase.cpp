

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deslbase.h"
#include "fapbase.h"
#include "fapstext.h"

CSL_ExprBase::CSL_ExprBase()
{
}

CSL_ExprBase::CSL_ExprBase(const string& aType)
{
    SetType(aType);
}

CSL_ExprBase::CSL_ExprBase(const string& aType, const string& aData): iData(aData)
{
    SetType(aType);
}

CSL_ExprBase::CSL_ExprBase(const CSL_ExprBase& aExp): iName(aExp.iName), iType(aExp.iType), iData(aExp.iData), iArgs(aExp.iArgs)
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
	if (expr != NULL) {
	    aArgr++;
	    // Known term
	    if (!expr->IsTypeOf(aReqType) && (aArgr != aArgs.end())) {
		expr->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	    }
	    else {
		aRes = expr;
	    }
	}
	else {
	    // Data - try to use proper constructor
	    CSL_ExprBase* constr = aEnv.GetExpr(aReqType, "");
	    if (constr != NULL) {
		constr->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	    }
	    else {
		// Raw term - no data
		aArgr++;
		aRes = new CSL_ExprBase("", argn);
	    }
	}
    }
}

const string CSL_ExprBase::AType() 
{
    return (iType.size() <= 1) ? string() :  (*(iType.rbegin()));
}

void CSL_ExprBase::ApplyArgs(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes, const string& aReqType)
{
    // Get first term from args and evaluate it
    if (aArgr != aArgs.end()) {
	CSL_ExprBase* evalres = NULL;
	EvalArgs(aEnv, AType(), aArgs, aArgr, evalres);
	Apply(aEnv, aArgs, aArgr, *evalres, aRes, aReqType);
    }
    else {
	// TODO YB Panic
    }
}

void CSL_ExprBase::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase* next = Clone();
	next->AcceptArg(aArg);
	if (!next->IsTypeOf(aReqType)) {
	    // TODO [YB] Should be next->AType() instead of aReqType?
	    next->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	}
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

void CSL_ExprBase::ParseSet(const string& aData, vector<string>& aRes)
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

TBool CSL_ExprBase::ParseTerms(const string& aData, vector<string>& aRes)
{
    size_t pos = 0;
    return ParseTerms(aData, pos, aRes);
}

TBool CSL_ExprBase::ParseTerms(const string& aData, size_t& aPos, vector<string>& aRes)
{
    TBool res = ETrue;
    while (aPos != string::npos && res && (aData[aPos] != ')')) {
	res = ParseTerm(aData, aPos, aRes);
    }
    return res;
}

TBool CSL_ExprBase::ParseTerm(const string& aData, size_t& aPos, vector<string>& aRes)
{
    TBool res = ETrue;
    size_t pb, pe = 0;
    pb = aData.find_first_not_of(' ', aPos);
    if (aData[pb] == '(') {
	vector<string> terms;
	size_t pbb = pb + 1;
	ParseTerms(aData, pbb, terms);
	string elem = aData.substr(pb+1, pbb-pb-1);
	aRes.push_back(elem);
	pe = aData.find_first_of(')', pbb);
	aPos = (pe == string::npos) ? pe, res = EFalse : pe + 1;
    }
    else {
	pe = aData.find_first_of(" )", pb);
	aPos = (pe == string::npos) ? pe : (aData[pe] == ')' ? pe: pe + 1);
	pe = (pe == string::npos) ? aData.size() : pe;
	string elem = aData.substr(pb, pe-pb);
	aRes.push_back(elem);
    }
    aPos = (aPos == aData.size()) ? string::npos : aPos;
    return res;
}

CSL_FunBase::CSL_FunBase(const string& aType, const string& aArgs, const string& aContents): CSL_ExprBase(aType, aContents) 
{
    SetAgrsInfo(aArgs);
}

void CSL_FunBase::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	AcceptArg(aArg);
	vector<string> args;
	ParseTerms(iData, args);
	vector<string>::iterator argsrest = args.begin();
	Env env(*this, aEnv);
	// If result type equals to name then don't request type for it is constructor of new type (not registered)
	CSL_ExprBase::EvalArgs(env, (RType().compare(Name()) == 0) ? "" : RType(), args, argsrest, aRes);
    }
}

CSL_ExprBase* CSL_FunBase::Env::GetExpr(const string& aName, const string& aRtype)
{
    if (iFun.iArgsInfo.count(aName) != 0) {
	return iFun.iArgs[iFun.iArgsInfo[aName]];
    }
    else {
	return iEnv.GetExpr(aName, aRtype);
    }
}

void CSL_FunBase::SetAgrsInfo(const string& aData)
{   
    size_t pb, pe = 0;
    int count = 0;
    while (pe != string::npos) {
	pb = aData.find_first_not_of(' ', pe);
	pe = aData.find(' ', pb);
	if (pe > pb) {
	   string elem = aData.substr(pb, pe-pb);
	   iArgsInfo[elem] = count++;
	}
    }
}



void CSL_EfStruct::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iFields.size() == 0) {
	// Field not defined, applying fields spec
	CSL_EfStruct* res  = (CSL_EfStruct*) Clone();
	res->iType.pop_back();
	vector<string> fldspecs;
	ParseTerms(aArg.Data(), fldspecs);
	int fid = fldspecs.size();
	for (vector<string>::const_reverse_iterator it = fldspecs.rbegin(); it != fldspecs.rend(); it++) {
	    const string& fldspec = *it;
	    size_t lb = fldspec.find("::");
	    if (lb == string::npos) {
		aEnv.Logger()->WriteFormat("Struct constructor: Type spec missed in field spec [%s]", fldspec.c_str());
	    }
	    size_t le = fldspec.size();
	    string fname = fldspec.substr(0, lb);
	    string ftype = fldspec.substr(lb+2, le - lb - -2);
	    res->iType.push_back(ftype);
	    res->iFields[fname] = felem(ftype, --fid);
	}
	res->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
    }
    else {
	if (iType.size() > 1) {
	    // Applying fields data 
	    CSL_EfStruct* res  = (CSL_EfStruct*) Clone();
	    res->AcceptArg(aArg);
	    res->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	}
    }
}

void CSL_EfFld::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	CSL_EfStruct* str = (CSL_EfStruct*) iArgs[0];
	aRes = str->GetField(iArgs[1]->Data())->Clone();
    }
}



void CSL_EfInp::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    string name = aArg.Data();
    CAE_StateBase* state = aEnv.Context();
    if (IsTuple(RType())) {
	string etype = TupleElemType(RType());
	CAE_StateBase::mult_point_inp_iterator beg =  state->MpInput_begin(name.c_str());
	CAE_StateBase::mult_point_inp_iterator end =  state->MpInput_end(name.c_str());
	for (CAE_StateBase::mult_point_inp_iterator it = beg; it != end; it++) {
	    CAE_State& sinp = *it;
	    if (etype.compare(aEnv.DataType(sinp)) == 0) { 
		if (aRes == NULL) {
		    aRes = new CSL_ExprBase(RType());
		}
		string ival = sinp.ValStr();
		CSL_ExprBase* elem = new CSL_ExprBase(etype, ival);
		aRes->AddArg(*elem);
	    }
	}
    }
    else {
	CAE_StateBase* sinp = state->Input(name.c_str());
	if (RType().compare(aEnv.DataType(*sinp)) == 0) { 
	    string ival = sinp->ValStr();
	    aRes = new CSL_ExprBase(RType(), ival);
	}
    }
}

void CSL_EfLtInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y;
	CSL_EfTInt::FromStr(x,iArgs[0]->Data());
	CSL_EfTInt::FromStr(y,aArg.Data());
	TBool res = x < y;
	aRes = new CSL_EfTBool(res);
    }
}


void CSL_EfGtInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y;
	CSL_EfTInt::FromStr(x,iArgs[0]->Data());
	CSL_EfTInt::FromStr(y,aArg.Data());
	TBool res = x > y;
	aRes = new CSL_EfTBool(res);
    }
}

void CSL_EfSet::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    string data = aArg.Data();
    CAE_StateBase* state = aEnv.Context();
    state->SetFromStr(data);
}

void CSL_EfAddInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    CSL_EfAddInt_1 int1(aArg);
    int1.ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
}

void CSL_EfAddInt_1::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    TInt x,y, res;
    CSL_EfTInt::FromStr(x, iArgs[0]->Data());
    CSL_EfTInt::FromStr(y, aArg.Data());
    res = x + y;
    aRes = new CSL_ExprBase("TInt", CSL_EfTInt::ToStr(res));
}

void CSL_EfSubInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y, res;
	CSL_EfTInt::FromStr(x, iArgs[0]->Data());
	CSL_EfTInt::FromStr(y, aArg.Data());
	res = x - y;
	aRes = new CSL_ExprBase("TInt", CSL_EfTInt::ToStr(res));
    }
}

void CSL_EfDivInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y, res = 0;
	CSL_EfTInt::FromStr(x, iArgs[0]->Data());
	CSL_EfTInt::FromStr(y, aArg.Data());
	if (y != 0)
	    res = x / y;
	aRes = new CSL_ExprBase("TInt", CSL_EfTInt::ToStr(res));
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

void CSL_EfTInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = new CSL_ExprBase("TInt", aArg.Data());
}

void CSL_EfFloat::FromStr(float& aData, const string& aStr)
{
    sscanf(aStr.c_str(), "%f", &aData);
}

string CSL_EfFloat::ToStr(const float& aData)
{
    int buflen = 30;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    sprintf(buf, "%f", aData);
    string res(buf);
    free(buf);
    return res;
}

void CSL_EfFloat::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = Clone(); 
    aRes->SetData(aArg.Data());
}

void CSL_EfAddFloat::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	float x,y, res;
	CSL_EfFloat::FromStr(x, iArgs[0]->Data());
	CSL_EfFloat::FromStr(y, aArg.Data());
	res = x + y;
	aRes = new CSL_EfFloat(res);
    }
}

void CSL_EfString::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = Clone(); 
    aRes->SetData(aArg.Data());
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

void CSL_EfTBool::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = new CSL_ExprBase("TInt", aArg.Data());
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

void CSL_EfVectF::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = new CSL_ExprBase("TVectF", aArg.Data());
}

void CSL_EfAddVectF::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    CSL_EfAddVectF1 f1(aArg.Data());
    f1.ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
}

void CSL_EfAddVectF1::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    CF_TdVectF x,y, res;
    CSL_EfVectF::FromStr(x, Data());
    CSL_EfVectF::FromStr(y, aArg.Data());
    res = x + y;
    string sres = CSL_EfVectF::ToStr(res);
    aRes = new CSL_ExprBase("TVectF", sres);
}

void CSL_EfIf::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TBool cond;
	CSL_EfTBool::FromStr(cond, iArgs[0]->Data());
	aRes = cond ? iArgs[1]: &aArg;
    }
}

void CSL_EfFilter::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TBool cond;
	string elemtype = TupleElemType(RType());
	CSL_ExprBase* filter = iArgs[0];
	aRes = new CSL_ExprBase(RType());
	for (vector<CSL_ExprBase*>::const_iterator eit = aArg.Args().begin(); eit != aArg.Args().end(); eit++) {
	    CSL_ExprBase* elem = *eit; 
	    vector<string> args;
	    vector<string>::iterator argr = args.end();
	    CSL_ExprBase* fres;
	    filter->Apply(aEnv, args, argr, *elem, fres, aReqType);
	    TBool passed;
	    CSL_EfTBool::FromStr(passed, fres->Data());
	    if (passed) {
		aRes->AddArg(*(elem->Clone()));
	    }
	}
    }
}

void CSL_EfCount::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    TInt count = aArg.Args().size();
    aRes = new CSL_EfTInt(count);
}


static map<string, string> KStateDataTypes;

CSL_Interpr::CSL_Interpr(MCAE_LogRec* aLogger): iLogger(aLogger)
{
    iRootExpr = new CSL_ExprBase();
    // Register embedded function
    SetExpr("VectF", new CSL_EfVectF());
    SetExpr("add", new CSL_EfAddInt());
    SetExpr("sub", new CSL_EfSubInt());
    SetExpr("div", new CSL_EfDivInt());
    SetExpr("add", new CSL_EfAddVectF());
    SetExpr("inp", new CSL_EfInp("TInt String"));
    SetExpr("inp", new CSL_EfInp("TVectF String"));
    SetExpr("inp", new CSL_EfInp("[TInt] String"));
    SetExpr("set", new CSL_EfSet("TInt TInt"));
    SetExpr("set", new CSL_EfSet("TVectF TVectF"));
    SetExpr("if", new CSL_EfIf("TInt TInt TInt TBool"));
    SetExpr("lt", new CSL_EfLtInt());
    SetExpr("gt", new CSL_EfGtInt());
    SetExpr("filter", new CSL_EfFilter("TInt"));
    SetExpr("count", new CSL_EfCount("TInt"));
    SetExpr("Struct", new CSL_EfStruct());
    SetExpr("TInt", new CSL_EfTInt());
    SetExpr("Float", new CSL_EfFloat());
    SetExpr("String", new CSL_EfString());
    SetExpr("Fld", new CSL_EfFld());

    // Set datatypes map
    KStateDataTypes["StInt"] = "TInt";
    KStateDataTypes["StVectF"] = "TVectF";
}

CSL_Interpr::~CSL_Interpr()
{
    delete iRootExpr;
}

void CSL_Interpr::EvalTrans(MAE_TransContext* aContext, CAE_StateBase* aState, const string& aTrans)
{
   iState = aState;
   size_t pb = 0, pe = 0;
   TBool fin = EFalse;
   do {
       pb = aTrans.find_first_not_of("\n\t ", pe);
       if (pb == string::npos) break;
       pe = aTrans.find('\n', pb);
       if (pe == string::npos) {
	   pe = aTrans.size();
	   fin = ETrue;
       }
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
	   qpb = line.find_first_of(':', nmpe + 1);
	   qpe = line.find(' ', qpb);
	   string funarg = line.substr(nmpe, qpb - nmpe);
	   string qual = line.substr(qpb, qpe-qpb);
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
	       CSL_ExprBase::ParseTerms(data, args);
	       vector<string>::iterator argsrest = args.begin();
	       CSL_ExprBase* res = NULL;
	       vector<string> stype;
	       CSL_ExprBase::ParseTerms(type, stype);
	       string rtype = stype.at(0);
	       if (funarg.find_first_not_of(' ') != string::npos) {
		   // Function
		   CSL_ExprBase* fun = new CSL_FunBase(type, funarg, data);
		   SetExpr(name, fun);
	       }
	       else {
		   // Expression
		   CSL_ExprBase::EvalArgs(*this, rtype, args, argsrest, res);
		   SetExpr(name, res);
	       }
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
	   CSL_ExprBase::ParseTerms(data, args);
	   vector<string>::iterator argsrest = args.begin();
	   CSL_ExprBase* res = new CSL_ExprBase();
	   exp->ApplyArgs(*this, args, argsrest, res, exp->AType());
       }
       const char* skw = kw.c_str();
   } while (!fin);
}

CSL_ExprBase* CSL_Interpr::GetExpr(const string& aName, const string& aRtype)
{
    CSL_ExprBase* res = NULL;
    if (aRtype.empty()) {
	res = iExprs[aName];
    }
    else {
	res = iExprs[aName + " " + aRtype];
	if (res == NULL) {
	    res = iExprs[aName + " *"];
	}
    }
    return res;
}

void CSL_Interpr::SetExpr(const string& aName, CSL_ExprBase* aExpr)
{
    aExpr->SetName(aName);
    iExprs[aName] = aExpr;
    for (vector<string>::const_iterator it = aExpr->Type().begin(); it != aExpr->Type().end(); it++) {
	iExprs[aName + " " + aExpr->TypeLn(it)] = aExpr;
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
