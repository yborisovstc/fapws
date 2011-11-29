

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deslbase.h"
#include "fapbase.h"
#include "fapstext.h"
#include "faplogger.h"

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

void *CSL_ExprBase::DoGetFbObj(const char *aName)
{
    if (strcmp(aName, Type()) == 0) {
	return this;
    }
    else 
	return NULL;
}

TBool CSL_ExprBase::operator ==(const CSL_ExprBase& aExpr)
{
    TBool res = ETrue;
    if (iData.compare(aExpr.iData) != 0) {
	res = EFalse;
    }
    else {
	if (iArgs.size() != aExpr.iArgs.size()) {
	    res = EFalse;
	}
	else {
	    for (TInt ct = 0; ct != iArgs.size(); ct++) {
		if (!(*iArgs[ct] == *aExpr.iArgs[ct])) {
		    res = EFalse; break;
		}
	    }
	}
    }
    return res;
}

void CSL_ExprBase::EvalArgs(MSL_ExprEnv& aEnv, const string& aReqType, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase*& aRes)
{
    if (aArgr != aArgs.end()) {
	string argn = *aArgr;
	aArgr++;
	// If name equals to result type then it is constructor - resolve by name only
	string type = (argn.compare(aReqType) == 0) || (aReqType.compare("*") == 0) ? "" : aReqType;
	vector<string>::iterator argrsaved = aArgr;
	multimap<string, CSL_ExprBase*>::iterator expsend, expi = aEnv.GetExprs(argn, type, expsend);
	string expkey = (expi != expsend) ? expi->first : "";
	// Try all appropriate expressions
	for (; expi != expsend && (expi->first.compare(expkey) == 0) && (aRes == NULL); expi++) {
	    CSL_ExprBase* expr = expi->second;
	    if (!expr->IsTypeOf(aReqType)) {
		aArgr = argrsaved;
		expr->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	    }
	    else {
		aRes = expr->Clone();
	    }
	}
	if (aRes == NULL) {
	    CSL_ExprBase* evalres = NULL;
	    CSL_ExprBase* expr = aEnv.GetExpr(argn, "*");
	    if (expr == NULL) {
		expr = aEnv.GetExpr(argn, "");
	    }
	    if (expr != NULL ) {
		if (!expr->IsTypeOf(aReqType) && expr->EType().size() > 1) {
		    aArgr = argrsaved;
		    expr->ApplyArgs(aEnv, aArgs, aArgr, evalres, aReqType);
		}
		else {
		    evalres = expr->Clone();
		}
	    }
	    else {
		evalres = new CSL_ExprBase("-", argn);
	    }

	    // Getting argument type for constructor
	    // TODO [YB] To implement attempting all constructor registered
	    if (evalres != NULL) {
		if (evalres->RType().compare(aReqType) == 0) {
		    aRes = evalres;
		}
		else {
		    CSL_ExprBase* constr = aEnv.GetExpr(aReqType, "~1 " + evalres->RType());
		    if (constr != NULL) {
			constr->Apply(aEnv, aArgs, aArgr, *evalres, aRes, aReqType);
		    }
		    // [YB] Disabled indirect constructors
		    /*
		       else  {
		    // No direct consructor found. Try next level.
		    constr = aEnv.GetExpr(aReqType, "");
		    if (constr && !((constr->AType().compare("-") == 0) && (evalres->RType().compare("-") != 0))) {
		    aArgr = argrsaved;
		    constr->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
		    }
		    else {
		    aEnv.Logger()->WriteFormat("Evaluating [%s ...]: Constructor [%s<-%s] not found", argn.c_str(), aReqType.c_str(), 
		    evalres->RType().c_str());
		    }
		    }
		    */
		    else {
			aEnv.Logger()->WriteFormat("ERROR: Evaluating [%s ...]: Constructor [%s<-%s] not found", argn.c_str(), aReqType.c_str(), 
				evalres->RType().c_str());
		    }
		}
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
	if (evalres != NULL) {
	    Apply(aEnv, aArgs, aArgr, *evalres, aRes, aReqType);
	}
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
    return ((aType.compare(TypeLn()) == 0) || ((iType.size() == 1) && (aType.compare("*")) == 0));
    //return (aType.compare(TypeLn()) == 0);
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
    else if (pb != string::npos) {
	pe = aData.find_first_of(" )", pb);
	aPos = (pe == string::npos) ? pe : (aData[pe] == ')' ? pe: pe + 1);
	pe = (pe == string::npos) ? aData.size() : pe;
	string elem = aData.substr(pb, pe-pb);
	aRes.push_back(elem);
    }
    else {
	res = EFalse;
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
	CSL_FunBase* next = (CSL_FunBase*) Clone();
	next->AcceptArg(aArg);
	vector<string> args;
	ParseTerms(iData, args);
	vector<string>::iterator argsrest = args.begin();
	Env env(*next, aEnv);
	CSL_ExprBase::EvalArgs(env, RType(), args, argsrest, aRes);
	delete next;
    }
}

CSL_ExprBase* CSL_FunBase::Env::GetExpr(const string& aName, const string& aRtype)
{
    CSL_ExprBase* res = NULL;
    if (iFun.iArgsInfo.count(aName) != 0) {
	CSL_ExprBase* arg = iFun.iArgs[iFun.iArgsInfo[aName]];
	if (aRtype.empty() || (aRtype.compare(arg->RType()) == 0)) {
	    res = arg;
	}
    }
    else {
	return iEnv.GetExpr(aName, aRtype);
    }
    return res;
}

multimap<string, CSL_ExprBase*>::iterator CSL_FunBase::Env::GetExprs(const string& aName, const string& aRtype,
	multimap<string, CSL_ExprBase*>::iterator& aEnd)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    if (iFun.iArgsInfo.count(aName) != 0) {
	CSL_ExprBase* arg = iFun.iArgs[iFun.iArgsInfo[aName]];
	if (aRtype.empty() || (aRtype.compare(arg->RType()) == 0)) {
	    iEnv.GetExprs(aName, aRtype, aEnd);
	    res = aEnd; res--;
	    (*res).second = arg;
	}
	else {
	    res = iEnv.GetExprs(aName, aRtype, aEnd);
	}
    }
    else {
	res = iEnv.GetExprs(aName, aRtype, aEnd);
    }
    return res;
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
	// Applying fields data 
	CSL_EfStruct* res  = (CSL_EfStruct*) Clone();
	res->AcceptArg(aArg);
	if (iType.size() > 2) {
	    res->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	}
	else
	    aRes = res;
    }
}

string CSL_EfStruct::ToString()
{
    string res("{");
    for (vector<CSL_ExprBase*>::iterator it = iArgs.begin(); it != iArgs.end(); it++) {
	res += ((it == iArgs.begin()) ? "" : ",") +  (*it)->ToString();
    }
    return res + "}";
}


CSL_EfVectG::CSL_EfVectG(const string& aElemType, TInt aSize): CSL_ExprBase("TVectG"), iElemType(aElemType), iSize(aSize)
{
    for (TInt i = 0; i < iSize; i++) {
	iType.push_back(iElemType);
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
	aRes = str->GetField(aArg.Data())->Clone();
    }
}

void CSL_EfVectG::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iElemType.empty()) {
	// Not defined, applying spec
	CSL_EfVectG* res  = (CSL_EfVectG*) Clone();
	res->iType.pop_back();
	vector<string> spec;
	ParseTerms(aArg.Data(), spec);
	if (spec.size() < 1 || spec.size() > 2) {
	    // Format should be TVectf (<Type> <Len>) <elements>
	    aEnv.Logger()->WriteFormat("Vector constructor: Incorrect spec");
	}
	else {
	    res->iElemType = spec.at(0);
	    if (spec.size() == 2) {
		CSL_EfTInt::FromStr(res->iSize, spec.at(1));
		for (TInt i = 0; i < res->iSize; i++) {
		    res->iType.push_back(res->iElemType);
		}
	    }
	}
	res->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
    }
    else {
	// Applying fields data 
	CSL_EfVectG* res  = (CSL_EfVectG*) Clone();
	res->AcceptArg(aArg);
	if (iType.size() > 2) {
	    res->ApplyArgs(aEnv, aArgs, aArgr, aRes, aReqType);
	}
	else
	    aRes = res;
    }
}


