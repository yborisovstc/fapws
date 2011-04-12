
#include <gdk/gdk.h>
#include "viewgtk.h"

static gboolean handle_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean handle_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data);
static gboolean handle_size_request_event( GtkWidget *widget, GtkRequisition *requisition, gpointer data);
static gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data);
static gboolean handle_enter_notify_event( GtkWidget *widget, GdkEventCrossing *event, gpointer data);


CAE_ViewGtk::CAE_ViewGtk(TType aType, GtkLayout* aWidget): iType(aType), iWidget(aWidget)
{
    PangoContext* pcnt = gtk_widget_get_pango_context(GTK_WIDGET(iWidget));
//    gtk_container_set_resize_mode(GTK_CONTAINER(iWidget), GTK_RESIZE_QUEUE);
    iWnd = new CAV_WindowGtk(this, NULL, "Main", iWidget, pcnt);
}

CAE_ViewGtk::~CAE_ViewGtk()
{
    delete iWnd;
}

const string& CAE_ViewGtk::Name() const
{
    return iName;
}

void CAE_ViewGtk::SetName(const string& aName)
{
    _FAP_ASSERT(iName.empty());
    iName = aName;
}

MAE_Window* CAE_ViewGtk::Wnd()
{
    return iWnd;
}

void CAE_ViewGtk::SetDetLevel(TInt aLevel)
{
    iDetLevel = aLevel;
}




gboolean handle_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    self->Observer()->OnExpose(self, CAE_ViewGtkUtils::Rect(event->area));
    return FALSE;
}

gboolean handle_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    MAE_ViewObserver::TBtnEv evt;
    if (event->type == GDK_BUTTON_PRESS) evt = MAE_ViewObserver::EBte_Press;
    else if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) evt = MAE_ViewObserver::EBte_DoublePress;
    return self->Observer()->OnButton(self, evt, event->button, CAV_Point(event->x, event->y));
}

gboolean handle_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    MAE_ViewObserver::TBtnEv evt;
    evt = MAE_ViewObserver::EBte_Release;
    return self->Observer()->OnButton(self, evt, event->button, CAV_Point(event->x, event->y));
}

gboolean handle_size_allocate_event( GtkWidget *widget, GtkAllocation *allocation, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    CAV_Rect rect(CAV_Point(allocation->x, allocation->y), allocation->width, allocation->height);
    self->Observer()->OnResized(self, rect);
}

gboolean handle_size_request_event( GtkWidget *widget, GtkRequisition *requisition, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    CAV_Rect rect;
    gint w,h;
    gtk_widget_get_size_request(widget, &w, &h);
//    gtk_widget_set_size_request(widget, 321, 223);
    gtk_widget_get_size_request(widget, &w, &h);
    //requisition->width = 500;
    //requisition->height = 300;
    self->Observer()->OnPrefSizeRequested(self, rect);
    requisition->width = rect.Width();
    requisition->height = rect.Height();
}

gboolean handle_motion_notify_event( GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    CAV_Point coord(event->x, event->y);
    self->Observer()->OnMotion(self, coord);
}

gboolean handle_enter_notify_event( GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
    CAV_WindowGtk* self = (CAV_WindowGtk*) data;
    self->Observer()->OnCrossing(self, (event->type == GDK_ENTER_NOTIFY));
}



