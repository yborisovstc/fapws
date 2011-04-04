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

// TODO [YB] Consider the following use-case
// Tick#1 Subj requested "up", Contr set mutation, Storage applies mutation, reactivate Subj, activate Contr (depends on Stor str chg)
// Tick#2 Contr updating but the inputs "up" from subj are not recalculates yes, so they are not consistent to new Storage structure
// So Contr recalculation going to be wrong. It seems to be common problem that the inp set can be incorrect what transition process.

static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_contr_log.txt";
static const char* KSpecFileName = "ut_contr_spec.xml";

static const TUint32 KMass_Min = 1;
static const TUint32 KMass_Max = 100;

void utcontr_update_up(CAE_Object* aObject, CAE_StateBase* aState);
void utcontr_update_contr(CAE_Object* aObject, CAE_StateBase* aState);
void utcontr_update_timer_start(CAE_Object* aObject, CAE_StateBase* aState);
void utcontr_update_timer_stop(CAE_Object* aObject, CAE_StateBase* aState);
void utcontr_update_timer_count(CAE_Object* aObject, CAE_StateBase* aState);

const TTransInfo KTinfo_Update_up = TTransInfo(utcontr_update_up, "trans_up");
const TTransInfo KTinfo_Update_contr = TTransInfo(utcontr_update_contr, "trans_controller");
const TTransInfo KTinfo_Update_timer_start = TTransInfo(utcontr_update_timer_start, "trans_timer_start");
const TTransInfo KTinfo_Update_timer_stop = TTransInfo(utcontr_update_timer_stop, "trans_timer_stop");
const TTransInfo KTinfo_Update_timer_count = TTransInfo(utcontr_update_timer_count, "trans_timer_count");

static const TTransInfo* tinfos[] = {
    &KTinfo_Update_up, 
    &KTinfo_Update_contr,
    &KTinfo_Update_timer_start,
    &KTinfo_Update_timer_stop,
    &KTinfo_Update_timer_count,
    NULL};



class UT_FAP_Contr : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Contr);
    CPPUNIT_TEST(test_Contr_main);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Contr_main();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Contr );

void utcontr_update_up(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TBool>& self = (CAE_TState<TBool>&) *aState;
    const TUint32& data = self.Inp("data");
    if (self.Input("data_up") != NULL) {
	const TUint32& data_up = self.Inp("data_up");
	self = (data > data_up);
    }
}

void utcontr_update_timer_start(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TBool>& sself = (CAE_TState<TBool>&) *aState;
    const TBool&  stop = sself.Inp("stop");
}

void utcontr_update_timer_stop(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TBool>& sself = (CAE_TState<TBool>&) *aState;
}

void utcontr_update_timer_count(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_TState<TInt>& sself = (CAE_TState<TInt>&) *aState;
}

