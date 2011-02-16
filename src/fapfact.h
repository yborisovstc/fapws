#ifndef __FAP_FACT_H
#define __FAP_FACT_H

//************************************************************
// Finite automata programming- factories  
// Yuri Borisov  26/08/06		  Initial version 
//*************************************************************

#include <fapbase.h>
#include <vector>


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
	FAPWS_API CAE_ProviderBase();
	FAPWS_API virtual ~CAE_ProviderBase();
	virtual MAE_ChroMan* Chman() const { return NULL;};
};

// Factory of element of object
class CAE_Fact: public MAE_Provider
{
    public:
	FAPWS_API static CAE_Fact* NewL();
	FAPWS_API virtual ~CAE_Fact();
	FAPWS_API void AddProviderL(CAE_ProviderBase* aProv);
	// From MAE_Provider
	virtual CAE_State* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	virtual CAE_State* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const;
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
    protected:
	FAPWS_API CAE_Fact();
	FAPWS_API void ConstructL();
    private:
	CAE_ProviderBase* GetProviderAt(TInt aInd) const;
    private:
	vector<CAE_ProviderBase*>* iProviders;
};


// General provider
class CAE_ProviderGen: public CAE_ProviderBase
{
    public:
	FAPWS_API CAE_ProviderGen();
	FAPWS_API virtual ~CAE_ProviderGen();
	// From MAE_Provider
	virtual CAE_State* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	virtual CAE_State* CreateStateL(const char *aTypeUid, const char* aInstName, CAE_Object* aMan) const;
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
