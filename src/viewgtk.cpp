
#include <gdk/gdk.h>
#include "viewgtk.h"

static gboolean handle_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

CAE_ViewGtk::CAE_ViewGtk(TType aType, GtkWidget* aWidget): iType(aType), iWidget(aWidget), iObserver(NULL)
{
    PangoContext* pcnt = gtk_widget_get_pango_context(iWidget);
    GdkWindow* gwnd = iWidget->window;
    MAE_Window::Attr attr;
    attr.iRect = CAV_Rect();
    iWnd = new CAV_WindowGtk(NULL, gwnd, pcnt);
    
    g_signal_connect(G_OBJECT(iWidget), "expose_event", G_CALLBACK(handle_expose_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_press_event", G_CALLBACK (handle_button_press_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_release_event", G_CALLBACK (handle_button_release_event), this);
}

CAE_ViewGtk::~CAE_ViewGtk()
{
    delete iWnd;
}

const string& CAE_ViewGtk::Name()
{
    return iName;
}

void CAE_ViewGtk::SetName(const string& aName)
{
    _FAP_ASSERT(iName.empty());
    iName = aName;
}

void CAE_ViewGtk::SetObserver(MAE_ViewObserver* aObs)
{
    _FAP_ASSERT(iObserver == NULL);
    iObserver = aObs;
}

void CAE_ViewGtk::ResetObserver(MAE_ViewObserver* aObs)
{
    _FAP_ASSERT(iObserver == aObs);
    iObserver = NULL;
}

gboolean handle_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    CAE_ViewGtk* self = (CAE_ViewGtk*) data;
    self->Observer()->OnExpose(self, CAE_ViewGtkUtils::Rect(event->area));
}

gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAE_ViewGtk* self = (CAE_ViewGtk*) data;
    MAE_ViewObserver::TBtnEv evt;
    if (event->type == GDK_BUTTON_PRESS) evt = MAE_ViewObserver::EBte_Press;
    else if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) evt = MAE_ViewObserver::EBte_DoublePress;
    return self->Observer()->OnButton(self, self->Wnd(), evt, event->button, CAV_Point(event->x, event->y));
}

gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAE_ViewGtk* self = (CAE_ViewGtk*) data;
    MAE_ViewObserver::TBtnEv evt;
    evt = MAE_ViewObserver::EBte_Release;
    return self->Observer()->OnButton(self, self->Wnd(), evt, event->button, CAV_Point(event->x, event->y));
}

MAE_Window* CAE_ViewGtk::Wnd()
{
    return iWnd;
}

void CAE_ViewGtk::SetDetLevel(TInt aLevel)
{
    iDetLevel = aLevel;
}



CAV_WindowGtk::CAV_WindowGtk(CAV_WindowGtk* aParent, GdkWindow* aGdkWnd, PangoContext* aPContext): 
    iParent(aParent), iGdkWnd(aGdkWnd), iGc(NULL), iPContext(aPContext)
{
    iGc = new CAV_Gc(this, gdk_gc_new(iGdkWnd));
}

CAV_WindowGtk::~CAV_WindowGtk()
{
    if (iGc != NULL) {
	delete iGc;
    }
}

MAE_Gc* CAV_WindowGtk::Gc()
{
    return iGc;
}

CAV_Rect CAV_WindowGtk::Rect()
{
    int width, height;
    gdk_drawable_get_size(iGdkWnd, &width, &height);
    return CAV_Rect(CAV_Point(0, 0), CAV_Point(width, height));
}

void CAV_WindowGtk::Clear()
{
    gdk_window_clear(iGdkWnd);
}

MAE_Window* CAV_WindowGtk::CreateWindow(const string& aName, const Attr& aAttr)
{
    _FAP_ASSERT(iChilds.count(aName) == 0);
    GdkWindowAttr attr;
    attr.x = aAttr.iRect.iTl.iX;
    attr.y = aAttr.iRect.iTl.iY;
    attr.width = aAttr.iRect.Width();
    attr.height = aAttr.iRect.Height();
    gint mask = GDK_WA_X | GDK_WA_X;
    GdkWindow *wnd = gdk_window_new(iGdkWnd, &attr, mask);
    iChilds[aName] = new CAV_WindowGtk(this, wnd, iPContext);
}

MAE_Window* CAV_WindowGtk::Wnd(const string& aName)
{
    return iChilds.find(aName)->second;
}

CAV_Gc::CAV_Gc(CAV_WindowGtk* aWnd, GdkGC* aGdkGc): iWnd(aWnd), iGdkGc(aGdkGc)
{
}

GdkWindow* CAV_Gc::GdkWnd()
{
    return iWnd->iGdkWnd;
}

PangoContext* CAV_Gc::PContext()
{
    return iWnd->iPContext;
}

void CAV_Gc::DrawRect(CAV_Rect aRect, TBool aFilled)
{
    gdk_draw_rectangle(iWnd->iGdkWnd, iGdkGc, aFilled, aRect.iTl.iX, aRect.iTl.iY, aRect.iBr.iX - aRect.iTl.iX, aRect.iBr.iY - aRect.iTl.iY);
}


CAV_Rect CAE_ViewGtkUtils::Rect(GdkRectangle aRect)
{
    return CAV_Rect(CAV_Point(aRect.x, aRect.y), CAV_Point(aRect.x + aRect.width, aRect.y + aRect.height));
}

void CAV_Gc::DrawText(const string& aText, CAV_Rect aRect)
{
    PangoLayout* lout = pango_layout_new(iWnd->iPContext);
    pango_layout_set_text(lout, aText.c_str(), aText.size());
    gdk_draw_layout(iWnd->iGdkWnd, iGdkGc, aRect.iTl.iX, aRect.iTl.iY, lout);
}

void CAV_Gc::DrawLine(CAV_Point aPt1, CAV_Point aPt2)
{
    gdk_draw_line(iWnd->iGdkWnd, iGdkGc, aPt1.iX, aPt1.iY, aPt2.iX, aPt2.iY);
}

MAE_TextLayout* CAV_Gc::CreateTextLayout()
{
    return new CAV_TextLayoutGtk(*this);
}

CAV_TextLayoutGtk::CAV_TextLayoutGtk(CAV_Gc& aGc): iGc(aGc)
{
    iGdkTextLayout = pango_layout_new(iGc.PContext());
}

CAV_TextLayoutGtk::~CAV_TextLayoutGtk()
{
}

void CAV_TextLayoutGtk::SetText(const string& aText)
{
    pango_layout_set_text(iGdkTextLayout, aText.c_str(), aText.size());
}

void CAV_TextLayoutGtk::Draw(CAV_Point aPt)
{
    gdk_draw_layout(iGc.GdkWnd(), iGc.iGdkGc, aPt.iX, aPt.iY, iGdkTextLayout);
}

void CAV_TextLayoutGtk::GetSizePu(CAV_Point& aSize)
{
    pango_layout_get_pixel_size(iGdkTextLayout, &(aSize.iX), &(aSize.iY));
}