void utcontr_update_contr(CAE_Object* aObject, CAE_StateBase* aState)
{
    CAE_StateCtr* selfp = aState->GetFbObj(selfp);
    CAE_StateCtr& self = *selfp;
    const TUint32& pos = self.Inp("pos");
    _FAP_ASSERT(self.Ref() != NULL);
    CAE_Object::ChromoPx* cpx = self.Ref()->GetFbObj(cpx);
    _FAP_ASSERT(cpx != NULL);
    CAE_ChromoNode croot = cpx->Chr().Root();
    CAE_ChromoNode smut = cpx->Mut().Root();

    // TODO [YB] Consider different method - to check if pair is subj or its upper neighbour and update the pair
    // Update position
    // Update position #1: find the subject under given position
    string outname = "_Most";
    for (TUint32 posi = 0; posi < pos; posi++)
    {
	CAE_ChromoNode::Iterator nexti = croot.Find(ENt_Conn, ENa_ConnPair, outname + ".output");
	if (nexti != croot.End()) {
	    string nextconnid = (*nexti).Name();
	    outname = nextconnid.substr(0, nextconnid.find("."));
	}
    }
    // Update position #2: check if the pair for Storage output has been changed
    CAE_ChromoNode::Iterator stor_outp = croot.Find(ENt_Cext, "output"); 
    string stor_outp_pair = (*stor_outp).Attr(ENa_ConnPair);
    // Update position #3: specify mutation to change output connection
    if (stor_outp_pair.compare(outname + ".output") != 0) 
    {
	CAE_ChromoNode chnode = smut.AddChild(ENt_MutChange);
	chnode.SetAttr(ENa_Type, ENt_Cext);
	chnode.SetAttr(ENa_Id, "output");
	chnode.SetAttr(ENa_MutChgAttr, ENa_ConnPair);
	chnode.SetAttr(ENa_MutChgVal, outname + ".output");
    }


    for (CAE_State::mult_point_inp_iterator i = self.MpInput_begin("subj_req"); i != self.MpInput_end("subj_req"); i++)
    {
	CAE_TState<TBool>& st_up_requested = i.State("_1");
	if (~st_up_requested) {
	    // Swap requesting subject with the upper neighboor
	    // #1 Find subject, upper and lower neighboors
	    string subj_name = st_up_requested.MansName(0);
	    CAE_ChromoNode::Iterator iconn_subj = croot.Find(ENt_Conn, subj_name + ".input");
	    CAE_ChromoNode conn_subj = *iconn_subj;
	    string upper_f = conn_subj.Attr(ENa_ConnPair);
	    string upper_name = upper_f.substr(0, upper_f.find("."));
	    CAE_ChromoNode::Iterator iconn_upper = croot.Find(ENt_Conn, upper_name + ".input");
	    CAE_ChromoNode conn_upper = *iconn_upper;
	    string upper_pair = conn_upper.Attr(ENa_ConnPair);
	    CAE_ChromoNode::Iterator iconn_lower = croot.Find(ENt_Conn, ENa_ConnPair, subj_name + ".output");
	    CAE_ChromoNode conn_lower = *iconn_lower;
	    string lower_f = conn_lower.Name();
	    string lower_name = lower_f.substr(0, lower_f.find("."));
	    // #2 Remove connections
	    CAE_ChromoNode rm = smut.AddChild(ENt_MutRm);
	    CAE_ChromoNode rm_subj = rm.AddChild(ENt_Node);
	    rm_subj.SetAttr(ENa_Type, ENt_Conn);
	    rm_subj.SetAttr(ENa_Id, conn_subj.Name());
	    CAE_ChromoNode rm_upper = rm.AddChild(ENt_Node);
	    rm_upper.SetAttr(ENa_Id, conn_upper.Name());
	    rm_upper.SetAttr(ENa_Type, ENt_Conn);
	    CAE_ChromoNode rm_lower = rm.AddChild(ENt_Node);
	    rm_lower.SetAttr(ENa_Id, conn_lower.Name());
	    rm_lower.SetAttr(ENa_Type, ENt_Conn);
	    // #3 Add new connections
	    CAE_ChromoNode add = smut.AddChild(ENt_MutAdd);
	    CAE_ChromoNode add_subj = add.AddChild(ENt_Conn);
	    add_subj.SetAttr(ENa_Id, subj_name + ".input");
	    add_subj.SetAttr(ENa_ConnPair, upper_pair);
	    CAE_ChromoNode add_upper = add.AddChild(ENt_Conn);
	    add_upper.SetAttr(ENa_Id, upper_name + ".input");
	    add_upper.SetAttr(ENa_ConnPair, subj_name + ".output");
	    CAE_ChromoNode add_lower = add.AddChild(ENt_Conn);
	    add_lower.SetAttr(ENa_Id, lower_name + ".input");
	    add_lower.SetAttr(ENa_ConnPair, upper_name + ".output");

	    // Update position #4: switch to upper if currently on requested subj
	    if (stor_outp_pair.compare(subj_name + ".output") == 0) 
	    {
		CAE_ChromoNode chnode = smut.AddChild(ENt_MutChange);
		chnode.SetAttr(ENa_Type, ENt_Cext);
		chnode.SetAttr(ENa_Id, "output");
		chnode.SetAttr(ENa_MutChgAttr, ENa_ConnPair);
		chnode.SetAttr(ENa_MutChgVal, upper_name + ".output");
	    }
	    else if (stor_outp_pair.compare(upper_name + ".output") == 0) 
	    {
		CAE_ChromoNode chnode = smut.AddChild(ENt_MutChange);
		chnode.SetAttr(ENa_Type, ENt_Cext);
		chnode.SetAttr(ENa_Id, "output");
		chnode.SetAttr(ENa_MutChgAttr, ENa_ConnPair);
		chnode.SetAttr(ENa_MutChgVal, subj_name + ".output");
	    }
	}
    }


    TBool updated = (smut.Begin() != smut.End()); 
    if (updated) {
	cpx->SetUpdated();
    }
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

void UT_FAP_Contr::test_Contr_main()
{
    printf("\n === Test of controlling\n");
    CAE_Object *order = iEnv->Root()->GetComp("Order");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [Order]", order != 0);

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }
    CAE_ConnPointBase* c_pos = order->GetOutpN("output");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [Order.output]", c_pos != 0);
    CAE_Base* b_pos = c_pos->GetSrcPin("_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [Order.output] src pin", b_pos != 0);
    CAE_StateBase* s_pos = b_pos->GetFbObj(s_pos);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [Order.output] src pin state", b_pos != 0);
    CAE_TState<TUint32>& pos = *s_pos;

    printf("\n Position: Name = %s, Value = %d\n", pos.MansName(0), ~pos);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data on given postion", strcmp(pos.MansName(0), "Alex") == 0);
    CPPUNIT_ASSERT_MESSAGE("Incorrect data on given postion", ~pos == 7);
}