string CSL_EfVectG::ToString()
{
    string res("{");
    for (vector<CSL_ExprBase*>::iterator it = iArgs.begin(); it != iArgs.end(); it++) {
	res += ((it == iArgs.begin()) ? "" : ",") +  (*it)->ToString();
    }
    return res + "}";
}

void CSL_EfAddVectG::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	CSL_ExprBase& arg0 = *(iArgs[0]);
	CSL_EfVectG* arg0v = (CSL_EfVectG*) &arg0;
	CSL_ExprBase& arg1 = aArg;
	CSL_EfVectG* res = new CSL_EfVectG(arg0v->iElemType, arg0v->iSize);
	for (TInt i = 0; i < res->iSize; i++) {
	    // Get add expression for element's type
	    CSL_ExprBase* addexp = aEnv.GetExpr("add", "~1 " + res->iElemType)->Clone();
	    CSL_ExprBase* arg0elt = arg0.Args()[i];
	    CSL_ExprBase* arg1elt = arg1.Args()[i];
	    addexp->AcceptArg(*arg0elt);
	    CSL_ExprBase* elemres = NULL;
	    addexp->Apply(aEnv, aArgs, aArgr, *arg1elt, elemres, res->iElemType);
	    res->AcceptArg(*elemres);
	}
	aRes = res;
    }
}

