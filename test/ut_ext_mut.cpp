#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of extended inputs and mutation
 * Extended input is just the rule for what inputs can be added into state.
 * It is like "Mass.*" where the extention '*' can be replaced by particular value
 * Mutation is the changing of the object when it creating from parent.
 */

static const char* KLogFileName = "ut_ext_mut.txt";
static const char* KSpecFileName = "ut_spec_ext_mut.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;
// For how many snails is the feed prepared on theirs way
static const TUint32 KMaxFeed = 2;

void update_mass(CAE_Object* aObject, CAE_StateBase* aState);
void update_coord(CAE_Object* aObject, CAE_StateBase* aState);

const TTransInfo KTinfo_Update_mass = TTransInfo(update_mass, "trans_mass");
const TTransInfo KTinfo_Update_coord = TTransInfo(update_coord, "trans_coord");
static const TTransInfo* tinfos[] = {&KTinfo_Update_mass, &KTinfo_Update_coord, NULL};



class UT_FAP_ExtMut : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_ExtMut);
    CPPUNIT_TEST(test_ExtMut_main);
    CPPUNIT_TEST(test_ExtMut_deta1);
    CPPUNIT_TEST(test_ExtMut_deta2);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_ExtMut_main();
    void test_ExtMut_deta1();
    void test_ExtMut_deta2();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_ExtMut );

void update_mass(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TUint32>& self = (CAE_TState<TUint32>&) *aState;
    const TUint32& coord_s = self.Inp("coord_self");

    TInt feed = KMaxFeed;
    for (CAE_State::mult_point_inp_iterator i = self.MpInput_begin("coord_others"); i != self.MpInput_end("coord_others"); i++)
    {
	const TUint32& coord_o = *i;
	if (coord_o > coord_s && feed > 0)
	    feed--;
    }

    TUint32 newmass = ~self + feed - 1;
    if (newmass > KMass_Max) newmass = KMass_Max;
    if (newmass < KMass_Min) newmass = KMass_Min;
    self = newmass;
}

void update_coord(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TUint32>& self = (CAE_TState<TUint32>&) *aState;
    const TUint32& mass_s = self.Inp("mass");
    if (mass_s > 0) 
	self = ~self + KMass_Max/mass_s;
}


void UT_FAP_ExtMut::setUp()
{
}

void UT_FAP_ExtMut::tearDown()
{
}

void UT_FAP_ExtMut::test_ExtMut_main()
{
    printf("\n === Test of creation object from spec");

    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    CAE_Object *snail_1 = iEnv->Root()->GetComp("snail_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1]", snail_1 != 0);
    CAE_Object *snail_3 = iEnv->Root()->GetComp("snail_3");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3]", snail_3 != 0);
    CAE_StateBase *coord_3 = snail_3->GetOutpState("coord");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3.coord]", coord_3 != 0);
    // Test adding ext inputs
//    mass_1->AddExtInputL("coord_others", coord_3);
    
    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    delete iEnv;
}

// This test is for so called "detached" chromo when delta chromo form is used. 
// This is the case for all components inherited from parent. The heir actually doesn't contain
// parent's comps in it chromo but in runtime only. So mutating of such "deattached" chromo
// has to be done on heir's level. The test checks changing attr in such detached chromo

void UT_FAP_ExtMut::test_ExtMut_deta1()
{
    printf("\n === Test of mutations - Detached - changing state init in inherited subsystem\n");

    iEnv = CAE_Env::NewL(NULL, NULL, "ut_mut_deta1.xml", 1, NULL, "ut_mut_deta1.log");
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("my_incr");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [my_incr] output", c_incr != 0);
    CAE_Base* b_incr = c_incr->GetSrcPin("_1");
    CAE_StateBase* sb_incr = b_incr->GetFbObj(sb_incr);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_incr._1] src pin state", sb_incr != 0);
    CAE_TState<TInt>& s_incr = *sb_incr;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_incr._1] value", ~s_incr == 42);

    delete iEnv;
}

// This test is for so called "detached" chromo when delta chromo form is used. 
// This is the case for all components inherited from parent. The heir actually doesn't contain
// parent's comps in it chromo but in runtime only. So mutating of such "deattached" chromo
// has to be done on heir's level. The test checks deleting node in such detached chromo.

void UT_FAP_ExtMut::test_ExtMut_deta2()
{
    printf("\n === Test of mutations - Detached - deleting state in inherited subsystem\n");

    iEnv = CAE_Env::NewL(NULL, NULL, "ut_mut_deta2.xml", 1, NULL, "ut_mut_deta2.log");
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("my_incr");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [my_incr] output", c_incr != 0);
    CAE_Base* sb_incr = c_incr->GetSrcPin("_1");
    CPPUNIT_ASSERT_MESSAGE("Failed: [c_incr._1] src pin state still exists", sb_incr == 0);

    delete iEnv;
}
