#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <deslbase.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of embedded modules
 */


class UT_FAP_Embmod : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Embmod);
    CPPUNIT_TEST(test_Embmod_vect);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Embmod_vect();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Embmod );


void UT_FAP_Embmod::setUp()
{
}

void UT_FAP_Embmod::tearDown()
{
//    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}


static const char* KLogFileName_vect = "ut_embmod_vect.txt";
static const char* KSpecFileName_vect = "ut_embmod_vect.xml";

void UT_FAP_Embmod::test_Embmod_vect()
{
    printf("\n === Test of module [Vector]\n");

    iEnv = CAE_Env::NewL(NULL, NULL, KSpecFileName_vect, 1, NULL, KLogFileName_vect);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<18; i++)
    {
        iEnv->Step();
    }


    CAE_ConnPointBase* c_rock = iEnv->Root()->GetOutpN("rock");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [rock] output", c_rock != 0);
    CAE_StateBase* sb_rock_coord = c_rock->GetSrcPin("coord")->GetFbObj(sb_rock_coord);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [rock.coord] src pin state", sb_rock_coord != NULL);
    CAE_StateEx* s_rock_coord = sb_rock_coord->GetFbObj(s_rock_coord);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [rock.coord] ext state", s_rock_coord != NULL);
    CSL_ExprBase& e_rock_coord = s_rock_coord->Value();
    string coord_xs = e_rock_coord.Args()[0]->Data();
    string coord_ys = e_rock_coord.Args()[1]->Data();
    float coord_x, coord_y;
    sscanf(coord_xs.c_str(), "%f", &coord_x);
    sscanf(coord_ys.c_str(), "%f", &coord_y);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [rock.coord] value", coord_x == 1440.0 && coord_y > 79.2999191 && coord_y < 79.2999192);

    delete iEnv;
}