// [YB] TODO To try to implement smul with result type given from args
/*
void CSL_EfSmulVectG::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	CSL_ExprBase& arg0 = *(iArgs[0]);
	CSL_EfVectG* arg0v = (CSL_EfVectG*) &arg0;
	CSL_ExprBase& arg1 = aArg;
	CSL_ExprBase* addexp = aEnv.GetExpr("add", "~1 " + res->iElemType)->Clone();
	CSL_ExprBase* res = aEnv.GetExpr(res->iElemType);
	res->AcceptArg(*sumarg);
	for (TInt i = 0; i < res->iSize; i++) {
	    // Get add expression for element's type
	    CSL_ExprBase* mplr = aEnv.GetExpr("mpl", "~1 " + res->iElemType)->Clone();
	    CSL_ExprBase* arg0elt = arg0.Args()[i];
	    CSL_ExprBase* arg1elt = arg1.Args()[i];
	    mplr->AcceptArg(*arg0elt);
	    CSL_ExprBase* elemres = NULL;
	    mplr->Apply(aEnv, aArgs, aArgr, *arg1elt, elemres, res->iElemType);
	    res->AcceptArg(*mpl);
	}
	aRes = res;
    }
}
*/
void CSL_EfSmulVectG::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	CSL_ExprBase& arg0 = *(iArgs[0]);
	CSL_EfVectG* arg0v = (CSL_EfVectG*) &arg0;
	CSL_ExprBase& arg1 = aArg;
	float sum = 0.0;
	for (TInt i = 0; i < arg0v->iSize; i++) {
	    float x,y;
	    CSL_ExprBase* arg0elt = arg0.Args()[i];
	    CSL_ExprBase* arg1elt = arg1.Args()[i];
	    CSL_EfFloat::FromStr(x, arg0elt->Data());
	    CSL_EfFloat::FromStr(y, arg1elt->Data());
	    sum += x * y;
	}
	aRes = new CSL_EfFloat(sum);
    }
}

void CSL_EfInp::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    string name = aArg.Data();
    CAE_StateBase* state = aEnv.Context()->GetFbObj(state);
    string etype = TupleElemType(RType());
    CAE_StateBase::mult_point_inp_iterator beg =  state->MpInput_begin(name.c_str());
    CAE_StateBase::mult_point_inp_iterator end =  state->MpInput_end(name.c_str());
    CAE_StateBase::mult_point_inp_iterator itc = beg;
    itc++;
    if (beg != end && itc != end) {
	string type = aEnv.DataType(*itc);
	// TODO [YB] Migrate to tuple constructor
	aRes = new CSL_ExprBase("[" + type + "]");
	// Multipoint input - result in tuple
	for (CAE_StateBase::mult_point_inp_iterator it = beg; it != end; it++) {
	    CAE_State& sinp = *it;
	    string ival = sinp.ValStr();
	    CSL_ExprBase* elem = new CSL_ExprBase(etype, ival);
	    aRes->AddArg(*elem);
	}
    }
    else {
	// Singlepoint input
	CAE_StateBase* sinp = state->Input(name.c_str());
	if (sinp != NULL) {
	    CAE_StateEx *sinpex = sinp->GetFbObj(sinpex);
	    if (sinpex != NULL) {
		aRes = sinpex->Value().Clone();
	    }
	    else {
		string ival = sinp->ValStr();
		string type = aEnv.DataType(*sinp);
		CSL_ExprBase* constr = aEnv.GetExpr(type, "");
		CSL_ExprBase data("-", ival);
		if (constr != NULL) {
		    constr->Apply(aEnv, aArgs, aArgr, data, aRes, aReqType);
		}
		else {
		    aEnv.Logger()->WriteFormat("ERROR: Evaluating input [%s]: Constructor [%s] not found", name.c_str(), type.c_str());
		}
	    }
	}
	else {
	    aEnv.Logger()->WriteFormat("ERROR: Evaluating input [%s]: input not connected", name.c_str());
	}
    }
}

