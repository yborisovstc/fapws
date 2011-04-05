
#ifndef __FAP_VIEWPRIM_H
#define __FAP_VIEWPRIM_H

// View interface

#include "fapplat.h"
#include <string.h>
#include <string>

using namespace std;

class MAE_Window;
class MAE_ViewObserver;

class CAV_Point
{
    public:
	CAV_Point(): iX(0), iY(0) {};
	CAV_Point(TInt aX, TInt aY): iX(aX), iY(aY) {};
	CAV_Point(const CAV_Point& aPt): iX(aPt.iX), iY(aPt.iY) {};
	CAV_Point operator+(const CAV_Point& aArg) { return CAV_Point(iX + aArg.iX, iY + aArg.iY);};
	CAV_Point operator-(const CAV_Point& aArg) { return CAV_Point(iX - aArg.iX, iY - aArg.iY);};
	CAV_Point& operator+=(const CAV_Point& aArg) { iX += aArg.iX, iY += aArg.iY; return *this;};
	CAV_Point& operator-=(const CAV_Point& aArg) { iX -= aArg.iX, iY -= aArg.iY; return *this;};
    public:
	TInt iX, iY;
};

class CAV_Rect
{
    public:
	CAV_Rect(): iTl(), iBr() {};
	CAV_Rect(CAV_Point aTl, CAV_Point aBr): iTl(aTl), iBr(aBr) {};
	CAV_Rect(CAV_Point aTl): iTl(aTl), iBr(aTl) {};
	TInt Height() const { return iBr.iY - iTl.iY; };
	TInt Width() const { return iBr.iX - iTl.iX; };
	void Move(CAV_Point aShift) { iTl += aShift; iBr += aShift; };
	void Resize(CAV_Point aPt) { iBr += aPt; };
	CAV_Point Tr() { return CAV_Point(iBr.iX, iTl.iY);};
	CAV_Point Bl() { return CAV_Point(iTl.iX, iBr.iY);};
	TBool Intersects(CAV_Point aPt) { return aPt.iX >= iTl.iX && aPt.iX <= iBr.iX && aPt.iY >= iTl.iY && aPt.iY <= iBr.iY;};
	TBool Intersects(CAV_Rect aRc) { return Intersects(aRc.iTl) || Intersects(aRc.iBr) || Intersects(aRc.Tr()) || Intersects(aRc.Bl());};
    public:
	CAV_Point iTl, iBr; 
};


#endif // __FAP_VIEWPRIM_H
