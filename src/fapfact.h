#ifndef __FAP_FACT_H
#define __FAP_FACT_H

//************************************************************
// Finite automata programming- factories  
// Yuri Borisov  26/08/06		  Initial version 
//*************************************************************

#include <fapbase.h>
#include <vector>

class Uri
{
    public:
	Uri(const string& aUri);
	const string& Scheme() {return iScheme;};
	const string& Auth() {return iAuth;};
	const string& Path() {return iPath;};
    private:
	void Parse(const string& aUri);
    private:
	string iScheme;
	string iAuth;
	string iPath;
};


class DesUri
{
    public:
	typedef pair<NodeType, string> TElem;
    public:
	DesUri(const string& aUri);
	const vector<TElem>& Elems() {return iElems;};
	string GetUri(vector<TElem>::const_iterator aStart);
    private:
	void Parse();
    private:
	string iUri;
	string iScheme;
	vector<TElem> iElems;
};

//*********************************************************
// Factory of Chromosome manager for XML based chromosome
//*********************************************************

class CAE_ChroManBase: public MAE_ChroMan
{
    public:
	virtual ~CAE_ChroManBase() {};
};


// Chromosome manager for XML based chromosome
class CAE_ChroManXFact
{
    public:
	static CAE_ChroManBase *CreateChroManX(const char *aFileName);
};



// Base class of provider implementation
class CAE_ProviderBase: public MAE_Provider
{
    public:
	CAE_ProviderBase(const string& aName);
	const string& Name() const { return iName;};
	virtual ~CAE_ProviderBase();
	virtual MAE_ChroMan* Chman() const { return NULL;};
    private:
	string iName;
};

// Factory of element of object
class CAE_Fact: public MAE_Provider
{
    public:
	FAPWS_API static CAE_Fact* NewL();
	FAPWS_API virtual ~CAE_Fact();
	void AddProvider(CAE_ProviderBase* aProv);
	TBool LoadPlugin(const string& aName);
	void LoadAllPlugins();
	// From MAE_Provider
	virtual CAE_StateBase* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	FAPWS_API virtual CAE_EBase* CreateObjectL(TUint32 aTypeUid) const;
	FAPWS_API virtual CAE_EBase* CreateObjectL(const char *aName) const;
	virtual const TTransInfo* GetTransf(const char *aName) const;
	virtual void RegisterState(const TStateInfo *aInfo);
	virtual void RegisterStates(const TStateInfo **aInfos);
	virtual void RegisterTransf(const TTransInfo *aTrans);
	virtual void RegisterTransfs(const TTransInfo **aNames);
	virtual const CAE_Formatter* GetFormatter(int aUid) const;
	virtual void RegisterFormatter(CAE_Formatter *aForm);
	virtual CAE_ChromoBase* CreateChromo() const;
	virtual CAE_TranExBase* CreateTranEx(MCAE_LogRec* aLogger) const;
	virtual MAE_Opv* CreateViewProxy();
    protected:
	FAPWS_API CAE_Fact();
	FAPWS_API void ConstructL();
    private:
	CAE_ProviderBase* GetBaseProvider() const;
    private:
	map<string, CAE_ProviderBase*> iProviders;
};

class MAE_Plugin;
// General provider
class CAE_ProviderGen: public CAE_ProviderBase
{
    public:
	FAPWS_API CAE_ProviderGen();
	FAPWS_API virtual ~CAE_ProviderGen();
	// From MAE_Provider
	virtual CAE_StateBase* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	virtual CAE_EBase* CreateObjectL(TUint32 aTypeUid) const;
	virtual CAE_EBase* CreateObjectL(const char *aName) const;
	virtual const TTransInfo* GetTransf(const char *aName) const;
	virtual void RegisterState(const TStateInfo *aInfo);
	virtual void RegisterStates(const TStateInfo **aInfos);
	virtual void RegisterTransf(const TTransInfo *aTrans);
	virtual void RegisterTransfs(const TTransInfo **aNames);
	virtual const CAE_Formatter* GetFormatter(int aUid) const;
	virtual void RegisterFormatter(CAE_Formatter *aForm);
	virtual CAE_ChromoBase* CreateChromo() const;
	virtual CAE_TranExBase* CreateTranEx(MCAE_LogRec* aLogger) const;
	virtual MAE_Opv* CreateViewProxy();
    private:
	void RegisterFormatter(int aUid, TLogFormatFun aFun);
	const TStateInfo* GetStateInfo(const char *aType) const;
    private:
	// Register of states
	vector<const TStateInfo*>* iStateInfos;
	// Register of transitions
	vector<const TTransInfo*>* iTransfs;
	// Register of state logging formatter
	vector<CAE_Formatter *>* iFormatters;	
};

#endif // __FAP_FACT_H