CAV_WindowGtk::CAV_WindowGtk(const MAE_View* aView, CAV_WindowGtk* aParent, const string& aName, GtkLayout* aWidget, PangoContext* aPContext): 
    iView(aView), iParent(aParent), iName(aName), iWidget(aWidget), iGc(NULL), iPContext(aPContext), iObserver(NULL)
{
    iGc = new CAV_Gc(this, gdk_gc_new(aWidget->bin_window));
    
    g_signal_connect(G_OBJECT(iWidget), "expose_event", G_CALLBACK(handle_expose_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_press_event", G_CALLBACK (handle_button_press_event), this);
    g_signal_connect (G_OBJECT (iWidget), "button_release_event", G_CALLBACK (handle_button_release_event), this);
    g_signal_connect (G_OBJECT (iWidget), "size_allocate", G_CALLBACK (handle_size_allocate_event), this);
    g_signal_connect (G_OBJECT (iWidget), "size_request", G_CALLBACK (handle_size_request_event), this);
    g_signal_connect (G_OBJECT (iWidget), "motion_notify_event", G_CALLBACK (handle_motion_notify_event), this);
    g_signal_connect (G_OBJECT (iWidget), "enter_notify_event", G_CALLBACK (handle_enter_notify_event), this);
}

CAV_WindowGtk::~CAV_WindowGtk()
{
    if (iGc != NULL) {
	delete iGc;
    }
    gtk_widget_destroy(GTK_WIDGET(iWidget));
    iParent->RemoveChild(this);
}

void CAV_WindowGtk::Destroy()
{
    delete this;
}

MAE_Gc* CAV_WindowGtk::Gc()
{
    return iGc;
}

const string& CAV_WindowGtk::Name()
{
    return iName;
}

void CAV_WindowGtk::SetObserver(MAE_ViewObserver* aObs)
{
    _FAP_ASSERT(iObserver == NULL);
    iObserver = aObs;
}

void CAV_WindowGtk::ResetObserver(MAE_ViewObserver* aObs)
{
    _FAP_ASSERT(iObserver == aObs);
    iObserver = NULL;
}

CAV_Rect CAV_WindowGtk::Rect()
{
    guint width, height;
    gtk_layout_get_size(iWidget, &width, &height);
    // Dbg
    int x, y, w, h;
    gdk_window_get_position(iWidget->bin_window, &x, &y);
    gdk_drawable_get_size(iWidget->bin_window, &w, &h);
    //
    return CAV_Rect(CAV_Point(0, 0), CAV_Point(width, height));
}

void CAV_WindowGtk::SetRect(const CAV_Rect& aRect)
{
    gtk_layout_set_size(iWidget, aRect.Width(), aRect.Height());
    // TODO [YB] Using gtk_layout_move cause infinite loop of alloc-event. To analyze
//    if (iParent != NULL)
//	gtk_layout_move(iParent->iWidget, GTK_WIDGET(iWidget), aRect.iTl.iX, aRect.iTl.iY);
    GtkAllocation alloc;
    alloc.x = aRect.iTl.iX;
    alloc.y = aRect.iTl.iY;
    alloc.width = aRect.Width();
    alloc.height = aRect.Height();
    gtk_widget_size_allocate(GTK_WIDGET(iWidget), &alloc);
    // Dbg
    guint width, height;
    gtk_layout_get_size(iWidget, &width, &height);
    int x, y, w, h;
    gdk_window_get_position(iWidget->bin_window, &x, &y);
    gdk_drawable_get_size(iWidget->bin_window, &w, &h);
}

void CAV_WindowGtk::SetPrefSize(const CAV_Rect& aRect)
{
    gtk_widget_set_size_request(GTK_WIDGET(iWidget), aRect.Width(), aRect.Height());
}

CAV_Rect CAV_WindowGtk::GetPrefSize()
{
    gint w, h;
    gtk_widget_get_size_request(GTK_WIDGET(iWidget), &w, &h);
    return CAV_Rect(CAV_Point(0, 0), w, h);
}

CAV_Rect CAV_WindowGtk::CalcPrefSize()
{
    GtkRequisition req;
    gtk_widget_size_request(GTK_WIDGET(iWidget), &req);
    return CAV_Rect(CAV_Point(0, 0), req.width, req.height);
}

void CAV_WindowGtk::Show(TBool aAll)
{
    // Dbg
    int x, y, w, h;
    gdk_window_get_position(iWidget->bin_window, &x, &y);
    gdk_drawable_get_size(iWidget->bin_window, &w, &h);
    //
    gtk_widget_show(GTK_WIDGET(iWidget));
    if (aAll)
	gtk_widget_show_all(GTK_WIDGET(iWidget));
}

void CAV_WindowGtk::Clear()
{
    gdk_window_clear(iWidget->bin_window);
}

MAE_Window* CAV_WindowGtk::CreateWindow(const string& aName)
{
    CAV_WindowGtk* res = NULL;
    _FAP_ASSERT(iChilds.count(aName) == 0);
    GtkLayout *widget = GTK_LAYOUT(gtk_layout_new(NULL, NULL));
    gtk_container_set_resize_mode(GTK_CONTAINER(iWidget), GTK_RESIZE_QUEUE);
    gtk_layout_put(iWidget, GTK_WIDGET(widget), 0, 0);
    gtk_widget_realize(GTK_WIDGET(widget));
    gdk_window_set_events(widget->bin_window, GDK_ALL_EVENTS_MASK);
    GtkRequisition req;
    gtk_widget_size_request(GTK_WIDGET(widget), &req);
    GtkWidget* gw = GTK_WIDGET(widget);
    if (widget != NULL) {
	res = new CAV_WindowGtk(iView, this, aName, widget, iPContext);
	iChilds[aName] = res;
    }
    return res;
}

void CAV_WindowGtk::RemoveChild(CAV_WindowGtk* aWnd)
{
    map<string, CAV_WindowGtk*>::iterator it = iChilds.find(aWnd->Name());
    if (it != iChilds.end())
	iChilds.erase(it);
}

MAE_Window* CAV_WindowGtk::Wnd(const string& aName)
{
    return iChilds.find(aName)->second;
}

void CAV_WindowGtk::SetBg(TState aState, const CAE_Color& aColor)
{
    GdkColor color;
    CAE_ViewGtkUtils::ColorToGtk(aColor, &color);
    GtkStateType type  = CAE_ViewGtkUtils::StateToGtk(aState);
    gtk_widget_modify_bg(GTK_WIDGET(iWidget), type, &color);
}

MAE_Window::TState CAV_WindowGtk::GetState()
{
    GtkStateType gtk = gtk_widget_get_state(GTK_WIDGET(iWidget));
    return CAE_ViewGtkUtils::GtkToState(gtk);
}

void CAV_WindowGtk::SetState(TState aState)
{
    GtkStateType gtkstate = CAE_ViewGtkUtils::StateToGtk(aState);
    gtk_widget_set_state(GTK_WIDGET(iWidget), gtkstate);
}



CAV_Gc::CAV_Gc(CAV_WindowGtk* aWnd, GdkGC* aGdkGc): iWnd(aWnd), iGdkGc(aGdkGc)
{
}

GdkWindow* CAV_Gc::GdkWnd()
{
    return iWnd->iWidget->bin_window;
}

PangoContext* CAV_Gc::PContext()
{
    return iWnd->iPContext;
}

void CAV_Gc::DrawRect(CAV_Rect aRect, TBool aFilled)
{
    gdk_draw_rectangle(GdkWnd(), iGdkGc, aFilled, aRect.iTl.iX, aRect.iTl.iY, aRect.iBr.iX - aRect.iTl.iX, aRect.iBr.iY - aRect.iTl.iY);
}


CAV_Rect CAE_ViewGtkUtils::Rect(GdkRectangle aRect)
{
    return CAV_Rect(CAV_Point(aRect.x, aRect.y), CAV_Point(aRect.x + aRect.width, aRect.y + aRect.height));
}

void CAE_ViewGtkUtils::ColorToGtk(const CAE_Color& aColor, GdkColor* aGdkColor)
{
    aGdkColor->pixel = 0;
    aGdkColor->red = aColor.iRed;
    aGdkColor->green = aColor.iGreen;
    aGdkColor->blue = aColor.iBlue;
}

GtkStateType CAE_ViewGtkUtils::StateToGtk(MAE_Window::TState aState)
{
    if (aState == MAE_Window::ESt_Normal) return GTK_STATE_NORMAL;
    else if (aState == MAE_Window::ESt_Active) return GTK_STATE_ACTIVE;
    else if (aState == MAE_Window::ESt_Prelight) return GTK_STATE_PRELIGHT;
    else if (aState == MAE_Window::ESt_Selected) return GTK_STATE_SELECTED;
    else if (aState == MAE_Window::ESt_Insens) return GTK_STATE_INSENSITIVE;
}

MAE_Window::TState CAE_ViewGtkUtils::GtkToState(GtkStateType aGtkState)
{
    if (aGtkState == GTK_STATE_NORMAL) return MAE_Window::ESt_Normal;
    else if (aGtkState == GTK_STATE_ACTIVE) return MAE_Window::ESt_Active;
    else if (aGtkState == GTK_STATE_PRELIGHT) return MAE_Window::ESt_Prelight;
    else if (aGtkState == GTK_STATE_SELECTED) return MAE_Window::ESt_Selected;
    else if (aGtkState == GTK_STATE_INSENSITIVE) return MAE_Window::ESt_Insens;
}




void CAV_Gc::DrawText(const string& aText, CAV_Rect aRect)
{
    PangoLayout* lout = pango_layout_new(iWnd->iPContext);
    pango_layout_set_text(lout, aText.c_str(), aText.size());
    gdk_draw_layout(GdkWnd(), iGdkGc, aRect.iTl.iX, aRect.iTl.iY, lout);
}

void CAV_Gc::DrawLine(CAV_Point aPt1, CAV_Point aPt2)
{
    gdk_draw_line(GdkWnd(), iGdkGc, aPt1.iX, aPt1.iY, aPt2.iX, aPt2.iY);
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

