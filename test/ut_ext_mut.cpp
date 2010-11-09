#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of extended inputs and mutation
 * Extended input is just the rule for what inputs can be added into state.
 * It is like "Mass.*" where the extention '*' can be replaced by particular value
 * Mutation is the changing of the object when it creating from parent.
 */


static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_ext_mut.txt";
static const char* KSpecFileName = "ut_spec_ext_mut.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;
// For how many snails is the feed prepared on theirs way
static const TUint32 KMaxFeed = 3;

void update_mass(CAE_Object* aObject, CAE_State* aState);
void update_coord(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_mass = TTransInfo(update_mass, "trans_mass");
const TTransInfo KTinfo_Update_coord = TTransInfo(update_coord, "trans_coord");
static const TTransInfo* tinfos[] = {&KTinfo_Update_mass, &KTinfo_Update_coord, NULL};



class UT_FAP_ExtMut : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_ExtMut);
    CPPUNIT_TEST(test_ExtMut_main);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_ExtMut_main();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_ExtMut );

void update_mass(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = (CAE_TState<TUint32>*)aState;
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [mass]", pself != 0);
    CAE_TState<TUint32> *pcoord_s = (CAE_TState<TUint32>*) aState->Input("coord_self");
    CPPUNIT_ASSERT_MESSAGE("Fail getting coord_self", pcoord_s != 0);

    TUint32 vself = ~*pself;
    TUint32 vcoord_self = ~*pcoord_s;
    // Getting the num of others snails being forvard
    TInt feed = KMaxFeed;
    for (TInt i = 1; ; i++) {
	CAE_TState<TUint32> *pcoord_o = (CAE_TState<TUint32>*) aState->Input("coord_others", i);
	if (pcoord_o == NULL) break;
	TUint32 vcoord_o = ~*pcoord_o;
	if (vcoord_o > vcoord_self && feed > 0)
	    feed--;
    }
    TUint32 newmass = vself + feed - 1;
    if (newmass > KMass_Max) newmass = KMass_Max;
    if (newmass < KMass_Min) newmass = KMass_Min;
    *pself = newmass;
}

void update_coord(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = CAE_TState<TUint32>::Interpret(aState);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [coord]", pself != 0);
    CAE_TState<TUint32> *pmass_s = (CAE_TState<TUint32>*) aState->Input("mass");
    CPPUNIT_ASSERT_MESSAGE("Fail getting [mass]", pmass_s != 0);
    TUint32 vself = ~*pself;
    TUint32 vmass_s = ~*pmass_s;
    if (vmass_s > 0) {
	*pself = vself + KMass_Max/vmass_s;
    }
}


void UT_FAP_ExtMut::setUp()
{
    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_ExtMut::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_ExtMut::test_ExtMut_main()
{
    printf("\n === Test of creation object from spec");
    CAE_Object *snail_1 = iEnv->Root()->GetComp("snail_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1]", snail_1 != 0);
    CAE_Object *snail_3 = iEnv->Root()->GetComp("snail_3");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3]", snail_3 != 0);
    CAE_State *mass_1 = snail_1->GetInput("mass");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1.mass]", mass_1 != 0);
    CAE_State *coord_3 = snail_3->GetOutput("coord");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3.coord]", coord_3 != 0);
    mass_1->AddExtInputL("coord_others", coord_3);
    
    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }
}


