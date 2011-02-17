#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of controlling of objects
 * There can be state that is assosiated with object's chromosome
 * The state deals with it's "data" via specific interface of object (controlling iface)
 * This controlling state "updates" object via setting mutation to the object
 * Object is able to process mutation on system tick (on confirm phase)
 */


static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_contr_log.txt";
static const char* KSpecFileName = "ut_contr_spec.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;

void utcontr_update_up(CAE_Object* aObject, CAE_State* aState);
void utcontr_update_contr(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_up = TTransInfo(utcontr_update_up, "trans_up");
const TTransInfo KTinfo_Update_contr = TTransInfo(utcontr_update_contr, "trans_controller");

static const TTransInfo* tinfos[] = {&KTinfo_Update_up, &KTinfo_Update_contr, NULL};



class UT_FAP_Contr : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Contr);
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

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Contr );

void utcontr_update_up(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32>& self = (CAE_TState<TUint32>&) *aState;
}

void utcontr_update_contr(CAE_Object* aObject, CAE_State* aState)
{
}


void UT_FAP_Contr::setUp()
{
    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_Contr::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_Contr::test_Conn_main()
{
    printf("\n === Test of controlling\n");
    
    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }
}