void CSL_EfAnd::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TBool x,y;
	CSL_EfTBool::FromStr(x,iArgs[0]->Data());
	CSL_EfTBool::FromStr(y,aArg.Data());
	TBool res = x && y;
	aRes = new CSL_EfTBool(res);
    }
}

void CSL_EfEqInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y;
	CSL_EfTInt::FromStr(x,iArgs[0]->Data());
	CSL_EfTInt::FromStr(y,aArg.Data());
	TBool res = x == y;
	aRes = new CSL_EfTBool(res);
    }
}

void CSL_EfLtInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
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

void CSL_EfLtFloat::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	float x,y;
	CSL_EfFloat::FromStr(x,iArgs[0]->Data());
	CSL_EfFloat::FromStr(y,aArg.Data());
	TBool res = x < y;
	aRes = new CSL_EfTBool(res);
    }
}

void CSL_EfLeFloat::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	float x,y;
	CSL_EfFloat::FromStr(x,iArgs[0]->Data());
	CSL_EfFloat::FromStr(y,aArg.Data());
	TBool res = x <= y;
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
    CAE_StateBase* state = aEnv.Context()->GetFbObj(state);
    CAE_StateEx* stateext = state->GetFbObj(stateext);
    if (stateext != NULL) {
	stateext->Set(&aArg);
    }
    else {
	state->SetFromStr(data);
    }
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


void CSL_EfMplInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    if (iType.size() > 2) {
	CSL_ExprBase::Apply(aEnv, aArgs, aArgr, aArg, aRes, aReqType);
    }
    else {
	TInt x,y, res = 0;
	CSL_EfTInt::FromStr(x, iArgs[0]->Data());
	CSL_EfTInt::FromStr(y, aArg.Data());
	if (y != 0)
	    res = x * y;
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

void CSL_EfRandInt::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, CSL_ExprBase*& aRes, const string& aReqType)
{
    TInt res = rand();
    aRes = new CSL_EfTInt(res);
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
    aRes = Clone();
    aRes->AcceptArg();
    aRes->SetData(aArg.Data());
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
    aRes->AcceptArg();
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
    aRes->AcceptArg();
    aRes->SetData(aArg.Data());
}


void CSL_EfTBool::FromStr(TBool& aData, const string& aStr)
{
    aData = (aStr.compare(0, 4, "True") == 0) || (aStr.compare(0, 4, "true") == 0);
}

string CSL_EfTBool::ToStr(const TBool& aData)
{
    return aData ? "True" : "False";
}

void CSL_EfTBool::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
{
    aRes = Clone();
    aRes->AcceptArg();
    TBool res;
    FromStr(res, aArg.Data());
    aRes->SetData(ToStr(res));
}

void CSL_EfVectF::FromStr(CF_TdVectF& aData, const string& aStr)
{
    sscanf(aStr.c_str(), "{%f,%f}", &(aData.iX), &(aData.iY));
}

