#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <deslbase.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of embedded transitions
 */

static const char* KLogFileName = "ut_emtran_log.txt";
static const char* KSpecFileName = "ut_emtran_spec_2.xml";


static const TTransInfo* tinfos[] = { NULL};



class UT_FAP_Emtran : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Emtran);
    CPPUNIT_TEST(test_Emtran_main);
    CPPUNIT_TEST(test_Emtran_vect);
    CPPUNIT_TEST(test_Emtran_vectgf);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Emtran_main();
    void test_Emtran_vect();
    void test_Emtran_vectgf();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Emtran );


void UT_FAP_Emtran::setUp()
{
}

void UT_FAP_Emtran::tearDown()
{
//    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_Emtran::test_Emtran_main()
{
    printf("\n === Test of embedded trans - Snails\n");

    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_snail_1 = iEnv->Root()->GetOutpN("snail_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1] output", c_snail_1 != 0);
    CAE_StateBase* sb_snail_1_coord = c_snail_1->GetSrcPin("coord")->GetFbObj(sb_snail_1_coord);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1.coord] src pin state", sb_snail_1_coord != 0);
    CAE_TState<TInt>& s_snail_1_coord = *sb_snail_1_coord;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [snail_1.coord] value", ~s_snail_1_coord == 504);

    CAE_ConnPointExtC* ext = c_snail_1->GetFbObj(ext);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail] extender", ext != 0);
    for (vector<CAE_ConnPointExtC::Slot>::iterator it = ext->Slots().begin(); it != ext->Slots().end(); it++) {
	CAE_ConnPointBase* cp = it->Srcs()["coord"].first;
	CPPUNIT_ASSERT_MESSAGE("Fail to get [coord] conn point in [snail]extender", cp != 0);
	CAE_StateBase* sb_snail_coord = cp->GetSrcPin("_1")->GetFbObj(sb_snail_coord);
	CPPUNIT_ASSERT_MESSAGE("Fail to get [coord] src pin state in [snail] extender", sb_snail_coord != 0);
    };

    printf("\n === Test of embedded trans - Throw\n");

    iEnv = CAE_Env::NewL(NULL, tinfos, "ut_emtran_spec_3.xml", 1, NULL, "ut_emtran_log_3.txt");
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


static const char* KLogFileName_Vect = "ut_emtran_vect_log.txt";
static const char* KSpecFileName_Vect = "ut_emtran_vect.xml";

void UT_FAP_Emtran::test_Emtran_vect()
{
    printf("\n === Test of embedded trans - vect\n");

    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName_Vect, 1, NULL, KLogFileName_Vect);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("output");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [output] output", c_incr != 0);
    CAE_StateBase* sb_incr = c_incr->GetSrcPin("_1")->GetFbObj(sb_incr);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_incr._1] src pin state", sb_incr != 0);
    CAE_TState<CF_TdVectF>& s_incr = *sb_incr;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_incr._1] value", ~s_incr == CF_TdVectF(41.0, 43.0));


    delete iEnv;
}


void UT_FAP_Emtran::test_Emtran_vectgf()
{
    printf("\n === Test of embedded trans - generic vector\n");

    iEnv = CAE_Env::NewL(NULL, tinfos, "ut_emtran_vectgf.xml", 1, NULL, "ut_emtran_vectgf.log");
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }
    // Check result of sums
    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("output");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [output] output", c_incr != 0);
    CAE_StateBase* sb_incr = c_incr->GetSrcPin("_1")->GetFbObj(sb_incr);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_incr._1] src pin state", sb_incr != NULL);
    CAE_StateEx* s_incr = sb_incr->GetFbObj(s_incr);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_incr._1] ext state", s_incr != NULL);
    CSL_ExprBase& e_incr = s_incr->Value();
    string scoord_0 = e_incr.Args()[0]->Data();
    string scoord_1 = e_incr.Args()[1]->Data();
    string scoord_2 = e_incr.Args()[2]->Data();
    float coord_0, coord_1, coord_2;
    sscanf(scoord_0.c_str(), "%f", &coord_0);
    sscanf(scoord_1.c_str(), "%f", &coord_1);
    sscanf(scoord_2.c_str(), "%f", &coord_2);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [incr.output] value", coord_0 == 41.0 && coord_1 == 83.0 && coord_2 == 125.0);
    // Check scalar multiplication
    CAE_ConnPointBase* c_smult = iEnv->Root()->GetOutpN("outp_smult");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [outp_smult] output", c_smult != 0);
    CAE_StateBase* sb_smult = c_smult->GetSrcPin("_1")->GetFbObj(sb_smult);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_smult._1] src pin state", sb_smult != NULL);
    CAE_StateEx* s_smult = sb_smult->GetFbObj(s_smult);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_smult._1] ext state", s_smult != NULL);
    CSL_ExprBase& e_smult = s_smult->Value();
    string str_smult = e_smult.Data();
    float res_smult;
    sscanf(str_smult.c_str(), "%f", &res_smult);
    CPPUNIT_ASSERT_MESSAGE("Incorrect [incr.outp_smult] value", res_smult == 283.0);

    delete iEnv;
}



