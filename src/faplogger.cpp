
//************************************************************
// Finite automata programming base- logger, level 1 prototype  
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
// Yuri Borisov  28/09/05  FAP_CR_010  Avoid panic if log spec is absent
//*************************************************************
// TODO [YB] print sync signal in log record 

#include "faplogger.h"
#include "fapbase.h"
#include "panics.h"

#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>


const char* KLogFileName = "faplog.txt";

const TInt KStateFormattedLen = 100;
const TInt KSpecFileBufLen = 400;

const char* KLogSpecElementSeparator =  ",";
const TInt KLogSpecElementSeparatorLen = 1;


// CAE_LogRec

CAE_LogRec::CAE_LogRec(): iLogFile(NULL), iLogFileName(NULL)
{
}

CAE_LogRec::~CAE_LogRec()
{
    if (iLogFileValid) 
	fclose(iLogFile);
    if (iLogFileName != NULL)
	free(iLogFileName);
}

CAE_LogRec* CAE_LogRec::NewL(const char* aLogFileName)
{
	CAE_LogRec* self = new CAE_LogRec();
	self->ConstructL(aLogFileName);
	return self;
}

void CAE_LogRec::ConstructL(const char* aLogFileName)
{
    iLogFileName = strdup(aLogFileName);
    TInt ret= 0;
    //if (CreateLog() == 0)
    if (1)
    {
	remove(iLogFileName); //!! [YB] added instead of storing to old file
	iLogFile = fopen(iLogFileName, "w+");
	if(iLogFile)
	{
	    iLogFileValid=ETrue;
	    ret= fputs("----------New Log----------\015\012", iLogFile);
	    fflush(iLogFile);
	}
	else
	    iLogFileValid= EFalse;
    }
    else
	iLogFileValid= EFalse;
}

TInt CAE_LogRec::CreateLog()
{
    iLogFile = fopen(iLogFileName, "w+");
    return (iLogFile != NULL);
}

void CAE_LogRec::WriteRecord(const char* aText)
{
	if (iLogFile)
	{
		fputs(aText, iLogFile);
		fputs("\015\012", iLogFile);
		fflush(iLogFile);
	}
}

void CAE_LogRec::WriteFormat(const char* aFmt,...)
{
    char buf[KLogRecBufSize] = "";
    va_list list;
    va_start(list,aFmt);
	vsprintf(buf, aFmt, list);
	TInt len = strlen(buf);
    WriteRecord(buf);
}

void CAE_LogRec::Flush()
{
	if (iLogFile)
		fflush(iLogFile);
}


// CAE_LogSpec

CAE_LogSpec::CAE_LogSpec():iName(NULL), iElt(NULL)
{
}

CAE_LogSpec::~CAE_LogSpec()
{
	if (iName)
		delete iName;
}

TInt CAE_LogSpec::SetNameL(const char* aName)
{
	TInt srclen = strlen(aName);
	return SetNameL(aName, srclen);
}

TInt CAE_LogSpec::SetNameL(const char* aName, int aLen)
{
	TInt res = KFapErr_None;
	if (aLen >= KLogSpec_NameMaxLen)
		res = KFapErr_Overflow;
	else
	{
		if (iName == NULL)
		{
			iName = new char[KLogSpec_NameMaxLen];
		}
		strncpy(iName, aName, aLen);
		iName[aLen] = 0x00;
	}
	return res;
}


// CAE_LogSpecCon

CAE_LogSpecCon::CAE_LogSpecCon(const char* aSpecFileName):
	iSpecs(NULL)
{
	iSpecFileName = aSpecFileName;
	
}

CAE_LogSpecCon::~CAE_LogSpecCon()
{
    if (iSpecsFile)
    {
	fclose(iSpecsFile);
	iSpecsFile = NULL;
    }
    if (iSpecs != NULL)
    {
	for (TInt i= 0; i < iSpecs->size(); i++)
	{
	    delete iSpecs->at(i);
	}
	delete iSpecs;
    }
}

