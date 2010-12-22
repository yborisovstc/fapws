
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

const char* KOldFileExtension = ".old";
const char* KTimeFormat = "%02d.%02d:%02d:%06d ";
const char* KTextFormat = "%S\015\012";

const TUint8 KFullStopChar='.';
const TUint8 KTabChar='\t';
const TUint8 KCarriageReturnChar='\n';
const TUint8 KLineFeedChar='\r';

#if defined (_PLAT_WIN32_)  // Windows platform
const char* KLogFileName = "C:\\LOGS\\FAP\\FAPLOG.TXT";
const char* KLogDirName = "C:\\LOGS\\FAP";
#endif // _PLAT_WIN32_ 

#if defined (_PLAT_LINUX_)  // Linux  platform
const char* KLogFileName = "faplog.txt";
const char* KLogDirName = "/tmp/LOGS/FAP";
#endif  // Linux

const TInt KLogFileNameLen = 200;
const TInt KLogFileNameDriveLen = 3;
const TInt KLogFileNameDirLen = 120;
const TInt KLogFileNameNameLen = 120;
const TInt KLogFileNameExtLen = 5;

const TInt KSpecsGranularity = 3;
const TInt KStateFormattedLen = 100;
const TInt KSpecFileBufLen = 400;

const char* KLogSpecElementSeparator =  ",";
const TInt KLogSpecElementSeparatorLen = 1;

const char K_Char_CR = 0x0d;
const char K_Path_Sep = '/';

#if (0)
void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
	char* pch = NULL;
	pch = strtok(path, K_Path_Sep);
	if (pch != NULL)
		strcpy(pch, drive);
}
#endif


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
    if (CreateLog() == 0)
    {
	char drive[KLogFileNameDriveLen];
	char dir[KLogFileNameDirLen];
	char fname[KLogFileNameNameLen];
	char ext[KLogFileNameExtLen];
	char oldname[KLogFileNameLen];
	//!! [YB] 12-Dec-08 - this functionality needs to be ported further
	// It's just removed temporarily 
	//		_splitpath(iLogFileName, drive, dir, fname, ext);
	//		_makepath( oldname, drive, dir, fname, KOldFileExtension);
	//		remove(oldname);
	//		rename(iLogFileName, oldname);
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
//!! [YB] - temporarily removed creation of catalogue 
#if (0)
    TInt ret = -1;
    char drive[KLogFileNameDriveLen];
    char dir[KLogFileNameDirLen];
    char fname[KLogFileNameNameLen];
    char ext[KLogFileNameExtLen];
    char path[KLogFileNameLen];

    _splitpath(iLogFileName, drive, dir, fname, ext);
    _makepath( path, drive, dir, NULL, NULL);
    TInt plen = strlen(path);
    if (path[plen-1] == '\\')
	path[plen-1] = 0x00;
    ret = _access(path, 0x00);
    if (ret == -1)
    {
	ret = mkdir(path);
    }
    return ret;
#endif
    const char *home_dir = getenv("HOME");
    // TODO [YB] To avoid home as the base of logfile path
    char *logfile = (char *) malloc(strlen(home_dir) + strlen(iLogFileName) + 1);
    strcpy(logfile, home_dir);
    strcat(logfile, "/");
    strcat(logfile, iLogFileName);
//    iLogFile = open(logfile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    iLogFile = fopen(logfile, "w+");
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
	CAE_Base* elt = iFapRoot->FindByName(iSpecs->At(aInd)->Name());
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
			CAE_StateBase* state = spec->iElt->GetFbObj(state);
			if (state != NULL)
			{
				char buf[KStateFormattedLen] = "";
				FormatState(state, buf);
				iRec->WriteFormat("Step %05d >  %s = %s", aStep, spec->Name(), buf);
			}
		}
	}
}

void CAE_LogCtrl::FormatState(CAE_StateBase* aState, char* aRes)
{
    for (TInt i=0; i < aState->Len(); i++)
    {
	int symb = ((TUint8*) aState->iNew)[i];
	char fmtsmb[3];
	sprintf(fmtsmb, "%02x", symb);
	strcat(aRes, fmtsmb);
    }
}
