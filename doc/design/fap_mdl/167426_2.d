format 70

classinstance 128002 class_ref 203650 // MAE_Opv
  name "proxy"   xyz 210 44 2005 life_line_z 2000
classinstance 128130 class_ref 203522 // MAE_ObjCtrl
  name "object_ctrl"   xyz 333 45 2005 life_line_z 2000
classinstance 128258 class_ref 203778 // CAE_Object_v1
  name "object"   xyz 504 46 2005 life_line_z 2000
classinstance 128386 class_ref 203906 // __client
  name ""   xyz 9 44 2005 life_line_z 2000
classinstance 128770 class_ref 142082 // CAE_Env
  name ""   xyz 126 45 2005 life_line_z 2000
durationcanvas 128514 classinstance_ref 128386 // :__client
  xyzwh 32 123 2010 11 276
end
durationcanvas 128898 classinstance_ref 128770 // :CAE_Env
  xyzwh 154 145 2010 11 39
end
durationcanvas 129282 classinstance_ref 128258 // object:CAE_Object_v1
  xyzwh 566 194 2010 11 25
end
reflexivemsg 128642 synchronous
  to durationcanvas_ref 128514
  yz 123 2015 explicitmsg "__create_view_proxy"
  show_full_operations_definition default drawing_language default
  label_xy -7 107
msg 129026 synchronous
  from durationcanvas_ref 128514
  to durationcanvas_ref 128898
  yz 148 2015 msg operation_ref 185602 // "Root()"
  show_full_operations_definition default drawing_language default
  label_xy 70 132
msg 129154 return
  from durationcanvas_ref 128898
  to durationcanvas_ref 128514
  yz 173 2015 explicitmsg "__root_system"
  show_full_operations_definition default drawing_language default
  label_xy 47 157
msg 129410 synchronous
  from durationcanvas_ref 128514
  to durationcanvas_ref 129282
  yz 196 2015 msg operation_ref 185474 // "SetBaseViewProxy()"
  show_full_operations_definition default drawing_language default
  label_xy 248 180
end
