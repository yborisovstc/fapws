#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <stdlib.h>

#include "ut_events_counter.h"

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_EventsCounter );

const char* KEvent = "Event";
const char* KEventInpSeq = "CFT_InpSeq";
const char* KEventInp = "Inp";
const char* KEventNum = "EventNum";
const char* KEventMaxNum = "EventNumMax";
const char* KCounter = "Counter";
const char* KCounterEvt = "Evt";
const char* KCounterCnt = "Count";
TBool seq1[10] = {ETrue,ETrue,ETrue,ETrue,ETrue,ETrue,ETrue,ETrue,ETrue,ETrue};
TBool seq2[10] = {EFalse,EFalse,EFalse,EFalse,EFalse,EFalse,EFalse,EFalse,EFalse,EFalse};
TBool seq3[10] = {ETrue,EFalse,ETrue,EFalse,ETrue,EFalse,ETrue,EFalse,ETrue,EFalse};

const TInt KObUid_CAE_Var_State_InpSeq = KObUid_CAE_Var_State_Ext + 1 ;   // tagField 0x00010001

template<> inline TInt CAE_TState<CFT_InpSeq>::DataTypeUid() {return KObUid_CAE_Var_State_InpSeq;};

_TEMPLATE_ TBool CAE_TState<CFT_InpSeq>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CFT_InpSeq>::DoOperation()
{
}

CFT_InpSeqState::CFT_InpSeqState(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, 
        StateType aType, TLogFormatFun aLogFormFun):
      CAE_TState<CFT_InpSeq>(aInstName, aMan, aTrans, aType, aLogFormFun)
{
}

CFT_InpSeqState* CFT_InpSeqState::NewL(const char* aInstName, CAE_Object* aMan,  TTransInfo aTrans, StateType aType, TLogFormatFun aLogFormFun)
{
    CFT_InpSeqState* self = new CFT_InpSeqState(aInstName, aMan, aTrans, aType, aLogFormFun);
    self->ConstructL();
    return self;
}

CFT_InpSeq CFT_InpSeqState::operator= (CFT_InpSeq aVal)
{
    if (memcmp(&aVal, iNew, iLen))
        SetUpdated();
    CFT_InpSeq* newval =  (CFT_InpSeq*)iNew;
    *newval = aVal;
    return aVal;
}


CFT_InpSeq& CFT_InpSeq::operator=(const CFT_InpSeq& aInpSeq)
{
    delete[] iArray;
    iLen = aInpSeq.iLen;
    iArray = new TBool[iLen];
    memcpy(iArray, aInpSeq.iArray, iLen);
    return *this;
}

TInt CFT_InpSeq::Count0to1(TInt aAt)
{
    TInt res = 0;
    TBool cur = EFalse;
    TBool prev = ETrue;
    for (TInt i=0; i<aAt; i++)
    {
        cur = iArray[i];
        if (!prev && cur)
            res++;
        prev = cur;
    }
    return res;
}


CFT_Generator::CFT_Generator(const char* aInstName, CAE_Object* aMan):
    CAE_Object(aInstName, aMan)
{
}

void CFT_Generator::ConstructL(TInt aEventMaxNum)
{
    CAE_Object::ConstructL();

    iInpSeq = CFT_InpSeqState::NewL(KEventInpSeq, this, TTransInfo(), CAE_StateBase::EType_Input);
    iInp = CAE_TState<TBool>::NewL(KEventInp, this,  CAE_TRANS(GenerateEvent), CAE_StateBase::EType_Reg);
    iEventNum = CAE_TState<TInt>::NewL(KEventNum, this, TTransInfo(), CAE_StateBase::EType_Reg);
    iEventNumMax = CAE_TState<TInt>::NewL(KEventMaxNum, this, TTransInfo(), CAE_StateBase::EType_Reg);

    // Setting up initial value should be done as NEW value of state
    *iInp = 0;
    iInp->AddInputL(iEventNumMax);
    iInp->AddInputL(iEventNum);
    iInp->AddInputL(iInpSeq);

    *iEventNum = 0;
    *iEventNumMax = aEventMaxNum;
}

CFT_Generator* CFT_Generator::NewL(const char* aInstName, CAE_Object* aMan,
        TInt aEventNumMax)
{
    CFT_Generator* self = new CFT_Generator(aInstName, aMan);
    self->ConstructL(aEventNumMax);
    return self;
}

void CFT_Generator::GenerateEvent(CAE_State* aState)
{
    CAE_TState<TInt> *ev_num = (CAE_TState<TInt> *) aState->Input(KEventNum);
    CAE_TState<TInt> *ev_mnum = (CAE_TState<TInt> *) aState->Input(KEventMaxNum);
    CAE_TState<CFT_InpSeq> *ev_inpseq = (CAE_TState<CFT_InpSeq> *) aState->Input(KEventInpSeq);
    CAE_TState<TBool> *ev_inp = (CAE_TState<TBool> *) aState;
    printf("GenerateEvent, ev_inp = %d, ev_num = %d, ev_mnum = %d\n", (*ev_inp)(), (*ev_num)(), (*ev_mnum)());
    CFT_InpSeq inpseq = (*ev_inpseq)();
    if ((*ev_mnum)() == 0)
    {
        // initialization
        (*ev_inp) = ETrue;
    }
    if ((*ev_mnum)() && (*ev_num)() < (*ev_mnum)())
    {
        if (inpseq.iArray && (*ev_num)() < inpseq.iLen)
            (*ev_inp) = inpseq.iArray[(*ev_num)()];
        (*ev_num) = (*ev_num)() + 1;
    }
}