CAE_LogSpecCon* CAE_LogSpecCon::NewL(const char* aSpecFileName)
{
	CAE_LogSpecCon* self = new CAE_LogSpecCon(aSpecFileName);
	self->ConstructL();
	return self;
}


void CAE_LogSpecCon::ConstructL()
{
	iSpecsFile = fopen(iSpecFileName, "r");
	iSpecs = new vector<CAE_LogSpec*>;
	if (iSpecsFile != NULL)
	{
	    ParseSpecsL();
	}
}

void CAE_LogSpecCon::ParseSpecsL()
{
	char* buf = new char[KSpecFileBufLen];
	char* tagbegin = NULL;
	char* tail = NULL;
	char* tagend = NULL;
	int taglen = 0;
	char* res = fgets(buf, KSpecFileBufLen, iSpecsFile);
	TBool fin = EFalse;
	tail = buf;
	while (!fin)
	{
		tagbegin = tail;
		tagend = strstr(tail, KLogSpecElementSeparator);
		CAE_LogSpec* spec = new CAE_LogSpec();
		if (tagend != NULL)
		{
			taglen = tagend - tagbegin;
			tail = tagend + KLogSpecElementSeparatorLen;
			spec->SetNameL(tagbegin, taglen);
		}
		else
		{
			fin= ETrue;
			spec->SetNameL(tagbegin);
		}
		iSpecs->push_back(spec);
	}
	delete buf;
}


// CAE_LogCtrl

CAE_LogCtrl::CAE_LogCtrl(CAE_Object* aFapRoot): 
	iRec(NULL), iSpecs(NULL), iFapRoot(aFapRoot)
{
}

CAE_LogCtrl::~CAE_LogCtrl()
{
	delete iSpecs;
	delete iRec;
}

void CAE_LogCtrl::ConstructL(const char* aSpecFileName, const char *aLogFileName)
{
	iRec = CAE_LogRec::NewL(aLogFileName ? aLogFileName : KLogFileName);
	iSpecs = CAE_LogSpecCon::NewL(aSpecFileName);
}


CAE_LogCtrl* CAE_LogCtrl::NewL(CAE_Object* aFapRoot, const char* aSpecFileName, const char *aLogFileName)
{
	CAE_LogCtrl* self = new  CAE_LogCtrl(aFapRoot);
	self->ConstructL(aSpecFileName, aLogFileName);
	return self;
}

TInt CAE_LogCtrl::ResolveSpec(TInt aInd)
{
	_FAP_ASSERT(aInd < iSpecs->Count());
	TInt res = KFapErr_None;
	CAE_EBase* elt = iFapRoot->FindByName(iSpecs->At(aInd)->Name());
	if (elt != NULL)
		iSpecs->At(aInd)->iElt = elt;
	else res = KFapErr_NotFound;
	return res;
}

void CAE_LogCtrl::DoLogL(TInt aStep)
{
	for (TInt i = 0; i < iSpecs->Count(); i++)
	{
		CAE_LogSpec* spec = iSpecs->At(i);
		if (spec->iElt == NULL)
		{
			ResolveSpec(i);
		}
		if (spec->iElt != NULL && spec->iElt->IsUpdated())
		{
			CAE_State* state = spec->iElt->GetFbObj(state);
			if (state != NULL)
			{
				char buf[KStateFormattedLen] = "";
				FormatState(state, buf);
				iRec->WriteFormat("Step %05d >  %s = %s", aStep, spec->Name(), buf);
			}
		}
	}
}

void CAE_LogCtrl::FormatState(CAE_State* aState, char* aRes)
{
    for (TInt i=0; i < aState->Len(); i++)
    {
	int symb = ((TUint8*) aState->iNew)[i];
	char fmtsmb[3];
	sprintf(fmtsmb, "%02x", symb);
	strcat(aRes, fmtsmb);
    }
}
