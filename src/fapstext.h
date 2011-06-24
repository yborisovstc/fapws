#ifndef __FAP_STEXT_H
#define __FAP_STEXT_H

// FAP States extentions

#include <math.h>
#include "fapbase.h"
#include <stdlib.h>

struct CF_TdVectF;
struct CF_TdPointF;

struct CF_TdPoint
{
    CF_TdPoint() {}
    CF_TdPoint(const CF_TdPoint& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
    CF_TdPoint(TInt aX, TInt aY): iX(aX), iY(aY) {}
    inline CF_TdPoint(const CF_TdPointF& aSrc);
    inline CF_TdPoint(const CF_TdVectF& aSrc);
    CF_TdPoint& operator=(const CF_TdPoint& aOpd) { iX=aOpd.iX; iY=aOpd.iY; return *this;}
    CF_TdPoint operator+(const CF_TdPoint& aOpd) { return CF_TdPoint(iX+aOpd.iX, iY+aOpd.iY);}
    CF_TdPoint operator+(const CF_TdPoint& aOpd) const { return CF_TdPoint(iX+aOpd.iX, iY+aOpd.iY);}
    CF_TdPoint operator-(const CF_TdPoint& aOpd) { return CF_TdPoint(iX-aOpd.iX, iY-aOpd.iY);}
    CF_TdPoint operator-(const CF_TdPoint& aOpd) const { return CF_TdPoint(iX-aOpd.iX, iY-aOpd.iY);}
    CF_TdPoint operator*(TInt aOpd) { return CF_TdPoint(iX*aOpd, iY*aOpd); }
    CF_TdPoint operator*(TInt aOpd) const { return CF_TdPoint(iX*aOpd, iY*aOpd); }
    float M2() {return ((float) iX* (float) iX + (float) iY* (float) iY);}
    float Mod() {return sqrt(M2());}
    TInt iX;
    TInt iY;
};


struct CF_TdPointF
{
    CF_TdPointF() {}
    CF_TdPointF(const CF_TdPointF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
    inline CF_TdPointF(const CF_TdVectF& aSrc);
    CF_TdPointF(const CF_TdPoint& aSrc): iX(aSrc.iX), iY(aSrc.iY)  {}
    CF_TdPointF(float aX, float aY): iX(aX), iY(aY) {}
    CF_TdPointF& operator=(const CF_TdPointF& aOpd) { iX=aOpd.iX; iY=aOpd.iY; return *this;}
    CF_TdPointF operator+(const CF_TdPointF& aOpd) { return CF_TdPointF(iX+aOpd.iX, iY+aOpd.iY);}
    CF_TdPointF operator+(const CF_TdPointF& aOpd) const { return CF_TdPointF(iX+aOpd.iX, iY+aOpd.iY);}
    CF_TdPointF operator-(const CF_TdPointF& aOpd) { return CF_TdPointF(iX-aOpd.iX, iY-aOpd.iY);}
    CF_TdPointF operator-(const CF_TdPointF& aOpd) const { return CF_TdPointF(iX-aOpd.iX, iY-aOpd.iY);}
    float M2() {return (iX*iX + iY*iY);}
    float Mod() {return sqrt(M2());}
    float iX;
    float iY;
};

struct CF_TdVect
{
    TInt iX;
    TInt iY;
};

struct CF_TdVectF
{
    CF_TdVectF() {}
    CF_TdVectF(const CF_TdVectF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
    CF_TdVectF(const CF_TdPointF& aEnd): iX(aEnd.iX), iY(aEnd.iY) {}
    CF_TdVectF(const CF_TdPointF& aBeg, const CF_TdPointF& aEnd): iX(aEnd.iX-aBeg.iX), iY(aEnd.iY-aBeg.iY) {}
    CF_TdVectF(float aX, float aY): iX(aX), iY(aY) {}
    TBool      operator==(const CF_TdVectF& aOpd) { return iX == aOpd.iX && iY == aOpd.iY;} 
    CF_TdVectF operator+(const CF_TdVectF& aOpd) { return CF_TdVectF(iX+aOpd.iX, iY+aOpd.iY);} 
    CF_TdVectF operator+(const CF_TdVectF& aOpd) const { return CF_TdVectF(iX+aOpd.iX, iY+aOpd.iY);} 
    CF_TdVectF operator-(const CF_TdVectF& aOpd) { return CF_TdVectF(iX-aOpd.iX, iY-aOpd.iY);} 
    CF_TdVectF operator-(const CF_TdVectF& aOpd) const { return CF_TdVectF(iX-aOpd.iX, iY-aOpd.iY);} 
    CF_TdVectF operator*(float aOpd) { return CF_TdVectF(iX*aOpd, iY*aOpd); }
    CF_TdVectF operator*(float aOpd) const { return CF_TdVectF(iX*aOpd, iY*aOpd); }
    CF_TdVectF operator/(float aOpd) { return CF_TdVectF(iX/aOpd, iY/aOpd); }
    CF_TdVectF operator/(float aOpd) const { return CF_TdVectF(iX/aOpd, iY/aOpd); }
    CF_TdVectF& operator=(const CF_TdVectF& aOpd) { iX=aOpd.iX; iY=aOpd.iY; return *this;}
    CF_TdVectF& operator+=(const CF_TdVectF& aOpd) { iX+=aOpd.iX; iY+=aOpd.iY; return *this;}
    CF_TdVectF& operator-=(const CF_TdVectF& aOpd) { iX-=aOpd.iX; iY-=aOpd.iY; return *this;}
    float M2() {return (iX*iX + iY*iY);}
    float Mod() {return sqrt(M2());}
 
    float iX;
    float iY;
};

inline CF_TdPointF::CF_TdPointF(const CF_TdVectF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}

inline CF_TdPoint::CF_TdPoint(const CF_TdVectF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
inline CF_TdPoint::CF_TdPoint(const CF_TdPointF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}

struct CF_Rect
{
    CF_Rect(float aLeft, float aTop, float aRight, float aBottom) {
	iLeftUpper.iX=aLeft; iLeftUpper.iY=aTop; iRightLower.iX=aRight; iRightLower.iY=aBottom;}
    CF_Rect() {}
    CF_TdPoint iLeftUpper;
    CF_TdPoint iRightLower;
};

// User defined state, based on DEST expression
class CAE_StateEx : public CAE_StateBase
{
    public:
	CAE_StateEx(const string& aType, const string& aInstName, CAE_Object* aMan);
	virtual ~CAE_StateEx();
	static inline const char *Type(); 
	// TODO [YB] Migrate to const version
	CSL_ExprBase& Value() { return *iCurr;};
    protected:
	virtual void Confirm();
	virtual char* DataToStr(TBool aCurr) const;
	virtual void DataFromStr(const char* aStr, void *aData) const;
	virtual void DoSet(void* aData);
	virtual void DoSetFromStr(const char *aStr);
	virtual void *DoGetFbObj(const char *aName);
    private:
	CSL_ExprBase* iCurr;
	CSL_ExprBase* iNew;

};

inline const char *CAE_StateEx::Type() { return "StateEx";} 

#endif // __FAP_STEXT_H