CFT_Counter::CFT_Counter(const char* aInstName, CAE_Object* aMan):
    CAE_Object(aInstName, aMan)
{
}

void CFT_Counter::ConstructL(CFT_Generator* aEvent)
{
    CAE_Object::ConstructL();

    iEvt = CAE_TState<TBool>::NewL(KCounterEvt, this, TTransInfo(), CAE_StateBase::EType_Reg);
    iCnt = CAE_TState<TInt>::NewL(KCounterCnt, this, CAE_TRANS(ReceiveEvent), CAE_StateBase::EType_Reg);

    // TODO Setting up initial value should be done as NEW value of state
    // find out how to make this without additional state
    // Note: it is not clear how to make initialization !!!
    ~(*iCnt) = -1;

    iCnt->AddInputL(aEvent->Inp());
    iCnt->AddInputL(iEvt);
}

CFT_Counter* CFT_Counter::NewL(const char* aInstName, CAE_Object* aMan,
        CFT_Generator* aEvent)
{
    CFT_Counter* self = new CFT_Counter(aInstName, aMan);
    self->ConstructL(aEvent);
    return self;
}

void CFT_Counter::ReceiveEvent(CAE_State* aState)
{
    CAE_TState<TBool> *inp = (CAE_TState<TBool> *) aState->Input(KEventInp);
    CAE_TState<TBool> *evt = (CAE_TState<TBool> *) aState->Input(KCounterEvt);
    CAE_TState<TInt> *cnt = (CAE_TState<TInt> *) aState;
    printf("ReceiveEvent, inp = %d, evt = %d, cnt=%d\n", (*inp)(), (*evt)(), (*cnt)());
    if ((*cnt)() == -1)
    {
        // initialization
        (*cnt) = 0;
        (*evt) = ETrue;
    }
    else {
        (*cnt) = (*cnt)() + (!(*evt)() && (*inp)());
        (*evt) = (*inp)();
    }
}


void UT_FAP_EventsCounter::setUp()
{
    iEnv = CAE_Env::NewL(1, NULL);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_EventsCounter::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_EventsCounter::test_EventsCounterS1()
{
    CFT_InpSeq s(sizeof(seq1), seq1);
    test_EventsCounterWithSeq(s);
}

void UT_FAP_EventsCounter::test_EventsCounterS2()
{
    CFT_InpSeq s(sizeof(seq1), seq2);
    test_EventsCounterWithSeq(s);
}

void UT_FAP_EventsCounter::test_EventsCounterS3()
{
    CFT_InpSeq s(sizeof(seq1), seq3);
    test_EventsCounterWithSeq(s);
}

void UT_FAP_EventsCounter::test_EventsCounterWithSeq(CFT_InpSeq& aInpSeq)
{
    CFT_Generator* ev_gen = CFT_Generator::NewL(KEvent, iEnv->Root(), aInpSeq.iLen);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CFT_Generator", ev_gen != 0);
    CFT_Counter* ev_cnt = CFT_Counter::NewL(KCounter, iEnv->Root(), ev_gen);
    // We set initial value of CFT_Generator
    // as state NEW value. So we need at least one step of env to get this value current
    ev_gen->SetInput(aInpSeq);
    iEnv->Step();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect EventNumMax", aInpSeq.iLen, ev_gen->EventNumMax());
    TBool tr = ETrue;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Evt", tr, ev_cnt->Evt());
    for (TInt i=0; i<aInpSeq.iLen; i++)
    {
        iEnv->Step();
        char buf[256];
        sprintf(buf, "Incorrect EventNum at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, i+1, ev_gen->EventNum()->Value());
        sprintf(buf, "Incorrect Inp at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, aInpSeq.iArray[i], ev_gen->Input());
        sprintf(buf, "Incorrect Evt at attempt %d", i);
        if (i == 0){CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, tr, ev_cnt->Evt());}
        else {CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, aInpSeq.iArray[i-1], ev_cnt->Evt());}
        sprintf(buf, "Incorrect Cnt at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, aInpSeq.Count0to1(i), ev_cnt->Count());
    }
    iEnv->Step();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect EventNum", aInpSeq.iLen, ev_gen->EventNum()->Value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Cnt", aInpSeq.Count0to1(aInpSeq.iLen), ev_cnt->Count());
    iEnv->Step();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect EventNum", aInpSeq.iLen, ev_gen->EventNum()->Value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect Cnt", aInpSeq.Count0to1(aInpSeq.iLen), ev_cnt->Count());
    printf("Result:   ");
    for (int i=0; i<aInpSeq.iLen; i++)
    {printf("%d   ", aInpSeq.iArray[i]);} printf("cnt = %d\n", ev_cnt->Count());
}
