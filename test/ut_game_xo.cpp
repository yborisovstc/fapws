#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <stdlib.h>
#include <sys/time.h>

#include "ut_game_xo.h"

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_GameXO );


const TInt KFieldDem = 5;

typedef TInt TField[KFieldDem*KFieldDem];

struct CF_TField
{
    CF_TField() { for (TInt i=0; i<KFieldDem*KFieldDem; i++) iField[i] = -1;}
    CF_TField(TField aField) { memcmp(iField, aField, sizeof(TField)); }
    TField& operator() () { return iField; }
    TInt& operator[] (TInt i) { return iField[i]; }
    TInt& getIJ (TInt i, TInt j) { return *(iField + i*KFieldDem + j); }
    TField iField;
};

template<> inline const char *CAE_TState<CF_TField>::Type() {return "StField"; };

_TEMPLATE_ TBool CAE_TState<CF_TField>::SetTrans(TTransInfo aTinfo)
{
    TBool res = ETrue;
    CAE_State::SetTrans(aTinfo);
    return res;
}

_TEMPLATE_ void CAE_TState<CF_TField>::DoOperation()
{
}

_TEMPLATE_ void CAE_TState<CF_TField>::DataFromStr(const char* aStr, void *aData) const
{
}

_TEMPLATE_ char *CAE_TState<CF_TField>::DataToStr(TBool aCurr) const
{
    return CAE_State::DataToStr(aCurr);
}


class CFT_GameXO: public CAE_Object
{
public:
    static CFT_GameXO* NewL(const char* aInstName, CAE_Object* aMan, TBool aStartX);
    CFT_GameXO(const char* aInstName, CAE_Object* aMan);
    void Draw();
    TBool CheckStop();
private:
    void ConstructL(TBool aStartX);
protected:
    CAE_TRANS_DEF(UpdateField, CFT_GameXO);
    CAE_TRANS_DEF(UpdateStep, CFT_GameXO);
    CAE_TRANS_DEF(UpdateFin, CFT_GameXO);
private:
    CAE_TState<CF_TField>* iField;
    CAE_TState<TBool>*  iStartX;
    CAE_TState<TInt>*   iStepNum;
    CAE_TState<TBool>*   iFin;
};

CFT_GameXO::CFT_GameXO(const char* aInstName, CAE_Object* aMan):
    CAE_Object(aInstName, aMan)
{
}

void CFT_GameXO::ConstructL(TBool aStartX)
{
    CAE_Object::ConstructL();
    iField = CAE_TState<CF_TField>::NewL("Field", this, CAE_TRANS(UpdateField), CAE_State::EType_Reg);
    iStartX = CAE_TState<TBool>::NewL("StartX", this, TTransInfo(), CAE_State::EType_Reg);
    iStepNum = CAE_TState<TInt>::NewL("StepNum", this, CAE_TRANS(UpdateStep), CAE_State::EType_Reg);
    iFin = CAE_TState<TBool>::NewL("Fin", this, CAE_TRANS(UpdateFin), CAE_State::EType_Output);

    ~(*iStartX) = aStartX;
    ~(*iField) = CF_TField();
    ~(*iStepNum) = 0;
    ~(*iFin) = EFalse;

    iField->AddInputL("StartX", iStartX);
    iField->AddInputL("StepNum", iStepNum);
    iField->AddInputL("Fin", iFin);
    iStepNum->AddInputL("Field", iField);
    iStepNum->AddInputL("Fin", iFin);
    iFin->AddInputL("Fileld", iField);
    iFin->AddInputL("StepNum", iStepNum);
    Draw();
}

CFT_GameXO* CFT_GameXO::NewL(const char* aInstName, CAE_Object* aMan, TBool aStartX)
{
    CFT_GameXO* self = new CFT_GameXO(aInstName, aMan);
    self->ConstructL(aStartX);
    return self;
}