string CSL_EfVectF::ToStr(const CF_TdVectF& aData)
{
    int buflen = 80;
    char* buf = (char *) malloc(buflen);
    memset(buf, 0, buflen);
    sprintf(buf, "{%6.3f,%6.3f}", aData.iX, aData.iY);
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

void CSL_EfIf::Apply(MSL_ExprEnv& aEnv, vector<string>& aArgs, vector<string>::iterator& aArgr, CSL_ExprBase& aArg, 
	CSL_ExprBase*& aRes, const string& aReqType)
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

CSL_Interpr::CSL_Interpr(MCAE_LogRec* aLogger): iLogger(aLogger), iELogger(*this), iContext(NULL)
{ 
    // Initialize random seed 
    srand ( time(NULL) );
    iRootExpr = new CSL_ExprBase();
    // Register embedded function
    SetExprEmb("TVectF", new CSL_EfVectF());
    SetExprEmb("add", new CSL_EfAddInt());
    SetExprEmb("add", new CSL_EfAddVectF());
    SetExprEmb("add", new CSL_EfAddFloat());
    SetExprEmb("add", new CSL_EfAddVectG());
    SetExprEmb("sub", new CSL_EfSubInt());
    SetExprEmb("div", new CSL_EfDivInt());
    SetExprEmb("mpl", new CSL_EfMplInt());
    SetExprEmb("rand", new CSL_EfRandInt());
    SetExprEmb("smul", new CSL_EfSmulVectG());
    SetExprEmb("inp", new CSL_EfInp("* String"));
    //SetExprEmb("inp", new CSL_EfInp("TInt String"));
    //SetExprEmb("inp", new CSL_EfInp("TVectF String"));
    //SetExprEmb("inp", new CSL_EfInp("[TInt] String"));
    //SetExprEmb("set", new CSL_EfSet("TInt TInt"));
    //SetExprEmb("set", new CSL_EfSet("TVectF TVectF"));
    SetExprEmb("if", new CSL_EfIf("TInt TInt TInt TBool"));
    SetExprEmb("if", new CSL_EfIf("* * * TBool"));
    SetExprEmb("and", new CSL_EfAnd());
    SetExprEmb("eq", new CSL_EfEqInt());
    SetExprEmb("lt", new CSL_EfLtInt());
    SetExprEmb("lt", new CSL_EfLtFloat());
    SetExprEmb("le", new CSL_EfLeFloat());
    SetExprEmb("gt", new CSL_EfGtInt());
    SetExprEmb("filter", new CSL_EfFilter("TInt"));
    SetExprEmb("count", new CSL_EfCount("TInt"));
    SetExprEmb("Struct", new CSL_EfStruct());
    SetExprEmb("Struct", "-", new CSL_EfStruct());
    SetExprEmb("TBool", new CSL_EfTBool());
    SetExprEmb("TInt", new CSL_EfTInt());
    SetExprEmb("Float", new CSL_EfFloat());
    SetExprEmb("String", new CSL_EfString());
    SetExprEmb("Fld", new CSL_EfFld());
    SetExprEmb("TVectG", new CSL_EfVectG());

    // Set datatypes map
    KStateDataTypes["StInt"] = "TInt";
    KStateDataTypes["StBool"] = "TBool";
    KStateDataTypes["StVectF"] = "TVectF";
    KStateDataTypes["TVectG"] = "TVectG";
}

CSL_Interpr::~CSL_Interpr()
{
    delete iRootExpr;
}

void CSL_Interpr::EvalTrans(MAE_TransContext* aContext, CAE_EBase* aExpContext, const string& aTrans)
{
    iContext = aContext;
    iExprs.clear();
    iExpContext = aExpContext;
    TInt logspecdata = iExpContext->GetLogSpecData(KBaseLe_Trans);
    size_t pb = 0, pe = 0;
    TBool fin = EFalse;
    iLine = 0;
    do {
	iLine++;
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
	if (kw.compare("rem") == 0) {
	}
	else if (kw.compare("let") == 0) {
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
		    if (res != NULL) {
			SetExpr(name, res);
			if (logspecdata & KBaseDa_Trex ) {
			    Logger()->WriteFormat("[%s] = %s", name.c_str(), res->ToString().c_str());
			}
		    }
		    else {
			Logger()->WriteFormat("Error evaluating [%s] as [%s]", name.c_str(), data.c_str());
		    }
		}
		// TODO [YB] To check type
	    }
	}
	else {
	    // Expression
	    // TODO [YB] To cleanup here
	    CAE_StateBase* state = aExpContext->GetFbObj(state);
	    string state_dtype = DataType(*state);
	    if (state_dtype.empty()) {
		Logger()->WriteFormat("Couldn't get state data type");
	    }
	    /*
	    size_t tpb, tpe;
	    tpb = line.find_first_not_of(' ', kwpe + 1);
	    tpe = line.find('\n', tpb);
	    string data = line.substr(tpb, tpe-tpb);
	    */
	    vector<string> args;
	    CSL_ExprBase::ParseTerms(line, args);
	    vector<string>::iterator argsrest = args.begin();
	    CSL_ExprBase* res = NULL;
	    CSL_ExprBase::EvalArgs(*this, state_dtype, args, argsrest, res);
	    if (res != NULL) {
		CAE_StateEx* stateext = state->GetFbObj(stateext);
		if (stateext != NULL) {
		    stateext->Set(res);
		}
		else {
		    state->SetFromStr(res->Data());
		}
		delete res;
	    }
	    else {
		Logger()->WriteFormat("Error evaluating [%s]", line.c_str());
	    }
	}
    } while (!fin);
}

