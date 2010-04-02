#ifndef __UT_FAP_EVENTS_COUNTER_H
#define __UT_FAP_EVENTS_COUNTER_H

#include "fapext.h"

#include <cppunit/extensions/HelperMacros.h>

class CFT_InpSeq;

/**
 * Class to perform unit test of CAE_Env, CAE_Object, CAE_TState classes
 *
 * Test procedure is the following:
 * Create two CAE_Object:
 *   first one is Event Generator (CFT_Generator). It sends N events (event is TBool)
 *   second is Event Counter (CFT_Counter), counts number of transitions from EFalse to ETrue
 */
class UT_FAP_EventsCounter : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_EventsCounter);
    CPPUNIT_TEST(test_EventsCounterS1);
    CPPUNIT_TEST(test_EventsCounterS2);
    CPPUNIT_TEST(test_EventsCounterS3);
    CPPUNIT_TEST_SUITE_END();

public:
    /**
     * Set up the test session
     */
    virtual void setUp();
    /**
     * Clean up the test session
     */
    virtual void tearDown();

private:
    void test_EventsCounterWithSeq(CFT_InpSeq& aInpSeq);
    /**
     * Test Event Counter
     */
    void test_EventsCounterS1();
    void test_EventsCounterS2();
    void test_EventsCounterS3();

private:
    CAE_Env* iEnv;
};

class CFT_InpSeq
{
public:
    CFT_InpSeq():iLen(0),iArray(0) {}
    CFT_InpSeq(TInt aLen, TBool* aArray):iLen(aLen) {iArray = new TBool[iLen]; memcpy(iArray, aArray, iLen);}
    CFT_InpSeq(const CFT_InpSeq& aInpSeq):iLen(aInpSeq.iLen) {iArray = new TBool[iLen]; memcpy(iArray, aInpSeq.iArray, iLen);}
    CFT_InpSeq& operator=(const CFT_InpSeq& aInpSeq);
    ~CFT_InpSeq() {delete[] iArray;}
    TInt Count0to1(TInt aAt);
public:
    TInt   iLen;
    TBool* iArray;
};

class CFT_InpSeqState: public CAE_TState<CFT_InpSeq>
{
public:
    CFT_InpSeqState(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, 
        StateType aType= CAE_StateBase::EType_Reg, TLogFormatFun aLogFormFun = NULL);
    static CFT_InpSeqState* NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, 
        StateType aType= CAE_StateBase::EType_Reg, TLogFormatFun aLogFormFun = NULL);
    CFT_InpSeq operator= (CFT_InpSeq aVal);
};

class CFT_Generator: public CAE_Object
{
public:
    static CFT_Generator* NewL(const char* aInstName, CAE_Object* aMan, TInt aEventMaxNum);
    CFT_Generator(const char* aInstName, CAE_Object* aMan);
    void SetInput(CFT_InpSeq& aInpSeq) {*iInpSeq = aInpSeq;}
    void SetInput(TBool aInp) {*iInp = aInp;}
    CAE_TState<TInt>* EventNum() {return iEventNum;}
    CAE_TState<TBool>* Inp() {return iInp;}

    TInt EventNumMax() {return iEventNumMax->Value();}
    TBool Input() {return iInp->Value();}
private:
    void ConstructL(TInt aEventMaxNum);
protected:
    CAE_TRANS_DEF(GenerateEvent, CFT_Generator);
private:
    CFT_InpSeqState*  iInpSeq;
    CAE_TState<TBool>*   iInp;
    CAE_TState<TInt>*    iEventNum;
    CAE_TState<TInt>*    iEventNumMax;
};

class CFT_Counter: public CAE_Object
{
public:
    static CFT_Counter* NewL(const char* aInstName, CAE_Object* aMan, CFT_Generator* aEvent);
    CFT_Counter(const char* aInstName, CAE_Object* aMan);
    TInt Count() {return iCnt->Value();}
    TBool Evt() {return iEvt->Value();}
private:
    void ConstructL(CFT_Generator* aEvent);
protected:
    CAE_TRANS_DEF(ReceiveEvent, CFT_Counter);
private:
    CAE_TState<TBool>*   iEvt;
    CAE_TState<TInt>*    iCnt;
};


#endif // __UT_FAP_EVENTS_COUNTER_H
