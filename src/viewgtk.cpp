
#include <gdk/gdk.h>
#include "viewgtk.h"

static gboolean handle_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

CAE_ViewGtk::CAE_ViewGtk(TType aType, GtkWidget* aWidget): iType(aType), iWidget(aWidget), iObserver(NULL)
{
    PangoContext* pcnt = gtk_widget_get_pango_context(iWidget);
    iWnd = new CAV_WindowGtk(iWidget->window, pcnt);
    
    g_signal_connect(G_OBJECT(iWidget), "expose_event", G_CALLBACK(handle_expose_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_press_event", G_CALLBACK (handle_button_press_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_release_event", G_CALLBACK (handle_button_release_event), this);
}

CAE_ViewGtk::~CAE_ViewGtk()
{
    delete iWnd;
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
    return self->Observer()->OnButton(self, evt, event->button, CAV_Point(event->x, event->y));
}

gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAE_ViewGtk* self = (CAE_ViewGtk*) data;
    MAE_ViewObserver::TBtnEv evt;
    evt = MAE_ViewObserver::EBte_Release;
    return self->Observer()->OnButton(self, evt, event->button, CAV_Point(event->x, event->y));
}

MAE_Window* CAE_ViewGtk::Wnd()
{
    return iWnd;
}

void CAE_ViewGtk::SetDetLevel(TInt aLevel)
{
    iDetLevel = aLevel;
}



CAV_WindowGtk::CAV_WindowGtk(GdkWindow* aGdkWnd, PangoContext* aPContext): 
    iGdkWnd(aGdkWnd), iGc(NULL), iPContext(aPContext)
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




CAV_Gc::CAV_Gc(CAV_WindowGtk* aWnd, GdkGC* aGdkGc): iWnd(aWnd), iGdkGc(aGdkGc)
{
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