CSL_ExprBase* CSL_Interpr::GetExpr(const string& aName, const string& aRtype)
{
    // Look at emb terms first then for current
    CSL_ExprBase* res = NULL;
    string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
    multimap<string, CSL_ExprBase*>::iterator it = iExprsEmb.find(key);
    if (it != iExprsEmb.end()) {
	res = it->second;
    }
    if (res == NULL) {
	multimap<string, CSL_ExprBase*>::iterator it = iExprs.find(key);
	if (it != iExprs.end()) {
	    res = it->second;
	}
    }
    return (res != NULL) ? res : ((iContext != NULL) ? iContext->GetExpr(aName, aRtype) : res);
}

multimap<string, CSL_ExprBase*>::iterator CSL_Interpr::GetExprs(const string& aName, const string& aRtype, 
	multimap<string, CSL_ExprBase*>::iterator& aEnd)
{
    multimap<string, CSL_ExprBase*>::iterator res;
    // Look at emb terms first then for current
    string key = (aRtype.size() == 0) ? aName : aName + " " + aRtype;
    multimap<string, CSL_ExprBase*>::iterator it = iExprsEmb.find(key);
    if (it != iExprsEmb.end()) {
	res = it; aEnd = iExprsEmb.end();
    }
    else {
	multimap<string, CSL_ExprBase*>::iterator it = iExprs.find(key);
	if (it != iExprs.end()) {
	    res = it; aEnd = iExprs.end();
	}
	else {
	    multimap<string, CSL_ExprBase*>::iterator it = iContext->GetExprs(aName, aRtype, aEnd);
	    if (it != aEnd) {
		res = it;
	    }
	    else {
		aEnd = iExprsEmb.end(); res = aEnd;
	    }
	}
    }
    return res;
}

void CSL_Interpr::SetExpr(const string& aName, CSL_ExprBase* aExpr, multimap<string, CSL_ExprBase*>& aExprs)
{
    aExpr->SetName(aName);
    aExprs.insert(exprsmapelem(aName, aExpr));
    // Register expr with first arg type
    if (aExpr->EType().size() > 1) {
	aExprs.insert(exprsmapelem(aName + " ~1 " + *(aExpr->EType().rbegin()), aExpr));
    }
    for (vector<string>::const_iterator it = aExpr->EType().begin(); it != aExpr->EType().end(); it++) {
	aExprs.insert(exprsmapelem(aName + " " + aExpr->TypeLn(it), aExpr));
    }
}

void CSL_Interpr::SetExprEmb(const string& aName, const string& aType, CSL_ExprBase* aExpr)
{
    aExpr->SetName(aName);
    iExprsEmb.insert(exprsmapelem(aName + " " + aType, aExpr));
}

CAE_EBase* CSL_Interpr::Context()
{
    return iExpContext;
}

string CSL_Interpr::ContextType() 
{ 
    return GetStateDataType(iExpContext->TypeName());
};


string CSL_Interpr::GetStateDataType(const string& aStateType)
{
    return KStateDataTypes[aStateType];
}

string CSL_Interpr::DataType(CAE_StateBase& aState)
{
    CAE_StateEx* stateex = aState.GetFbObj(stateex);
    if (stateex != NULL) {
	return aState.TypeName();
    }
    else {
	CAE_State* state = aState.GetFbObj(state);
	if (state != NULL) {
	    return GetStateDataType(aState.TypeName());
	}
	else
	    return string();
    }
}

void CSL_Interpr::Logger::WriteFormat(const char* aFmt,...)
{
    static const int KLogAddRecBufSize = 20;
    char bufa[KLogAddRecBufSize] = "";
    char buf[CAE_LogRec::KLogRecBufSize] = "";
    va_list list;
    va_start(list,aFmt);
    sprintf(buf, "Trans [%s, ln %d]: ", iMain.iExpContext->Name().c_str(), iMain.iLine);
    int len = strlen(buf);
    vsprintf(buf+len, aFmt, list);
    iMain.iLogger->WriteRecord(buf);
}

void CSL_Interpr::GetConstructors(vector<string>& aRes)
{
    for (multimap<string, CSL_ExprBase*>::iterator it = iExprsEmb.begin(); it != iExprsEmb.end(); it++) {
	CSL_ExprBase* elem = it->second;
	if (it->first.compare(elem->RType()) == 0) {
	    aRes.push_back(elem->Name());
	}
    }
    
}

