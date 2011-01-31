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


static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_conn_log.txt";
static const char* KSpecFileName = "ut_conn_spec.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;
// For how many snails is the feed prepared on theirs way
static const TUint32 KMaxFeed = 4;

void utconn_update_mass(CAE_Object* aObject, CAE_State* aState);
void utconn_update_coord(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_mass = TTransInfo(utconn_update_mass, "trans_mass");
const TTransInfo KTinfo_Update_coord = TTransInfo(utconn_update_coord, "trans_coord");
static const TTransInfo* tinfos[] = {&KTinfo_Update_mass, &KTinfo_Update_coord, NULL};



class UT_FAP_Conn : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Conn);
    CPPUNIT_TEST(test_Conn_main);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Conn_main();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Conn );

void utconn_update_mass(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32>& self = (CAE_TState<TUint32>&) *aState;
    const TUint32& coord_s = self.Inp("coord_self");
    const TBool& frugal_s = self.Inp("frugal");

    TInt feed = KMaxFeed;
/*
    for (TInt i = 1; self.Input("coord_others", i) != NULL; i++) {
	const TUint32& coord_o = self.Inp("coord_others", i);
	if (coord_o > coord_s && feed > 0)
	    feed--;
    }
    */
    for (CAE_State::mult_point_inp_iterator i = self.MpInput_begin("neighbor"); i != self.MpInput_end("neighbor"); i++)
    {
	const TUint32& coord_o = i.State("coord");
	const TBool& frugal_o = i.State("frugal");
	if (coord_o > coord_s && feed > 0)
	    feed -= frugal_o ? 1 : 2;
    }

    TUint32 newmass = ~self + ((feed <= 1) ? feed : (frugal_s ? 1 : 2));
    if (newmass > KMass_Max) newmass = KMass_Max;
    if (newmass < KMass_Min) newmass = KMass_Min;
    self = newmass;
}

void utconn_update_coord(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32>& self = (CAE_TState<TUint32>&) *aState;
    const TUint32& mass_s = self.Inp("mass");
    if (mass_s > 0) 
	self = ~self + KMass_Max/mass_s;
}


void UT_FAP_Conn::setUp()
{
    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_Conn::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_Conn::test_Conn_main()
{
    printf("\n === Test of connections");
    CAE_Object *snail_1 = iEnv->Root()->GetComp("snail_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1]", snail_1 != 0);
    CAE_Object *snail_3 = iEnv->Root()->GetComp("snail_3");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3]", snail_3 != 0);
    CAE_State *coord_3 = snail_3->GetOutpState("coord");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_3.coord]", coord_3 != 0);
    
    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }
}