void CFT_GameXO::UpdateField(CAE_State* aState)
{
    if (!iFin->Value()) {
        TBool startX = iStartX->Value();
        TInt stepNum = ~*iStepNum;
        TBool stepX = startX && (stepNum%2 == 0) || !startX && (stepNum%2 == 1);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        srand(tv.tv_sec * tv.tv_usec);
        TInt pos = rand() % (KFieldDem*KFieldDem);

        CF_TField field = ~*iField;
        for (TInt i=0; i<KFieldDem*KFieldDem; i++)
        {
            TInt curpos = (pos + i) % (KFieldDem*KFieldDem);
            if (field[curpos] == -1)
            {
                field[curpos] = stepX ? 1 : 0;
                break;
            }
        }
        (*iField) = field;
    }
}

void CFT_GameXO::UpdateStep(CAE_State* aState)
{
    if (!iFin->Value())
        (*iStepNum) = iStepNum->Value() + 1;
}

void CFT_GameXO::UpdateFin(CAE_State* aState)
{
    // TODO [YB] New values must not be used in transition functions!!
    TInt stepNew = !*iStepNum;
    CF_TField field = !*iField;
    if (stepNew >= KFieldDem*KFieldDem)
        (*iFin) = ETrue;
    else
    {
        TInt elem1;
        TBool same = EFalse;
        for (TInt i=0; i<KFieldDem; i++)
        {
            /* check |(-)| - | - |      |   | - |   |      |   |   | - |
                     | - |   |   |  or  | - |(-)| - |  or  |   |   | - |
                     | - |   |   |      |   | - |   |      | - | - |(-)|
                (-) elem1; - elem2
            */
            elem1 = field.getIJ(i, i);
            if (elem1 != -1) {
                same = ETrue;
                for (TInt j=0; j<KFieldDem; j++)
                {
                    TInt elem2 = field.getIJ(i, j);
                    if (elem1 != elem2) { same = EFalse; break; }
                }
                if (!same)
                {
                    same = ETrue;
                    for (TInt j=0; j<KFieldDem; j++)
                    {
                        TInt elem2 = field.getIJ(j, i);
                        if (elem1 != elem2) { same = EFalse; break; }
                    }
                }
            }
            if (same) {break;}
        }

        if (!same)
        {
            /* check | - |   | - |
                     |   | - |   |
                     | - |   | - |
            */
            elem1 = field.getIJ(0, 0);
            if (elem1 != -1)
            {
                same = ETrue;
                for (TInt i=1; i<KFieldDem; i++)
                {
                    TInt elem2 = field.getIJ(i, i);
                    if (elem1 != elem2) { same = EFalse; break;}
                }
            }
            if (!same)
            {
                elem1 = field.getIJ(0, KFieldDem-1);
                if (elem1!=-1) {
                    same = ETrue;
                    for (TInt i=1; i<KFieldDem; i++)
                    {
                        TInt elem2 = field.getIJ(i, KFieldDem-1-i);
                        if (elem1 != elem2) { same = EFalse; break;}
                    }
                }
            }
        }

        if (same)
        {
            (*iFin) = ETrue;
            printf("\n%c is a Winner !!!\n", (elem1 == 0) ? 'O' : 'X');
        }
    }
}

TBool CFT_GameXO::CheckStop()
{
    return iFin->Value();
}

void CFT_GameXO::Draw()
{
    char f;
    printf("\n step = %d, startX =%d \n", iStepNum->Value(), iStartX->Value());
    CF_TField field = ~*iField;
    for (TInt i=0; i<KFieldDem; i++)
    {
        printf("|");
        for (TInt j=0; j<KFieldDem; j++)
        {
            TInt curpos = i*KFieldDem + j;
            if (field[curpos] != -1)
            {
                f = (field[curpos] == 1) ? 'X' : '0';
            }
            else {
                f = ' ';
            }
            printf("| %c |", f);
        }
        printf("|\n");
    }
}


void UT_FAP_GameXO::setUp()
{
}

void UT_FAP_GameXO::tearDown()
{
}

void UT_FAP_GameXO::test_gameXO()
{
    CAE_Env* env = CAE_Env::NewL(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", env != 0);
    CFT_GameXO* gameXO = CFT_GameXO::NewL("GameXO", NULL, 1);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CFT_GameXO", gameXO != 0);
    env->AddL(gameXO);
    while (!gameXO->CheckStop())
    {
        env->Step();
        gameXO->Draw();
    }
    env->Step();
    gameXO->Draw();
    env->Step();
    gameXO->Draw();
}

