#ifndef __FAP_LOGGER_H
#define __FAP_LOGGER_H

//************************************************************
// Finite automata programming base- logger, level 1 prototype  
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include <stdio.h>
#include <vector>
#include "fapbase.h"
#include "fapplat.h"

// File logger record
class CAE_LogRec: public MCAE_LogRec
{
	enum
	{
		KLogRecBufSize = 400
	};
public:
	static CAE_LogRec* NewL(const char* aLogFileName);
	virtual ~CAE_LogRec();
	// From MCAE_LogRec
	virtual void WriteRecord(const char* aText);
	virtual void WriteFormat(const char* aFmt,...);
	virtual void Flush();
protected:
	CAE_LogRec();
	void ConstructL(const char* aLogFileName);
	TInt CreateLog();
private:
	FILE* iLogFile;
	char*	iLogFileName;
	TBool iLogFileValid;
};

class CAE_LogSpecCon;
class CAE_LogCtrl;
const TInt KLogSpec_NameMaxLen = 40;

// Logging specification
class CAE_LogSpec
{
	friend class CAE_LogSpecCon;
	friend class CAE_LogCtrl;
public:
	CAE_LogSpec();
	virtual ~CAE_LogSpec();
	TInt SetNameL(const char* aName);
	TInt SetNameL(const char* aName, int aLen);
	const char* Name() const {return iName;};
private:
	char*	  iName;
	CAE_Base* iElt;
};


// Logging specification container
class CAE_LogSpecCon
{
public:
	static CAE_LogSpecCon* NewL(const char* aSpecFileName);
	~CAE_LogSpecCon();
	inline TInt Count() const;
	inline CAE_LogSpec* At(TInt aInd) const;
protected:
	CAE_LogSpecCon(const char* aSpecFileName);
	void ConstructL();
private:
	void ParseSpecsL();
private:
	FILE*	iSpecsFile;
	vector<CAE_LogSpec*>*  iSpecs; // CArrayPtrFlat<CAE_LogSpec>*
	const char*	iSpecFileName;
//	HBufC8* iPBuf;
};

inline TInt CAE_LogSpecCon::Count() const { return iSpecs->size();};
	
inline CAE_LogSpec* CAE_LogSpecCon::At(TInt aInd) const { return iSpecs->at(aInd);};


class CAE_Object;
class CAE_State;

class CAE_LogCtrl
{
public:
	static CAE_LogCtrl* NewL(CAE_Object* aFapRoot, const char* aSpecFileName, const char *aLogFileName = NULL);
	~CAE_LogCtrl();
	void DoLogL(TInt aStep);
	inline MCAE_LogRec *LogRec();
protected:
	void ConstructL(const char* aSpecFileName, const char *aLogFileName = NULL);
	CAE_LogCtrl(CAE_Object* aFapRoot);
	TInt ResolveSpec(TInt aInd);
	void FormatState(CAE_State* aState, char* aRes);
private:
	CAE_LogRec*		iRec;
	CAE_Object*		iFapRoot;
	CAE_LogSpecCon* iSpecs;
};


inline MCAE_LogRec *CAE_LogCtrl::LogRec() { return iRec; }

#endif // __FAP_LOGGER_H
