/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * GtkAccelGroup: Accelerator manager for GtkObjects.
 * Copyright (C) 1998 Tim Janik
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


/* <gtk/gtkaccelgroup.h>  Mask values, may be OR'ed together */
typedef enum
{
 GTK_ACCEL_VISIBLE, GTK_ACCEL_SIGNAL_VISIBLE, GTK_ACCEL_LOCKED, GTK_ACCEL_MASK
} GtkAccelFlags;

/* <gtk/gtkaccelgroup.h> */
typedef struct _GtkAccelGroup
{
	guint ref_count;
	guint lock_count;
	GdkModifierType modifier_mask;
	GSList *attach_objects;
} GtkAccelGroup;

/* <gtk/gtkaccelgroup.h> */
typedef struct _GtkAccelEntry
{
	GtkAccelGroup *accel_group;
	guint accelerator_key;
	GdkModifierType accelerator_mods;
	 
	GtkAccelFlags accel_flags;
	GtkObject *object;
	guint signal_id;
} GtkAccelEntry;

/* <gtk/gtkaccelgroup.h> */
gboolean gtk_accelerator_valid(guint keyval, GdkModifierType modifiers)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accelerator_parse(const gchar *accelerator, guint *accelerator_key, GdkModifierType *accelerator_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
gchar* gtk_accelerator_name(guint accelerator_key, GdkModifierType accelerator_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accelerator_set_default_mod_mask(GdkModifierType default_mod_mask)
{
}

/* <gtk/gtkaccelgroup.h> */
guint gtk_accelerator_get_default_mod_mask(void)
{
}

/* <gtk/gtkaccelgroup.h> */
GtkAccelGroup* gtk_accel_group_new(void)
{
}

/* <gtk/gtkaccelgroup.h> */
GtkAccelGroup* gtk_accel_group_get_default(void)
{
}

/* <gtk/gtkaccelgroup.h> */
GtkAccelGroup* gtk_accel_group_ref(GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_unref(GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_lock(GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_unlock(GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkaccelgroup.h> */
gboolean gtk_accel_groups_activate(GtkObject *object, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
gboolean gtk_accel_group_activate(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_attach(GtkAccelGroup *accel_group, GtkObject *object)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_detach(GtkAccelGroup *accel_group, GtkObject *object)
{
}

/* <gtk/gtkaccelgroup.h> */
GtkAccelEntry* gtk_accel_group_get_entry(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_lock_entry(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_unlock_entry(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_add(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods, GtkAccelFlags accel_flags, GtkObject *object, const gchar *accel_signal)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_remove(GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods, GtkObject *object)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_handle_add(GtkObject *object, guint accel_signal_id, GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods, GtkAccelFlags accel_flags)
{
}

/* <gtk/gtkaccelgroup.h> */
void gtk_accel_group_handle_remove(GtkObject *object, GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods)
{
}

/* <gtk/gtkaccelgroup.h> */
guint gtk_accel_group_create_add(GtkType class_type, GtkSignalRunType signal_flags, guint handler_offset)
{
}

/* <gtk/gtkaccelgroup.h> */
guint gtk_accel_group_create_remove(GtkType class_type, GtkSignalRunType signal_flags, guint handler_offset)
{
}

/* <gtk/gtkaccelgroup.h> */
GSList* gtk_accel_groups_from_object(GtkObject *object)
{
}

/* <gtk/gtkaccelgroup.h> */
GSList* gtk_accel_group_entries_from_object(GtkObject *object)
{
}

/* <gtk/gtkaccellabel.h> */
#define GTK_TYPE_ACCEL_LABEL (gtk_accel_label_get_type ())

/* <gtk/gtkaccellabel.h> */
#define GTK_ACCEL_LABEL(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ACCEL_LABEL, GtkAccelLabel))

/* <gtk/gtkaccellabel.h> */
#define GTK_ACCEL_LABEL_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ACCEL_LABEL, GtkAccelLabelClass))

/* <gtk/gtkaccellabel.h> */
#define GTK_IS_ACCEL_LABEL(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ACCEL_LABEL))

/* <gtk/gtkaccellabel.h> */
#define GTK_IS_ACCEL_LABEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ACCEL_LABEL))

/* <gtk/gtkaccellabel.h> */
typedef struct _GtkAccelLabel
{
	GtkLabel label;

	guint queue_id;
	guint accel_padding;
	GtkWidget *accel_widget;
	gchar *accel_string;
	guint16 accel_string_width;
} GtkAccelLabel;

/* <gtk/gtkaccellabel.h> */
typedef struct _GtkAccelLabelClass
{
	GtkLabelClass parent_class;

	gchar *signal_quote1;
	gchar *signal_quote2;
	gchar *mod_name_shift;
	gchar *mod_name_control;
	gchar *mod_name_alt;
	gchar *mod_separator;
	gchar *accel_seperator;
	guint latin1_to_char : 1;
} GtkAccelLabelClass;


/* <gtk/gtkaccellabel.h> */
GtkType gtk_accel_label_get_type(void)
{
}

/* <gtk/gtkaccellabel.h> */
GtkWidget* gtk_accel_label_new(const gchar *string)
{
}

/* <gtk/gtkaccellabel.h> */
guint gtk_accel_label_get_accel_width(GtkAccelLabel *accel_label)
{
}

/* <gtk/gtkaccellabel.h> */
void gtk_accel_label_set_accel_widget(GtkAccelLabel *accel_label, GtkWidget *accel_widget)
{
}

/* <gtk/gtkaccellabel.h> */
gboolean gtk_accel_label_refetch(GtkAccelLabel *accel_label)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkadjustment.h
::::::::::::::

/* <gtk/gtkadjustment.h> */
#define GTK_TYPE_ADJUSTMENT (gtk_adjustment_get_type ())

/* <gtk/gtkadjustment.h> */
#define GTK_ADJUSTMENT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ADJUSTMENT, GtkAdjustment))

/* <gtk/gtkadjustment.h> */
#define GTK_ADJUSTMENT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ADJUSTMENT, GtkAdjustmentClass))

/* <gtk/gtkadjustment.h> */
#define GTK_IS_ADJUSTMENT(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ADJUSTMENT))

/* <gtk/gtkadjustment.h> */
#define GTK_IS_ADJUSTMENT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ADJUSTMENT))



/* <gtk/gtkadjustment.h> */
typedef struct _GtkAdjustment
{
	GtkData data;
	 
	gfloat lower;
	gfloat upper;
	gfloat value;
	gfloat step_increment;
	gfloat page_increment;
	gfloat page_size;
} GtkAdjustment;


/* <gtk/gtkadjustment.h> */
typedef struct _GtkAdjustmentClass
{
	GtkDataClass parent_class;
	void (* changed) (GtkAdjustment *adjustment);
	void (* value_changed) (GtkAdjustment *adjustment);
} GtkAdjustmentClass;



/* <gtk/gtkadjustment.h> */
GtkType gtk_adjustment_get_type(void)
{
}

/* <gtk/gtkadjustment.h> */
GtkObject* gtk_adjustment_new(gfloat value, gfloat lower, gfloat upper, gfloat step_increment, gfloat page_increment, gfloat page_size)
{
}

/* <gtk/gtkadjustment.h> */
void gtk_adjustment_changed(GtkAdjustment *adjustment)
{
}

/* <gtk/gtkadjustment.h> */
void gtk_adjustment_value_changed(GtkAdjustment *adjustment)
{
}

/* <gtk/gtkadjustment.h> */
void gtk_adjustment_clamp_page(GtkAdjustment *adjustment, gfloat lower, gfloat upper)
{
}

/* <gtk/gtkadjustment.h> */
void gtk_adjustment_set_value(GtkAdjustment *adjustment, gfloat value)
{
}
::::::::::::::
/usr/include/gtk-1.2/gtk/gtkalignment.h
::::::::::::::

/* <gtk/gtkalignment.h> */
#define GTK_TYPE_ALIGNMENT (gtk_alignment_get_type ())

/* <gtk/gtkalignment.h> */
#define GTK_ALIGNMENT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ALIGNMENT, GtkAlignment))

/* <gtk/gtkalignment.h> */
#define GTK_ALIGNMENT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ALIGNMENT, GtkAlignmentClass))

/* <gtk/gtkalignment.h> */
#define GTK_IS_ALIGNMENT(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ALIGNMENT))

/* <gtk/gtkalignment.h> */
#define GTK_IS_ALIGNMENT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ALIGNMENT))



/* <gtk/gtkalignment.h> */
typedef struct _GtkAlignment
{
	GtkBin bin;

	gfloat xalign;
	gfloat yalign;
	gfloat xscale;
	gfloat yscale;
} GtkAlignment;


/* <gtk/gtkalignment.h> */
typedef struct _GtkAlignmentClass
{
	GtkBinClass parent_class;
} GtkAlignmentClass;



/* <gtk/gtkalignment.h> */
GtkType gtk_alignment_get_type(void)
{
}

/* <gtk/gtkalignment.h> */
GtkWidget* gtk_alignment_new(gfloat xalign, gfloat yalign, gfloat xscale, gfloat yscale)
{
}

/* <gtk/gtkalignment.h> */
void gtk_alignment_set(GtkAlignment *alignment, gfloat xalign, gfloat yalign, gfloat xscale, gfloat yscale)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkarg.h
::::::::::::::


/* <gtk/gtkarg.h> */
typedef struct _GtkArgInfo
{
	GtkType class_type;
	gchar *name;
	 
	GtkType type;
	guint arg_flags;
	gchar *full_name;
	 
	 /* private fields */
	guint arg_id;
	guint seq_id;
} GtkArgInfo;




/* <gtk/gtkarg.h> */
GtkArg* gtk_arg_new(GtkType arg_type)
{
}

/* <gtk/gtkarg.h> */

/* <gtk/gtkarg.h> */
GtkArg* gtk_arg_copy(GtkArg *src_arg, GtkArg *dest_arg)
{
}

/* <gtk/gtkarg.h> */
void gtk_arg_free(GtkArg *arg, gboolean free_contents)
{
}

/* <gtk/gtkarg.h> */
void gtk_arg_reset(GtkArg *arg)
{
}

/* <gtk/gtkarg.h> */
gboolean gtk_arg_values_equal(const GtkArg *arg1, const GtkArg *arg2)
{
}

/* <gtk/gtkarg.h> */
gchar* gtk_args_collect(GtkType object_type, GHashTable *arg_info_hash_table, GSList **arg_list_p, GSList **info_list_p, const gchar *first_arg_name, va_list var_args)
{
}

/* <gtk/gtkarg.h> */
void gtk_args_collect_cleanup(GSList *arg_list, GSList *info_list)
{
}

/* <gtk/gtkarg.h> */
gchar* gtk_arg_get_info(GtkType object_type, GHashTable *arg_info_hash_table, const gchar *arg_name, GtkArgInfo **info_p)
{
}

/* <gtk/gtkarg.h> */
GtkArgInfo* gtk_arg_type_new_static(GtkType base_class_type, const gchar *arg_name, guint class_n_args_offset, GHashTable *arg_info_hash_table, GtkType arg_type, guint arg_flags, guint arg_id)
{
}

/* <gtk/gtkarg.h> */
GtkArg* gtk_args_query(GtkType class_type, GHashTable *arg_info_hash_table, guint32 **arg_flags, guint *n_args_p)
{
}

/* <gtk/gtkarg.h> */
gchar* gtk_arg_name_strip_type(const gchar *arg_name)
{
}

/* <gtk/gtkarg.h> */
gint gtk_arg_info_equal(gconstpointer arg_info_1, gconstpointer arg_info_2)
{
}

/* <gtk/gtkarg.h> */
guint gtk_arg_info_hash(gconstpointer arg_info)
{
}

/* <gtk/gtkarg.h> */
void gtk_arg_to_valueloc(GtkArg *arg, gpointer value_pointer)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkarrow.h
::::::::::::::


/* <gtk/gtkarrow.h> */
#define GTK_TYPE_ARROW (gtk_arrow_get_type ())

/* <gtk/gtkarrow.h> */
#define GTK_ARROW(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ARROW, GtkArrow))

/* <gtk/gtkarrow.h> */
#define GTK_ARROW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ARROW, GtkArrowClass))

/* <gtk/gtkarrow.h> */
#define GTK_IS_ARROW(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ARROW))

/* <gtk/gtkarrow.h> */
#define GTK_IS_ARROW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ARROW))



/* <gtk/gtkarrow.h> */
typedef struct _GtkArrow
{
	GtkMisc misc;

	gint16 arrow_type;
	gint16 shadow_type;
} GtkArrow;


/* <gtk/gtkarrow.h> */
typedef struct _GtkArrowClass
{
	GtkMiscClass parent_class;
} GtkArrowClass;



/* <gtk/gtkarrow.h> */
GtkType gtk_arrow_get_type(void)
{
}

/* <gtk/gtkarrow.h> */
GtkWidget* gtk_arrow_new(GtkArrowType arrow_type, GtkShadowType shadow_type)
{
}

/* <gtk/gtkarrow.h> */
void gtk_arrow_set(GtkArrow *arrow, GtkArrowType arrow_type, GtkShadowType shadow_type)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkaspectframe.h
::::::::::::::


/* <gtk/gtkaspectframe.h> */
#define GTK_TYPE_ASPECT_FRAME (gtk_aspect_frame_get_type ())

/* <gtk/gtkaspectframe.h> */
#define GTK_ASPECT_FRAME(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ASPECT_FRAME, GtkAspectFrame))

/* <gtk/gtkaspectframe.h> */
#define GTK_ASPECT_FRAME_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ASPECT_FRAME, GtkAspectFrameClass))

/* <gtk/gtkaspectframe.h> */
#define GTK_IS_ASPECT_FRAME(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ASPECT_FRAME))

/* <gtk/gtkaspectframe.h> */
#define GTK_IS_ASPECT_FRAME_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ASPECT_FRAME))




/* <gtk/gtkaspectframe.h> */
typedef struct _GtkAspectFrame
{
	GtkFrame frame;

	gfloat xalign;
	gfloat yalign;
	gfloat ratio;
	gboolean obey_child;

	GtkAllocation center_allocation;
} GtkAspectFrame;


/* <gtk/gtkaspectframe.h> */
typedef struct _GtkAspectFrameClass
{
	GtkBinClass parent_class;
} GtkAspectFrameClass;



/* <gtk/gtkaspectframe.h> */
GtkType gtk_aspect_frame_get_type(void)
{
}

/* <gtk/gtkaspectframe.h> */
GtkWidget* gtk_aspect_frame_new(const gchar *label, gfloat xalign, gfloat yalign, gfloat ratio, gboolean obey_child)
{
}

/* <gtk/gtkaspectframe.h> */
void gtk_aspect_frame_set(GtkAspectFrame *aspect_frame, gfloat xalign, gfloat yalign, gfloat ratio, gboolean obey_child)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkbbox.h
::::::::::::::


/* <gtk/gtkbbox.h> */
#define GTK_TYPE_BUTTON_BOX (gtk_button_box_get_type ())

/* <gtk/gtkbbox.h> */
#define GTK_BUTTON_BOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_BUTTON_BOX, GtkButtonBox))

/* <gtk/gtkbbox.h> */
#define GTK_BUTTON_BOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_BUTTON_BOX, GtkButtonBoxClass))

/* <gtk/gtkbbox.h> */
#define GTK_IS_BUTTON_BOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_BUTTON_BOX))

/* <gtk/gtkbbox.h> */
#define GTK_IS_BUTTON_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_BUTTON_BOX))
	 


/* <gtk/gtkbbox.h> */
#define GTK_BUTTONBOX_DEFAULT -1
	

/* <gtk/gtkbbox.h> */
typedef struct _GtkButtonBox
{
	GtkBox box;
	gint spacing;
	gint child_min_width;
	gint child_min_height;
	gint child_ipad_x;
	gint child_ipad_y;
	GtkButtonBoxStyle layout_style;
} GtkButtonBox;


/* <gtk/gtkbbox.h> */
typedef struct _GtkButtonBoxClass
{
	GtkBoxClass parent_class;
} GtkButtonBoxClass;



/* <gtk/gtkbbox.h> */
GtkType gtk_button_box_get_type(void)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_get_child_size_default(gint *min_width, gint *min_height)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_get_child_ipadding_default(gint *ipad_x, gint *ipad_y)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_child_size_default(gint min_width, gint min_height)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_child_ipadding_default(gint ipad_x, gint ipad_y)
{
}

/* <gtk/gtkbbox.h> */
gint gtk_button_box_get_spacing(GtkButtonBox *widget)
{
}

/* <gtk/gtkbbox.h> */
GtkButtonBoxStyle gtk_button_box_get_layout(GtkButtonBox *widget)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_get_child_size(GtkButtonBox *widget, gint *min_width, gint *min_height)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_get_child_ipadding(GtkButtonBox *widget, gint *ipad_x, gint *ipad_y)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_spacing(GtkButtonBox *widget, gint spacing)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_layout(GtkButtonBox *widget, GtkButtonBoxStyle layout_style)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_child_size(GtkButtonBox *widget, gint min_width, gint min_height)
{
}

/* <gtk/gtkbbox.h> */
void gtk_button_box_set_child_ipadding(GtkButtonBox *widget, gint ipad_x, gint ipad_y)
{
}


::::::::::::::
/usr/include/gtk-1.2/gtk/gtkbindings.h
::::::::::::::


/* <gtk/gtkbindings.h> */
typedef struct _GtkPatternSpec
{
	GtkMatchType match_type;
	guint pattern_length;
	gchar *pattern;
	gchar *pattern_reversed;
	gpointer user_data;
	guint seq_id;
} GtkPatternSpec;


/* <gtk/gtkbindings.h> */
void gtk_pattern_spec_init(GtkPatternSpec *pspec, const gchar *pattern)
{
}

/* <gtk/gtkbindings.h> */
void gtk_pattern_spec_free_segs(GtkPatternSpec *pspec)
{
}

/* <gtk/gtkbindings.h> */
gboolean gtk_pattern_match(GtkPatternSpec *pspec, guint string_length, const gchar *string, const gchar *string_reversed)
{
}

/* <gtk/gtkbindings.h> */
gboolean gtk_pattern_match_string(GtkPatternSpec *pspec, const gchar *string)
{
}

/* <gtk/gtkbindings.h> */
gboolean gtk_pattern_match_simple(const gchar *pattern, const gchar *string)
{
}


/* Binding sets
 */


/* <gtk/gtkbindings.h> */
typedef struct _GtkBindingSet
{
	gchar *set_name;
	gint priority;
	GSList *widget_path_pspecs;
	GSList *widget_class_pspecs;
	GSList *class_branch_pspecs;
	GtkBindingEntry *entries;
	GtkBindingEntry *current;
} GtkBindingSet;


/* <gtk/gtkbindings.h> */
typedef struct _GtkBindingEntry
{
	 /* key portion
 */
	guint keyval;
	guint modifiers;
	 
	GtkBindingSet *binding_set;
	guint destroyed : 1;
	guint in_emission : 1;
	GtkBindingEntry *set_next;
	GtkBindingEntry *hash_next;
	GtkBindingSignal *signals;
} GtkBindingEntry;


/* <gtk/gtkbindings.h> */
typedef struct _GtkBindingSignal
{
	GtkBindingSignal *next;
	gchar *signal_name;
	guint n_args;
	GtkBindingArg *args;
} GtkBindingSignal;


/* <gtk/gtkbindings.h> */
typedef struct _GtkBindingArg
{
	GtkType arg_type;
	union {
	glong long_data;
	gdouble double_data;
	gchar *string_data;
	} d;
} GtkBindingArg;


/* Application-level methods */


/* <gtk/gtkbindings.h> */
GtkBindingSet* gtk_binding_set_new(const gchar *set_name)
{
}

/* <gtk/gtkbindings.h> */
GtkBindingSet* gtk_binding_set_by_class(gpointer object_class)
{
}

/* <gtk/gtkbindings.h> */
GtkBindingSet* gtk_binding_set_find(const gchar *set_name)
{
}

/* <gtk/gtkbindings.h> */
gboolean gtk_bindings_activate(GtkObject *object, guint keyval, guint modifiers)
{
}

/* <gtk/gtkbindings.h> */
gboolean gtk_binding_set_activate(GtkBindingSet *binding_set, guint keyval, guint modifiers, GtkObject *object)
{
}

/* <gtk/gtkbindings.h> */
#define gtk_binding_entry_add gtk_binding_entry_clear

/* <gtk/gtkbindings.h> */
void gtk_binding_entry_clear(GtkBindingSet *binding_set, guint keyval, guint modifiers)
{
}

/* <gtk/gtkbindings.h> */
void gtk_binding_entry_add_signal(GtkBindingSet *binding_set, guint keyval, guint modifiers, const gchar *signal_name, guint n_args, ...)
{
}

/* <gtk/gtkbindings.h> */
void gtk_binding_set_add_path(GtkBindingSet *binding_set, GtkPathType path_type, const gchar *path_pattern, GtkPathPriorityType priority)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkbin.h
::::::::::::::


/* <gtk/gtkbin.h> */
#define GTK_TYPE_BIN (gtk_bin_get_type ())

/* <gtk/gtkbin.h> */
#define GTK_BIN(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_BIN, GtkBin))

/* <gtk/gtkbin.h> */
#define GTK_BIN_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_BIN, GtkBinClass))

/* <gtk/gtkbin.h> */
#define GTK_IS_BIN(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_BIN))

/* <gtk/gtkbin.h> */
#define GTK_IS_BIN_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_BIN))




/* <gtk/gtkbin.h> */
typedef struct _GtkBin
{
	GtkContainer container;

	GtkWidget *child;
} GtkBin;


/* <gtk/gtkbin.h> */
typedef struct _GtkBinClass
{
	GtkContainerClass parent_class;
} GtkBinClass;



/* <gtk/gtkbin.h> */
GtkType gtk_bin_get_type(void)
{
}


::::::::::::::
/usr/include/gtk-1.2/gtk/gtkbox.h
::::::::::::::

/* <gtk/gtkbox.h> */
#define GTK_TYPE_BOX (gtk_box_get_type ())

/* <gtk/gtkbox.h> */
#define GTK_BOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_BOX, GtkBox))

/* <gtk/gtkbox.h> */
#define GTK_BOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_BOX, GtkBoxClass))

/* <gtk/gtkbox.h> */
#define GTK_IS_BOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_BOX))

/* <gtk/gtkbox.h> */
#define GTK_IS_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_BOX))



/* <gtk/gtkbox.h> */
typedef struct _GtkBox
{
	GtkContainer container;
	 
	GList *children;
	gint16 spacing;
	guint homogeneous : 1;
} GtkBox;


/* <gtk/gtkbox.h> */
typedef struct _GtkBoxClass
{
	GtkContainerClass parent_class;
} GtkBoxClass;


/* <gtk/gtkbox.h> */
typedef struct _GtkBoxChild
{
	GtkWidget *widget;
	guint16 padding;
	guint expand : 1;
	guint fill : 1;
	guint pack : 1;
} GtkBoxChild;



/* <gtk/gtkbox.h> */
GtkType gtk_box_get_type(void)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_pack_start(GtkBox *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_pack_end(GtkBox *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_pack_start_defaults(GtkBox *box, GtkWidget *widget)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_pack_end_defaults(GtkBox *box, GtkWidget *widget)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_set_homogeneous(GtkBox *box, gboolean homogeneous)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_set_spacing(GtkBox *box, gint spacing)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_reorder_child(GtkBox *box, GtkWidget *child, gint position)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_query_child_packing(GtkBox *box, GtkWidget *child, gboolean *expand, gboolean *fill, guint *padding, GtkPackType *pack_type)
{
}

/* <gtk/gtkbox.h> */
void gtk_box_set_child_packing(GtkBox *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding, GtkPackType pack_type)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkbutton.h
::::::::::::::


/* <gtk/gtkbutton.h> */
#define GTK_TYPE_BUTTON (gtk_button_get_type ())

/* <gtk/gtkbutton.h> */
#define GTK_BUTTON(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_BUTTON, GtkButton))

/* <gtk/gtkbutton.h> */
#define GTK_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_BUTTON, GtkButtonClass))

/* <gtk/gtkbutton.h> */
#define GTK_IS_BUTTON(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_BUTTON))

/* <gtk/gtkbutton.h> */
#define GTK_IS_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_BUTTON))



/* <gtk/gtkbutton.h> */
typedef struct _GtkButton
{
	GtkBin bin;

	GtkWidget *child /* deprecapted field,
 * use GTK_BIN (button)->child instead
 */;

	guint in_button : 1;
	guint button_down : 1;
	guint relief : 2;
} GtkButton;


/* <gtk/gtkbutton.h> */
typedef struct _GtkButtonClass
{
	GtkBinClass parent_class;
	 
	void (* pressed) (GtkButton *button);
	void (* released) (GtkButton *button);
	void (* clicked) (GtkButton *button);
	void (* enter) (GtkButton *button);
	void (* leave) (GtkButton *button);
} GtkButtonClass;



/* <gtk/gtkbutton.h> */
GtkType gtk_button_get_type(void)
{
}

/* <gtk/gtkbutton.h> */
GtkWidget* gtk_button_new(void)
{
}

/* <gtk/gtkbutton.h> */
GtkWidget* gtk_button_new_with_label(const gchar *label)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_pressed(GtkButton *button)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_released(GtkButton *button)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_clicked(GtkButton *button)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_enter(GtkButton *button)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_leave(GtkButton *button)
{
}

/* <gtk/gtkbutton.h> */
void gtk_button_set_relief(GtkButton *button, GtkReliefStyle newstyle)
{
}

/* <gtk/gtkbutton.h> */
GtkReliefStyle gtk_button_get_relief(GtkButton *button)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkcalendar.h
::::::::::::::

/* <gtk/gtkcalendar.h> */
#define GTK_TYPE_CALENDAR (gtk_calendar_get_type ())

/* <gtk/gtkcalendar.h> */
#define GTK_CALENDAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CALENDAR, GtkCalendar))

/* <gtk/gtkcalendar.h> */
#define GTK_CALENDAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CALENDAR, GtkCalendarClass))

/* <gtk/gtkcalendar.h> */
#define GTK_IS_CALENDAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CALENDAR))

/* <gtk/gtkcalendar.h> */
#define GTK_IS_CALENDAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CALENDAR))



/* <gtk/gtkcalendar.h> */
typedef enum
{
	GTK_CALENDAR_SHOW_HEADING = 1 << 0,
	GTK_CALENDAR_SHOW_DAY_NAMES = 1 << 1,
	GTK_CALENDAR_NO_MONTH_CHANGE = 1 << 2,
	GTK_CALENDAR_SHOW_WEEK_NUMBERS = 1 << 3,
	GTK_CALENDAR_WEEK_START_MONDAY = 1 << 4
} GtkCalendarDisplayOptions;


/* <gtk/gtkcalendar.h> */
typedef struct _GtkCalendar
{
	GtkWidget widget;
	 
	GtkStyle *header_style;
	GtkStyle *label_style;
	 
	gint month;
	gint year;
	gint selected_day;
	 
	gint day_month[6][7];
	gint day[6][7];
	 
	gint num_marked_dates;
	gint marked_date[31];
	GtkCalendarDisplayOptions display_flags;
	GdkColor marked_date_color[31];
	 
	GdkGC *gc;
	GdkGC *xor_gc;

	gint focus_row;
	gint focus_col;

	gint highlight_row;
	gint highlight_col;
	 
	gpointer private_data;
	gchar grow_space [32];
} GtkCalendar;


/* <gtk/gtkcalendar.h> */
typedef struct _GtkCalendarClass
{
	GtkWidgetClass parent_class;
	 
	 /* Signal handlers */
	void (* month_changed) (GtkCalendar *calendar);
	void (* day_selected) (GtkCalendar *calendar);
	void (* day_selected_double_click) (GtkCalendar *calendar);
	void (* prev_month) (GtkCalendar *calendar);
	void (* next_month) (GtkCalendar *calendar);
	void (* prev_year) (GtkCalendar *calendar);
	void (* next_year) (GtkCalendar *calendar);
	 
} GtkCalendarClass;



/* <gtk/gtkcalendar.h> */
GtkType gtk_calendar_get_type(void)
{
}

/* <gtk/gtkcalendar.h> */
GtkWidget* gtk_calendar_new(void)
{
}


/* <gtk/gtkcalendar.h> */
gint gtk_calendar_select_month(GtkCalendar *calendar, guint month, guint year)
{
}

/* <gtk/gtkcalendar.h> */
void gtk_calendar_select_day(GtkCalendar *calendar, guint day)
{
}


/* <gtk/gtkcalendar.h> */
gint gtk_calendar_mark_day(GtkCalendar *calendar, guint day)
{
}

/* <gtk/gtkcalendar.h> */
gint gtk_calendar_unmark_day(GtkCalendar *calendar, guint day)
{
}

/* <gtk/gtkcalendar.h> */
void gtk_calendar_clear_marks(GtkCalendar *calendar)
{
}



/* <gtk/gtkcalendar.h> */
void gtk_calendar_display_options(GtkCalendar *calendar, GtkCalendarDisplayOptions flags)
{
}


/* <gtk/gtkcalendar.h> */
void gtk_calendar_get_date(GtkCalendar *calendar, guint *year, guint *month, guint *day)
{
}

/* <gtk/gtkcalendar.h> */
void gtk_calendar_freeze(GtkCalendar *calendar)
{
}

/* <gtk/gtkcalendar.h> */
void gtk_calendar_thaw(GtkCalendar *calendar)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkcheckbutton.h
::::::::::::::

/* <gtk/gtkcheckbutton.h> */
#define GTK_TYPE_CHECK_BUTTON (gtk_check_button_get_type ())

/* <gtk/gtkcheckbutton.h> */
#define GTK_CHECK_BUTTON(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CHECK_BUTTON, GtkCheckButton))

/* <gtk/gtkcheckbutton.h> */
#define GTK_CHECK_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHECK_BUTTON, GtkCheckButtonClass))

/* <gtk/gtkcheckbutton.h> */
#define GTK_IS_CHECK_BUTTON(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CHECK_BUTTON))

/* <gtk/gtkcheckbutton.h> */
#define GTK_IS_CHECK_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHECK_BUTTON))




/* <gtk/gtkcheckbutton.h> */
typedef struct _GtkCheckButton
{
	GtkToggleButton toggle_button;
} GtkCheckButton;


/* <gtk/gtkcheckbutton.h> */
typedef struct _GtkCheckButtonClass
{
	GtkToggleButtonClass parent_class;

	guint16 indicator_size;
	guint16 indicator_spacing;

	void (* draw_indicator) (GtkCheckButton *check_button, GdkRectangle *area);
} GtkCheckButtonClass;



/* <gtk/gtkcheckbutton.h> */
GtkType gtk_check_button_get_type(void)
{
}

/* <gtk/gtkcheckbutton.h> */
GtkWidget* gtk_check_button_new(void)
{
}

/* <gtk/gtkcheckbutton.h> */
GtkWidget* gtk_check_button_new_with_label(const gchar *label)
{
}


/* <gtk/gtkcheckbutton.h> */
void _gtk_check_button_get_props(GtkCheckButton *check_button, gint *indicator_size, gint *indicator_spacing)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkcheckmenuitem.h
::::::::::::::

/* <gtk/gtkcheckmenuitem.h> */
#define GTK_TYPE_CHECK_MENU_ITEM (gtk_check_menu_item_get_type ())

/* <gtk/gtkcheckmenuitem.h> */
#define GTK_CHECK_MENU_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CHECK_MENU_ITEM, GtkCheckMenuItem))

/* <gtk/gtkcheckmenuitem.h> */
#define GTK_CHECK_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CHECK_MENU_ITEM, GtkCheckMenuItemClass))

/* <gtk/gtkcheckmenuitem.h> */
#define GTK_IS_CHECK_MENU_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CHECK_MENU_ITEM))

/* <gtk/gtkcheckmenuitem.h> */
#define GTK_IS_CHECK_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CHECK_MENU_ITEM))



/* <gtk/gtkcheckmenuitem.h> */
typedef struct _GtkCheckMenuItem
{
	GtkMenuItem menu_item;
	 
	guint active : 1;
	guint always_show_toggle : 1;
} GtkCheckMenuItem;


/* <gtk/gtkcheckmenuitem.h> */
typedef struct _GtkCheckMenuItemClass
{
	GtkMenuItemClass parent_class;
	 
	void (* toggled) (GtkCheckMenuItem *check_menu_item);
	void (* draw_indicator) (GtkCheckMenuItem *check_menu_item, GdkRectangle *area);
} GtkCheckMenuItemClass;



/* <gtk/gtkcheckmenuitem.h> */
GtkType gtk_check_menu_item_get_type(void)
{
}

/* <gtk/gtkcheckmenuitem.h> */
GtkWidget* gtk_check_menu_item_new(void)
{
}

/* <gtk/gtkcheckmenuitem.h> */
GtkWidget* gtk_check_menu_item_new_with_label(const gchar *label)
{
}

/* <gtk/gtkcheckmenuitem.h> */
void gtk_check_menu_item_set_active(GtkCheckMenuItem *check_menu_item, gboolean is_active)
{
}

/* <gtk/gtkcheckmenuitem.h> */
void gtk_check_menu_item_set_show_toggle(GtkCheckMenuItem *menu_item, gboolean always)
{
}

/* <gtk/gtkcheckmenuitem.h> */
void gtk_check_menu_item_toggled(GtkCheckMenuItem *check_menu_item)
{
}

::::::::::::::
/usr/include/gtk-1.2/gtk/gtkclist.h
::::::::::::::

enum {
	GTK_CLIST_IN_DRAG = 1 << 0,
	GTK_CLIST_ROW_HEIGHT_SET = 1 << 1,
	GTK_CLIST_SHOW_TITLES = 1 << 2,
	GTK_CLIST_CHILD_HAS_FOCUS = 1 << 3,
	GTK_CLIST_ADD_MODE = 1 << 4,
	GTK_CLIST_AUTO_SORT = 1 << 5,
	GTK_CLIST_AUTO_RESIZE_BLOCKED = 1 << 6,
	GTK_CLIST_REORDERABLE = 1 << 7,
	GTK_CLIST_USE_DRAG_ICONS = 1 << 8,
	GTK_CLIST_DRAW_DRAG_LINE = 1 << 9,
	GTK_CLIST_DRAW_DRAG_RECT = 1 << 10
}; 

/* cell types */

/* <gtk/gtkclist.h> */
typedef enum
{
	GTK_CELL_EMPTY,
	GTK_CELL_TEXT,
	GTK_CELL_PIXMAP,
	GTK_CELL_PIXTEXT,
	GTK_CELL_WIDGET
} GtkCellType;


/* <gtk/gtkclist.h> */
typedef enum
{
	GTK_CLIST_DRAG_NONE,
	GTK_CLIST_DRAG_BEFORE,
	GTK_CLIST_DRAG_INTO,
	GTK_CLIST_DRAG_AFTER
} GtkCListDragPos;


/* <gtk/gtkclist.h> */
typedef enum
{
	GTK_BUTTON_IGNORED = 0,
	GTK_BUTTON_SELECTS = 1 << 0,
	GTK_BUTTON_DRAGS = 1 << 1,
	GTK_BUTTON_EXPANDS = 1 << 2
} GtkButtonAction;


/* <gtk/gtkclist.h> */
#define GTK_TYPE_CLIST (gtk_clist_get_type ())

/* <gtk/gtkclist.h> */
#define GTK_CLIST(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CLIST, GtkCList))

/* <gtk/gtkclist.h> */
#define GTK_CLIST_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CLIST, GtkCListClass))

/* <gtk/gtkclist.h> */
#define GTK_IS_CLIST(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CLIST))

/* <gtk/gtkclist.h> */
#define GTK_IS_CLIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CLIST))


/* <gtk/gtkclist.h> */
#define GTK_CLIST_FLAGS(clist) (GTK_CLIST (clist)->flags)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_SET_FLAG(clist,flag) (GTK_CLIST_FLAGS (clist) |= (GTK_ ## flag))

/* <gtk/gtkclist.h> */
#define GTK_CLIST_UNSET_FLAG(clist,flag) (GTK_CLIST_FLAGS (clist) &= ~(GTK_ ## flag))


/* <gtk/gtkclist.h> */
#define GTK_CLIST_IN_DRAG(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_IN_DRAG)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_ROW_HEIGHT_SET(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_ROW_HEIGHT_SET)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_SHOW_TITLES(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_SHOW_TITLES)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_CHILD_HAS_FOCUS(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_CHILD_HAS_FOCUS)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_ADD_MODE(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_ADD_MODE)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_AUTO_SORT(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_AUTO_SORT)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_AUTO_RESIZE_BLOCKED(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_AUTO_RESIZE_BLOCKED)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_REORDERABLE(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_REORDERABLE)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_USE_DRAG_ICONS(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_USE_DRAG_ICONS)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_DRAW_DRAG_LINE(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_DRAW_DRAG_LINE)

/* <gtk/gtkclist.h> */
#define GTK_CLIST_DRAW_DRAG_RECT(clist) (GTK_CLIST_FLAGS (clist) & GTK_CLIST_DRAW_DRAG_RECT)


/* <gtk/gtkclist.h> */
#define GTK_CLIST_ROW(_glist_) ((GtkCListRow *)((_glist_)->data))

/* pointer casting for cells */

/* <gtk/gtkclist.h> */
#define GTK_CELL_TEXT(cell) (((GtkCellText *) &(cell)))

/* <gtk/gtkclist.h> */
#define GTK_CELL_PIXMAP(cell) (((GtkCellPixmap *) &(cell)))

/* <gtk/gtkclist.h> */
#define GTK_CELL_PIXTEXT(cell) (((GtkCellPixText *) &(cell)))

/* <gtk/gtkclist.h> */
#define GTK_CELL_WIDGET(cell) (((GtkCellWidget *) &(cell)))


/* <gtk/gtkclist.h> */
typedef gint(*GtkCListCompareFunc) (GtkCList *clist, gconstpointer ptr1, gconstpointer ptr2)
{
}


/* <gtk/gtkclist.h> */
typedef struct _GtkCListCellInfo
{
	gint row;
	gint column;
} GtkCListCellInfo;


/* <gtk/gtkclist.h> */
typedef struct _GtkCListDestInfo
{
	GtkCListCellInfo cell;
	GtkCListDragPos insert_pos;
} GtkCListDestInfo;


/* <gtk/gtkclist.h> */
typedef struct _GtkCList
{
	GtkContainer container;
	 
	guint16 flags;
	 
	 /* mem chunks */
	GMemChunk *row_mem_chunk;
	GMemChunk *cell_mem_chunk;

	guint freeze_count;
	 
	 /* allocation rectangle after the conatiner_border_width
 * and the width of the shadow border */
	GdkRectangle internal_allocation;
	 
	 /* rows */
	gint rows;
	gint row_center_offset;
	gint row_height;
	GList *row_list;
	GList *row_list_end;
	 
	 /* columns */
	gint columns;
	GdkRectangle column_title_area;
	GdkWindow *title_window;
	 
	 /* dynamicly allocated array of column structures */
	GtkCListColumn *column;
	 
	 /* the scrolling window and its height and width to
 * make things a little speedier */
	GdkWindow *clist_window;
	gint clist_window_width;
	gint clist_window_height;
	 
	 /* offsets for scrolling */
	gint hoffset;
	gint voffset;
	 
	 /* border shadow style */
	GtkShadowType shadow_type;
	 
	 /* the list's selection mode (gtkenums.h) */
	GtkSelectionMode selection_mode;
	 
	 /* list of selected rows */
	GList *selection;
	GList *selection_end;
	 
	GList *undo_selection;
	GList *undo_unselection;
	gint undo_anchor;
	 
	 /* mouse buttons */
	guint8 button_actions[5];

	guint8 drag_button;

	 /* dnd */
	GtkCListCellInfo click_cell;

	 /* scroll adjustments */
	GtkAdjustment *hadjustment;
	GtkAdjustment *vadjustment;
	 
	 /* xor GC for the vertical drag line */
	GdkGC *xor_gc;
	 
	 /* gc for drawing unselected cells */
	GdkGC *fg_gc;
	GdkGC *bg_gc;
	 
	 /* cursor used to indicate dragging */
	GdkCursor *cursor_drag;
	 
	 /* the current x-pixel location of the xor-drag line */
	gint x_drag;
	 
	 /* focus handling */
	gint focus_row;
	 
	 /* dragging the selection */
	gint anchor;
	GtkStateType anchor_state;
	gint drag_pos;
	gint htimer;
	gint vtimer;
	 
	GtkSortType sort_type;
	GtkCListCompareFunc compare;
	gint sort_column;
} GtkCList;


/* <gtk/gtkclist.h> */
typedef struct _GtkCListClass
{
	GtkContainerClass parent_class;
	 
	void (*set_scroll_adjustments) (GtkCList *clist, GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);
	void (*refresh) (GtkCList *clist);
	void (*select_row) (GtkCList *clist, gint row, gint column, GdkEvent *event);
	void (*unselect_row) (GtkCList *clist, gint row, gint column, GdkEvent *event);
	void (*row_move) (GtkCList *clist, gint source_row, gint dest_row);
	void (*click_column) (GtkCList *clist, gint column);
	void (*resize_column) (GtkCList *clist, gint column, gint width);
	void (*toggle_focus_row) (GtkCList *clist);
	void (*select_all) (GtkCList *clist);
	void (*unselect_all) (GtkCList *clist);
	void (*undo_selection) (GtkCList *clist);
	void (*start_selection) (GtkCList *clist);
	void (*end_selection) (GtkCList *clist);
	void (*extend_selection) (GtkCList *clist, GtkScrollType scroll_type, gfloat position, gboolean auto_start_selection);
	void (*scroll_horizontal) (GtkCList *clist, GtkScrollType scroll_type, gfloat position);
	void (*scroll_vertical) (GtkCList *clist, GtkScrollType scroll_type, gfloat position);
	void (*toggle_add_mode) (GtkCList *clist);
	void (*abort_column_resize) (GtkCList *clist);
	void (*resync_selection) (GtkCList *clist, GdkEvent *event);
	GList* (*selection_find) (GtkCList *clist, gint row_number, GList *row_list_element);
	void (*draw_row) (GtkCList *clist, GdkRectangle *area, gint row, GtkCListRow *clist_row);
	void (*draw_drag_highlight) (GtkCList *clist, GtkCListRow *target_row, gint target_row_number, GtkCListDragPos drag_pos);
	void (*clear) (GtkCList *clist);
	void (*fake_unselect_all) (GtkCList *clist, gint row);
	void (*sort_list) (GtkCList *clist);
	gint (*insert_row) (GtkCList *clist, gint row, gchar *text[]);
	void (*remove_row) (GtkCList *clist, gint row);
	void (*set_cell_contents) (GtkCList *clist, GtkCListRow *clist_row, gint column, GtkCellType type, const gchar *text, guint8 spacing, GdkPixmap *pixmap, GdkBitmap *mask);
	void (*cell_size_request) (GtkCList *clist, GtkCListRow *clist_row, gint column, GtkRequisition *requisition);

} GtkCListClass;


/* <gtk/gtkclist.h> */
typedef struct _GtkCListColumn
{
	gchar *title;
	GdkRectangle area;
	 
	GtkWidget *button;
	GdkWindow *window;
	 
	gint width;
	gint min_width;
	gint max_width;
	GtkJustification justification;
	 
	guint visible : 1;  
	guint width_set : 1;
	guint resizeable : 1;
	guint auto_resize : 1;
	guint button_passive : 1;
} GtkCListColumn;


/* <gtk/gtkclist.h> */
typedef struct _GtkCListRow
{
	GtkCell *cell;
	GtkStateType state;
	 
	GdkColor foreground;
	GdkColor background;
	 
	GtkStyle *style;

	gpointer data;
	GtkDestroyNotify destroy;
	 
	guint fg_set : 1;
	guint bg_set : 1;
	guint selectable : 1;
} GtkCListRow;

/* Cell Structures */

/* <gtk/gtkclist.h> */
typedef struct _GtkCellText
{
	GtkCellType type;
	 
	gint16 vertical;
	gint16 horizontal;
	 
	GtkStyle *style;

	gchar *text;
} GtkCellText;


/* <gtk/gtkclist.h> */
typedef struct _GtkCellPixmap
{
	GtkCellType type;
	 
	gint16 vertical;
	gint16 horizontal;
	 
	GtkStyle *style;

	GdkPixmap *pixmap;
	GdkBitmap *mask;
} GtkCellPixmap;


/* <gtk/gtkclist.h> */
typedef struct _GtkCellPixText
{
	GtkCellType type;
	 
	gint16 vertical;
	gint16 horizontal;
	 
	GtkStyle *style;

	gchar *text;
	guint8 spacing;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
} GtkCellPixText;


/* <gtk/gtkclist.h> */
typedef struct _GtkCellWidget
{
	GtkCellType type;
	 
	gint16 vertical;
	gint16 horizontal;
	 
	GtkStyle *style;

	GtkWidget *widget;
} GtkCellWidget;


/* <gtk/gtkclist.h> */
typedef struct _GtkCell
{
	GtkCellType type;
	 
	gint16 vertical;
	gint16 horizontal;
	 
	GtkStyle *style;

	union {
	gchar *text;
	 
	struct {
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	} pm;
	 
	struct {
	gchar *text;
	guint8 spacing;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	} pt;
	 
	GtkWidget *widget;
	} u;
} GtkCell;


/* <gtk/gtkclist.h> */
GtkType gtk_clist_get_type(void)
{
}

/* constructors useful for gtk-- wrappers */

/* <gtk/gtkclist.h> */
void gtk_clist_construct(GtkCList *clist, gint columns, gchar *titles[])
{
}

/* create a new GtkCList */

/* <gtk/gtkclist.h> */
GtkWidget* gtk_clist_new(gint columns)
{
}

/* <gtk/gtkclist.h> */
GtkWidget* gtk_clist_new_with_titles(gint columns, gchar *titles[])
{
}

/* set adjustments of clist */

/* <gtk/gtkclist.h> */
void gtk_clist_set_hadjustment(GtkCList *clist, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_set_vadjustment(GtkCList *clist, GtkAdjustment *adjustment)
{
}

/* get adjustments of clist */

/* <gtk/gtkclist.h> */
GtkAdjustment* gtk_clist_get_hadjustment(GtkCList *clist)
{
}

/* <gtk/gtkclist.h> */
GtkAdjustment* gtk_clist_get_vadjustment(GtkCList *clist)
{
}

/* set the border style of the clist */

/* <gtk/gtkclist.h> */
void gtk_clist_set_shadow_type(GtkCList *clist, GtkShadowType type)
{
}

/* set the clist's selection mode */

/* <gtk/gtkclist.h> */
void gtk_clist_set_selection_mode(GtkCList *clist, GtkSelectionMode mode)
{
}

/* enable clists reorder ability */

/* <gtk/gtkclist.h> */
void gtk_clist_set_reorderable(GtkCList *clist, gboolean reorderable)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_set_use_drag_icons(GtkCList *clist, gboolean use_icons)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_set_button_actions(GtkCList *clist, guint button, guint8 button_actions)
{
}

/* freeze all visual updates of the list, and then thaw the list after
 * you have made a number of changes and the updates wil occure in a
 * more efficent mannor than if you made them on a unfrozen list
 */

/* <gtk/gtkclist.h> */
void gtk_clist_freeze(GtkCList *clist)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_thaw(GtkCList *clist)
{
}

/* show and hide the column title buttons */

/* <gtk/gtkclist.h> */
void gtk_clist_column_titles_show(GtkCList *clist)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_column_titles_hide(GtkCList *clist)
{
}

/* set the column title to be a active title (responds to button presses, 
 * prelights, and grabs keyboard focus), or passive where it acts as just
 * a title
 */

/* <gtk/gtkclist.h> */
void gtk_clist_column_title_active(GtkCList *clist, gint column)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_column_title_passive(GtkCList *clist, gint column)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_column_titles_active(GtkCList *clist)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_column_titles_passive(GtkCList *clist)
{
}

/* set the title in the column title button */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_title(GtkCList *clist, gint column, const gchar *title)
{
}

/* returns the title of column. Returns NULL if title is not set */

/* <gtk/gtkclist.h> */
gchar * gtk_clist_get_column_title(GtkCList *clist, gint column)
{
}

/* set a widget instead of a title for the column title button */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_widget(GtkCList *clist, gint column, GtkWidget *widget)
{
}

/* returns the column widget */

/* <gtk/gtkclist.h> */
GtkWidget * gtk_clist_get_column_widget(GtkCList *clist, gint column)
{
}

/* set the justification on a column */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_justification(GtkCList *clist, gint column, GtkJustification justification)
{
}

/* set visibility of a column */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_visibility(GtkCList *clist, gint column, gboolean visible)
{
}

/* enable/disable column resize operations by mouse */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_resizeable(GtkCList *clist, gint column, gboolean resizeable)
{
}

/* resize column automatically to its optimal width */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_auto_resize(GtkCList *clist, gint column, gboolean auto_resize)
{
}


/* <gtk/gtkclist.h> */
gint gtk_clist_columns_autosize(GtkCList *clist)
{
}

/* return the optimal column width, i.e. maximum of all cell widths */

/* <gtk/gtkclist.h> */
gint gtk_clist_optimal_column_width(GtkCList *clist, gint column)
{
}

/* set the pixel width of a column; this is a necessary step in
 * creating a CList because otherwise the column width is chozen from
 * the width of the column title, which will never be right
 */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_width(GtkCList *clist, gint column, gint width)
{
}

/* set column minimum/maximum width. min/max_width < 0 => no restriction */

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_min_width(GtkCList *clist, gint column, gint min_width)
{
}

/* <gtk/gtkclist.h> */
void gtk_clist_set_column_max_width(GtkCList *clist, gint column, gint max_width)
{
}

/* change the height of the rows, the default (height=0) is
 * the hight of the current font.
 */

/* <gtk/gtkclist.h> */
void gtk_clist_set_row_height(GtkCList *clist, guint height)
{
}

/* scroll the viewing area of the list to the given column and row;
 * row_align and col_align are between 0-1 representing the location the
 * row should appear on the screnn, 0.0 being top or left, 1.0 being
 * bottom or right; if row or column is -1 then then there is no change
 */

/* <gtk/gtkclist.h> */
void gtk_clist_moveto(GtkCList *clist, gint row, gint column, gfloat row_align, gfloat col_align)
{
}

/* returns whether the row is visible */

/* <gtk/gtkclist.h> */
GtkVisibility gtk_clist_row_is_visible(GtkCList *clist, gint row)
{
}

/* returns the cell type */

/* <gtk/gtkclist.h> */
GtkCellType gtk_clist_get_cell_type(GtkCList *clist, gint row, gint column)
{
}

/* sets a given cell's text, replacing its current contents */

/* <gtk/gtkclist.h> */
void gtk_clist_set_text(GtkCList *clist, gint row, gint column, const gchar *text)
{
}

/* for the "get" functions, any of the return pointer can be
 * NULL if you are not interested
 */

/* <gtk/gtkclist.h> */
gint gtk_clist_get_text(GtkCList *clist, gint row, gint column, gchar **text)
{
}

/* sets a given cell's pixmap, replacing its current contents */

/* <gtk/gtkclist.h> */
void gtk_clist_set_pixmap(GtkCList *clist, gint row, gint column, GdkPixmap *pixmap, GdkBitmap *mask)
{
}


/* <gtk/gtkclist.h> */
gint gtk_clist_get_pixmap(GtkCList *clist, gint row, gint column, GdkPixmap **pixmap, GdkBitmap **mask)
{
}

/* sets a given cell's pixmap and text, replacing its current contents */

/* <gtk/gtkclist.h> */
void gtk_clist_set_pixtext(GtkCList *clist, gint row, gint column, const gchar *text, guint8 spacing, GdkPixmap *pixmap, GdkBitmap *mask)
{
}


/* <gtk/gtkclist.h> */
gint gtk_clist_get_pixtext(GtkCList *clist, gint row, gint column, gchar **text, guint8 *spacing, GdkPixmap **pixmap, GdkBitmap **mask)
{
}

/* sets the foreground color of a row, the color must already
 * be allocated
 */

/* <gtk/gtkclist.h> */
void gtk_clist_set_foreground(GtkCList *clist, gint row, GdkColor *color)
{
}

/* sets the background color of a row, the color must already
 * be allocated
 */

/* <gtk/gtkclist.h> */
void gtk_clist_set_background(GtkCList *clist, gint row, GdkColor *color)
{
}

/* set / get cell styles */

/* <gtk/gtkclist.h> */
void gtk_clist_set_cell_style(GtkCList *clist, gint row, gint column, GtkStyle *style)
{
}


/* <gtk/gtkclist.h> */
GtkStyle *gtk_clist_get_cell_style(GtkCList *clist, gint row, gint column)
{
}


/* <gtk/gtkclist.h> */
void gtk_clist_set_row_style(GtkCList *clist, gint row, GtkStyle *style)
{
}


/* <gtk/gtkclist.h> */
GtkStyle *gtk_clist_get_row_style(GtkCList *clist, gint row)
{
}

/* this sets a horizontal and vertical shift for drawing
 * the contents of a cell; it can be positive or negitive;
 * this is particulary useful for indenting items in a column
 */

/* <gtk/gtkclist.h> */
void gtk_clist_set_shift(GtkCList *clist, gint row, gint column, gint vertical, gint horizontal)
{
}

/* set/get selectable flag of a single row */

/* <gtk/gtkclist.h> */
void gtk_clist_set_selectable(GtkCList *clist, gint row, gboolean selectable)
{
}

/* <gtk/gtkclist.h> */
gboolean gtk_clist_get_selectable(GtkCList *clist, gint row)
{
}

/* prepend/append returns the index of the row you just added,
 * making it easier to append and modify a row
 */

/* <gtk/gtkclist.h> */
gint gtk_clist_prepend(GtkCList *clist, gchar *text[])
{
}

/* <gtk/gtkclist.h> */
gint gtk_clist_append(GtkCList *clist, gchar *text[])
{
}

/* inserts a row at index row and returns the row where it was
 * actually inserted (may be different from "row" in auto_sort mode)
 */

/* <gtk/gtkclist.h> */
gint gtk_clist_insert(GtkCList *clist, gint row, gchar *text[])
{
}

/* removes row at index row */

/* <gtk/gtkclist.h> */
void gtk_clist_remove(GtkCList *clist, gint row)
{
}

/* sets a arbitrary data pointer for a given row */

/* <gtk/gtkclist.h> */
void gtk_clist_set_row_data(GtkCList *clist, gint row, gpointer data)
{
}

/* sets a data pointer for a given row with destroy notification */

/* <gtk/gtkclist.h> */
void gtk_clist_set_row_data_full(GtkCList *clist, gint row, gpointer data, GtkDestroyNotify destroy)
{
}

/* returns the data set for a row */

/* <gtk/gtkclist.h> */
gpointer gtk_clist_get_row_data(GtkCList *clist, gint row)
{
}

/* givin a data pointer, find the first (and hopefully only!)
 * row that points to that data, or -1 if none do
 */

/* <gtk/gtkclist.h> */
gint gtk_clist_find_row_from_data(GtkCList *clist, gpointer data)
{
}

/* force selection of a row */

/* <gtk/gtkclist.h> */
void gtk_clist_select_row(GtkCList *clist, gint row, gint column)
{
}

/* force unselection of a row */

/* <gtk/gtkclist.h> */
void gtk_clist_unselect_row(GtkCList *clist, gint row, gint column)
{
}

/* undo the last select/unselect operation */

/* <gtk/gtkclist.h> */
void gtk_clist_undo_selection(GtkCList *clist)
{
}

/* clear the entire list -- this is much faster than removing
 * each item with gtk_clist_remove
 */

/* <gtk/gtkclist.h> */
void gtk_clist_clear(GtkCList *clist)
{
}

/* return the row column corresponding to the x and y coordinates,
 * the returned values are only valid if the x and y coordinates
 * are respectively to a window == clist->clist_window
 */

/* <gtk/gtkclist.h> */
gint gtk_clist_get_selection_info(GtkCList *clist, gint x, gint y, gint *row, gint *column)
{
}

/* in multiple or extended mode, select all rows */

/* <gtk/gtkclist.h> */
void gtk_clist_select_all(GtkCList *clist)
{
}

/* in all modes except browse mode, deselect all rows */

/* <gtk/gtkclist.h> */
void gtk_clist_unselect_all(GtkCList *clist)
{
}

/* swap the position of two rows */

/* <gtk/gtkclist.h> */
void gtk_clist_swap_rows(GtkCList *clist, gint row1, gint row2)
{
}

/* move row from source_row position to dest_row position */

/* <gtk/gtkclist.h> */
void gtk_clist_row_move(GtkCList *clist, gint source_row, gint dest_row)
{
}

/* sets a compare function different to the default */

/* <gtk/gtkclist.h> */
void gtk_clist_set_compare_func(GtkCList *clist, GtkCListCompareFunc cmp_func)
{
}

/* the column to sort by */

/* <gtk/gtkclist.h> */
void gtk_clist_set_sort_column(GtkCList *clist, gint column)
{
}

/* how to sort : ascending or descending */

/* <gtk/gtkclist.h> */
void gtk_clist_set_sort_type(GtkCList *clist, GtkSortType sort_type)
{
}

/* sort the list with the current compare function */

/* <gtk/gtkclist.h> */
void gtk_clist_sort(GtkCList *clist)
{
}

/* Automatically sort upon insertion */

/* <gtk/gtkclist.h> */
void gtk_clist_set_auto_sort(GtkCList *clist, gboolean auto_sort)
{
}


/* <gtk/gtkcolorsel.h> */
#define GTK_TYPE_COLOR_SELECTION (gtk_color_selection_get_type ())

/* <gtk/gtkcolorsel.h> */
#define GTK_COLOR_SELECTION(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_COLOR_SELECTION, GtkColorSelection))

/* <gtk/gtkcolorsel.h> */
#define GTK_COLOR_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_COLOR_SELECTION, GtkColorSelectionClass))

/* <gtk/gtkcolorsel.h> */
#define GTK_IS_COLOR_SELECTION(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_COLOR_SELECTION))

/* <gtk/gtkcolorsel.h> */
#define GTK_IS_COLOR_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_COLOR_SELECTION))


/* <gtk/gtkcolorsel.h> */
#define GTK_TYPE_COLOR_SELECTION_DIALOG (gtk_color_selection_dialog_get_type ())

/* <gtk/gtkcolorsel.h> */
#define GTK_COLOR_SELECTION_DIALOG(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_COLOR_SELECTION_DIALOG, GtkColorSelectionDialog))

/* <gtk/gtkcolorsel.h> */
#define GTK_COLOR_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_COLOR_SELECTION_DIALOG, GtkColorSelectionDialogClass))

/* <gtk/gtkcolorsel.h> */
#define GTK_IS_COLOR_SELECTION_DIALOG(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_COLOR_SELECTION_DIALOG))

/* <gtk/gtkcolorsel.h> */
#define GTK_IS_COLOR_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_COLOR_SELECTION_DIALOG))


/* <gtk/gtkcolorsel.h> */
typedef struct _GtkColorSelection
{
	GtkVBox vbox;

	GtkWidget *wheel_area;
	GtkWidget *value_area;
	GtkWidget *sample_area;
	GtkWidget *sample_area_eb;

	GtkWidget *scales[8];
	GtkWidget *entries[8];
	GtkWidget *opacity_label;

	GdkGC *wheel_gc;
	GdkGC *value_gc;
	GdkGC *sample_gc;

	GtkUpdateType policy;
	gint use_opacity;
	gint timer_active;
	gint timer_tag;
	gdouble values[8];
	gdouble old_values[8];

	guchar *wheel_buf;
	guchar *value_buf;
	guchar *sample_buf;
} GtkColorSelection;


/* <gtk/gtkcolorsel.h> */
typedef struct _GtkColorSelectionClass
{
	GtkVBoxClass parent_class;

	void (* color_changed) (GtkColorSelection *colorsel);
} GtkColorSelectionClass;


/* <gtk/gtkcolorsel.h> */
typedef struct _GtkColorSelectionDialog
{
	GtkWindow window;

	GtkWidget *colorsel;
	GtkWidget *main_vbox;
	GtkWidget *ok_button;
	GtkWidget *reset_button;
	GtkWidget *cancel_button;
	GtkWidget *help_button;
} GtkColorSelectionDialog;


/* <gtk/gtkcolorsel.h> */
typedef struct _GtkColorSelectionDialogClass
{
	GtkWindowClass parent_class;
} GtkColorSelectionDialogClass;


/* ColorSelection */


/* <gtk/gtkcolorsel.h> */
GtkType gtk_color_selection_get_type(void)
{
}


/* <gtk/gtkcolorsel.h> */
GtkWidget* gtk_color_selection_new(void)
{
}


/* <gtk/gtkcolorsel.h> */
void gtk_color_selection_set_update_policy(GtkColorSelection *colorsel, GtkUpdateType policy)
{
}


/* <gtk/gtkcolorsel.h> */
void gtk_color_selection_set_opacity(GtkColorSelection *colorsel, gint use_opacity)
{
}


/* <gtk/gtkcolorsel.h> */
void gtk_color_selection_set_color(GtkColorSelection *colorsel, gdouble *color)
{
}


/* <gtk/gtkcolorsel.h> */
void gtk_color_selection_get_color(GtkColorSelection *colorsel, gdouble *color)
{
}

/* ColorSelectionDialog */


/* <gtk/gtkcolorsel.h> */
GtkType gtk_color_selection_dialog_get_type(void)
{
}


/* <gtk/gtkcolorsel.h> */
GtkWidget* gtk_color_selection_dialog_new(const gchar *title)
{
}



/* <gtk/gtkcombo.h> */
#define GTK_COMBO(obj) GTK_CHECK_CAST (obj, gtk_combo_get_type (), GtkCombo)

/* <gtk/gtkcombo.h> */
#define GTK_COMBO_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_combo_get_type (), GtkComboClass)

/* <gtk/gtkcombo.h> */
#define GTK_IS_COMBO(obj) GTK_CHECK_TYPE (obj, gtk_combo_get_type ())


/* you should access only the entry and list fields directly */

/* <gtk/gtkcombo.h> */
typedef struct _GtkCombo
{
	GtkHBox hbox;
	GtkWidget *entry;
	GtkWidget *button;
	GtkWidget *popup;
	GtkWidget *popwin;
	GtkWidget *list;

	guint entry_change_id;
	guint list_change_id;

	guint value_in_list:1;
	guint ok_if_empty:1;
	guint case_sensitive:1;
	guint use_arrows:1;
	guint use_arrows_always:1;

	guint16 current_button;
	guint activate_id;
} GtkCombo;


/* <gtk/gtkcombo.h> */
typedef struct _GtkComboClass
{
	GtkHBoxClass parent_class;
} GtkComboClass;


/* <gtk/gtkcombo.h> */
guint gtk_combo_get_type(void)
{
}


/* <gtk/gtkcombo.h> */
GtkWidget *gtk_combo_new(void)
{
}
/* the text in the entry must be or not be in the list */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_value_in_list(GtkCombo* combo, gint val, gint ok_if_empty)
{
}
/* set/unset arrows working for changing the value (can be annoying */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_use_arrows(GtkCombo* combo, gint val)
{
}
/* up/down arrows change value if current value not in list */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_use_arrows_always(GtkCombo* combo, gint val)
{
}
/* perform case-sensitive compares */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_case_sensitive(GtkCombo* combo, gint val)
{
}
/* call this function on an item if it isn't a label or you
	want it to have a different value to be displayed in the entry */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_item_string(GtkCombo* combo, GtkItem* item, const gchar* item_value)
{
}
/* simple interface */

/* <gtk/gtkcombo.h> */
void gtk_combo_set_popdown_strings(GtkCombo* combo, GList *strings)
{
}


/* <gtk/gtkcombo.h> */
void gtk_combo_disable_activate(GtkCombo* combo)
{
}

/* <gtk/gtkcompat.h> */
#define gtk_accel_label_accelerator_width gtk_accel_label_get_accel_width

/* <gtk/gtkcompat.h> */
#define gtk_container_border_width gtk_container_set_border_width

/* <gtk/gtkcompat.h> */
#define gtk_notebook_current_page gtk_notebook_get_current_page

/* <gtk/gtkcompat.h> */
#define gtk_packer_configure gtk_packer_set_child_packing

/* <gtk/gtkcompat.h> */
#define gtk_paned_gutter_size gtk_paned_set_gutter_size

/* <gtk/gtkcompat.h> */
#define gtk_paned_handle_size gtk_paned_set_handle_size

/* <gtk/gtkcompat.h> */
#define gtk_scale_value_width gtk_scale_get_value_width

/* <gtk/gtkcompat.h> */
#define gtk_window_position gtk_window_set_position

/* <gtk/gtkcompat.h> */
#define gtk_toggle_button_set_state gtk_toggle_button_set_active

/* <gtk/gtkcompat.h> */
#define gtk_check_menu_item_set_state gtk_check_menu_item_set_active

/* strongly deprecated: */

/* <gtk/gtkcompat.h> */
#define gtk_ctree_set_reorderable(t,r) gtk_clist_set_reorderable((GtkCList*) (t),(r))

/* <gtk/gtkcompat.h> */
#define gtk_style_apply_default_pixmap(s,gw,st,a,x,y,w,h) \
	gtk_style_apply_default_background (s,gw,TRUE,st,a,x,y,w,h)

/* <gtk/gtkcontainer.h> */
#define GTK_TYPE_CONTAINER (gtk_container_get_type ())

/* <gtk/gtkcontainer.h> */
#define GTK_CONTAINER(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CONTAINER, GtkContainer))

/* <gtk/gtkcontainer.h> */
#define GTK_CONTAINER_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CONTAINER, GtkContainerClass))

/* <gtk/gtkcontainer.h> */
#define GTK_IS_CONTAINER(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CONTAINER))

/* <gtk/gtkcontainer.h> */
#define GTK_IS_CONTAINER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CONTAINER))


/* <gtk/gtkcontainer.h> */
#define GTK_IS_RESIZE_CONTAINER(widget) (GTK_IS_CONTAINER (widget) && ((GtkContainer*) (widget))->resize_mode != GTK_RESIZE_PARENT)



/* <gtk/gtkcontainer.h> */
typedef struct _GtkContainer
{
	GtkWidget widget;
	 
	GtkWidget *focus_child;
	 
	guint border_width : 16;
	guint need_resize : 1;
	guint resize_mode : 2;
	guint reallocate_redraws : 1;
	 
	 /* The list of children that requested a resize
 */
	GSList *resize_widgets;
} GtkContainer;


/* <gtk/gtkcontainer.h> */
typedef struct _GtkContainerClass
{
	GtkWidgetClass parent_class;
	 
	guint n_child_args;

	void (* add) (GtkContainer *container, GtkWidget *widget);
	void (* remove) (GtkContainer *container, GtkWidget *widget);
	void (* check_resize) (GtkContainer *container);
	void (* forall) (GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callbabck_data);
	gint (* focus) (GtkContainer *container, GtkDirectionType direction);
	void (* set_focus_child) (GtkContainer *container, GtkWidget *widget);
	GtkType (*child_type) (GtkContainer *container);
	void (*set_child_arg) (GtkContainer *container, GtkWidget *child, GtkArg *arg, guint arg_id);
	void (*get_child_arg) (GtkContainer *container, GtkWidget *child, GtkArg *arg, guint arg_id);
	gchar* (*composite_name) (GtkContainer *container, GtkWidget *child);

	 /* Padding for future expansion */
	GtkFunction pad1;
	GtkFunction pad2;
} GtkContainerClass;

/* Application-level methods */


/* <gtk/gtkcontainer.h> */
GtkType gtk_container_get_type(void)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_set_border_width(GtkContainer *container, guint border_width)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_add(GtkContainer *container, GtkWidget *widget)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_remove(GtkContainer *container, GtkWidget *widget)
{
}


/* <gtk/gtkcontainer.h> */
void gtk_container_set_resize_mode(GtkContainer *container, GtkResizeMode resize_mode)
{
}


/* <gtk/gtkcontainer.h> */
void gtk_container_check_resize(GtkContainer *container)
{
}


/* <gtk/gtkcontainer.h> */
void gtk_container_foreach(GtkContainer *container, GtkCallback callback, gpointer callback_data)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_foreach_full(GtkContainer *container, GtkCallback callback, GtkCallbackMarshal marshal, gpointer callback_data, GtkDestroyNotify notify)
{
}

/* <gtk/gtkcontainer.h> */
GList* gtk_container_children(GtkContainer *container)
{
}

/* <gtk/gtkcontainer.h> */
gint gtk_container_focus(GtkContainer *container, GtkDirectionType direction)
{
}

/* Widget-level methods */


/* <gtk/gtkcontainer.h> */
void gtk_container_set_reallocate_redraws(GtkContainer *container, gboolean needs_redraws)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_set_focus_child(GtkContainer *container, GtkWidget *child)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_set_focus_vadjustment(GtkContainer *container, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_set_focus_hadjustment(GtkContainer *container, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_register_toplevel(GtkContainer *container)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_unregister_toplevel(GtkContainer *container)
{
}

/* <gtk/gtkcontainer.h> */
GList* gtk_container_get_toplevels(void)
{
}


/* <gtk/gtkcontainer.h> */
void gtk_container_resize_children(GtkContainer *container)
{
}


/* <gtk/gtkcontainer.h> */
GtkType gtk_container_child_type(GtkContainer *container)
{
}

/* the `arg_name' argument needs to be a const static string */

/* <gtk/gtkcontainer.h> */
void gtk_container_add_child_arg_type(const gchar *arg_name, GtkType arg_type, guint arg_flags, guint arg_id)
{
}
	 
/* Allocate a GtkArg array of size nargs that hold the
 * names and types of the args that can be used with
 * gtk_container_child_getv/gtk_container_child_setv.
 * if (arg_flags!=NULL),
 * (*arg_flags) will be set to point to a newly allocated
 * guint array that holds the flags of the args.
 * It is the callers response to do a
 * g_free (returned_args); g_free (*arg_flags).
 */

/* <gtk/gtkcontainer.h> */
GtkArg* gtk_container_query_child_args(GtkType class_type, guint32 **arg_flags, guint *nargs)
{
}

/* gtk_container_child_getv() sets an arguments type and value, or just
 * its type to GTK_TYPE_INVALID.
 * if GTK_FUNDAMENTAL_TYPE (arg->type) == GTK_TYPE_STRING, it's the callers
 * response to do a g_free (GTK_VALUE_STRING (arg));
 */

/* <gtk/gtkcontainer.h> */
void gtk_container_child_getv(GtkContainer *container, GtkWidget *child, guint n_args, GtkArg *args)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_child_setv(GtkContainer *container, GtkWidget *child, guint n_args, GtkArg *args)
{
}

/* gtk_container_add_with_args() takes a variable argument list of the form:
 * (..., gchar *arg_name, ARG_VALUES, [repeatedly name/value pairs,] NULL)
 * where ARG_VALUES type depend on the argument and can consist of
 * more than one c-function argument.
 */

/* <gtk/gtkcontainer.h> */
void gtk_container_add_with_args(GtkContainer *container, GtkWidget *widget, const gchar *first_arg_name, ...)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_addv(GtkContainer *container, GtkWidget *widget, guint n_args, GtkArg *args)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_child_set(GtkContainer *container, GtkWidget *child, const gchar *first_arg_name, ...)
{
}
	 

/* Non-public methods */


/* <gtk/gtkcontainer.h> */
void gtk_container_queue_resize(GtkContainer *container)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_clear_resize_widgets(GtkContainer *container)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_arg_set(GtkContainer *container, GtkWidget *child, GtkArg *arg, GtkArgInfo *info)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_arg_get(GtkContainer *container, GtkWidget *child, GtkArg *arg, GtkArgInfo *info)
{
}

/* <gtk/gtkcontainer.h> */
gchar* gtk_container_child_args_collect(GtkType object_type, GSList **arg_list_p, GSList **info_list_p, const gchar *first_arg_name, va_list args)
{
}

/* <gtk/gtkcontainer.h> */
gchar* gtk_container_child_arg_get_info(GtkType object_type, const gchar *arg_name, GtkArgInfo **info_p)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_forall(GtkContainer *container, GtkCallback callback, gpointer callback_data)
{
}

/* <gtk/gtkcontainer.h> */
gchar* gtk_container_child_composite_name(GtkContainer *container, GtkWidget *child)
{
}

/* <gtk/gtkcontainer.h> */
void gtk_container_dequeue_resize_handler(GtkContainer *container)
{
}

/* <gtk/gtkctree.h> */
#define GTK_TYPE_CTREE (gtk_ctree_get_type ())

/* <gtk/gtkctree.h> */
#define GTK_CTREE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CTREE, GtkCTree))

/* <gtk/gtkctree.h> */
#define GTK_CTREE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CTREE, GtkCTreeClass))

/* <gtk/gtkctree.h> */
#define GTK_IS_CTREE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CTREE))

/* <gtk/gtkctree.h> */
#define GTK_IS_CTREE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CTREE))


/* <gtk/gtkctree.h> */
#define GTK_CTREE_ROW(_node_) ((GtkCTreeRow *)(((GList *)(_node_))->data))

/* <gtk/gtkctree.h> */
#define GTK_CTREE_NODE(_node_) ((GtkCTreeNode *)((_node_)))

/* <gtk/gtkctree.h> */
#define GTK_CTREE_NODE_NEXT(_nnode_) ((GtkCTreeNode *)(((GList *)(_nnode_))->next))

/* <gtk/gtkctree.h> */
#define GTK_CTREE_NODE_PREV(_pnode_) ((GtkCTreeNode *)(((GList *)(_pnode_))->prev))

/* <gtk/gtkctree.h> */
#define GTK_CTREE_FUNC(_func_) ((GtkCTreeFunc)(_func_))


/* <gtk/gtkctree.h> */
typedef enum
{
	GTK_CTREE_POS_BEFORE,
	GTK_CTREE_POS_AS_CHILD,
	GTK_CTREE_POS_AFTER
} GtkCTreePos;


/* <gtk/gtkctree.h> */
typedef enum
{
	GTK_CTREE_LINES_NONE,
	GTK_CTREE_LINES_SOLID,
	GTK_CTREE_LINES_DOTTED,
	GTK_CTREE_LINES_TABBED
} GtkCTreeLineStyle;


/* <gtk/gtkctree.h> */
typedef enum
{
	GTK_CTREE_EXPANDER_NONE,
	GTK_CTREE_EXPANDER_SQUARE,
	GTK_CTREE_EXPANDER_TRIANGLE,
	GTK_CTREE_EXPANDER_CIRCULAR
} GtkCTreeExpanderStyle;


/* <gtk/gtkctree.h> */
typedef enum
{
	GTK_CTREE_EXPANSION_EXPAND,
	GTK_CTREE_EXPANSION_EXPAND_RECURSIVE,
	GTK_CTREE_EXPANSION_COLLAPSE,
	GTK_CTREE_EXPANSION_COLLAPSE_RECURSIVE,
	GTK_CTREE_EXPANSION_TOGGLE,
	GTK_CTREE_EXPANSION_TOGGLE_RECURSIVE
} GtkCTreeExpansionType;


/* <gtk/gtkctree.h> */
typedef void(*GtkCTreeFunc) (GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
}


/* <gtk/gtkctree.h> */
typedef gboolean(*GtkCTreeGNodeFunc) (GtkCTree *ctree, guint depth, GNode *gnode, GtkCTreeNode *cnode, gpointer data)
{
}


/* <gtk/gtkctree.h> */
typedef gboolean(*GtkCTreeCompareDragFunc) (GtkCTree *ctree, GtkCTreeNode *source_node, GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling)
{
}


/* <gtk/gtkctree.h> */
typedef struct _GtkCTree
{
	GtkCList clist;
	 
	GdkGC *lines_gc;
	 
	gint tree_indent;
	gint tree_spacing;
	gint tree_column;

	guint line_style : 2;
	guint expander_style : 2;
	guint show_stub : 1;

	GtkCTreeCompareDragFunc drag_compare;
} GtkCTree;


/* <gtk/gtkctree.h> */
typedef struct _GtkCTreeClass
{
	GtkCListClass parent_class;
	 
	void (*tree_select_row) (GtkCTree *ctree, GtkCTreeNode *row, gint column);
	void (*tree_unselect_row) (GtkCTree *ctree, GtkCTreeNode *row, gint column);
	void (*tree_expand) (GtkCTree *ctree, GtkCTreeNode *node);
	void (*tree_collapse) (GtkCTree *ctree, GtkCTreeNode *node);
	void (*tree_move) (GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling);
	void (*change_focus_row_expansion) (GtkCTree *ctree, GtkCTreeExpansionType action);
} GtkCTreeClass;


/* <gtk/gtkctree.h> */
typedef struct _GtkCTreeRow
{
	GtkCListRow row;
	 
	GtkCTreeNode *parent;
	GtkCTreeNode *sibling;
	GtkCTreeNode *children;
	 
	GdkPixmap *pixmap_closed;
	GdkBitmap *mask_closed;
	GdkPixmap *pixmap_opened;
	GdkBitmap *mask_opened;
	 
	guint16 level;
	 
	guint is_leaf : 1;
	guint expanded : 1;
} GtkCTreeRow;


/* <gtk/gtkctree.h> */
typedef struct _GtkCTreeNode
{
	GList list;
} GtkCTreeNode;


/***********************************************************
 * Creation, insertion, deletion *
 ***********************************************************/


/* <gtk/gtkctree.h> */
GtkType gtk_ctree_get_type(void)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_construct(GtkCTree *ctree, gint columns, gint tree_column, gchar *titles[])
{
}

/* <gtk/gtkctree.h> */
GtkWidget * gtk_ctree_new_with_titles(gint columns, gint tree_column, gchar *titles[])
{
}

/* <gtk/gtkctree.h> */
GtkWidget * gtk_ctree_new(gint columns, gint tree_column)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_insert_node(GtkCTree *ctree, GtkCTreeNode *parent, GtkCTreeNode *sibling, gchar *text[], guint8 spacing, GdkPixmap *pixmap_closed, GdkBitmap *mask_closed, GdkPixmap *pixmap_opened, GdkBitmap *mask_opened, gboolean is_leaf, gboolean expanded)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_remove_node(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_insert_gnode(GtkCTree *ctree, GtkCTreeNode *parent, GtkCTreeNode *sibling, GNode *gnode, GtkCTreeGNodeFunc func, gpointer data)
{
}

/* <gtk/gtkctree.h> */
GNode * gtk_ctree_export_to_gnode(GtkCTree *ctree, GNode *parent, GNode *sibling, GtkCTreeNode *node, GtkCTreeGNodeFunc func, gpointer data)
{
}

/***********************************************************
 * Generic recursive functions, querying / finding tree *
 * information *
 ***********************************************************/


/* <gtk/gtkctree.h> */
void gtk_ctree_post_recursive(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeFunc func, gpointer data)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_post_recursive_to_depth(GtkCTree *ctree, GtkCTreeNode *node, gint depth, GtkCTreeFunc func, gpointer data)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_pre_recursive(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeFunc func, gpointer data)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_pre_recursive_to_depth(GtkCTree *ctree, GtkCTreeNode *node, gint depth, GtkCTreeFunc func, gpointer data)
{
}

/* <gtk/gtkctree.h> */
gboolean gtk_ctree_is_viewable(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_last(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_find_node_ptr(GtkCTree *ctree, GtkCTreeRow *ctree_row)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_node_nth(GtkCTree *ctree, guint row)
{
}

/* <gtk/gtkctree.h> */
gboolean gtk_ctree_find(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *child)
{
}

/* <gtk/gtkctree.h> */
gboolean gtk_ctree_is_ancestor(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *child)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_find_by_row_data(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
}
/* returns a GList of all GtkCTreeNodes with row->data == data. */

/* <gtk/gtkctree.h> */
GList * gtk_ctree_find_all_by_row_data(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
}

/* <gtk/gtkctree.h> */
GtkCTreeNode * gtk_ctree_find_by_row_data_custom(GtkCTree *ctree, GtkCTreeNode *node, gpointer data, GCompareFunc func)
{
}
/* returns a GList of all GtkCTreeNodes with row->data == data. */

/* <gtk/gtkctree.h> */
GList * gtk_ctree_find_all_by_row_data_custom(GtkCTree *ctree, GtkCTreeNode *node, gpointer data, GCompareFunc func)
{
}

/* <gtk/gtkctree.h> */
gboolean gtk_ctree_is_hot_spot(GtkCTree *ctree, gint x, gint y)
{
}

/***********************************************************
 * Tree signals : move, expand, collapse, (un)select *
 ***********************************************************/


/* <gtk/gtkctree.h> */
void gtk_ctree_move(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_expand(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_expand_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_expand_to_depth(GtkCTree *ctree, GtkCTreeNode *node, gint depth)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_collapse(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_collapse_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_collapse_to_depth(GtkCTree *ctree, GtkCTreeNode *node, gint depth)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_toggle_expansion(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_toggle_expansion_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_select(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_select_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_unselect(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_unselect_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_real_select_recursive(GtkCTree *ctree, GtkCTreeNode *node, gint state)
{
}

/***********************************************************
 * Analogons of GtkCList functions *
 ***********************************************************/


/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_text(GtkCTree *ctree, GtkCTreeNode *node, gint column, const gchar *text)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_pixmap(GtkCTree *ctree, GtkCTreeNode *node, gint column, GdkPixmap *pixmap, GdkBitmap *mask)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_pixtext(GtkCTree *ctree, GtkCTreeNode *node, gint column, const gchar *text, guint8 spacing, GdkPixmap *pixmap, GdkBitmap *mask)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_node_info(GtkCTree *ctree, GtkCTreeNode *node, const gchar *text, guint8 spacing, GdkPixmap *pixmap_closed, GdkBitmap *mask_closed, GdkPixmap *pixmap_opened, GdkBitmap *mask_opened, gboolean is_leaf, gboolean expanded)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_shift(GtkCTree *ctree, GtkCTreeNode *node, gint column, gint vertical, gint horizontal)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_selectable(GtkCTree *ctree, GtkCTreeNode *node, gboolean selectable)
{
}

/* <gtk/gtkctree.h> */
gboolean gtk_ctree_node_get_selectable(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
GtkCellType gtk_ctree_node_get_cell_type(GtkCTree *ctree, GtkCTreeNode *node, gint column)
{
}

/* <gtk/gtkctree.h> */
gint gtk_ctree_node_get_text(GtkCTree *ctree, GtkCTreeNode *node, gint column, gchar **text)
{
}

/* <gtk/gtkctree.h> */
gint gtk_ctree_node_get_pixmap(GtkCTree *ctree, GtkCTreeNode *node, gint column, GdkPixmap **pixmap, GdkBitmap **mask)
{
}

/* <gtk/gtkctree.h> */
gint gtk_ctree_node_get_pixtext(GtkCTree *ctree, GtkCTreeNode *node, gint column, gchar **text, guint8 *spacing, GdkPixmap **pixmap, GdkBitmap **mask)
{
}

/* <gtk/gtkctree.h> */
gint gtk_ctree_get_node_info(GtkCTree *ctree, GtkCTreeNode *node, gchar **text, guint8 *spacing, GdkPixmap **pixmap_closed, GdkBitmap **mask_closed, GdkPixmap **pixmap_opened, GdkBitmap **mask_opened, gboolean *is_leaf, gboolean *expanded)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_row_style(GtkCTree *ctree, GtkCTreeNode *node, GtkStyle *style)
{
}

/* <gtk/gtkctree.h> */
GtkStyle * gtk_ctree_node_get_row_style(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_cell_style(GtkCTree *ctree, GtkCTreeNode *node, gint column, GtkStyle *style)
{
}

/* <gtk/gtkctree.h> */
GtkStyle * gtk_ctree_node_get_cell_style(GtkCTree *ctree, GtkCTreeNode *node, gint column)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_foreground(GtkCTree *ctree, GtkCTreeNode *node, GdkColor *color)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_background(GtkCTree *ctree, GtkCTreeNode *node, GdkColor *color)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_row_data(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_set_row_data_full(GtkCTree *ctree, GtkCTreeNode *node, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkctree.h> */
gpointer gtk_ctree_node_get_row_data(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_node_moveto(GtkCTree *ctree, GtkCTreeNode *node, gint column, gfloat row_align, gfloat col_align)
{
}

/* <gtk/gtkctree.h> */
GtkVisibility gtk_ctree_node_is_visible(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/***********************************************************
 * GtkCTree specific functions *
 ***********************************************************/


/* <gtk/gtkctree.h> */
void gtk_ctree_set_indent(GtkCTree *ctree, gint indent)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_spacing(GtkCTree *ctree, gint spacing)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_show_stub(GtkCTree *ctree, gboolean show_stub)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_line_style(GtkCTree *ctree, GtkCTreeLineStyle line_style)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_expander_style(GtkCTree *ctree, GtkCTreeExpanderStyle expander_style)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_set_drag_compare_func(GtkCTree *ctree, GtkCTreeCompareDragFunc cmp_func)
{
}

/***********************************************************
 * Tree sorting functions *
 ***********************************************************/


/* <gtk/gtkctree.h> */
void gtk_ctree_sort_node(GtkCTree *ctree, GtkCTreeNode *node)
{
}

/* <gtk/gtkctree.h> */
void gtk_ctree_sort_recursive(GtkCTree *ctree, GtkCTreeNode *node)
{
}





/* <gtk/gtkcurve.h> */
#define GTK_TYPE_CURVE (gtk_curve_get_type ())

/* <gtk/gtkcurve.h> */
#define GTK_CURVE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_CURVE, GtkCurve))

/* <gtk/gtkcurve.h> */
#define GTK_CURVE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_CURVE, GtkCurveClass))

/* <gtk/gtkcurve.h> */
#define GTK_IS_CURVE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_CURVE))

/* <gtk/gtkcurve.h> */
#define GTK_IS_CURVE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_CURVE))



/* <gtk/gtkcurve.h> */
typedef struct _GtkCurve
{
	GtkDrawingArea graph;

	gint cursor_type;
	gfloat min_x;
	gfloat max_x;
	gfloat min_y;
	gfloat max_y;
	GdkPixmap *pixmap;
	GtkCurveType curve_type;
	gint height;  /* (cached) graph height in pixels */
	gint grab_point;  /* point currently grabbed */
	gint last;

	 /* (cached) curve points: */
	gint num_points;
	GdkPoint *point;

	 /* control points: */
	gint num_ctlpoints;  /* number of control points */
	gfloat (*ctlpoint)[2];  /* array of control points */
} GtkCurve;


/* <gtk/gtkcurve.h> */
typedef struct _GtkCurveClass
{
	GtkDrawingAreaClass parent_class;

	void (* curve_type_changed) (GtkCurve *curve);
} GtkCurveClass;



/* <gtk/gtkcurve.h> */
GtkType gtk_curve_get_type(void)
{
}

/* <gtk/gtkcurve.h> */
GtkWidget* gtk_curve_new(void)
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_reset(GtkCurve *curve)
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_set_gamma(GtkCurve *curve, gfloat gamma)
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_set_range(GtkCurve *curve, gfloat min_x, gfloat max_x, gfloat min_y, gfloat max_y)
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_get_vector(GtkCurve *curve, int veclen, gfloat vector[])
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_set_vector(GtkCurve *curve, int veclen, gfloat vector[])
{
}

/* <gtk/gtkcurve.h> */
void gtk_curve_set_curve_type(GtkCurve *curve, GtkCurveType type)
{
}


/* <gtk/gtkdata.h> */
#define GTK_TYPE_DATA (gtk_data_get_type ())

/* <gtk/gtkdata.h> */
#define GTK_DATA(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_DATA, GtkData))

/* <gtk/gtkdata.h> */
#define GTK_DATA_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATA, GtkDataClass))

/* <gtk/gtkdata.h> */
#define GTK_IS_DATA(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_DATA))

/* <gtk/gtkdata.h> */
#define GTK_IS_DATA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATA))




/* <gtk/gtkdata.h> */
typedef struct _GtkData
{
	GtkObject object;
} GtkData;


/* <gtk/gtkdata.h> */
typedef struct _GtkDataClass
{
	GtkObjectClass parent_class;

	void (* disconnect) (GtkData *data);
} GtkDataClass;


/* <gtk/gtkdebug.h> */
typedef enum {
	GTK_DEBUG_OBJECTS = 1 << 0,
	GTK_DEBUG_MISC = 1 << 1,
	GTK_DEBUG_SIGNALS = 1 << 2,
	GTK_DEBUG_DND = 1 << 3,
	GTK_DEBUG_PLUGSOCKET = 1 << 4
} GtkDebugFlag;


/* <gtk/gtkdebug.h> */
#define GTK_NOTE(type, action)

/* <gtk/gtkdebug.h> */
extern guint gtk_debug_flags;


/* <gtk/gtkdialog.h> */
#define GTK_TYPE_DIALOG (gtk_dialog_get_type ())

/* <gtk/gtkdialog.h> */
#define GTK_DIALOG(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_DIALOG, GtkDialog))

/* <gtk/gtkdialog.h> */
#define GTK_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_DIALOG, GtkDialogClass))

/* <gtk/gtkdialog.h> */
#define GTK_IS_DIALOG(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_DIALOG))

/* <gtk/gtkdialog.h> */
#define GTK_IS_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DIALOG))



/* <gtk/gtkdialog.h> */
typedef struct _GtkDialog
{
	GtkWindow window;

	GtkWidget *vbox;
	GtkWidget *action_area;
} GtkDialog;


/* <gtk/gtkdialog.h> */
typedef struct _GtkDialogClass
{
	GtkWindowClass parent_class;
} GtkDialogClass;



/* <gtk/gtkdialog.h> */
GtkType gtk_dialog_get_type(void)
{
}

/* <gtk/gtkdialog.h> */
GtkWidget* gtk_dialog_new(void)
{
}


/* <gtk/gtkdnd.h> */
typedef enum {
	GTK_DEST_DEFAULT_MOTION = 1 << 0, /* respond to "drag_motion" */
	GTK_DEST_DEFAULT_HIGHLIGHT = 1 << 1, /* auto-highlight */
	GTK_DEST_DEFAULT_DROP = 1 << 2, /* respond to "drag_drop" */
	GTK_DEST_DEFAULT_ALL = 0x07
} GtkDestDefaults;

/* Flags for the GtkTargetEntry on the destination side 
 */

/* <gtk/gtkdnd.h> */
typedef enum {
	GTK_TARGET_SAME_APP = 1 << 0,  /*< nick=same-app >*/
	GTK_TARGET_SAME_WIDGET = 1 << 1  /*< nick=same-widget >*/
} GtkTargetFlags;

/* Destination side */


/* <gtk/gtkdnd.h> */
void gtk_drag_get_data(GtkWidget *widget, GdkDragContext *context, GdkAtom target, guint32 time)
{
}

/* <gtk/gtkdnd.h> */
void gtk_drag_finish(GdkDragContext *context, gboolean success, gboolean del, guint32 time)
{
}


/* <gtk/gtkdnd.h> */
GtkWidget *gtk_drag_get_source_widget(GdkDragContext *context)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_highlight(GtkWidget *widget)
{
}

/* <gtk/gtkdnd.h> */
void gtk_drag_unhighlight(GtkWidget *widget)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_dest_set(GtkWidget *widget, GtkDestDefaults flags, const GtkTargetEntry *targets, gint n_targets, GdkDragAction actions)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_dest_set_proxy(GtkWidget *widget, GdkWindow *proxy_window, GdkDragProtocol protocol, gboolean use_coordinates)
{
}

/* There probably should be functions for setting the targets
 * as a GtkTargetList
 */


/* <gtk/gtkdnd.h> */
void gtk_drag_dest_unset(GtkWidget *widget)
{
}

/* Source side */


/* <gtk/gtkdnd.h> */
void gtk_drag_source_set(GtkWidget *widget, GdkModifierType start_button_mask, const GtkTargetEntry *targets, gint n_targets, GdkDragAction actions)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_source_unset(GtkWidget *widget)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_source_set_icon(GtkWidget *widget, GdkColormap *colormap, GdkPixmap *pixmap, GdkBitmap *mask)
{
}

/* There probably should be functions for setting the targets
 * as a GtkTargetList
 */


/* <gtk/gtkdnd.h> */
GdkDragContext *gtk_drag_begin(GtkWidget *widget, GtkTargetList *targets, GdkDragAction actions, gint button, GdkEvent *event)
{
}

/* Set the image being dragged around
 */

/* <gtk/gtkdnd.h> */
void gtk_drag_set_icon_widget(GdkDragContext *context, GtkWidget *widget, gint hot_x, gint hot_y)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_set_icon_pixmap(GdkDragContext *context, GdkColormap *colormap, GdkPixmap *pixmap, GdkBitmap *mask, gint hot_x, gint hot_y)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_set_icon_default(GdkDragContext *context)
{
}


/* <gtk/gtkdnd.h> */
void gtk_drag_set_default_icon(GdkColormap *colormap, GdkPixmap *pixmap, GdkBitmap *mask, gint hot_x, gint hot_y)
{
}


/* Internal functions */

/* <gtk/gtkdnd.h> */
void gtk_drag_source_handle_event(GtkWidget *widget, GdkEvent *event)
{
}

/* <gtk/gtkdnd.h> */
void gtk_drag_dest_handle_event(GtkWidget *toplevel, GdkEvent *event)
{
}


/* <gtk/gtkdrawingarea.h> */
#define GTK_TYPE_DRAWING_AREA (gtk_drawing_area_get_type ())

/* <gtk/gtkdrawingarea.h> */
#define GTK_DRAWING_AREA(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_DRAWING_AREA, GtkDrawingArea))

/* <gtk/gtkdrawingarea.h> */
#define GTK_DRAWING_AREA_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_DRAWING_AREA, GtkDrawingAreaClass))

/* <gtk/gtkdrawingarea.h> */
#define GTK_IS_DRAWING_AREA(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_DRAWING_AREA))

/* <gtk/gtkdrawingarea.h> */
#define GTK_IS_DRAWING_AREA_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DRAWING_AREA))



/* <gtk/gtkdrawingarea.h> */
typedef struct _GtkDrawingArea
{
	GtkWidget widget;

	gpointer draw_data;
} GtkDrawingArea;


/* <gtk/gtkdrawingarea.h> */
typedef struct _GtkDrawingAreaClass
{
	GtkWidgetClass parent_class;
} GtkDrawingAreaClass;



/* <gtk/gtkdrawingarea.h> */
GtkType gtk_drawing_area_get_type(void)
{
}

/* <gtk/gtkdrawingarea.h> */
GtkWidget* gtk_drawing_area_new(void)
{
}

/* <gtk/gtkdrawingarea.h> */
void gtk_drawing_area_size(GtkDrawingArea *darea, gint width, gint height)
{
}


/* <gtk/gtkeditable.h> */
#define GTK_TYPE_EDITABLE (gtk_editable_get_type ())

/* <gtk/gtkeditable.h> */
#define GTK_EDITABLE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_EDITABLE, GtkEditable))

/* <gtk/gtkeditable.h> */
#define GTK_EDITABLE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_EDITABLE, GtkEditableClass))

/* <gtk/gtkeditable.h> */
#define GTK_IS_EDITABLE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_EDITABLE))

/* <gtk/gtkeditable.h> */
#define GTK_IS_EDITABLE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_EDITABLE))



/* <gtk/gtkeditable.h> */
typedef struct _GtkEditable
{
	GtkWidget widget;

	 /*< public >*/
	guint current_pos;

	guint selection_start_pos;
	guint selection_end_pos;
	guint has_selection : 1;

	 /*< private >*/
	guint editable : 1;
	guint visible : 1;
	GdkIC *ic;
	GdkICAttr *ic_attr;
	 
	gchar *clipboard_text;
} GtkEditable;


/* <gtk/gtkeditable.h> */
typedef struct _GtkEditableClass
{
	GtkWidgetClass parent_class;
	 
	 /* Signals for notification/filtering of changes */
	void (* changed) (GtkEditable *editable);
	void (* insert_text) (GtkEditable *editable, const gchar *text, gint length, gint *position);
	void (* delete_text) (GtkEditable *editable, gint start_pos, gint end_pos);

	 /* Bindings actions */
	void (* activate) (GtkEditable *editable);
	void (* set_editable) (GtkEditable *editable, gboolean is_editable);
	void (* move_cursor) (GtkEditable *editable, gint x, gint y);
	void (* move_word) (GtkEditable *editable, gint n);
	void (* move_page) (GtkEditable *editable, gint x, gint y);
	void (* move_to_row) (GtkEditable *editable, gint row);
	void (* move_to_column) (GtkEditable *editable, gint row);
	void (* kill_char) (GtkEditable *editable, gint direction);
	void (* kill_word) (GtkEditable *editable, gint direction);
	void (* kill_line) (GtkEditable *editable, gint direction);
	void (* cut_clipboard) (GtkEditable *editable);
	void (* copy_clipboard) (GtkEditable *editable);
	void (* paste_clipboard) (GtkEditable *editable);

	 /* Virtual functions. get_chars is in paricular not a signal because
 * it returns malloced memory. The others are not signals because
 * they would not be particularly useful as such. (All changes to
 * selection and position do not go through these functions)
 */
	void (* update_text) (GtkEditable *editable, gint start_pos, gint end_pos);
	gchar* (* get_chars) (GtkEditable *editable, gint start_pos, gint end_pos);
	void (* set_selection)(GtkEditable *editable, gint start_pos, gint end_pos);
	void (* set_position) (GtkEditable *editable, gint position);
} GtkEditableClass;


/* <gtk/gtkeditable.h> */
GtkType gtk_editable_get_type(void)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_select_region(GtkEditable *editable, gint start, gint end)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_insert_text(GtkEditable *editable, const gchar *new_text, gint new_text_length, gint *position)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_delete_text(GtkEditable *editable, gint start_pos, gint end_pos)
{
}

/* <gtk/gtkeditable.h> */
gchar* gtk_editable_get_chars(GtkEditable *editable, gint start_pos, gint end_pos)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_cut_clipboard(GtkEditable *editable)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_copy_clipboard(GtkEditable *editable)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_paste_clipboard(GtkEditable *editable)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_claim_selection(GtkEditable *editable, gboolean claim, guint32 time)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_delete_selection(GtkEditable *editable)
{
}


/* <gtk/gtkeditable.h> */
void gtk_editable_changed(GtkEditable *editable)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_set_position(GtkEditable *editable, gint position)
{
}

/* <gtk/gtkeditable.h> */
gint gtk_editable_get_position(GtkEditable *editable)
{
}

/* <gtk/gtkeditable.h> */
void gtk_editable_set_editable(GtkEditable *editable, gboolean is_editable)
{
}

/* <gtk/gtkentry.h> */
#define GTK_TYPE_ENTRY (gtk_entry_get_type ())

/* <gtk/gtkentry.h> */
#define GTK_ENTRY(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ENTRY, GtkEntry))

/* <gtk/gtkentry.h> */
#define GTK_ENTRY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ENTRY, GtkEntryClass))

/* <gtk/gtkentry.h> */
#define GTK_IS_ENTRY(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ENTRY))

/* <gtk/gtkentry.h> */
#define GTK_IS_ENTRY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ENTRY))



/* <gtk/gtkentry.h> */
typedef struct _GtkEntry
{
	GtkEditable editable;

	GdkWindow *text_area;
	GdkPixmap *backing_pixmap;
	GdkCursor *cursor;
	GdkWChar *text;

	guint16 text_size; /* allocated size */
	guint16 text_length; /* length in use */
	guint16 text_max_length;
	gint scroll_offset;
	guint visible : 1; /* deprecated - see editable->visible */
	guint32 timer;
	guint button;

	 /* The x-offset of each character (including the last insertion position)
 * only valid when the widget is realized */
	gint *char_offset;

	 /* Same as 'text', but in multibyte */
	gchar *text_mb;
	 /* If true, 'text' and 'text_mb' are not coherent */
	guint text_mb_dirty : 1;
	 /* If true, we use the encoding of wchar_t as the encoding of 'text'.
 * Otherwise we use the encoding of multi-byte characters instead. */
	guint use_wchar : 1;
} GtkEntry;


/* <gtk/gtkentry.h> */
typedef struct _GtkEntryClass
{
	GtkEditableClass parent_class;
} GtkEntryClass;


/* <gtk/gtkentry.h> */
GtkType gtk_entry_get_type(void)
{
}

/* <gtk/gtkentry.h> */
GtkWidget* gtk_entry_new(void)
{
}

/* <gtk/gtkentry.h> */
GtkWidget* gtk_entry_new_with_max_length(guint16 max)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_set_text(GtkEntry *entry, const gchar *text)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_append_text(GtkEntry *entry, const gchar *text)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_prepend_text(GtkEntry *entry, const gchar *text)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_set_position(GtkEntry *entry, gint position)
{
}
/* returns a reference to the text */

/* <gtk/gtkentry.h> */
gchar* gtk_entry_get_text(GtkEntry *entry)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_select_region(GtkEntry *entry, gint start, gint end)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_set_visibility(GtkEntry *entry, gboolean visible)
{
}

/* <gtk/gtkentry.h> */
void gtk_entry_set_editable(GtkEntry *entry, gboolean editable)
{
}
/* text is truncated if needed */

/* <gtk/gtkentry.h> */
void gtk_entry_set_max_length(GtkEntry *entry, guint16 max)
{
}

/* <gtk/gtkenums.h>  Arrow types*/
typedef enum
{
	GTK_ARROW_UP,
	GTK_ARROW_DOWN,
	GTK_ARROW_LEFT,
	GTK_ARROW_RIGHT
} GtkArrowType;

/* <gtk/gtkenums.h> Attach options (for tables) */
typedef enum
{
	GTK_EXPAND = 1 << 0,
	GTK_SHRINK = 1 << 1,
	GTK_FILL = 1 << 2
} GtkAttachOptions;

/* <gtk/gtkenums.h> Button box styles */
typedef enum 
{
	GTK_BUTTONBOX_DEFAULT_STYLE,
	GTK_BUTTONBOX_SPREAD,
	GTK_BUTTONBOX_EDGE,
	GTK_BUTTONBOX_START,
	GTK_BUTTONBOX_END
} GtkButtonBoxStyle;

/* <gtk/gtkenums.h> Curve types */
typedef enum
{
	GTK_CURVE_TYPE_LINEAR,  /* linear interpolation */
	GTK_CURVE_TYPE_SPLINE,  /* spline interpolation */
	GTK_CURVE_TYPE_FREE  /* free form curve */
} GtkCurveType;
	
/* <gtk/gtkenums.h> Focus movement types */
typedef enum
{
	GTK_DIR_TAB_FORWARD,
	GTK_DIR_TAB_BACKWARD,
	GTK_DIR_UP,
	GTK_DIR_DOWN,
	GTK_DIR_LEFT,
	GTK_DIR_RIGHT
} GtkDirectionType;

/* <gtk/gtkenums.h> justification for label and maybe other widgets (text?) */
typedef enum
{
	GTK_JUSTIFY_LEFT,
	GTK_JUSTIFY_RIGHT,
	GTK_JUSTIFY_CENTER,
	GTK_JUSTIFY_FILL
} GtkJustification;

/* <gtk/gtkenums.h> GtkPatternSpec match types */
typedef enum
{
	GTK_MATCH_ALL,  /* "*A?A*" */
	GTK_MATCH_ALL_TAIL,  /* "*A?AA" */
	GTK_MATCH_HEAD,  /* "AAAA*" */
	GTK_MATCH_TAIL,  /* "*AAAA" */
	GTK_MATCH_EXACT,  /* "AAAAA" */
	GTK_MATCH_LAST
} GtkMatchType;

/* <gtk/gtkenums.h> Menu keyboard movement types */
typedef enum
{
	GTK_MENU_DIR_PARENT,
	GTK_MENU_DIR_CHILD,
	GTK_MENU_DIR_NEXT,
	GTK_MENU_DIR_PREV
} GtkMenuDirectionType;


/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_MENU_FACTORY_MENU,
	GTK_MENU_FACTORY_MENU_BAR,
	GTK_MENU_FACTORY_OPTION_MENU
} GtkMenuFactoryType;


/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_PIXELS,
	GTK_INCHES,
	GTK_CENTIMETERS
} GtkMetricType;

/* <gtk/gtkenums.h> Orientation for toolbars, etc. */
typedef enum
{
	GTK_ORIENTATION_HORIZONTAL,
	GTK_ORIENTATION_VERTICAL
} GtkOrientation;

/* <gtk/gtkenums.h> Placement type for scrolled window */
typedef enum
{
	GTK_CORNER_TOP_LEFT,
	GTK_CORNER_BOTTOM_LEFT,
	GTK_CORNER_TOP_RIGHT,
	GTK_CORNER_BOTTOM_RIGHT
} GtkCornerType;

/* <gtk/gtkenums.h> Packing types (for boxes) */
typedef enum
{
	GTK_PACK_START,
	GTK_PACK_END
} GtkPackType;

/* <gtk/gtkenums.h> priorities for path lookups */
typedef enum
{
	GTK_PATH_PRIO_LOWEST = 0,
	GTK_PATH_PRIO_GTK = 4,
	GTK_PATH_PRIO_APPLICATION = 8,
	GTK_PATH_PRIO_RC = 12,
	GTK_PATH_PRIO_HIGHEST = 15,
	GTK_PATH_PRIO_MASK = 0x0f
} GtkPathPriorityType;

/* <gtk/gtkenums.h> widget path types */
typedef enum
{
	GTK_PATH_WIDGET,
	GTK_PATH_WIDGET_CLASS,
	GTK_PATH_CLASS
} GtkPathType;

/* <gtk/gtkenums.h> Scrollbar policy types (for scrolled windows) */
typedef enum
{
	GTK_POLICY_ALWAYS,
	GTK_POLICY_AUTOMATIC,
	GTK_POLICY_NEVER
} GtkPolicyType;


/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_POS_LEFT,
	GTK_POS_RIGHT,
	GTK_POS_TOP,
	GTK_POS_BOTTOM
} GtkPositionType;


/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_PREVIEW_COLOR,
	GTK_PREVIEW_GRAYSCALE
} GtkPreviewType;

/* Style for buttons */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_RELIEF_NORMAL,
	GTK_RELIEF_HALF,
	GTK_RELIEF_NONE
} GtkReliefStyle;

/* Resize type */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_RESIZE_PARENT, 	/* Pass resize request to the parent */
	GTK_RESIZE_QUEUE, 	/* Queue resizes on this widget */
	GTK_RESIZE_IMMEDIATE 	/* Perform the resizes now */
} GtkResizeMode;

/* signal run types */

/* <gtk/gtkenums.h> */
typedef enum  /*< flags >*/
{
	GTK_RUN_FIRST = 1 << 0,
	GTK_RUN_LAST = 1 << 1,
	GTK_RUN_BOTH = (GTK_RUN_FIRST | GTK_RUN_LAST),
	GTK_RUN_NO_RECURSE = 1 << 2,
	GTK_RUN_ACTION = 1 << 3,
	GTK_RUN_NO_HOOKS = 1 << 4
} GtkSignalRunType;

/* scrolling types */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_SCROLL_NONE,
	GTK_SCROLL_STEP_BACKWARD,
	GTK_SCROLL_STEP_FORWARD,
	GTK_SCROLL_PAGE_BACKWARD,
	GTK_SCROLL_PAGE_FORWARD,
	GTK_SCROLL_JUMP
} GtkScrollType;

/* list selection modes */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_SELECTION_SINGLE,
	GTK_SELECTION_BROWSE,
	GTK_SELECTION_MULTIPLE,
	GTK_SELECTION_EXTENDED
} GtkSelectionMode;

/* Shadow types */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_SHADOW_NONE,
	GTK_SHADOW_IN,
	GTK_SHADOW_OUT,
	GTK_SHADOW_ETCHED_IN,
	GTK_SHADOW_ETCHED_OUT
} GtkShadowType;

/* Widget states */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_STATE_NORMAL,
	GTK_STATE_ACTIVE,
	GTK_STATE_PRELIGHT,
	GTK_STATE_SELECTED,
	GTK_STATE_INSENSITIVE
} GtkStateType;

/* Directions for submenus */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_DIRECTION_LEFT,
	GTK_DIRECTION_RIGHT
} GtkSubmenuDirection;

/* Placement of submenus */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_TOP_BOTTOM,
	GTK_LEFT_RIGHT
} GtkSubmenuPlacement;

/* Style for toolbars */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_TOOLBAR_ICONS,
	GTK_TOOLBAR_TEXT,
	GTK_TOOLBAR_BOTH
} GtkToolbarStyle;

/* Trough types for GtkRange */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_TROUGH_NONE,
	GTK_TROUGH_START,
	GTK_TROUGH_END,
	GTK_TROUGH_JUMP
} GtkTroughType;

/* Data update types (for ranges) */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_UPDATE_CONTINUOUS,
	GTK_UPDATE_DISCONTINUOUS,
	GTK_UPDATE_DELAYED
} GtkUpdateType;

/* Generic visibility flags */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_VISIBILITY_NONE,
	GTK_VISIBILITY_PARTIAL,
	GTK_VISIBILITY_FULL
} GtkVisibility;

/* <gtk/gtkenums.h> Window position types */
typedef enum
{
	GTK_WIN_POS_NONE,
	GTK_WIN_POS_CENTER,
	GTK_WIN_POS_MOUSE,
	GTK_WIN_POS_CENTER_ALWAYS
} GtkWindowPosition;

/* Window types */

/* <gtk/gtkenums.h> */
typedef enum
{
	GTK_WINDOW_TOPLEVEL,
	GTK_WINDOW_DIALOG,
	GTK_WINDOW_POPUP
} GtkWindowType;

/* <gtk/gtkenums.h> How to sort */
typedef enum
{
	GTK_SORT_ASCENDING,
	GTK_SORT_DESCENDING
} GtkSortType;


/* <gtk/gtkeventbox.h> */
#define GTK_TYPE_EVENT_BOX (gtk_event_box_get_type ())

/* <gtk/gtkeventbox.h> */
#define GTK_EVENT_BOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_EVENT_BOX, GtkEventBox))

/* <gtk/gtkeventbox.h> */
#define GTK_EVENT_BOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_EVENT_BOX, GtkEventBoxClass))

/* <gtk/gtkeventbox.h> */
#define GTK_IS_EVENT_BOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_EVENT_BOX))

/* <gtk/gtkeventbox.h> */
#define GTK_IS_EVENT_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_EVENT_BOX))



/* <gtk/gtkeventbox.h> */
typedef struct _GtkEventBox
{
	GtkBin bin;
} GtkEventBox;


/* <gtk/gtkeventbox.h> */
typedef struct _GtkEventBoxClass
{
	GtkBinClass parent_class;
} GtkEventBoxClass;


/* <gtk/gtkeventbox.h> */
GtkType gtk_event_box_get_type(void)
{
}

/* <gtk/gtkeventbox.h> */
GtkWidget* gtk_event_box_new(void)
{
}

/* <gtk/gtkfeatures.h> */
#define GTK_MAJOR_VERSION (1)

/* <gtk/gtkfeatures.h> */
#define GTK_MINOR_VERSION (2)

/* <gtk/gtkfeatures.h> */
#define GTK_MICRO_VERSION (10)

/* <gtk/gtkfeatures.h> */
#define GTK_BINARY_AGE (10)

/* <gtk/gtkfeatures.h> */
#define GTK_INTERFACE_AGE (1)

/* <gtk/gtkfeatures.h> */
#define GTK_CHECK_VERSION(major,minor,micro) \
	(GTK_MAJOR_VERSION > (major) || \
	(GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION > (minor)) || \
	(GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION == (minor) && \
	GTK_MICRO_VERSION >= (micro)))


/* new gtk_container_set_focus_[hv]adjustment()
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_CONTAINER_FOCUS_ADJUSTMENTS 1-0-1

/* newly exported gtk_signal_init()
 * new gtk_signal_n_emissions*()
 * "signal-name" is now an alias for "signal_name"
 * new gtk_signal_emitv*()
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_SIGNAL_INIT 1-0-2
	 
/* Gtk+ 1.1.0 version tag.
 * - new gtk_rc_set_image_loader () to install custom image loaders for rc
 * files.
 * - GtkAccel groups replaced GtkAcceleratorTables
 * - Gdk supports full crossing event now.
 * - Buttons featur relief styles now.
 * - gdk_rgb_*() functions are in place.
 * - stringified enum values can be queried for enum types now.
 * - new key binding system is in place (GtkBindingSet).
 * - simple algorithm for pattern matching is exported now (GtkPatternSpec).
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_0 1-1-0

/* Gtk+ 1.1.2 version tag
 * - ctree function name changes
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_2 1-1-2

/* Gtk+ 1.1.4 version tag
 * - clist v/hscrollbar -> v/hadjustment changes
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_4 1-1-4

/* Gtk+ 1.1.5 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_5 1-1-5

/* Gtk+ 1.1.6 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_6 1-1-6

/* Gtk+ 1.1.7 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_7 1-1-7

/* Gtk+ 1.1.8 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_8 1-1-8

/* Gtk+ 1.1.9 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_9 1-1-9

/* Gtk+ 1.1.10 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_10 1-1-10

/* Gtk+ 1.1.11 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_11 1-1-11

/* Gtk+ 1.1.12 version tag
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_12 1-1-12

/* Gtk+ 1.1.13 version tag
 * gtk_toggle_button_set_state name changes
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_13 1-1-13

/* Gtk+ 1.1.14 version tag
 * working gtk layout, etc
 */

/* <gtk/gtkfeatures.h> */
#define GTK_HAVE_FEATURES_1_1_14 1-1-14


/* <gtk/gtkfilesel.h> */
#define GTK_TYPE_FILE_SELECTION (gtk_file_selection_get_type ())

/* <gtk/gtkfilesel.h> */
#define GTK_FILE_SELECTION(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FILE_SELECTION, GtkFileSelection))

/* <gtk/gtkfilesel.h> */
#define GTK_FILE_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FILE_SELECTION, GtkFileSelectionClass))

/* <gtk/gtkfilesel.h> */
#define GTK_IS_FILE_SELECTION(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FILE_SELECTION))

/* <gtk/gtkfilesel.h> */
#define GTK_IS_FILE_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_SELECTION))



/* <gtk/gtkfilesel.h> */
typedef struct _GtkFileSelection
{
	GtkWindow window;

	GtkWidget *dir_list;
	GtkWidget *file_list;
	GtkWidget *selection_entry;
	GtkWidget *selection_text;
	GtkWidget *main_vbox;
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
	GtkWidget *help_button;
	GtkWidget *history_pulldown;
	GtkWidget *history_menu;
	GList *history_list;
	GtkWidget *fileop_dialog;
	GtkWidget *fileop_entry;
	gchar *fileop_file;
	gpointer cmpl_state;
	 
	GtkWidget *fileop_c_dir;
	GtkWidget *fileop_del_file;
	GtkWidget *fileop_ren_file;
	 
	GtkWidget *button_area;
	GtkWidget *action_area;
	 
} GtkFileSelection;


/* <gtk/gtkfilesel.h> */
typedef struct _GtkFileSelectionClass
{
	GtkWindowClass parent_class;
} GtkFileSelectionClass;



/* <gtk/gtkfilesel.h> */
GtkType gtk_file_selection_get_type(void)
{
}

/* <gtk/gtkfilesel.h> */
GtkWidget* gtk_file_selection_new(const gchar *title)
{
}

/* <gtk/gtkfilesel.h> */
void gtk_file_selection_set_filename(GtkFileSelection *filesel, const gchar *filename)
{
}

/* <gtk/gtkfilesel.h> */
gchar* gtk_file_selection_get_filename(GtkFileSelection *filesel)
{
}

/* <gtk/gtkfilesel.h> */
void gtk_file_selection_complete(GtkFileSelection *filesel, const gchar *pattern)
{
}

/* <gtk/gtkfilesel.h> */
void gtk_file_selection_show_fileop_buttons(GtkFileSelection *filesel)
{
}

/* <gtk/gtkfilesel.h> */
void gtk_file_selection_hide_fileop_buttons(GtkFileSelection *filesel)
{
}


/* <gtk/gtkfixed.h> */
#define GTK_TYPE_FIXED (gtk_fixed_get_type ())

/* <gtk/gtkfixed.h> */
#define GTK_FIXED(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FIXED, GtkFixed))

/* <gtk/gtkfixed.h> */
#define GTK_FIXED_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FIXED, GtkFixedClass))

/* <gtk/gtkfixed.h> */
#define GTK_IS_FIXED(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FIXED))

/* <gtk/gtkfixed.h> */
#define GTK_IS_FIXED_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FIXED))



/* <gtk/gtkfixed.h> */
typedef struct _GtkFixed
{
	GtkContainer container;

	GList *children;
} GtkFixed;


/* <gtk/gtkfixed.h> */
typedef struct _GtkFixedClass
{
	GtkContainerClass parent_class;
} GtkFixedClass;


/* <gtk/gtkfixed.h> */
typedef struct _GtkFixedChild
{
	GtkWidget *widget;
	gint16 x;
	gint16 y;
} GtkFixedChild;



/* <gtk/gtkfixed.h> */
GtkType gtk_fixed_get_type(void)
{
}

/* <gtk/gtkfixed.h> */
GtkWidget* gtk_fixed_new(void)
{
}

/* <gtk/gtkfixed.h> */
void gtk_fixed_put(GtkFixed *fixed, GtkWidget *widget, gint16 x, gint16 y)
{
}

/* <gtk/gtkfixed.h> */
void gtk_fixed_move(GtkFixed *fixed, GtkWidget *widget, gint16 x, gint16 y)
{
}


/* <gtk/gtkfontsel.h> */
#define GTK_TYPE_FONT_SELECTION (gtk_font_selection_get_type ())

/* <gtk/gtkfontsel.h> */
#define GTK_FONT_SELECTION(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FONT_SELECTION, GtkFontSelection))

/* <gtk/gtkfontsel.h> */
#define GTK_FONT_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FONT_SELECTION, GtkFontSelectionClass))

/* <gtk/gtkfontsel.h> */
#define GTK_IS_FONT_SELECTION(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FONT_SELECTION))

/* <gtk/gtkfontsel.h> */
#define GTK_IS_FONT_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FONT_SELECTION))


/* <gtk/gtkfontsel.h> */
#define GTK_TYPE_FONT_SELECTION_DIALOG (gtk_font_selection_dialog_get_type ())

/* <gtk/gtkfontsel.h> */
#define GTK_FONT_SELECTION_DIALOG(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FONT_SELECTION_DIALOG, GtkFontSelectionDialog))

/* <gtk/gtkfontsel.h> */
#define GTK_FONT_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FONT_SELECTION_DIALOG, GtkFontSelectionDialogClass))

/* <gtk/gtkfontsel.h> */
#define GTK_IS_FONT_SELECTION_DIALOG(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FONT_SELECTION_DIALOG))

/* <gtk/gtkfontsel.h> */
#define GTK_IS_FONT_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FONT_SELECTION_DIALOG))


/* <gtk/gtkfontsel.h> */
#define GTK_NUM_FONT_PROPERTIES 6


/* <gtk/gtkfontsel.h> */
#define GTK_NUM_STYLE_PROPERTIES 5


/* Used to determine whether we are using point or pixel sizes. */

/* <gtk/gtkfontsel.h> */
typedef enum
{
	GTK_FONT_METRIC_PIXELS,
	GTK_FONT_METRIC_POINTS
} GtkFontMetricType;

/* Used for determining the type of a font style, and also for setting filters.
	These can be combined if a style has bitmaps and scalable fonts available.*/

/* <gtk/gtkfontsel.h> */
typedef enum
{
	GTK_FONT_BITMAP = 1 << 0,
	GTK_FONT_SCALABLE = 1 << 1,
	GTK_FONT_SCALABLE_BITMAP = 1 << 2,

	GTK_FONT_ALL = 0x07
} GtkFontType;

/* These are the two types of filter available - base and user. The base
	filter is set by the application and can't be changed by the user. */

/* <gtk/gtkfontsel.h> */
#define GTK_NUM_FONT_FILTERS 2

/* <gtk/gtkfontsel.h> */
typedef enum
{
	GTK_FONT_FILTER_BASE,
	GTK_FONT_FILTER_USER
} GtkFontFilterType;

/* These hold the arrays of current filter settings for each property.
	If nfilters is 0 then all values of the property are OK. If not the
	filters array contains the indexes of the valid property values. */

/* <gtk/gtkfontsel.h> */
typedef struct _GtkFontFilter
{
	gint font_type;
	guint16 *property_filters[GTK_NUM_FONT_PROPERTIES];
	guint16 property_nfilters[GTK_NUM_FONT_PROPERTIES];
} GtkFontFilter;



/* <gtk/gtkfontsel.h> */
typedef struct _GtkFontSelection
{
	GtkNotebook notebook;
	 
	 /* These are on the font page. */
	GtkWidget *main_vbox;
	GtkWidget *font_label;
	GtkWidget *font_entry;
	GtkWidget *font_clist;
	GtkWidget *font_style_entry;
	GtkWidget *font_style_clist;
	GtkWidget *size_entry;
	GtkWidget *size_clist;
	GtkWidget *pixels_button;
	GtkWidget *points_button;
	GtkWidget *filter_button;
	GtkWidget *preview_entry;
	GtkWidget *message_label;
	 
	 /* These are on the font info page. */
	GtkWidget *info_vbox;
	GtkWidget *info_clist;
	GtkWidget *requested_font_name;
	GtkWidget *actual_font_name;
	 
	 /* These are on the filter page. */
	GtkWidget *filter_vbox;
	GtkWidget *type_bitmaps_button;
	GtkWidget *type_scalable_button;
	GtkWidget *type_scaled_bitmaps_button;
	GtkWidget *filter_clists[GTK_NUM_FONT_PROPERTIES];
	 
	GdkFont *font;
	gint font_index;
	gint style;
	GtkFontMetricType metric;
	 /* The size is either in pixels or deci-points, depending on the metric. */
	gint size;
	 
	 /* This is the last size explicitly selected. When the user selects different
	fonts we try to find the nearest size to this. */
	gint selected_size;
	 
	 /* These are the current property settings. They are indexes into the
	strings in the GtkFontSelInfo properties array. */
	guint16 property_values[GTK_NUM_STYLE_PROPERTIES];
	 
	 /* These are the base and user font filters. */
	GtkFontFilter filters[GTK_NUM_FONT_FILTERS];
} GtkFontSelection;



/* <gtk/gtkfontsel.h> */
typedef struct _GtkFontSelectionClass
{
	GtkNotebookClass parent_class;
} GtkFontSelectionClass;



/* <gtk/gtkfontsel.h> */
typedef struct _GtkFontSelectionDialog
{
	GtkWindow window;
	 
	GtkWidget *fontsel;
	 
	GtkWidget *main_vbox;
	GtkWidget *action_area;
	GtkWidget *ok_button;
	 /* The 'Apply' button is not shown by default but you can show/hide it. */
	GtkWidget *apply_button;
	GtkWidget *cancel_button;
	 
	 /* If the user changes the width of the dialog, we turn auto-shrink off. */
	gint dialog_width;
	gboolean auto_resize;
} GtkFontSelectionDialog;


/* <gtk/gtkfontsel.h> */
typedef struct _GtkFontSelectionDialogClass
{
	GtkWindowClass parent_class;
} GtkFontSelectionDialogClass;



/*****************************************************************************
 * GtkFontSelection functions.
 * see the comments in the GtkFontSelectionDialog functions.
 *****************************************************************************/


/* <gtk/gtkfontsel.h> */
GtkType gtk_font_selection_get_type(void)
{
}

/* <gtk/gtkfontsel.h> */
GtkWidget* gtk_font_selection_new(void)
{
}

/* <gtk/gtkfontsel.h> */
gchar* gtk_font_selection_get_font_name(GtkFontSelection *fontsel)
{
}

/* <gtk/gtkfontsel.h> */
GdkFont* gtk_font_selection_get_font(GtkFontSelection *fontsel)
{
}

/* <gtk/gtkfontsel.h> */
gboolean gtk_font_selection_set_font_name(GtkFontSelection *fontsel, const gchar *fontname)
{
}

/* <gtk/gtkfontsel.h> */
void gtk_font_selection_set_filter(GtkFontSelection *fontsel, GtkFontFilterType filter_type, GtkFontType font_type, gchar **foundries, gchar **weights, gchar **slants, gchar **setwidths, gchar **spacings, gchar **charsets)
{
}

/* <gtk/gtkfontsel.h> */
gchar* gtk_font_selection_get_preview_text(GtkFontSelection *fontsel)
{
}

/* <gtk/gtkfontsel.h> */
void gtk_font_selection_set_preview_text(GtkFontSelection *fontsel, const gchar *text)
{
}



/*****************************************************************************
 * GtkFontSelectionDialog functions.
 * most of these functions simply call the corresponding function in the
 * GtkFontSelection.
 *****************************************************************************/


/* <gtk/gtkfontsel.h> */
GtkType gtk_font_selection_dialog_get_type(void)
{
}

/* <gtk/gtkfontsel.h> */
GtkWidget* gtk_font_selection_dialog_new(const gchar *title)
{
}

/* This returns the X Logical Font Description fontname, or NULL if no font
	is selected. Note that there is a slight possibility that the font might not
	have been loaded OK. You should call gtk_font_selection_dialog_get_font()
	to see if it has been loaded OK.
	You should g_free() the returned font name after you're done with it. */

/* <gtk/gtkfontsel.h> */
gchar* gtk_font_selection_dialog_get_font_name(GtkFontSelectionDialog *fsd)
{
}

/* This will return the current GdkFont, or NULL if none is selected or there
	was a problem loading it. Remember to use gdk_font_ref/unref() if you want
	to use the font (in a style, for example). */

/* <gtk/gtkfontsel.h> */
GdkFont* gtk_font_selection_dialog_get_font(GtkFontSelectionDialog *fsd)
{
}

/* This sets the currently displayed font. It should be a valid X Logical
	Font Description font name (anything else will be ignored), e.g.
	"-adobe-courier-bold-o-normal--25-*-*-*-*-*-*-*" 
	It returns TRUE on success. */

/* <gtk/gtkfontsel.h> */
gboolean gtk_font_selection_dialog_set_font_name(GtkFontSelectionDialog *fsd, const gchar *fontname)
{
}

/* This sets one of the font filters, to limit the fonts shown. The filter_type
	is GTK_FONT_FILTER_BASE or GTK_FONT_FILTER_USER. The font type is a
	combination of the bit flags GTK_FONT_BITMAP, GTK_FONT_SCALABLE and
	GTK_FONT_SCALABLE_BITMAP (or GTK_FONT_ALL for all font types).
	The foundries, weights etc. are arrays of strings containing property
	values, e.g. 'bold', 'demibold', and *MUST* finish with a NULL.
	Standard long names are also accepted, e.g. 'italic' instead of 'i'.

	e.g. to allow only fixed-width fonts ('char cell' or 'monospaced') to be
	selected use:

	gchar *spacings[] = { "c", "m", NULL };
	gtk_font_selection_dialog_set_filter (GTK_FONT_SELECTION_DIALOG (fontsel),
	GTK_FONT_FILTER_BASE, GTK_FONT_ALL,
	NULL, NULL, NULL, NULL, spacings, NULL);

	to allow only true scalable fonts to be selected use:

	gtk_font_selection_dialog_set_filter (GTK_FONT_SELECTION_DIALOG (fontsel),
	GTK_FONT_FILTER_BASE, GTK_FONT_SCALABLE,
	NULL, NULL, NULL, NULL, NULL, NULL);
*/

/* <gtk/gtkfontsel.h> */
void gtk_font_selection_dialog_set_filter(GtkFontSelectionDialog *fsd, GtkFontFilterType filter_type, GtkFontType font_type, gchar **foundries, gchar **weights, gchar **slants, gchar **setwidths, gchar **spacings, gchar **charsets)
{
}

/* This returns the text in the preview entry. You should copy the returned
	text if you need it. */

/* <gtk/gtkfontsel.h> */
gchar* gtk_font_selection_dialog_get_preview_text(GtkFontSelectionDialog *fsd)
{
}

/* This sets the text in the preview entry. It will be copied by the entry,
	so there's no need to g_strdup() it first. */

/* <gtk/gtkfontsel.h> */
void gtk_font_selection_dialog_set_preview_text(GtkFontSelectionDialog *fsd, const gchar *text)
{
}


/* <gtk/gtkframe.h> */
#define GTK_TYPE_FRAME (gtk_frame_get_type ())

/* <gtk/gtkframe.h> */
#define GTK_FRAME(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_FRAME, GtkFrame))

/* <gtk/gtkframe.h> */
#define GTK_FRAME_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FRAME, GtkFrameClass))

/* <gtk/gtkframe.h> */
#define GTK_IS_FRAME(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_FRAME))

/* <gtk/gtkframe.h> */
#define GTK_IS_FRAME_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FRAME))



/* <gtk/gtkframe.h> */
typedef struct _GtkFrame
{
	GtkBin bin;
	 
	gchar *label;
	gint16 shadow_type;
	gint16 label_width;
	gint16 label_height;
	gfloat label_xalign;
	gfloat label_yalign;
} GtkFrame;


/* <gtk/gtkframe.h> */
typedef struct _GtkFrameClass
{
	GtkBinClass parent_class;
} GtkFrameClass;



/* <gtk/gtkframe.h> */
GtkType gtk_frame_get_type(void)
{
}

/* <gtk/gtkframe.h> */
GtkWidget* gtk_frame_new(const gchar *label)
{
}

/* <gtk/gtkframe.h> */
void gtk_frame_set_label(GtkFrame *frame, const gchar *label)
{
}

/* <gtk/gtkframe.h> */
void gtk_frame_set_label_align(GtkFrame *frame, gfloat xalign, gfloat yalign)
{
}

/* <gtk/gtkframe.h> */
void gtk_frame_set_shadow_type(GtkFrame *frame, GtkShadowType type)
{
}


/* <gtk/gtkgamma.h> */
#define GTK_GAMMA_CURVE(obj) \
	GTK_CHECK_CAST (obj, gtk_gamma_curve_get_type (), GtkGammaCurve)

/* <gtk/gtkgamma.h> */
#define GTK_GAMMA_CURVE_CLASS(klass) \
	GTK_CHECK_CLASS_CAST (klass, gtk_gamma_curve_get_type, GtkGammaCurveClass)

/* <gtk/gtkgamma.h> */
#define GTK_IS_GAMMA_CURVE(obj) \
	GTK_CHECK_TYPE (obj, gtk_gamma_curve_get_type ())



/* <gtk/gtkgamma.h> */
typedef struct _GtkGammaCurve
{
	GtkVBox vbox;

	GtkWidget *table;
	GtkWidget *curve;
	GtkWidget *button[5]; /* spline, linear, free, gamma, reset */

	gfloat gamma;
	GtkWidget *gamma_dialog;
	GtkWidget *gamma_text;
} GtkGammaCurve;


/* <gtk/gtkgamma.h> */
typedef struct _GtkGammaCurveClass
{
	GtkVBoxClass parent_class;
} GtkGammaCurveClass;



/* <gtk/gtkgamma.h> */
guint gtk_gamma_curve_get_type(void)
{
}

/* <gtk/gtkgamma.h> */
GtkWidget* gtk_gamma_curve_new(void)
{
}


/* <gtk/gtkgc.h> */
GdkGC* gtk_gc_get(gint depth, GdkColormap *colormap, GdkGCValues *values, GdkGCValuesMask values_mask)
{
}

/* <gtk/gtkgc.h> */
void gtk_gc_release(GdkGC *gc)
{
}

/* <gtk/gtkhandlebox.h> */
#define GTK_TYPE_HANDLE_BOX (gtk_handle_box_get_type ())

/* <gtk/gtkhandlebox.h> */
#define GTK_HANDLE_BOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_HANDLE_BOX, GtkHandleBox))

/* <gtk/gtkhandlebox.h> */
#define GTK_HANDLE_BOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HANDLE_BOX, GtkHandleBoxClass))

/* <gtk/gtkhandlebox.h> */
#define GTK_IS_HANDLE_BOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_HANDLE_BOX))

/* <gtk/gtkhandlebox.h> */
#define GTK_IS_HANDLE_BOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HANDLE_BOX))



/* <gtk/gtkhandlebox.h> */
typedef struct _GtkHandleBox
{
	GtkBin bin;

	GdkWindow *bin_window; /* parent window for children */
	GdkWindow *float_window;
	GtkShadowType shadow_type;
	guint handle_position : 2;
	guint float_window_mapped : 1;
	guint child_detached : 1;
	guint in_drag : 1;
	guint shrink_on_detach : 1;

	gint snap_edge : 3; /* -1 == unset */
	 
	 /* Variables used during a drag
 */
	gint deskoff_x, deskoff_y; /* Offset between root relative coordinates
 * and deskrelative coordinates */
	GtkAllocation attach_allocation;
	GtkAllocation float_allocation;
} GtkHandleBox;


/* <gtk/gtkhandlebox.h> */
typedef struct _GtkHandleBoxClass
{
	GtkBinClass parent_class;

	void (*child_attached) (GtkHandleBox *handle_box, GtkWidget *child);
	void (*child_detached) (GtkHandleBox *handle_box, GtkWidget *child);
} GtkHandleBoxClass;



/* <gtk/gtkhandlebox.h> */
GtkType gtk_handle_box_get_type(void)
{
}

/* <gtk/gtkhandlebox.h> */
GtkWidget* gtk_handle_box_new(void)
{
}

/* <gtk/gtkhandlebox.h> */
void gtk_handle_box_set_shadow_type(GtkHandleBox *handle_box, GtkShadowType type)
{
}

/* <gtk/gtkhandlebox.h> */
void gtk_handle_box_set_handle_position(GtkHandleBox *handle_box, GtkPositionType position)
{
}

/* <gtk/gtkhandlebox.h> */
void gtk_handle_box_set_snap_edge(GtkHandleBox *handle_box, GtkPositionType edge)
{
}

/* <gtk/gtkhbbox.h> */
#define GTK_HBUTTON_BOX(obj) GTK_CHECK_CAST (obj, gtk_hbutton_box_get_type (), GtkHButtonBox)

/* <gtk/gtkhbbox.h> */
#define GTK_HBUTTON_BOX_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_hbutton_box_get_type (), GtkHButtonBoxClass)

/* <gtk/gtkhbbox.h> */
#define GTK_IS_HBUTTON_BOX(obj) GTK_CHECK_TYPE (obj, gtk_hbutton_box_get_type ())




/* <gtk/gtkhbbox.h> */
typedef struct _GtkHButtonBox
{
	GtkButtonBox button_box;
} GtkHButtonBox;


/* <gtk/gtkhbbox.h> */
typedef struct _GtkHButtonBoxClass
{
	GtkButtonBoxClass parent_class;
} GtkHButtonBoxClass;



/* <gtk/gtkhbbox.h> */
guint gtk_hbutton_box_get_type(void)
{
}

/* <gtk/gtkhbbox.h> */
GtkWidget *gtk_hbutton_box_new(void)
{
}

/* buttons can be added by gtk_container_add() */


/* <gtk/gtkhbbox.h> */
gint gtk_hbutton_box_get_spacing_default(void)
{
}

/* <gtk/gtkhbbox.h> */
GtkButtonBoxStyle gtk_hbutton_box_get_layout_default(void)
{
}


/* <gtk/gtkhbbox.h> */
void gtk_hbutton_box_set_spacing_default(gint spacing)
{
}

/* <gtk/gtkhbbox.h> */
void gtk_hbutton_box_set_layout_default(GtkButtonBoxStyle layout)
{
}

/* <gtk/gtkhbox.h> */
#define GTK_TYPE_HBOX (gtk_hbox_get_type ())

/* <gtk/gtkhbox.h> */
#define GTK_HBOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_HBOX, GtkHBox))

/* <gtk/gtkhbox.h> */
#define GTK_HBOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HBOX, GtkHBoxClass))

/* <gtk/gtkhbox.h> */
#define GTK_IS_HBOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_HBOX))

/* <gtk/gtkhbox.h> */
#define GTK_IS_HBOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HBOX))




/* <gtk/gtkhbox.h> */
typedef struct _GtkHBox
{
	GtkBox box;
} GtkHBox;


/* <gtk/gtkhbox.h> */
typedef struct _GtkHBoxClass
{
	GtkBoxClass parent_class;
} GtkHBoxClass;



/* <gtk/gtkhbox.h> */
GtkType gtk_hbox_get_type(void)
{
}

/* <gtk/gtkhbox.h> */
GtkWidget* gtk_hbox_new(gboolean homogeneous, gint spacing)
{
}

/* <gtk/gtkhpaned.h> */
#define GTK_HPANED(obj) GTK_CHECK_CAST (obj, gtk_hpaned_get_type (), GtkHPaned)

/* <gtk/gtkhpaned.h> */
#define GTK_HPANED_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_hpaned_get_type (), GtkHPanedClass)

/* <gtk/gtkhpaned.h> */
#define GTK_IS_HPANED(obj) GTK_CHECK_TYPE (obj, gtk_hpaned_get_type ())



/* <gtk/gtkhpaned.h> */
typedef struct _GtkHPaned
{
	GtkPaned paned;
} GtkHPaned;


/* <gtk/gtkhpaned.h> */
typedef struct _GtkHPanedClass
{
	GtkPanedClass parent_class;
} GtkHPanedClass;



/* <gtk/gtkhpaned.h> */
guint gtk_hpaned_get_type(void)
{
}

/* <gtk/gtkhpaned.h> */
GtkWidget* gtk_hpaned_new(void)
{
}


/* <gtk/gtkhruler.h> */
#define GTK_HRULER(obj) GTK_CHECK_CAST (obj, gtk_hruler_get_type (), GtkHRuler)

/* <gtk/gtkhruler.h> */
#define GTK_HRULER_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_hruler_get_type (), GtkHRulerClass)

/* <gtk/gtkhruler.h> */
#define GTK_IS_HRULER(obj) GTK_CHECK_TYPE (obj, gtk_hruler_get_type ())



/* <gtk/gtkhruler.h> */
typedef struct _GtkHRuler
{
	GtkRuler ruler;
} GtkHRuler;


/* <gtk/gtkhruler.h> */
typedef struct _GtkHRulerClass
{
	GtkRulerClass parent_class;
} GtkHRulerClass;



/* <gtk/gtkhruler.h> */
guint gtk_hruler_get_type(void)
{
}

/* <gtk/gtkhruler.h> */
GtkWidget* gtk_hruler_new(void)
{
}


/* <gtk/gtkhscale.h> */
#define GTK_TYPE_HSCALE (gtk_hscale_get_type ())

/* <gtk/gtkhscale.h> */
#define GTK_HSCALE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_HSCALE, GtkHScale))

/* <gtk/gtkhscale.h> */
#define GTK_HSCALE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HSCALE, GtkHScaleClass))

/* <gtk/gtkhscale.h> */
#define GTK_IS_HSCALE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_HSCALE))

/* <gtk/gtkhscale.h> */
#define GTK_IS_HSCALE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HSCALE))




/* <gtk/gtkhscale.h> */
typedef struct _GtkHScale
{
	GtkScale scale;
} GtkHScale;


/* <gtk/gtkhscale.h> */
typedef struct _GtkHScaleClass
{
	GtkScaleClass parent_class;
} GtkHScaleClass;



/* <gtk/gtkhscale.h> */
GtkType gtk_hscale_get_type(void)
{
}

/* <gtk/gtkhscale.h> */
GtkWidget* gtk_hscale_new(GtkAdjustment *adjustment)
{
}


/* <gtk/gtkhscrollbar.h> */
#define GTK_TYPE_HSCROLLBAR (gtk_hscrollbar_get_type ())

/* <gtk/gtkhscrollbar.h> */
#define GTK_HSCROLLBAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_HSCROLLBAR, GtkHScrollbar))

/* <gtk/gtkhscrollbar.h> */
#define GTK_HSCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HSCROLLBAR, GtkHScrollbarClass))

/* <gtk/gtkhscrollbar.h> */
#define GTK_IS_HSCROLLBAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_HSCROLLBAR))

/* <gtk/gtkhscrollbar.h> */
#define GTK_IS_HSCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HSCROLLBAR))



/* <gtk/gtkhscrollbar.h> */
typedef struct _GtkHScrollbar
{
	GtkScrollbar scrollbar;
} GtkHScrollbar;


/* <gtk/gtkhscrollbar.h> */
typedef struct _GtkHScrollbarClass
{
	GtkScrollbarClass parent_class;
} GtkHScrollbarClass;



/* <gtk/gtkhscrollbar.h> */
GtkType gtk_hscrollbar_get_type(void)
{
}

/* <gtk/gtkhscrollbar.h> */
GtkWidget* gtk_hscrollbar_new(GtkAdjustment *adjustment)
{
}


/* <gtk/gtkhseparator.h> */
#define GTK_TYPE_HSEPARATOR (gtk_hseparator_get_type ())

/* <gtk/gtkhseparator.h> */
#define GTK_HSEPARATOR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_HSEPARATOR, GtkHSeparator))

/* <gtk/gtkhseparator.h> */
#define GTK_HSEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_HSEPARATOR, GtkHSeparatorClass))

/* <gtk/gtkhseparator.h> */
#define GTK_IS_HSEPARATOR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_HSEPARATOR))

/* <gtk/gtkhseparator.h> */
#define GTK_IS_HSEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HSEPARATOR))



/* <gtk/gtkhseparator.h> */
typedef struct _GtkHSeparator
{
	GtkSeparator separator;
} GtkHSeparator;


/* <gtk/gtkhseparator.h> */
typedef struct _GtkHSeparatorClass
{
	GtkSeparatorClass parent_class;
} GtkHSeparatorClass;



/* <gtk/gtkhseparator.h> */
GtkType gtk_hseparator_get_type(void)
{
}

/* <gtk/gtkhseparator.h> */
GtkWidget* gtk_hseparator_new(void)
{
}


/* <gtk/gtkimage.h> */
#define GTK_TYPE_IMAGE (gtk_image_get_type ())

/* <gtk/gtkimage.h> */
#define GTK_IMAGE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_IMAGE, GtkImage))

/* <gtk/gtkimage.h> */
#define GTK_IMAGE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_IMAGE, GtkImageClass))

/* <gtk/gtkimage.h> */
#define GTK_IS_IMAGE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_IMAGE))

/* <gtk/gtkimage.h> */
#define GTK_IS_IMAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_IMAGE))



/* <gtk/gtkimage.h> */
typedef struct _GtkImage
{
	GtkMisc misc;

	GdkImage *image;
	GdkBitmap *mask;
} GtkImage;


/* <gtk/gtkimage.h> */
typedef struct _GtkImageClass
{
	GtkMiscClass parent_class;
} GtkImageClass;



/* <gtk/gtkimage.h> */
GtkType gtk_image_get_type(void)
{
}

/* <gtk/gtkimage.h> */
GtkWidget* gtk_image_new(GdkImage *val, GdkBitmap *mask)
{
}

/* <gtk/gtkimage.h> */
void gtk_image_set(GtkImage *image, GdkImage *val, GdkBitmap *mask)
{
}

/* <gtk/gtkimage.h> */
void gtk_image_get(GtkImage *image, GdkImage **val, GdkBitmap **mask)
{
}



/* <gtk/gtkinputdialog.h> */
#define GTK_TYPE_INPUT_DIALOG (gtk_input_dialog_get_type ())

/* <gtk/gtkinputdialog.h> */
#define GTK_INPUT_DIALOG(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_INPUT_DIALOG, GtkInputDialog))

/* <gtk/gtkinputdialog.h> */
#define GTK_INPUT_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_INPUT_DIALOG, GtkInputDialogClass))

/* <gtk/gtkinputdialog.h> */
#define GTK_IS_INPUT_DIALOG(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_INPUT_DIALOG))

/* <gtk/gtkinputdialog.h> */
#define GTK_IS_INPUT_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_INPUT_DIALOG))



/* <gtk/gtkinputdialog.h> */
typedef struct _GtkInputDialog
{
	GtkDialog dialog;

	GtkWidget *axis_list;
	GtkWidget *axis_listbox;
	GtkWidget *mode_optionmenu;

	GtkWidget *close_button;
	GtkWidget *save_button;
	 
	GtkWidget *axis_items[GDK_AXIS_LAST];
	guint32 current_device;

	GtkWidget *keys_list;
	GtkWidget *keys_listbox;
} GtkInputDialog;


/* <gtk/gtkinputdialog.h> */
typedef struct _GtkInputDialogClass
{
	GtkWindowClass parent_class;

	void (* enable_device) (GtkInputDialog *inputd, guint32 devid);
	void (* disable_device) (GtkInputDialog *inputd, guint32 devid);
} GtkInputDialogClass;



/* <gtk/gtkinputdialog.h> */
GtkType gtk_input_dialog_get_type(void)
{
}

/* <gtk/gtkinputdialog.h> */
GtkWidget* gtk_input_dialog_new(void)
{
}


/* <gtk/gtkinvisible.h> */
#define GTK_TYPE_INVISIBLE (gtk_invisible_get_type ())

/* <gtk/gtkinvisible.h> */
#define GTK_INVISIBLE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_INVISIBLE, GtkInvisible))

/* <gtk/gtkinvisible.h> */
#define GTK_INVISIBLE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_INVISIBLE, GtkInvisibleClass))

/* <gtk/gtkinvisible.h> */
#define GTK_IS_INVISIBLE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_INVISIBLE))

/* <gtk/gtkinvisible.h> */
#define GTK_IS_INVISIBLE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_INVISIBLE))



/* <gtk/gtkinvisible.h> */
typedef struct _GtkInvisible
{
	GtkBin bin;
} GtkInvisible;


/* <gtk/gtkinvisible.h> */
typedef struct _GtkInvisibleClass
{
	GtkBinClass parent_class;
} GtkInvisibleClass;


/* <gtk/gtkinvisible.h> */
GtkType gtk_invisible_get_type(void)
{
}

/* <gtk/gtkinvisible.h> */
GtkWidget* gtk_invisible_new(void)
{
}


/* <gtk/gtkitemfactory.h> */
typedef void(*GtkPrintFunc) (gpointer func_data, gchar *str)
{
}

/* <gtk/gtkitemfactory.h> */
typedef gchar * (*GtkTranslateFunc) (const gchar *path, gpointer func_data)
{
}

/* <gtk/gtkitemfactory.h> */
typedef void(*GtkItemFactoryCallback) ()
{
}

/* <gtk/gtkitemfactory.h> */
typedef void(*GtkItemFactoryCallback1) (gpointer callback_data, guint callback_action, GtkWidget *widget)
{
}


/* <gtk/gtkitemfactory.h> */
#define GTK_TYPE_ITEM_FACTORY (gtk_item_factory_get_type ())

/* <gtk/gtkitemfactory.h> */
#define GTK_ITEM_FACTORY(object) (GTK_CHECK_CAST (object, GTK_TYPE_ITEM_FACTORY, GtkItemFactory))

/* <gtk/gtkitemfactory.h> */
#define GTK_ITEM_FACTORY_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_ITEM_FACTORY, GtkItemFactoryClass))

/* <gtk/gtkitemfactory.h> */
#define GTK_IS_ITEM_FACTORY(object) (GTK_CHECK_TYPE (object, GTK_TYPE_ITEM_FACTORY))

/* <gtk/gtkitemfactory.h> */
#define GTK_IS_ITEM_FACTORY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ITEM_FACTORY))




/* <gtk/gtkitemfactory.h> */
typedef struct _GtkItemFactory
{
	GtkObject object;

	gchar *path;
	GtkAccelGroup *accel_group;
	GtkWidget *widget;
	GSList *items;

	GtkTranslateFunc translate_func;
	gpointer translate_data;
	GtkDestroyNotify translate_notify;  
} GtkItemFactory;


/* <gtk/gtkitemfactory.h> */
typedef struct _GtkItemFactoryClass
{
	GtkObjectClass object_class;

	gchar *cpair_comment_single;

	GHashTable *item_ht;

	gpointer dummy;
} GtkItemFactoryClass;


/* <gtk/gtkitemfactory.h> */
typedef struct _GtkItemFactoryEntry
{
	gchar *path;
	gchar *accelerator;

	GtkItemFactoryCallback callback;
	guint callback_action;

	 /* possible values:
 * NULL -> "<Item>"
 * "" -> "<Item>"
 * "<Title>" -> create a title item
 * "<Item>" -> create a simple item
 * "<CheckItem>" -> create a check item
 * "<ToggleItem>" -> create a toggle item
 * "<RadioItem>" -> create a radio item
 * <path> -> path of a radio item to link against
 * "<Separator>" -> create a separator
 * "<Branch>" -> create an item to hold sub items
 * "<LastBranch>" -> create a right justified item to hold sub items
 */
	gchar *item_type;
} GtkItemFactoryEntry;


/* <gtk/gtkitemfactory.h> */
typedef struct _GtkItemFactoryItem
{
	gchar *path;
	guint accelerator_key;
	guint accelerator_mods;
	guint modified : 1;
	guint in_propagation : 1;
	gchar *dummy;

	GSList *widgets;
} GtkItemFactoryItem;



/* <gtk/gtkitemfactory.h> */
GtkType gtk_item_factory_get_type(void)
{
}

/* `container_type' must be of GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
 * or GTK_TYPE_OPTION_MENU.
 */

/* <gtk/gtkitemfactory.h> */
GtkItemFactory* gtk_item_factory_new(GtkType container_type, const gchar *path, GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_construct(GtkItemFactory *ifactory, GtkType container_type, const gchar *path, GtkAccelGroup *accel_group)
{
}
	 
/* These functions operate on GtkItemFactoryClass basis.
 */

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_parse_rc(const gchar *file_name)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_parse_rc_string(const gchar *rc_string)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_parse_rc_scanner(GScanner *scanner)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_add_foreign(GtkWidget *accel_widget, const gchar *full_path, GtkAccelGroup *accel_group, guint keyval, GdkModifierType modifiers)
{
}
	 

/* <gtk/gtkitemfactory.h> */
GtkItemFactory* gtk_item_factory_from_widget(GtkWidget *widget)
{
}

/* <gtk/gtkitemfactory.h> */
gchar* gtk_item_factory_path_from_widget(GtkWidget *widget)
{
}


/* <gtk/gtkitemfactory.h> */
GtkWidget* gtk_item_factory_get_item(GtkItemFactory *ifactory, const gchar *path)
{
}

/* <gtk/gtkitemfactory.h> */
GtkWidget* gtk_item_factory_get_widget(GtkItemFactory *ifactory, const gchar *path)
{
}

/* <gtk/gtkitemfactory.h> */
GtkWidget* gtk_item_factory_get_widget_by_action(GtkItemFactory *ifactory, guint action)
{
}

/* <gtk/gtkitemfactory.h> */
GtkWidget* gtk_item_factory_get_item_by_action(GtkItemFactory *ifactory, guint action)
{
}

/* If `path_pspec' is passed as `NULL', this function will iterate over
 * all hash entries. otherwise only those entries will be dumped for which
 * the pattern matches, e.g. "<Image>*...".
 */

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_dump_items(GtkPatternSpec *path_pspec, gboolean modified_only, GtkPrintFunc print_func, gpointer func_data)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_dump_rc(const gchar *file_name, GtkPatternSpec *path_pspec, gboolean modified_only)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_print_func(gpointer FILE_pointer, gchar *string)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_create_item(GtkItemFactory *ifactory, GtkItemFactoryEntry *entry, gpointer callback_data, guint callback_type)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_create_items(GtkItemFactory *ifactory, guint n_entries, GtkItemFactoryEntry *entries, gpointer callback_data)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_delete_item(GtkItemFactory *ifactory, const gchar *path)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_delete_entry(GtkItemFactory *ifactory, GtkItemFactoryEntry *entry)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_delete_entries(GtkItemFactory *ifactory, guint n_entries, GtkItemFactoryEntry *entries)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_popup(GtkItemFactory *ifactory, guint x, guint y, guint mouse_button, guint32 time)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_popup_with_data(GtkItemFactory *ifactory, gpointer popup_data, GtkDestroyNotify destroy, guint x, guint y, guint mouse_button, guint32 time)
{
}

/* <gtk/gtkitemfactory.h> */
gpointer gtk_item_factory_popup_data(GtkItemFactory *ifactory)
{
}

/* <gtk/gtkitemfactory.h> */
gpointer gtk_item_factory_popup_data_from_widget(GtkWidget *widget)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_set_translate_func(GtkItemFactory *ifactory, GtkTranslateFunc func, gpointer data, GtkDestroyNotify notify)
{
}

/* Compatibility functions for deprecated GtkMenuFactory code
 */

/* <gtk/gtkitemfactory.h> */
GtkItemFactory* gtk_item_factory_from_path(const gchar *path)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_create_menu_entries(guint n_entries, GtkMenuEntry *entries)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factories_path_delete(const gchar *ifactory_path, const gchar *path)
{
}

/* <gtk/gtkitemfactory.h> */
typedef void(*GtkItemFactoryCallback2) (GtkWidget *widget, gpointer callback_data, guint callback_action)
{
}

/* <gtk/gtkitemfactory.h> */
void gtk_item_factory_create_items_ac(GtkItemFactory *ifactory, guint n_entries, GtkItemFactoryEntry *entries, gpointer callback_data, guint callback_type)
{
}


/* <gtk/gtkitem.h> */
#define GTK_TYPE_ITEM (gtk_item_get_type ())

/* <gtk/gtkitem.h> */
#define GTK_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_ITEM, GtkItem))

/* <gtk/gtkitem.h> */
#define GTK_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ITEM, GtkItemClass))

/* <gtk/gtkitem.h> */
#define GTK_IS_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_ITEM))

/* <gtk/gtkitem.h> */
#define GTK_IS_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ITEM))



/* <gtk/gtkitem.h> */
typedef struct _GtkItem
{
	GtkBin bin;
} GtkItem;


/* <gtk/gtkitem.h> */
typedef struct _GtkItemClass
{
	GtkBinClass parent_class;

	void (* select) (GtkItem *item);
	void (* deselect) (GtkItem *item);
	void (* toggle) (GtkItem *item);
} GtkItemClass;



/* <gtk/gtkitem.h> */
GtkType gtk_item_get_type(void)
{
}

/* <gtk/gtkitem.h> */
void gtk_item_select(GtkItem *item)
{
}

/* <gtk/gtkitem.h> */
void gtk_item_deselect(GtkItem *item)
{
}

/* <gtk/gtkitem.h> */
void gtk_item_toggle(GtkItem *item)
{
}


/* <gtk/gtklabel.h> */
#define GTK_TYPE_LABEL (gtk_label_get_type ())

/* <gtk/gtklabel.h> */
#define GTK_LABEL(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_LABEL, GtkLabel))

/* <gtk/gtklabel.h> */
#define GTK_LABEL_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_LABEL, GtkLabelClass))

/* <gtk/gtklabel.h> */
#define GTK_IS_LABEL(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_LABEL))

/* <gtk/gtklabel.h> */
#define GTK_IS_LABEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LABEL))



/* <gtk/gtklabel.h> */
typedef struct _GtkLabel
{
	GtkMisc misc;

	gchar *label;
	GdkWChar *label_wc;
	gchar *pattern;

	GtkLabelWord *words;

	guint max_width : 16;
	guint jtype : 2;
	gboolean wrap;
} GtkLabel;


/* <gtk/gtklabel.h> */
typedef struct _GtkLabelClass
{
	GtkMiscClass parent_class;
} GtkLabelClass;



/* <gtk/gtklabel.h> */
GtkType gtk_label_get_type(void)
{
}

/* <gtk/gtklabel.h> */
GtkWidget* gtk_label_new(const gchar *str)
{
}

/* <gtk/gtklabel.h> */
void gtk_label_set_text(GtkLabel *label, const gchar *str)
{
}

/* <gtk/gtklabel.h> */
void gtk_label_set_justify(GtkLabel *label, GtkJustification jtype)
{
}

/* <gtk/gtklabel.h> */
void gtk_label_set_pattern(GtkLabel *label, const gchar *pattern)
{
}

/* <gtk/gtklabel.h> */
void gtk_label_set_line_wrap(GtkLabel *label, gboolean wrap)
{
}

/* <gtk/gtklabel.h> */
void gtk_label_get(GtkLabel *label, gchar **str)
{
}

/* Convenience function to set the name and pattern by parsing
 * a string with embedded underscores, and return the appropriate
 * key symbol for the accelerator.
 */

/* <gtk/gtklabel.h> */
guint gtk_label_parse_uline(GtkLabel *label, const gchar *string)
{
}


/* <gtk/gtklayout.h> */
#define GTK_TYPE_LAYOUT (gtk_layout_get_type ())

/* <gtk/gtklayout.h> */
#define GTK_LAYOUT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_LAYOUT, GtkLayout))

/* <gtk/gtklayout.h> */
#define GTK_LAYOUT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_LAYOUT, GtkLayoutClass))

/* <gtk/gtklayout.h> */
#define GTK_IS_LAYOUT(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_LAYOUT))

/* <gtk/gtklayout.h> */
#define GTK_IS_LAYOUT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LAYOUT))


/* <gtk/gtklayout.h> */
typedef struct _GtkLayout
{
	GtkContainer container;

	GList *children;

	guint width;
	guint height;

	guint xoffset;
	guint yoffset;

	GtkAdjustment *hadjustment;
	GtkAdjustment *vadjustment;
	 
	GdkWindow *bin_window;

	GdkVisibilityState visibility;
	gulong configure_serial;
	gint scroll_x;
	gint scroll_y;

	guint freeze_count;
} GtkLayout;


/* <gtk/gtklayout.h> */
typedef struct _GtkLayoutClass
{
	GtkContainerClass parent_class;

	void (*set_scroll_adjustments) (GtkLayout *layout, GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);
} GtkLayoutClass;


/* <gtk/gtklayout.h> */
GtkType gtk_layout_get_type(void)
{
}

/* <gtk/gtklayout.h> */
GtkWidget* gtk_layout_new(GtkAdjustment *hadjustment, GtkAdjustment *vadjustment)
{
}

/* <gtk/gtklayout.h> */
void gtk_layout_put(GtkLayout *layout, GtkWidget *widget, gint x, gint y)
{
}
	 

/* <gtk/gtklayout.h> */
void gtk_layout_move(GtkLayout *layout, GtkWidget *widget, gint x, gint y)
{
}
	 

/* <gtk/gtklayout.h> */
void gtk_layout_set_size(GtkLayout *layout, guint width, guint height)
{
}


/* <gtk/gtklayout.h> */
GtkAdjustment* gtk_layout_get_hadjustment(GtkLayout *layout)
{
}

/* <gtk/gtklayout.h> */
GtkAdjustment* gtk_layout_get_vadjustment(GtkLayout *layout)
{
}

/* <gtk/gtklayout.h> */
void gtk_layout_set_hadjustment(GtkLayout *layout, GtkAdjustment *adjustment)
{
}

/* <gtk/gtklayout.h> */
void gtk_layout_set_vadjustment(GtkLayout *layout, GtkAdjustment *adjustment)
{
}

/* These disable and enable moving and repainting the scrolling window
 * of the GtkLayout, respectively. If you want to update the layout's
 * offsets but do not want it to repaint itself, you should use these
 * functions.
 *
 * - I don't understand these are supposed to work, so I suspect
 * - they don't now. OWT 1/20/98
 */

/* <gtk/gtklayout.h> */
void gtk_layout_freeze(GtkLayout *layout)
{
}

/* <gtk/gtklayout.h> */
void gtk_layout_thaw(GtkLayout *layout)
{
}

/* <gtk/gtklist.h> */
#define GTK_TYPE_LIST (gtk_list_get_type ())

/* <gtk/gtklist.h> */
#define GTK_LIST(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_LIST, GtkList))

/* <gtk/gtklist.h> */
#define GTK_LIST_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_LIST, GtkListClass))

/* <gtk/gtklist.h> */
#define GTK_IS_LIST(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_LIST))

/* <gtk/gtklist.h> */
#define GTK_IS_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LIST))



/* <gtk/gtklist.h> */
typedef struct _GtkList
{
	GtkContainer container;

	GList *children;
	GList *selection;

	GList *undo_selection;
	GList *undo_unselection;

	GtkWidget *last_focus_child;
	GtkWidget *undo_focus_child;

	guint htimer;
	guint vtimer;

	gint anchor;
	gint drag_pos;
	GtkStateType anchor_state;

	guint selection_mode : 2;
	guint drag_selection:1;
	guint add_mode:1;
} GtkList;


/* <gtk/gtklist.h> */
typedef struct _GtkListClass
{
	GtkContainerClass parent_class;

	void (* selection_changed) (GtkList *list);
	void (* select_child) (GtkList *list, GtkWidget *child);
	void (* unselect_child) (GtkList *list, GtkWidget *child);
} GtkListClass;



/* <gtk/gtklist.h> */
GtkType gtk_list_get_type(void)
{
}

/* <gtk/gtklist.h> */
GtkWidget* gtk_list_new(void)
{
}

/* <gtk/gtklist.h> */
void gtk_list_insert_items(GtkList *list, GList *items, gint position)
{
}

/* <gtk/gtklist.h> */
void gtk_list_append_items(GtkList *list, GList *items)
{
}

/* <gtk/gtklist.h> */
void gtk_list_prepend_items(GtkList *list, GList *items)
{
}

/* <gtk/gtklist.h> */
void gtk_list_remove_items(GtkList *list, GList *items)
{
}

/* <gtk/gtklist.h> */
void gtk_list_remove_items_no_unref(GtkList *list, GList *items)
{
}

/* <gtk/gtklist.h> */
void gtk_list_clear_items(GtkList *list, gint start, gint end)
{
}

/* <gtk/gtklist.h> */
void gtk_list_select_item(GtkList *list, gint item)
{
}

/* <gtk/gtklist.h> */
void gtk_list_unselect_item(GtkList *list, gint item)
{
}

/* <gtk/gtklist.h> */
void gtk_list_select_child(GtkList *list, GtkWidget *child)
{
}

/* <gtk/gtklist.h> */
void gtk_list_unselect_child(GtkList *list, GtkWidget *child)
{
}

/* <gtk/gtklist.h> */
gint gtk_list_child_position(GtkList *list, GtkWidget *child)
{
}

/* <gtk/gtklist.h> */
void gtk_list_set_selection_mode(GtkList *list, GtkSelectionMode mode)
{
}


/* <gtk/gtklist.h> */
void gtk_list_extend_selection(GtkList *list, GtkScrollType scroll_type, gfloat position, gboolean auto_start_selection)
{
}

/* <gtk/gtklist.h> */
void gtk_list_start_selection(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_end_selection(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_select_all(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_unselect_all(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_scroll_horizontal(GtkList *list, GtkScrollType scroll_type, gfloat position)
{
}

/* <gtk/gtklist.h> */
void gtk_list_scroll_vertical(GtkList *list, GtkScrollType scroll_type, gfloat position)
{
}

/* <gtk/gtklist.h> */
void gtk_list_toggle_add_mode(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_toggle_focus_row(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_toggle_row(GtkList *list, GtkWidget *item)
{
}

/* <gtk/gtklist.h> */
void gtk_list_undo_selection(GtkList *list)
{
}

/* <gtk/gtklist.h> */
void gtk_list_end_drag_selection(GtkList *list)
{
}


/* <gtk/gtklistitem.h> */
#define GTK_TYPE_LIST_ITEM (gtk_list_item_get_type ())

/* <gtk/gtklistitem.h> */
#define GTK_LIST_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_LIST_ITEM, GtkListItem))

/* <gtk/gtklistitem.h> */
#define GTK_LIST_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_LIST_ITEM, GtkListItemClass))

/* <gtk/gtklistitem.h> */
#define GTK_IS_LIST_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_LIST_ITEM))

/* <gtk/gtklistitem.h> */
#define GTK_IS_LIST_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_LIST_ITEM))




/* <gtk/gtklistitem.h> */
typedef struct _GtkListItem
{
	GtkItem item;
} GtkListItem;


/* <gtk/gtklistitem.h> */
typedef struct _GtkListItemClass
{
	GtkItemClass parent_class;

	void (*toggle_focus_row) (GtkListItem *list_item);
	void (*select_all) (GtkListItem *list_item);
	void (*unselect_all) (GtkListItem *list_item);
	void (*undo_selection) (GtkListItem *list_item);
	void (*start_selection) (GtkListItem *list_item);
	void (*end_selection) (GtkListItem *list_item);
	void (*extend_selection) (GtkListItem *list_item, GtkScrollType scroll_type, gfloat position, gboolean auto_start_selection);
	void (*scroll_horizontal) (GtkListItem *list_item, GtkScrollType scroll_type, gfloat position);
	void (*scroll_vertical) (GtkListItem *list_item, GtkScrollType scroll_type, gfloat position);
	void (*toggle_add_mode) (GtkListItem *list_item);
} GtkListItemClass;



/* <gtk/gtklistitem.h> */
GtkType gtk_list_item_get_type(void)
{
}

/* <gtk/gtklistitem.h> */
GtkWidget* gtk_list_item_new(void)
{
}

/* <gtk/gtklistitem.h> */
GtkWidget* gtk_list_item_new_with_label(const gchar *label)
{
}

/* <gtk/gtklistitem.h> */
void gtk_list_item_select(GtkListItem *list_item)
{
}

/* <gtk/gtklistitem.h> */
void gtk_list_item_deselect(GtkListItem *list_item)
{
}


/* Priorities for redrawing and resizing
 */

/* <gtk/gtkmain.h> */
#define GTK_PRIORITY_REDRAW (G_PRIORITY_HIGH_IDLE + 20)

/* <gtk/gtkmain.h> */
#define GTK_PRIORITY_RESIZE (G_PRIORITY_HIGH_IDLE + 10)

/* Deprecated. Use G_PRIORITY #define's instead
 */

/* <gtk/gtkmain.h> (Deprecated) */
#define GTK_PRIORITY_HIGH G_PRIORITY_HIGH

/* <gtk/gtkmain.h> (Deprecated) */
#define GTK_PRIORITY_INTERNAL GTK_PRIORITY_REDRAW

/* <gtk/gtkmain.h> (Deprecated) */
#define GTK_PRIORITY_DEFAULT G_PRIORITY_DEFAULT_IDLE

/* <gtk/gtkmain.h> (Deprecated) */
#define GTK_PRIORITY_LOW G_PRIORITY_LOW


/* <gtk/gtkmain.h> */
typedef void(*GtkModuleInitFunc) (gint *argc, gchar ***argv)
{
}

/* <gtk/gtkmain.h> */
typedef gint(*GtkKeySnoopFunc) (GtkWidget *grab_widget, GdkEventKey *event, gpointer func_data)
{
}

/* Gtk version.
 */

/* <gtk/gtkmain.h> */
extern const guint gtk_major_version;

/* <gtk/gtkmain.h> */
extern const guint gtk_minor_version;

/* <gtk/gtkmain.h> */
extern const guint gtk_micro_version;

/* <gtk/gtkmain.h> */
extern const guint gtk_binary_age;

/* <gtk/gtkmain.h> */
extern const guint gtk_interface_age;

/* <gtk/gtkmain.h> */
gchar* gtk_check_version(guint required_major, guint required_minor, guint required_micro)
{
}


/* Initialization, exit, mainloop and miscellaneous routines
 */

/* <gtk/gtkmain.h> */
void gtk_init(int *argc, char ***argv)
{
}

/* <gtk/gtkmain.h> */
gboolean gtk_init_check(int *argc, char ***argv)
{
}

/* <gtk/gtkmain.h> */
void gtk_exit(gint error_code)
{
}

/* <gtk/gtkmain.h> */
gchar* gtk_set_locale(void)
{
}

/* <gtk/gtkmain.h> */
gint gtk_events_pending(void)
{
}

/* The following is the event func GTK+ registers with GDK
 * we expose it mainly to allow filtering of events between
 * GDK and GTK+.
 */

/* <gtk/gtkmain.h> */
void gtk_main_do_event(GdkEvent *event)
{
}


/* <gtk/gtkmain.h> */
void gtk_main(void)
{
}

/* <gtk/gtkmain.h> */
guint gtk_main_level(void)
{
}

/* <gtk/gtkmain.h> */
void gtk_main_quit(void)
{
}

/* <gtk/gtkmain.h> */
gint gtk_main_iteration(void)
{
}
/* gtk_main_iteration() calls gtk_main_iteration_do(TRUE) */

/* <gtk/gtkmain.h> */
gint gtk_main_iteration_do(gboolean blocking)
{
}


/* <gtk/gtkmain.h> */
gint gtk_true(void)
{
}

/* <gtk/gtkmain.h> */
gint gtk_false(void)
{
}


/* <gtk/gtkmain.h> */
void gtk_grab_add(GtkWidget *widget)
{
}

/* <gtk/gtkmain.h> */
GtkWidget* gtk_grab_get_current(void)
{
}

/* <gtk/gtkmain.h> */
void gtk_grab_remove(GtkWidget *widget)
{
}


/* <gtk/gtkmain.h> */
void gtk_init_add(GtkFunction function, gpointer data)
{
}

/* <gtk/gtkmain.h> */
void gtk_quit_add_destroy(guint main_level, GtkObject *object)
{
}

/* <gtk/gtkmain.h> */
guint gtk_quit_add(guint main_level, GtkFunction function, gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_quit_add_full(guint main_level, GtkFunction function, GtkCallbackMarshal marshal, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkmain.h> */
void gtk_quit_remove(guint quit_handler_id)
{
}

/* <gtk/gtkmain.h> */
void gtk_quit_remove_by_data(gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_timeout_add(guint32 interval, GtkFunction function, gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_timeout_add_full(guint32 interval, GtkFunction function, GtkCallbackMarshal marshal, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkmain.h> */
void gtk_timeout_remove(guint timeout_handler_id)
{
}


/* <gtk/gtkmain.h> */
guint gtk_idle_add(GtkFunction function, gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_idle_add_priority(gint priority, GtkFunction function, gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_idle_add_full(gint priority, GtkFunction function, GtkCallbackMarshal marshal, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkmain.h> */
void gtk_idle_remove(guint idle_handler_id)
{
}

/* <gtk/gtkmain.h> */
void gtk_idle_remove_by_data(gpointer data)
{
}

/* <gtk/gtkmain.h> */
guint gtk_input_add_full(gint source, GdkInputCondition condition, GdkInputFunction function, GtkCallbackMarshal marshal, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkmain.h> */
void gtk_input_remove(guint input_handler_id)
{
}



/* <gtk/gtkmain.h> */
guint gtk_key_snooper_install(GtkKeySnoopFunc snooper, gpointer func_data)
{
}

/* <gtk/gtkmain.h> */
void gtk_key_snooper_remove(guint snooper_handler_id)
{
}
	 

/* <gtk/gtkmain.h> */
GdkEvent* gtk_get_current_event(void)
{
}

/* <gtk/gtkmain.h> */
GtkWidget* gtk_get_event_widget(GdkEvent *event)
{
}

/* <gtk/gtkmarshal.h> */
#define gtk_signal_default_marshaller gtk_marshal_NONE__NONE

	void gtk_marshal_BOOL__NONE (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_BOOL__POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_BOOL__POINTER_POINTER_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_BOOL__POINTER_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_BOOL__POINTER_INT_INT_UINT gtk_marshal_BOOL__POINTER_INT_INT_INT

	void gtk_marshal_BOOL__POINTER_INT_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_BOOL__POINTER_STRING_STRING_POINTER gtk_marshal_BOOL__POINTER_POINTER_POINTER_POINTER

	void gtk_marshal_BOOL__POINTER_POINTER_POINTER_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_ENUM__ENUM gtk_marshal_INT__INT

	void gtk_marshal_INT__INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_INT__POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_INT__POINTER_CHAR_CHAR (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__BOOL (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__BOXED gtk_marshal_NONE__POINTER

	void gtk_marshal_NONE__POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__C_CALLBACK (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__C_CALLBACK_C_CALLBACK (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__ENUM gtk_marshal_NONE__INT

	void gtk_marshal_NONE__INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__ENUM_FLOAT gtk_marshal_NONE__INT_FLOAT

	void gtk_marshal_NONE__INT_FLOAT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__ENUM_FLOAT_BOOL gtk_marshal_NONE__INT_FLOAT_BOOL

	void gtk_marshal_NONE__INT_FLOAT_BOOL (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__INT_INT_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__NONE (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__OBJECT gtk_marshal_NONE__POINTER

	void gtk_marshal_NONE__POINTER_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__POINTER_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);

	void gtk_marshal_NONE__POINTER_POINTER_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_STRING_STRING gtk_marshal_NONE__POINTER_POINTER_POINTER


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_UINT gtk_marshal_NONE__POINTER_INT


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_UINT_ENUM gtk_marshal_NONE__POINTER_INT_INT

	void gtk_marshal_NONE__POINTER_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_POINTER_UINT_UINT gtk_marshal_NONE__POINTER_POINTER_INT_INT

	void gtk_marshal_NONE__POINTER_POINTER_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_INT_INT_POINTER_UINT_UINT gtk_marshal_NONE__POINTER_INT_INT_POINTER_INT_INT

	void gtk_marshal_NONE__POINTER_INT_INT_POINTER_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__POINTER_UINT_UINT gtk_marshal_NONE__POINTER_INT_INT


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__STRING gtk_marshal_NONE__POINTER


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__STRING_INT_POINTER gtk_marshal_NONE__POINTER_INT_POINTER

	void gtk_marshal_NONE__POINTER_INT_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__UINT gtk_marshal_NONE__INT


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__UINT_POINTER_UINT_ENUM_ENUM_POINTER gtk_marshal_NONE__INT_POINTER_INT_INT_INT_POINTER

	void gtk_marshal_NONE__INT_POINTER_INT_INT_INT_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__UINT_POINTER_UINT_UINT_ENUM gtk_marshal_NONE__INT_POINTER_INT_INT_INT

	void gtk_marshal_NONE__INT_POINTER_INT_INT_INT (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmarshal.h> */
#define gtk_marshal_NONE__UINT_STRING gtk_marshal_NONE__INT_POINTER

	void gtk_marshal_NONE__INT_POINTER (GtkObject * object, GtkSignalFunc func, gpointer func_data, GtkArg * args);


/* <gtk/gtkmenubar.h> */
#define GTK_TYPE_MENU_BAR (gtk_menu_bar_get_type ())

/* <gtk/gtkmenubar.h> */
#define GTK_MENU_BAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_MENU_BAR, GtkMenuBar))

/* <gtk/gtkmenubar.h> */
#define GTK_MENU_BAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_MENU_BAR, GtkMenuBarClass))

/* <gtk/gtkmenubar.h> */
#define GTK_IS_MENU_BAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_MENU_BAR))

/* <gtk/gtkmenubar.h> */
#define GTK_IS_MENU_BAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MENU_BAR))




/* <gtk/gtkmenubar.h> */
typedef struct _GtkMenuBar
{
	GtkMenuShell menu_shell;

	GtkShadowType shadow_type;
} GtkMenuBar;


/* <gtk/gtkmenubar.h> */
typedef struct _GtkMenuBarClass
{
	GtkMenuShellClass parent_class;
} GtkMenuBarClass;



/* <gtk/gtkmenubar.h> */
GtkType gtk_menu_bar_get_type(void)
{
}

/* <gtk/gtkmenubar.h> */
GtkWidget* gtk_menu_bar_new(void)
{
}

/* <gtk/gtkmenubar.h> */
void gtk_menu_bar_append(GtkMenuBar *menu_bar, GtkWidget *child)
{
}

/* <gtk/gtkmenubar.h> */
void gtk_menu_bar_prepend(GtkMenuBar *menu_bar, GtkWidget *child)
{
}

/* <gtk/gtkmenubar.h> */
void gtk_menu_bar_insert(GtkMenuBar *menu_bar, GtkWidget *child, gint position)
{
}

/* <gtk/gtkmenubar.h> */
void gtk_menu_bar_set_shadow_type(GtkMenuBar *menu_bar, GtkShadowType type)
{
}


/* <gtk/gtkmenufactory.h> */
typedef void(*GtkMenuCallback) (GtkWidget *widget, gpointer user_data)
{
}


/* <gtk/gtkmenufactory.h> */
typedef struct _GtkMenuEntry
{
	gchar *path;
	gchar *accelerator;
	GtkMenuCallback callback;
	gpointer callback_data;
	GtkWidget *widget;
} GtkMenuEntry;


/* <gtk/gtkmenufactory.h> */
typedef struct _GtkMenuPath
{
	char *path;
	GtkWidget *widget;
} GtkMenuPath;


/* <gtk/gtkmenufactory.h> */
typedef struct _GtkMenuFactory
{
	gchar *path;
	GtkMenuFactoryType type;
	GtkAccelGroup *accel_group;
	GtkWidget *widget;
	GList *subfactories;
} GtkMenuFactory;

/* Note: the use of GtkMenuFactory is strongly deprecated.
 * use GtkItemFactory instead (gtkitemfactory.h and gtkitemfactory.c).
 * gtkmenufactory.h and gtkmenufactory.c are sheduled for removal
 * in some future gtk versions.
 */


/* <gtk/gtkmenufactory.h> */
GtkMenuFactory* gtk_menu_factory_new(GtkMenuFactoryType type)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_destroy(GtkMenuFactory *factory)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_add_entries(GtkMenuFactory *factory, GtkMenuEntry *entries, int nentries)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_add_subfactory(GtkMenuFactory *factory, GtkMenuFactory *subfactory, const char *path)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_remove_paths(GtkMenuFactory *factory, char **paths, int npaths)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_remove_entries(GtkMenuFactory *factory, GtkMenuEntry *entries, int nentries)
{
}

/* <gtk/gtkmenufactory.h> */
void gtk_menu_factory_remove_subfactory(GtkMenuFactory *factory, GtkMenuFactory *subfactory, const char *path)
{
}

/* <gtk/gtkmenufactory.h> */
GtkMenuPath* gtk_menu_factory_find(GtkMenuFactory *factory, const char *path)
{
}


/* <gtk/gtkmenu.h> */
#define GTK_TYPE_MENU (gtk_menu_get_type ())

/* <gtk/gtkmenu.h> */
#define GTK_MENU(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_MENU, GtkMenu))

/* <gtk/gtkmenu.h> */
#define GTK_MENU_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_MENU, GtkMenuClass))

/* <gtk/gtkmenu.h> */
#define GTK_IS_MENU(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_MENU))

/* <gtk/gtkmenu.h> */
#define GTK_IS_MENU_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MENU))



/* <gtk/gtkmenu.h> */
typedef void(*GtkMenuPositionFunc) (GtkMenu *menu, gint *x, gint *y, gpointer user_data)
{
}

/* <gtk/gtkmenu.h> */
typedef void(*GtkMenuDetachFunc) (GtkWidget *attach_widget, GtkMenu *menu)
{
}


/* <gtk/gtkmenu.h> */
typedef struct _GtkMenu
{
	GtkMenuShell menu_shell;
	 
	GtkWidget *parent_menu_item;
	GtkWidget *old_active_menu_item;
	 
	GtkAccelGroup *accel_group;
	GtkMenuPositionFunc position_func;
	gpointer position_func_data;

	 /* Do _not_ touch these widgets directly. We hide the reference
 * count from the toplevel to the menu, so it must be restored
 * before operating on these widgets
 */
	GtkWidget *toplevel;
	GtkWidget *tearoff_window;

	guint torn_off : 1;
} GtkMenu;


/* <gtk/gtkmenu.h> */
typedef struct _GtkMenuClass
{
	GtkMenuShellClass parent_class;
} GtkMenuClass;



/* <gtk/gtkmenu.h> */
GtkType gtk_menu_get_type(void)
{
}

/* <gtk/gtkmenu.h> */
GtkWidget* gtk_menu_new(void)
{
}

/* Wrappers for the Menu Shell operations */

/* <gtk/gtkmenu.h> */
void gtk_menu_append(GtkMenu *menu, GtkWidget *child)
{
}

/* <gtk/gtkmenu.h> */
void gtk_menu_prepend(GtkMenu *menu, GtkWidget *child)
{
}

/* <gtk/gtkmenu.h> */
void gtk_menu_insert(GtkMenu *menu, GtkWidget *child, gint position)
{
}

/* Display the menu onscreen */

/* <gtk/gtkmenu.h> */
void gtk_menu_popup(GtkMenu *menu, GtkWidget *parent_menu_shell, GtkWidget *parent_menu_item, GtkMenuPositionFunc func, gpointer data, guint button, guint32 activate_time)
{
}

/* Position the menu according to its position function. Called
 * from gtkmenuitem.c when a menu-item changes its allocation
 */

/* <gtk/gtkmenu.h> */
void gtk_menu_reposition(GtkMenu *menu)
{
}


/* <gtk/gtkmenu.h> */
void gtk_menu_popdown(GtkMenu *menu)
{
}

/* Keep track of the last menu item selected. (For the purposes
 * of the option menu
 */

/* <gtk/gtkmenu.h> */
GtkWidget* gtk_menu_get_active(GtkMenu *menu)
{
}

/* <gtk/gtkmenu.h> */
void gtk_menu_set_active(GtkMenu *menu, guint index)
{
}

/* set/get the acclerator group that holds global accelerators (should
 * be added to the corresponding toplevel with gtk_window_add_accel_group().
 */

/* <gtk/gtkmenu.h> */
void gtk_menu_set_accel_group(GtkMenu *menu, GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkmenu.h> */
GtkAccelGroup* gtk_menu_get_accel_group(GtkMenu *menu)
{
}

/* get the accelerator group that is used internally by the menu for
 * underline accelerators while the menu is popped up.
 */

/* <gtk/gtkmenu.h> */
GtkAccelGroup* gtk_menu_get_uline_accel_group(GtkMenu *menu)
{
}

/* <gtk/gtkmenu.h> */
GtkAccelGroup* gtk_menu_ensure_uline_accel_group(GtkMenu *menu)
{
}


/* A reference count is kept for a widget when it is attached to
 * a particular widget. This is typically a menu item; it may also
 * be a widget with a popup menu - for instance, the Notebook widget.
 */

/* <gtk/gtkmenu.h> */
void gtk_menu_attach_to_widget(GtkMenu *menu, GtkWidget *attach_widget, GtkMenuDetachFunc detacher)
{
}

/* <gtk/gtkmenu.h> */
void gtk_menu_detach(GtkMenu *menu)
{
}

/* This should be dumped in favor of data set when the menu is popped
 * up - that is currently in the ItemFactory code, but should be
 * in the Menu code.
 */

/* <gtk/gtkmenu.h> */
GtkWidget* gtk_menu_get_attach_widget(GtkMenu *menu)
{
}


/* <gtk/gtkmenu.h> */
void gtk_menu_set_tearoff_state(GtkMenu *menu, gboolean torn_off)
{
}

/* This sets the window manager title for the window that
 * appears when a menu is torn off
 */

/* <gtk/gtkmenu.h> */
void gtk_menu_set_title(GtkMenu *menu, const gchar *title)
{
}


/* <gtk/gtkmenu.h> */
void gtk_menu_reorder_child(GtkMenu *menu, GtkWidget *child, gint position)
{
}


/* <gtk/gtkmenuitem.h> */
#define GTK_TYPE_MENU_ITEM (gtk_menu_item_get_type ())

/* <gtk/gtkmenuitem.h> */
#define GTK_MENU_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_MENU_ITEM, GtkMenuItem))

/* <gtk/gtkmenuitem.h> */
#define GTK_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_MENU_ITEM, GtkMenuItemClass))

/* <gtk/gtkmenuitem.h> */
#define GTK_IS_MENU_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_MENU_ITEM))

/* <gtk/gtkmenuitem.h> */
#define GTK_IS_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MENU_ITEM))




/* <gtk/gtkmenuitem.h> */
typedef struct _GtkMenuItem
{
	GtkItem item;
	 
	GtkWidget *submenu;
	 
	guint accelerator_signal;
	guint16 toggle_size;
	guint16 accelerator_width;
	 
	guint show_toggle_indicator : 1;
	guint show_submenu_indicator : 1;
	guint submenu_placement : 1;
	guint submenu_direction : 1;
	guint right_justify: 1;
	guint timer;
} GtkMenuItem;


/* <gtk/gtkmenuitem.h> */
typedef struct _GtkMenuItemClass
{
	GtkItemClass parent_class;
	 
	guint toggle_size;
	 /* If the following flag is true, then we should always hide
 * the menu when the MenuItem is activated. Otherwise, the 
 * it is up to the caller. For instance, when navigating
 * a menu with the keyboard, <Space> doesn't hide, but
 * <Return> does.
 */
	guint hide_on_activate : 1;
	 
	void (* activate) (GtkMenuItem *menu_item);
	void (* activate_item) (GtkMenuItem *menu_item);
} GtkMenuItemClass;



/* <gtk/gtkmenuitem.h> */
GtkType gtk_menu_item_get_type(void)
{
}

/* <gtk/gtkmenuitem.h> */
GtkWidget* gtk_menu_item_new(void)
{
}

/* <gtk/gtkmenuitem.h> */
GtkWidget* gtk_menu_item_new_with_label(const gchar *label)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_set_submenu(GtkMenuItem *menu_item, GtkWidget *submenu)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_remove_submenu(GtkMenuItem *menu_item)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_set_placement(GtkMenuItem *menu_item, GtkSubmenuPlacement placement)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_configure(GtkMenuItem *menu_item, gint show_toggle_indicator, gint show_submenu_indicator)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_select(GtkMenuItem *menu_item)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_deselect(GtkMenuItem *menu_item)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_activate(GtkMenuItem *menu_item)
{
}

/* <gtk/gtkmenuitem.h> */
void gtk_menu_item_right_justify(GtkMenuItem *menu_item)
{
}

/* <gtk/gtkmenushell.h> */
#define GTK_TYPE_MENU_SHELL (gtk_menu_shell_get_type ())

/* <gtk/gtkmenushell.h> */
#define GTK_MENU_SHELL(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_MENU_SHELL, GtkMenuShell))

/* <gtk/gtkmenushell.h> */
#define GTK_MENU_SHELL_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_MENU_SHELL, GtkMenuShellClass))

/* <gtk/gtkmenushell.h> */
#define GTK_IS_MENU_SHELL(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_MENU_SHELL))

/* <gtk/gtkmenushell.h> */
#define GTK_IS_MENU_SHELL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MENU_SHELL))



/* <gtk/gtkmenushell.h> */
typedef struct _GtkMenuShell
{
	GtkContainer container;
	 
	GList *children;
	GtkWidget *active_menu_item;
	GtkWidget *parent_menu_shell;
	 
	guint active : 1;
	guint have_grab : 1;
	guint have_xgrab : 1;
	guint button : 2;
	guint ignore_leave : 1;
	guint menu_flag : 1;
	guint ignore_enter : 1;
	 
	guint32 activate_time;
} GtkMenuShell;


/* <gtk/gtkmenushell.h> */
typedef struct _GtkMenuShellClass
{
	GtkContainerClass parent_class;
	 
	guint submenu_placement : 1;
	 
	void (*deactivate) (GtkMenuShell *menu_shell);
	void (*selection_done) (GtkMenuShell *menu_shell);

	void (*move_current) (GtkMenuShell *menu_shell, GtkMenuDirectionType direction);
	void (*activate_current) (GtkMenuShell *menu_shell, gboolean force_hide);
	void (*cancel) (GtkMenuShell *menu_shell);
} GtkMenuShellClass;



/* <gtk/gtkmenushell.h> */
GtkType gtk_menu_shell_get_type(void)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_append(GtkMenuShell *menu_shell, GtkWidget *child)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_prepend(GtkMenuShell *menu_shell, GtkWidget *child)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_insert(GtkMenuShell *menu_shell, GtkWidget *child, gint position)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_deactivate(GtkMenuShell *menu_shell)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_select_item(GtkMenuShell *menu_shell, GtkWidget *menu_item)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_deselect(GtkMenuShell *menu_shell)
{
}

/* <gtk/gtkmenushell.h> */
void gtk_menu_shell_activate_item(GtkMenuShell *menu_shell, GtkWidget *menu_item, gboolean force_deactivate)
{
}

/* <gtk/gtkmisc.h> */
#define GTK_TYPE_MISC (gtk_misc_get_type ())

/* <gtk/gtkmisc.h> */
#define GTK_MISC(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_MISC, GtkMisc))

/* <gtk/gtkmisc.h> */
#define GTK_MISC_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_MISC, GtkMiscClass))

/* <gtk/gtkmisc.h> */
#define GTK_IS_MISC(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_MISC))

/* <gtk/gtkmisc.h> */
#define GTK_IS_MISC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MISC))



/* <gtk/gtkmisc.h> */
typedef struct _GtkMisc
{
	GtkWidget widget;
	 
	gfloat xalign;
	gfloat yalign;
	 
	guint16 xpad;
	guint16 ypad;
} GtkMisc;


/* <gtk/gtkmisc.h> */
typedef struct _GtkMiscClass
{
	GtkWidgetClass parent_class;
} GtkMiscClass;



/* <gtk/gtkmisc.h> */
GtkType gtk_misc_get_type(void)
{
}

/* <gtk/gtkmisc.h> */
void gtk_misc_set_alignment(GtkMisc *misc, gfloat xalign, gfloat yalign)
{
}

/* <gtk/gtkmisc.h> */
void gtk_misc_set_padding(GtkMisc *misc, gint xpad, gint ypad)
{
}


/* <gtk/gtknotebook.h> */
#define GTK_TYPE_NOTEBOOK (gtk_notebook_get_type ())

/* <gtk/gtknotebook.h> */
#define GTK_NOTEBOOK(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_NOTEBOOK, GtkNotebook))

/* <gtk/gtknotebook.h> */
#define GTK_NOTEBOOK_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_NOTEBOOK, GtkNotebookClass))

/* <gtk/gtknotebook.h> */
#define GTK_IS_NOTEBOOK(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_NOTEBOOK))

/* <gtk/gtknotebook.h> */
#define GTK_IS_NOTEBOOK_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_NOTEBOOK))


/* <gtk/gtknotebook.h> */
#define GTK_NOTEBOOK_PAGE(_glist_) ((GtkNotebookPage *)((GList *)(_glist_))->data)



/* <gtk/gtknotebook.h> */
typedef struct _GtkNotebook
{
	GtkContainer container;
	 
	GtkNotebookPage *cur_page;
	GList *children;
	GList *first_tab;
	GList *focus_tab;
	 
	GtkWidget *menu;
	GdkWindow *panel;
	 
	guint32 timer;
	 
	guint16 tab_hborder;
	guint16 tab_vborder;
	 
	guint show_tabs : 1;
	guint homogeneous : 1;
	guint show_border : 1;
	guint tab_pos : 2;
	guint scrollable : 1;
	guint in_child : 2;
	guint click_child : 2;
	guint button : 2;
	guint need_timer : 1;
	guint child_has_focus : 1;
	guint have_visible_child : 1;
} GtkNotebook;


/* <gtk/gtknotebook.h> */
typedef struct _GtkNotebookClass
{
	GtkContainerClass parent_class;
	 
	void (* switch_page) (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num);
} GtkNotebookClass;


/* <gtk/gtknotebook.h> */
typedef struct _GtkNotebookPage
{
	GtkWidget *child;
	GtkWidget *tab_label;
	GtkWidget *menu_label;

	guint default_menu : 1;
	guint default_tab : 1;
	guint expand : 1;
	guint fill : 1;
	guint pack : 1;

	GtkRequisition requisition;
	GtkAllocation allocation;
} GtkNotebookPage;

/***********************************************************
 * Creation, insertion, deletion *
 ***********************************************************/


/* <gtk/gtknotebook.h> */
GtkType gtk_notebook_get_type(void)
{
}

/* <gtk/gtknotebook.h> */
GtkWidget * gtk_notebook_new(void)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_append_page(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_append_page_menu(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label, GtkWidget *menu_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_prepend_page(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_prepend_page_menu(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label, GtkWidget *menu_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_insert_page(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label, gint position)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_insert_page_menu(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label, GtkWidget *menu_label, gint position)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_remove_page(GtkNotebook *notebook, gint page_num)
{
}

/***********************************************************
 * query, set current NoteebookPage *
 ***********************************************************/


/* <gtk/gtknotebook.h> */
gint gtk_notebook_get_current_page(GtkNotebook *notebook)
{
}

/* <gtk/gtknotebook.h> */
GtkWidget* gtk_notebook_get_nth_page(GtkNotebook *notebook, gint page_num)
{
}

/* <gtk/gtknotebook.h> */
gint gtk_notebook_page_num(GtkNotebook *notebook, GtkWidget *child)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_page(GtkNotebook *notebook, gint page_num)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_next_page(GtkNotebook *notebook)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_prev_page(GtkNotebook *notebook)
{
}

/***********************************************************
 * set Notebook, NotebookTab style *
 ***********************************************************/


/* <gtk/gtknotebook.h> */
void gtk_notebook_set_show_border(GtkNotebook *notebook, gboolean show_border)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_show_tabs(GtkNotebook *notebook, gboolean show_tabs)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_pos(GtkNotebook *notebook, GtkPositionType pos)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_homogeneous_tabs(GtkNotebook *notebook, gboolean homogeneous)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_border(GtkNotebook *notebook, guint border_width)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_hborder(GtkNotebook *notebook, guint tab_hborder)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_vborder(GtkNotebook *notebook, guint tab_vborder)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_scrollable(GtkNotebook *notebook, gboolean scrollable)
{
}

/***********************************************************
 * enable/disable PopupMenu *
 ***********************************************************/


/* <gtk/gtknotebook.h> */
void gtk_notebook_popup_enable(GtkNotebook *notebook)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_popup_disable(GtkNotebook *notebook)
{
}

/***********************************************************
 * query/set NotebookPage Properties *
 ***********************************************************/


/* <gtk/gtknotebook.h> */
GtkWidget * gtk_notebook_get_tab_label(GtkNotebook *notebook, GtkWidget *child)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_label(GtkNotebook *notebook, GtkWidget *child, GtkWidget *tab_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_label_text(GtkNotebook *notebook, GtkWidget *child, const gchar *tab_text)
{
}

/* <gtk/gtknotebook.h> */
GtkWidget * gtk_notebook_get_menu_label(GtkNotebook *notebook, GtkWidget *child)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_menu_label(GtkNotebook *notebook, GtkWidget *child, GtkWidget *menu_label)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_menu_label_text(GtkNotebook *notebook, GtkWidget *child, const gchar *menu_text)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_query_tab_label_packing(GtkNotebook *notebook, GtkWidget *child, gboolean *expand, gboolean *fill, GtkPackType *pack_type)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_set_tab_label_packing(GtkNotebook *notebook, GtkWidget *child, gboolean expand, gboolean fill, GtkPackType pack_type)
{
}

/* <gtk/gtknotebook.h> */
void gtk_notebook_reorder_child(GtkNotebook *notebook, GtkWidget *child, gint position)
{
}


/* Macro for casting a pointer to a GtkObject or GtkObjectClass pointer.
 * The second portion of the ?: statments are just in place to offer
 * descriptive warning message.
 */

/* <gtk/gtkobject.h> */
#define GTK_OBJECT(object) ( \
	GTK_IS_OBJECT (object) ? \
	(GtkObject*) (object) : \
	(GtkObject*) gtk_type_check_object_cast ((GtkTypeObject*) (object), GTK_TYPE_OBJECT) \
)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_CLASS(klass) ( \
	GTK_IS_OBJECT_CLASS (klass) ? \
	(GtkObjectClass*) (klass) : \
	(GtkObjectClass*) gtk_type_check_class_cast ((GtkTypeClass*) (klass), GTK_TYPE_OBJECT) \
)

/* Macro for testing whether `object' and `klass' are of type GTK_TYPE_OBJECT.
 */

/* <gtk/gtkobject.h> */
#define GTK_IS_OBJECT(object) ( \
	(object) != NULL && \
	GTK_IS_OBJECT_CLASS (((GtkObject*) (object))->klass) \
)

/* <gtk/gtkobject.h> */
#define GTK_IS_OBJECT_CLASS(klass) ( \
	(klass) != NULL && \
	GTK_FUNDAMENTAL_TYPE (((GtkObjectClass*) (klass))->type) == GTK_TYPE_OBJECT \
)

/* Macros for extracting various fields from GtkObject and GtkObjectClass.
 */

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_TYPE(obj) (GTK_OBJECT (obj)->klass->type)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_SIGNALS(obj) (GTK_OBJECT (obj)->klass->signals)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_NSIGNALS(obj) (GTK_OBJECT (obj)->klass->nsignals)

/* GtkObject only uses the first 4 bits of the flags field.
 * Derived objects may use the remaining bits. Though this
 * is a kinda nasty break up, it does make the size of
 * derived objects smaller.
 */

/* <gtk/gtkobject.h> */
typedef enum
{
	GTK_DESTROYED = 1 << 0,
	GTK_FLOATING = 1 << 1,
	GTK_CONNECTED = 1 << 2,
	GTK_CONSTRUCTED = 1 << 3
} GtkObjectFlags;

/* Macros for extracting the object_flags from GtkObject.
 */

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_FLAGS(obj) (GTK_OBJECT (obj)->flags)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_DESTROYED(obj) ((GTK_OBJECT_FLAGS (obj) & GTK_DESTROYED) != 0)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_FLOATING(obj) ((GTK_OBJECT_FLAGS (obj) & GTK_FLOATING) != 0)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_CONNECTED(obj) ((GTK_OBJECT_FLAGS (obj) & GTK_CONNECTED) != 0)

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_CONSTRUCTED(obj) ((GTK_OBJECT_FLAGS (obj) & GTK_CONSTRUCTED) != 0)

/* Macros for setting and clearing bits in the object_flags field of GtkObject.
 */

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_SET_FLAGS(obj,flag) G_STMT_START{ (GTK_OBJECT_FLAGS (obj) |= (flag)); }G_STMT_END

/* <gtk/gtkobject.h> */
#define GTK_OBJECT_UNSET_FLAGS(obj,flag) G_STMT_START{ (GTK_OBJECT_FLAGS (obj) &= ~(flag)); }G_STMT_END

/* GtkArg flag bits for gtk_object_add_arg_type
 */

/* <gtk/gtkobject.h> */
typedef enum
{
	GTK_ARG_READABLE = 1 << 0,
	GTK_ARG_WRITABLE = 1 << 1,
	GTK_ARG_CONSTRUCT = 1 << 2,
	GTK_ARG_CONSTRUCT_ONLY = 1 << 3,
	GTK_ARG_CHILD_ARG = 1 << 4,
	GTK_ARG_MASK = 0x1f,
	 
	 /* aliases
 */
	GTK_ARG_READWRITE = GTK_ARG_READABLE | GTK_ARG_WRITABLE
} GtkArgFlags;


/* <gtk/gtkobject.h> */
typedef struct _GtkObject
{
	 /* GtkTypeObject related fields: */
	GtkObjectClass *klass;
	 
	 
	 /* 32 bits of flags. GtkObject only uses 4 of these bits and
 * GtkWidget uses the rest. This is done because structs are
 * aligned on 4 or 8 byte boundaries. If a new bitfield were
 * used in GtkWidget much space would be wasted.
 */
	guint32 flags;
	 
	 /* reference count.
 * refer to the file docs/refcounting.txt on this issue.
 */
	guint ref_count;
	 
	 /* A list of keyed data pointers, used for e.g. the list of signal
 * handlers or an object's user_data.
 */
	GData *object_data;
} GtkObject;

/* The GtkObjectClass is the base of the Gtk+ objects classes hierarchy,
 * it ``inherits'' from the GtkTypeClass by mirroring its fields, which
 * must always be kept in sync completely. The GtkObjectClass defines
 * the basic necessities for the object inheritance mechanism to work.
 * Namely, the `signals' and `nsignals' fields as well as the function
 * pointers, required to end an object's lifetime.
 */

/* <gtk/gtkobject.h> */
typedef struct _GtkObjectClass
{
	 /* GtkTypeClass fields: */
	GtkType type;
	 
	 
	 /* The signals this object class handles. "signals" is an
 * array of signal ID's.
 */
	guint *signals;
	 
	 /* The number of signals listed in "signals".
 */
	guint nsignals;
	 
	 /* The number of arguments per class.
 */
	guint n_args;
	GSList *construct_args;
	 
	 /* Non overridable class methods to set and get per class arguments */
	void (*set_arg) (GtkObject *object, GtkArg *arg, guint arg_id);
	void (*get_arg) (GtkObject *object, GtkArg *arg, guint arg_id);
	 
	 /* The functions that will end an objects life time. In one way ore
 * another all three of them are defined for all objects. If an
 * object class overrides one of the methods in order to perform class
 * specific destruction then it must still invoke its superclass'
 * implementation of the method after it is finished with its
 * own cleanup. (See the destroy function for GtkWidget for
 * an example of how to do this).
 */
	void (* shutdown) (GtkObject *object);
	void (* destroy) (GtkObject *object);
	 
	void (* finalize) (GtkObject *object);
} GtkObjectClass;



/* Application-level methods */


/* <gtk/gtkobject.h> */
GtkType gtk_object_get_type(void)
{
}

/* Append a user defined signal without default handler to a class. */

/* <gtk/gtkobject.h> */
guint gtk_object_class_user_signal_new(GtkObjectClass *klass, const gchar *name, GtkSignalRunType signal_flags, GtkSignalMarshaller marshaller, GtkType return_val, guint nparams, ...)
{
}

/* <gtk/gtkobject.h> */
guint gtk_object_class_user_signal_newv(GtkObjectClass *klass, const gchar *name, GtkSignalRunType signal_flags, GtkSignalMarshaller marshaller, GtkType return_val, guint nparams, GtkType *params)
{
}

/* <gtk/gtkobject.h> */
GtkObject* gtk_object_new(GtkType type, const gchar *first_arg_name, ...)
{
}

/* <gtk/gtkobject.h> */
GtkObject* gtk_object_newv(GtkType object_type, guint n_args, GtkArg *args)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_default_construct(GtkObject *object)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_constructed(GtkObject *object)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_sink(GtkObject *object)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_ref(GtkObject *object)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_unref(GtkObject *object)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_weakref(GtkObject *object, GtkDestroyNotify notify, gpointer data)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_weakunref(GtkObject *object, GtkDestroyNotify notify, gpointer data)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_destroy(GtkObject *object)
{
}

/* gtk_object_getv() sets an arguments type and value, or just
 * its type to GTK_TYPE_INVALID.
 * if GTK_FUNDAMENTAL_TYPE (arg->type) == GTK_TYPE_STRING, it's
 * the callers response to do a g_free (GTK_VALUE_STRING (arg));
 */

/* <gtk/gtkobject.h> */
void gtk_object_getv(GtkObject *object, guint n_args, GtkArg *args)
{
}
/* gtk_object_get() sets the variable values pointed to by the adresses
 * passed after the argument names according to the arguments value.
 * if GTK_FUNDAMENTAL_TYPE (arg->type) == GTK_TYPE_STRING, it's
 * the callers response to do a g_free (retrived_value);
 */

/* <gtk/gtkobject.h> */
void gtk_object_get(GtkObject *object, const gchar *first_arg_name, ...)
{
}

/* gtk_object_set() takes a variable argument list of the form:
 * (..., gchar *arg_name, ARG_VALUES, [repeatedly name/value pairs,] NULL)
 * where ARG_VALUES type depend on the argument and can consist of
 * more than one c-function argument.
 */

/* <gtk/gtkobject.h> */
void gtk_object_set(GtkObject *object, const gchar *first_arg_name, ...)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_setv(GtkObject *object, guint n_args, GtkArg *args)
{
}

/* Allocate a GtkArg array of size nargs that hold the
 * names and types of the args that can be used with
 * gtk_object_set/gtk_object_get. if (arg_flags!=NULL),
 * (*arg_flags) will be set to point to a newly allocated
 * guint array that holds the flags of the args.
 * It is the callers response to do a
 * g_free (returned_args); g_free (*arg_flags).
 */

/* <gtk/gtkobject.h> */
GtkArg* gtk_object_query_args(GtkType class_type, guint32 **arg_flags, guint *n_args)
{
}

/* Set 'data' to the "object_data" field of the object. The
 * data is indexed by the "key". If there is already data
 * associated with "key" then the new data will replace it.
 * If 'data' is NULL then this call is equivalent to
 * 'gtk_object_remove_data'.
 * The gtk_object_set_data_full variant acts just the same,
 * but takes an additional argument which is a function to
 * be called when the data is removed.
 * `gtk_object_remove_data' is equivalent to the above,
 * where 'data' is NULL
 * `gtk_object_get_data' gets the data associated with "key".
 */

/* <gtk/gtkobject.h> */
void gtk_object_set_data(GtkObject *object, const gchar *key, gpointer data)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_set_data_full(GtkObject *object, const gchar *key, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_remove_data(GtkObject *object, const gchar *key)
{
}

/* <gtk/gtkobject.h> */
gpointer gtk_object_get_data(GtkObject *object, const gchar *key)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_remove_no_notify(GtkObject *object, const gchar *key)
{
}

/* Set/get the "user_data" object data field of "object". It should
 * be noted that these functions are no different than calling
 * `gtk_object_set_data'/`gtk_object_get_data' with a key of "user_data".
 * They are merely provided as a convenience.
 */

/* <gtk/gtkobject.h> */
void gtk_object_set_user_data(GtkObject *object, gpointer data)
{
}

/* <gtk/gtkobject.h> */
gpointer gtk_object_get_user_data(GtkObject *object)
{
}


/* Object-level methods */

/* Append "signals" to those already defined in "class". */

/* <gtk/gtkobject.h> */
void gtk_object_class_add_signals(GtkObjectClass *klass, guint *signals, guint nsignals)
{
}
/* the `arg_name' argument needs to be a const static string */

/* <gtk/gtkobject.h> */
void gtk_object_add_arg_type(const gchar *arg_name, GtkType arg_type, guint arg_flags, guint arg_id)
{
}

/* Object data method variants that operate on key ids. */

/* <gtk/gtkobject.h> */
void gtk_object_set_data_by_id(GtkObject *object, GQuark data_id, gpointer data)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_set_data_by_id_full(GtkObject *object, GQuark data_id, gpointer data, GtkDestroyNotify destroy)
{
}

/* <gtk/gtkobject.h> */
gpointer gtk_object_get_data_by_id(GtkObject *object, GQuark data_id)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_remove_data_by_id(GtkObject *object, GQuark data_id)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_remove_no_notify_by_id(GtkObject *object, GQuark key_id)
{
}

/* <gtk/gtkobject.h> */
#define gtk_object_data_try_key g_quark_try_string

/* <gtk/gtkobject.h> */
#define gtk_object_data_force_id g_quark_from_string


/* Non-public methods */


/* <gtk/gtkobject.h> */
void gtk_object_arg_set(GtkObject *object, GtkArg *arg, GtkArgInfo *info)
{
}

/* <gtk/gtkobject.h> */
void gtk_object_arg_get(GtkObject *object, GtkArg *arg, GtkArgInfo *info)
{
}

/* <gtk/gtkobject.h> */
gchar* gtk_object_args_collect(GtkType object_type, GSList **arg_list_p, GSList **info_list_p, const gchar *first_arg_name, va_list var_args)
{
}

/* <gtk/gtkobject.h> */
gchar* gtk_object_arg_get_info(GtkType object_type, const gchar *arg_name, GtkArgInfo **info_p)
{
}

/* <gtk/gtkobject.h> */
void gtk_trace_referencing(GtkObject *object, const gchar *func, guint dummy, guint line, gboolean do_ref)
{
}

/* <gtk/gtkobject.h> only if G_ENABLE_DEBUG */
#define gtk_object_ref(o) G_STMT_START{gtk_trace_referencing((o),G_GNUC_PRETTY_FUNCTION,0,__LINE__,1);}G_STMT_END

/* <gtk/gtkobject.h> only if G_ENABLE_DEBUG */
#define gtk_object_unref(o) G_STMT_START{gtk_trace_referencing((o),G_GNUC_PRETTY_FUNCTION,0,__LINE__,0);}G_STMT_END


/* <gtk/gtkoptionmenu.h> */
#define GTK_TYPE_OPTION_MENU (gtk_option_menu_get_type ())

/* <gtk/gtkoptionmenu.h> */
#define GTK_OPTION_MENU(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_OPTION_MENU, GtkOptionMenu))

/* <gtk/gtkoptionmenu.h> */
#define GTK_OPTION_MENU_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_OPTION_MENU, GtkOptionMenuClass))

/* <gtk/gtkoptionmenu.h> */
#define GTK_IS_OPTION_MENU(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_OPTION_MENU))

/* <gtk/gtkoptionmenu.h> */
#define GTK_IS_OPTION_MENU_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_OPTION_MENU))



/* <gtk/gtkoptionmenu.h> */
typedef struct _GtkOptionMenu
{
	GtkButton button;
	 
	GtkWidget *menu;
	GtkWidget *menu_item;
	 
	guint16 width;
	guint16 height;
} GtkOptionMenu;


/* <gtk/gtkoptionmenu.h> */
typedef struct _GtkOptionMenuClass
{
	GtkButtonClass parent_class;
} GtkOptionMenuClass;



/* <gtk/gtkoptionmenu.h> */
GtkType gtk_option_menu_get_type(void)
{
}

/* <gtk/gtkoptionmenu.h> */
GtkWidget* gtk_option_menu_new(void)
{
}

/* <gtk/gtkoptionmenu.h> */
GtkWidget* gtk_option_menu_get_menu(GtkOptionMenu *option_menu)
{
}

/* <gtk/gtkoptionmenu.h> */
void gtk_option_menu_set_menu(GtkOptionMenu *option_menu, GtkWidget *menu)
{
}

/* <gtk/gtkoptionmenu.h> */
void gtk_option_menu_remove_menu(GtkOptionMenu *option_menu)
{
}

/* <gtk/gtkoptionmenu.h> */
void gtk_option_menu_set_history(GtkOptionMenu *option_menu, guint index)
{
}


/* <gtk/gtkpacker.h> */
#define GTK_TYPE_PACKER (gtk_packer_get_type ())

/* <gtk/gtkpacker.h> */
#define GTK_PACKER(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PACKER, GtkPacker))

/* <gtk/gtkpacker.h> */
#define GTK_PACKER_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PACKER, GtkPackerClass))

/* <gtk/gtkpacker.h> */
#define GTK_IS_PACKER(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PACKER))

/* <gtk/gtkpacker.h> */
#define GTK_IS_PACKER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PACKER))




/* <gtk/gtkpacker.h> */
typedef enum
{
	GTK_PACK_EXPAND = 1 << 0, /*< nick=expand >*/
	GTK_FILL_X = 1 << 1,
	GTK_FILL_Y = 1 << 2
} GtkPackerOptions;


/* <gtk/gtkpacker.h> */
typedef enum
{
	GTK_SIDE_TOP,
	GTK_SIDE_BOTTOM,
	GTK_SIDE_LEFT,
	GTK_SIDE_RIGHT
} GtkSideType;


/* <gtk/gtkpacker.h> */
typedef enum
{
	GTK_ANCHOR_CENTER,
	GTK_ANCHOR_NORTH,
	GTK_ANCHOR_NORTH_WEST,
	GTK_ANCHOR_NORTH_EAST,
	GTK_ANCHOR_SOUTH,
	GTK_ANCHOR_SOUTH_WEST,
	GTK_ANCHOR_SOUTH_EAST,
	GTK_ANCHOR_WEST,
	GTK_ANCHOR_EAST,
	GTK_ANCHOR_N = GTK_ANCHOR_NORTH,
	GTK_ANCHOR_NW = GTK_ANCHOR_NORTH_WEST,
	GTK_ANCHOR_NE = GTK_ANCHOR_NORTH_EAST,
	GTK_ANCHOR_S = GTK_ANCHOR_SOUTH,
	GTK_ANCHOR_SW = GTK_ANCHOR_SOUTH_WEST,
	GTK_ANCHOR_SE = GTK_ANCHOR_SOUTH_EAST,
	GTK_ANCHOR_W = GTK_ANCHOR_WEST,
	GTK_ANCHOR_E = GTK_ANCHOR_EAST
} GtkAnchorType;


/* <gtk/gtkpacker.h> */
typedef struct _GtkPackerChild
{
	GtkWidget *widget;
	 
	GtkAnchorType anchor;
	GtkSideType side;
	GtkPackerOptions options;
	 
	guint use_default : 1;
	 
	guint border_width : 16;
	guint pad_x : 16;
	guint pad_y : 16;
	guint i_pad_x : 16;
	guint i_pad_y : 16;
} GtkPackerChild;


/* <gtk/gtkpacker.h> */
typedef struct _GtkPacker
{
	GtkContainer parent;
	 
	GList *children;
	 
	guint spacing;
	 
	guint default_border_width : 16;
	guint default_pad_x : 16;
	guint default_pad_y : 16;
	guint default_i_pad_x : 16;
	guint default_i_pad_y : 16;
} GtkPacker;


/* <gtk/gtkpacker.h> */
typedef struct _GtkPackerClass
{
	GtkContainerClass parent_class;
} GtkPackerClass;



/* <gtk/gtkpacker.h> */
GtkType gtk_packer_get_type(void)
{
}

/* <gtk/gtkpacker.h> */
GtkWidget* gtk_packer_new(void)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_add_defaults(GtkPacker *packer, GtkWidget *child, GtkSideType side, GtkAnchorType anchor, GtkPackerOptions options)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_add(GtkPacker *packer, GtkWidget *child, GtkSideType side, GtkAnchorType anchor, GtkPackerOptions options, guint border_width, guint pad_x, guint pad_y, guint i_pad_x, guint i_pad_y)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_set_child_packing(GtkPacker *packer, GtkWidget *child, GtkSideType side, GtkAnchorType anchor, GtkPackerOptions options, guint border_width, guint pad_x, guint pad_y, guint i_pad_x, guint i_pad_y)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_reorder_child(GtkPacker *packer, GtkWidget *child, gint position)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_set_spacing(GtkPacker *packer, guint spacing)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_set_default_border_width(GtkPacker *packer, guint border)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_set_default_pad(GtkPacker *packer, guint pad_x, guint pad_y)
{
}

/* <gtk/gtkpacker.h> */
void gtk_packer_set_default_ipad(GtkPacker *packer, guint i_pad_x, guint i_pad_y)
{
}


/* <gtk/gtkpaned.h> */
#define GTK_TYPE_PANED (gtk_paned_get_type ())

/* <gtk/gtkpaned.h> */
#define GTK_PANED(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PANED, GtkPaned))

/* <gtk/gtkpaned.h> */
#define GTK_PANED_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PANED, GtkPanedClass))

/* <gtk/gtkpaned.h> */
#define GTK_IS_PANED(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PANED))

/* <gtk/gtkpaned.h> */
#define GTK_IS_PANED_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PANED))



/* <gtk/gtkpaned.h> */
typedef struct _GtkPaned
{
	GtkContainer container;
	 
	GtkWidget *child1;
	GtkWidget *child2;
	 
	GdkWindow *handle;
	GdkRectangle groove_rectangle;
	GdkGC *xor_gc;

	 /*< public >*/
	guint16 handle_size;
	guint16 gutter_size;

	 /*< private >*/
	gint child1_size;
	gint last_allocation;
	gint min_position;
	gint max_position;
	 
	guint position_set : 1;
	guint in_drag : 1;
	guint child1_shrink : 1;
	guint child1_resize : 1;
	guint child2_shrink : 1;
	guint child2_resize : 1;
	 
	gint16 handle_xpos;
	gint16 handle_ypos;
} GtkPaned;


/* <gtk/gtkpaned.h> */
typedef struct _GtkPanedClass
{
	GtkContainerClass parent_class;
} GtkPanedClass;



/* <gtk/gtkpaned.h> */
GtkType gtk_paned_get_type(void)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_add1(GtkPaned *paned, GtkWidget *child)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_add2(GtkPaned *paned, GtkWidget *child)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_pack1(GtkPaned *paned, GtkWidget *child, gboolean resize, gboolean shrink)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_pack2(GtkPaned *paned, GtkWidget *child, gboolean resize, gboolean shrink)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_set_position(GtkPaned *paned, gint position)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_set_handle_size(GtkPaned *paned, guint16 size)
{
}

/* <gtk/gtkpaned.h> */
void gtk_paned_set_gutter_size(GtkPaned *paned, guint16 size)
{
}

/* Internal function */

/* <gtk/gtkpaned.h> */
void gtk_paned_compute_position(GtkPaned *paned, gint allocation, gint child1_req, gint child2_req)
{
}


/* <gtk/gtkpaned.h> */
gboolean _gtk_paned_is_handle_full_size(GtkPaned *paned)
{
}

/* <gtk/gtkpaned.h> */
void _gtk_paned_get_handle_rect(GtkPaned *paned, GdkRectangle *rectangle)
{
}

/* <gtk/gtkpaned.h> */
gint _gtk_paned_get_gutter_size(GtkPaned *paned)
{
}


/* <gtk/gtkpixmap.h> */
#define GTK_TYPE_PIXMAP (gtk_pixmap_get_type ())

/* <gtk/gtkpixmap.h> */
#define GTK_PIXMAP(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PIXMAP, GtkPixmap))

/* <gtk/gtkpixmap.h> */
#define GTK_PIXMAP_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PIXMAP, GtkPixmapClass))

/* <gtk/gtkpixmap.h> */
#define GTK_IS_PIXMAP(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PIXMAP))

/* <gtk/gtkpixmap.h> */
#define GTK_IS_PIXMAP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PIXMAP))



/* <gtk/gtkpixmap.h> */
typedef struct _GtkPixmap
{
	GtkMisc misc;
	 
	GdkPixmap *pixmap;
	GdkBitmap *mask;

	GdkPixmap *pixmap_insensitive;
	guint build_insensitive : 1;
} GtkPixmap;


/* <gtk/gtkpixmap.h> */
typedef struct _GtkPixmapClass
{
	GtkMiscClass parent_class;
} GtkPixmapClass;



/* <gtk/gtkpixmap.h> */
GtkType gtk_pixmap_get_type(void)
{
}

/* <gtk/gtkpixmap.h> */
GtkWidget* gtk_pixmap_new(GdkPixmap *pixmap, GdkBitmap *mask)
{
}

/* <gtk/gtkpixmap.h> */
void gtk_pixmap_set(GtkPixmap *pixmap, GdkPixmap *val, GdkBitmap *mask)
{
}

/* <gtk/gtkpixmap.h> */
void gtk_pixmap_get(GtkPixmap *pixmap, GdkPixmap **val, GdkBitmap **mask)
{
}


/* <gtk/gtkpixmap.h> */
void gtk_pixmap_set_build_insensitive(GtkPixmap *pixmap, guint build)
{
}


/* <gtk/gtkplug.h> */
#define GTK_PLUG(obj) GTK_CHECK_CAST (obj, gtk_plug_get_type (), GtkPlug)

/* <gtk/gtkplug.h> */
#define GTK_PLUG_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_plug_get_type (), GtkPlugClass)

/* <gtk/gtkplug.h> */
#define GTK_IS_PLUG(obj) GTK_CHECK_TYPE (obj, gtk_plug_get_type ())



/* <gtk/gtkplug.h> */
typedef struct _GtkPlug
{
	GtkWindow window;

	GdkWindow *socket_window;
	gint same_app;
} GtkPlug;


/* <gtk/gtkplug.h> */
typedef struct _GtkPlugClass
{
	GtkWindowClass parent_class;
} GtkPlugClass;



/* <gtk/gtkplug.h> */
guint gtk_plug_get_type(void)
{
}

/* <gtk/gtkplug.h> */
void gtk_plug_construct(GtkPlug *plug, guint32 socket_id)
{
}

/* <gtk/gtkplug.h> */
GtkWidget* gtk_plug_new(guint32 socket_id)
{
}


/* <gtk/gtkpreview.h> */
#define GTK_TYPE_PREVIEW (gtk_preview_get_type ())

/* <gtk/gtkpreview.h> */
#define GTK_PREVIEW(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PREVIEW, GtkPreview))

/* <gtk/gtkpreview.h> */
#define GTK_PREVIEW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PREVIEW, GtkPreviewClass))

/* <gtk/gtkpreview.h> */
#define GTK_IS_PREVIEW(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PREVIEW))

/* <gtk/gtkpreview.h> */
#define GTK_IS_PREVIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PREVIEW))



/* <gtk/gtkpreview.h> */
typedef struct _GtkPreview
{
	GtkWidget widget;

	guchar *buffer;
	guint16 buffer_width;
	guint16 buffer_height;

	guint16 bpp;
	guint16 rowstride;

	GdkRgbDither dither;

	guint type : 1;
	guint expand : 1;
} GtkPreview;


/* <gtk/gtkpreview.h> */
typedef struct _GtkPreviewInfo
{
	GdkVisual *visual;
	GdkColormap *cmap;

	guchar *lookup;

	gdouble gamma;
} GtkPreviewInfo;


/* <gtk/gtkpreview.h> */
union _GtkDitherInfo
{
	gushort s[2];
	guchar c[4];
};


/* <gtk/gtkpreview.h> */
typedef struct _GtkPreviewClass
{
	GtkWidgetClass parent_class;

	GtkPreviewInfo info;

} GtkPreviewClass;



/* <gtk/gtkpreview.h> */
GtkType gtk_preview_get_type(void)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_uninit(void)
{
}

/* <gtk/gtkpreview.h> */
GtkWidget* gtk_preview_new(GtkPreviewType type)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_size(GtkPreview *preview, gint width, gint height)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_put(GtkPreview *preview, GdkWindow *window, GdkGC *gc, gint srcx, gint srcy, gint destx, gint desty, gint width, gint height)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_draw_row(GtkPreview *preview, guchar *data, gint x, gint y, gint w)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_set_expand(GtkPreview *preview, gboolean expand)
{
}


/* <gtk/gtkpreview.h> */
void gtk_preview_set_gamma(double gamma)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_set_color_cube(guint nred_shades, guint ngreen_shades, guint nblue_shades, guint ngray_shades)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_set_install_cmap(gint install_cmap)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_set_reserved(gint nreserved)
{
}

/* <gtk/gtkpreview.h> */
void gtk_preview_set_dither(GtkPreview *preview, GdkRgbDither dither)
{
}

/* <gtk/gtkpreview.h> */
GdkVisual* gtk_preview_get_visual(void)
{
}

/* <gtk/gtkpreview.h> */
GdkColormap* gtk_preview_get_cmap(void)
{
}

/* <gtk/gtkpreview.h> */
GtkPreviewInfo* gtk_preview_get_info(void)
{
}

/* This function reinitializes the preview colormap and visual from
 * the current gamma/color_cube/install_cmap settings. It must only
 * be called if there are no previews or users's of the preview
 * colormap in existence.
 */

/* <gtk/gtkpreview.h> */
void gtk_preview_reset(void)
{
}


/* <gtk/gtkprivate.h>  Flags used in the private_flags member of GtkWidget */
typedef enum
{
	PRIVATE_GTK_USER_STYLE = 1 << 0,
	PRIVATE_GTK_REDRAW_PENDING = 1 << 1,
	PRIVATE_GTK_RESIZE_PENDING = 1 << 2,
	PRIVATE_GTK_RESIZE_NEEDED = 1 << 3,
	PRIVATE_GTK_LEAVE_PENDING = 1 << 4,
	PRIVATE_GTK_HAS_SHAPE_MASK = 1 << 5,
	PRIVATE_GTK_IN_REPARENT = 1 << 6,
	PRIVATE_GTK_IS_OFFSCREEN = 1 << 7,
	PRIVATE_GTK_FULLDRAW_PENDING = 1 << 8
} GtkPrivateFlags;

/* Macros for extracting a widgets private_flags from GtkWidget.
 */

/* <gtk/gtkprivate.h> */
#define GTK_PRIVATE_FLAGS(wid) (GTK_WIDGET (wid)->private_flags)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_USER_STYLE(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_USER_STYLE) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_REDRAW_PENDING(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_REDRAW_PENDING) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_CONTAINER_RESIZE_PENDING(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_RESIZE_PENDING) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_RESIZE_NEEDED(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_RESIZE_NEEDED) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_LEAVE_PENDING(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_LEAVE_PENDING) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_HAS_SHAPE_MASK(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_HAS_SHAPE_MASK) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_IN_REPARENT(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_IN_REPARENT) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_IS_OFFSCREEN(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_IS_OFFSCREEN) != 0)

/* <gtk/gtkprivate.h> */
#define GTK_WIDGET_FULLDRAW_PENDING(obj) ((GTK_PRIVATE_FLAGS (obj) & PRIVATE_GTK_FULLDRAW_PENDING) != 0)

/* Macros for setting and clearing private widget flags.
 * we use a preprocessor string concatenation here for a clear
 * flags/private_flags distinction at the cost of single flag operations.
 */

/* <gtk/gtkprivate.h> */
#define GTK_PRIVATE_SET_FLAG(wid,flag) G_STMT_START{ (GTK_PRIVATE_FLAGS (wid) |= (PRIVATE_ ## flag)); }G_STMT_END

/* <gtk/gtkprivate.h> */
#define GTK_PRIVATE_UNSET_FLAG(wid,flag) G_STMT_START{ (GTK_PRIVATE_FLAGS (wid) &= ~(PRIVATE_ ## flag)); }G_STMT_END


/* <gtk/gtkprogressbar.h> */
#define GTK_TYPE_PROGRESS_BAR (gtk_progress_bar_get_type ())

/* <gtk/gtkprogressbar.h> */
#define GTK_PROGRESS_BAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PROGRESS_BAR, GtkProgressBar))

/* <gtk/gtkprogressbar.h> */
#define GTK_PROGRESS_BAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PROGRESS_BAR, GtkProgressBarClass))

/* <gtk/gtkprogressbar.h> */
#define GTK_IS_PROGRESS_BAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PROGRESS_BAR))

/* <gtk/gtkprogressbar.h> */
#define GTK_IS_PROGRESS_BAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PROGRESS_BAR))



/* <gtk/gtkprogressbar.h> */
typedef enum
{
	GTK_PROGRESS_CONTINUOUS,
	GTK_PROGRESS_DISCRETE
} GtkProgressBarStyle;


/* <gtk/gtkprogressbar.h> */
typedef enum
{
	GTK_PROGRESS_LEFT_TO_RIGHT,
	GTK_PROGRESS_RIGHT_TO_LEFT,
	GTK_PROGRESS_BOTTOM_TO_TOP,
	GTK_PROGRESS_TOP_TO_BOTTOM
} GtkProgressBarOrientation;


/* <gtk/gtkprogressbar.h> */
typedef struct _GtkProgressBar
{
	GtkProgress progress;

	GtkProgressBarStyle bar_style;
	GtkProgressBarOrientation orientation;

	guint blocks;
	gint in_block;

	gint activity_pos;
	guint activity_step;
	guint activity_blocks;
	guint activity_dir : 1;
} GtkProgressBar;


/* <gtk/gtkprogressbar.h> */
typedef struct _GtkProgressBarClass
{
	GtkProgressClass parent_class;
} GtkProgressBarClass;



/* <gtk/gtkprogressbar.h> */
GtkType gtk_progress_bar_get_type(void)
{
}

/* <gtk/gtkprogressbar.h> */
GtkWidget* gtk_progress_bar_new(void)
{
}

/* <gtk/gtkprogressbar.h> */
GtkWidget* gtk_progress_bar_new_with_adjustment(GtkAdjustment *adjustment)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_set_bar_style(GtkProgressBar *pbar, GtkProgressBarStyle style)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_set_discrete_blocks(GtkProgressBar *pbar, guint blocks)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_set_activity_step(GtkProgressBar *pbar, guint step)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_set_activity_blocks(GtkProgressBar *pbar, guint blocks)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_set_orientation(GtkProgressBar *pbar, GtkProgressBarOrientation orientation)
{
}

/* <gtk/gtkprogressbar.h> */
void gtk_progress_bar_update(GtkProgressBar *pbar, gfloat percentage)
{
}


/* <gtk/gtkprogress.h> */
#define GTK_TYPE_PROGRESS (gtk_progress_get_type ())

/* <gtk/gtkprogress.h> */
#define GTK_PROGRESS(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_PROGRESS, GtkProgress))

/* <gtk/gtkprogress.h> */
#define GTK_PROGRESS_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_PROGRESS, GtkProgressClass))

/* <gtk/gtkprogress.h> */
#define GTK_IS_PROGRESS(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_PROGRESS))

/* <gtk/gtkprogress.h> */
#define GTK_IS_PROGRESS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PROGRESS))



/* <gtk/gtkprogress.h> */
typedef struct _GtkProgress
{
	GtkWidget widget;

	GtkAdjustment *adjustment;
	GdkPixmap *offscreen_pixmap;
	gchar *format;
	gfloat x_align;
	gfloat y_align;

	guint show_text : 1;
	guint activity_mode : 1;
} GtkProgress;


/* <gtk/gtkprogress.h> */
typedef struct _GtkProgressClass
{
	GtkWidgetClass parent_class;

	void (* paint) (GtkProgress *progress);
	void (* update) (GtkProgress *progress);
	void (* act_mode_enter) (GtkProgress *progress);
} GtkProgressClass;



/* <gtk/gtkprogress.h> */
GtkType gtk_progress_get_type(void)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_show_text(GtkProgress *progress, gint show_text)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_text_alignment(GtkProgress *progress, gfloat x_align, gfloat y_align)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_format_string(GtkProgress *progress, const gchar *format)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_adjustment(GtkProgress *progress, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_configure(GtkProgress *progress, gfloat value, gfloat min, gfloat max)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_percentage(GtkProgress *progress, gfloat percentage)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_value(GtkProgress *progress, gfloat value)
{
}

/* <gtk/gtkprogress.h> */
gfloat gtk_progress_get_value(GtkProgress *progress)
{
}

/* <gtk/gtkprogress.h> */
void gtk_progress_set_activity_mode(GtkProgress *progress, guint activity_mode)
{
}

/* <gtk/gtkprogress.h> */
gchar* gtk_progress_get_current_text(GtkProgress *progress)
{
}

/* <gtk/gtkprogress.h> */
gchar* gtk_progress_get_text_from_value(GtkProgress *progress, gfloat value)
{
}

/* <gtk/gtkprogress.h> */
gfloat gtk_progress_get_current_percentage(GtkProgress *progress)
{
}

/* <gtk/gtkprogress.h> */
gfloat gtk_progress_get_percentage_from_value(GtkProgress *progress, gfloat value)
{
}


/* <gtk/gtkradiobutton.h> */
#define GTK_TYPE_RADIO_BUTTON (gtk_radio_button_get_type ())

/* <gtk/gtkradiobutton.h> */
#define GTK_RADIO_BUTTON(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_RADIO_BUTTON, GtkRadioButton))

/* <gtk/gtkradiobutton.h> */
#define GTK_RADIO_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_RADIO_BUTTON, GtkRadioButtonClass))

/* <gtk/gtkradiobutton.h> */
#define GTK_IS_RADIO_BUTTON(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_RADIO_BUTTON))

/* <gtk/gtkradiobutton.h> */
#define GTK_IS_RADIO_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_RADIO_BUTTON))



/* <gtk/gtkradiobutton.h> */
typedef struct _GtkRadioButton
{
	GtkCheckButton check_button;
	 
	GSList *group;
} GtkRadioButton;


/* <gtk/gtkradiobutton.h> */
typedef struct _GtkRadioButtonClass
{
	GtkCheckButtonClass parent_class;
} GtkRadioButtonClass;



/* <gtk/gtkradiobutton.h> */
GtkType gtk_radio_button_get_type(void)
{
}

/* <gtk/gtkradiobutton.h> */
GtkWidget* gtk_radio_button_new(GSList *group)
{
}

/* <gtk/gtkradiobutton.h> */
GtkWidget* gtk_radio_button_new_from_widget(GtkRadioButton *group)
{
}

/* <gtk/gtkradiobutton.h> */
GtkWidget* gtk_radio_button_new_with_label(GSList *group, const gchar *label)
{
}

/* <gtk/gtkradiobutton.h> */
GtkWidget* gtk_radio_button_new_with_label_from_widget(GtkRadioButton *group, const gchar *label)
{
}

/* <gtk/gtkradiobutton.h> */
GSList* gtk_radio_button_group(GtkRadioButton *radio_button)
{
}

/* <gtk/gtkradiobutton.h> */
void gtk_radio_button_set_group(GtkRadioButton *radio_button, GSList *group)
{
}


/* <gtk/gtkradiomenuitem.h> */
#define GTK_TYPE_RADIO_MENU_ITEM (gtk_radio_menu_item_get_type ())

/* <gtk/gtkradiomenuitem.h> */
#define GTK_RADIO_MENU_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_RADIO_MENU_ITEM, GtkRadioMenuItem))

/* <gtk/gtkradiomenuitem.h> */
#define GTK_RADIO_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_RADIO_MENU_ITEM, GtkRadioMenuItemClass))

/* <gtk/gtkradiomenuitem.h> */
#define GTK_IS_RADIO_MENU_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_RADIO_MENU_ITEM))

/* <gtk/gtkradiomenuitem.h> */
#define GTK_IS_RADIO_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_RADIO_MENU_ITEM))



/* <gtk/gtkradiomenuitem.h> */
typedef struct _GtkRadioMenuItem
{
	GtkCheckMenuItem check_menu_item;
	 
	GSList *group;
} GtkRadioMenuItem;


/* <gtk/gtkradiomenuitem.h> */
typedef struct _GtkRadioMenuItemClass
{
	GtkCheckMenuItemClass parent_class;
} GtkRadioMenuItemClass;



/* <gtk/gtkradiomenuitem.h> */
GtkType gtk_radio_menu_item_get_type(void)
{
}

/* <gtk/gtkradiomenuitem.h> */
GtkWidget* gtk_radio_menu_item_new(GSList *group)
{
}

/* <gtk/gtkradiomenuitem.h> */
GtkWidget* gtk_radio_menu_item_new_with_label(GSList *group, const gchar *label)
{
}

/* <gtk/gtkradiomenuitem.h> */
GSList* gtk_radio_menu_item_group(GtkRadioMenuItem *radio_menu_item)
{
}

/* <gtk/gtkradiomenuitem.h> */
void gtk_radio_menu_item_set_group(GtkRadioMenuItem *radio_menu_item, GSList *group)
{
}


/* <gtk/gtkrange.h> */
#define GTK_TYPE_RANGE (gtk_range_get_type ())

/* <gtk/gtkrange.h> */
#define GTK_RANGE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_RANGE, GtkRange))

/* <gtk/gtkrange.h> */
#define GTK_RANGE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_RANGE, GtkRangeClass))

/* <gtk/gtkrange.h> */
#define GTK_IS_RANGE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_RANGE))

/* <gtk/gtkrange.h> */
#define GTK_IS_RANGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_RANGE))



/* <gtk/gtkrange.h> */
typedef struct _GtkRange
{
	GtkWidget widget;

	GdkWindow *trough;
	GdkWindow *slider;
	GdkWindow *step_forw;
	GdkWindow *step_back;

	gint16 x_click_point;
	gint16 y_click_point;

	guint8 button;
	gint8 digits;
	guint policy : 2;
	guint scroll_type : 3;
	guint in_child : 3;
	guint click_child : 3;
	guint need_timer : 1;

	guint32 timer;

	gfloat old_value;
	gfloat old_lower;
	gfloat old_upper;
	gfloat old_page_size;

	GtkAdjustment *adjustment;
} GtkRange;


/* <gtk/gtkrange.h> */
typedef struct _GtkRangeClass
{
	GtkWidgetClass parent_class;

	gint slider_width;
	gint stepper_size;
	gint stepper_slider_spacing;
	gint min_slider_size;

	guint8 trough;
	guint8 slider;
	guint8 step_forw;
	guint8 step_back;

	void (* draw_background) (GtkRange *range);
	void (* clear_background) (GtkRange *range);
	void (* draw_trough) (GtkRange *range);
	void (* draw_slider) (GtkRange *range);
	void (* draw_step_forw) (GtkRange *range);
	void (* draw_step_back) (GtkRange *range);
	void (* slider_update) (GtkRange *range);
	gint (* trough_click) (GtkRange *range, gint x, gint y, gfloat *jump_perc);
	gint (* trough_keys) (GtkRange *range, GdkEventKey *key, GtkScrollType *scroll, GtkTroughType *trough);
	void (* motion) (GtkRange *range, gint xdelta, gint ydelta);
	gint (* timer) (GtkRange *range);
} GtkRangeClass;



/* <gtk/gtkrange.h> */
GtkType gtk_range_get_type(void)
{
}

/* <gtk/gtkrange.h> */
GtkAdjustment* gtk_range_get_adjustment(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_set_update_policy(GtkRange *range, GtkUpdateType policy)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_set_adjustment(GtkRange *range, GtkAdjustment *adjustment)
{
}


/* <gtk/gtkrange.h> */
void gtk_range_draw_background(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_clear_background(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_draw_trough(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_draw_slider(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_draw_step_forw(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_draw_step_back(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_slider_update(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
gint gtk_range_trough_click(GtkRange *range, gint x, gint y, gfloat *jump_perc)
{
}


/* <gtk/gtkrange.h> */
void gtk_range_default_hslider_update(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_default_vslider_update(GtkRange *range)
{
}

/* <gtk/gtkrange.h> */
gint gtk_range_default_htrough_click(GtkRange *range, gint x, gint y, gfloat *jump_perc)
{
}

/* <gtk/gtkrange.h> */
gint gtk_range_default_vtrough_click(GtkRange *range, gint x, gint y, gfloat *jump_perc)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_default_hmotion(GtkRange *range, gint xdelta, gint ydelta)
{
}

/* <gtk/gtkrange.h> */
void gtk_range_default_vmotion(GtkRange *range, gint xdelta, gint ydelta)
{
}


/* <gtk/gtkrange.h> */
void _gtk_range_get_props(GtkRange *range, gint *slider_width, gint *trough_border, gint *stepper_size, gint *stepper_spacing)
{
}


/* <gtk/gtkrc.h> */
typedef enum {
	GTK_RC_FG = 1 << 0,
	GTK_RC_BG = 1 << 1,
	GTK_RC_TEXT = 1 << 2,
	GTK_RC_BASE = 1 << 3
} GtkRcFlags;


/* <gtk/gtkrc.h> */
typedef struct _GtkRcStyle
{
	gchar *name;
	gchar *font_name;
	gchar *fontset_name;
	gchar *bg_pixmap_name[5];

	GtkRcFlags color_flags[5];
	GdkColor fg[5];
	GdkColor bg[5];
	GdkColor text[5];
	GdkColor base[5];

	GtkThemeEngine *engine;
	gpointer engine_data;
} GtkRcStyle;


/* <gtk/gtkrc.h> */
void gtk_rc_init(void)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_add_default_file(const gchar *filename)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_set_default_files(gchar **filenames)
{
}

/* <gtk/gtkrc.h> */
gchar** gtk_rc_get_default_files(void)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_parse(const gchar *filename)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_parse_string(const gchar *rc_string)
{
}

/* <gtk/gtkrc.h> */
gboolean gtk_rc_reparse_all(void)
{
}

/* <gtk/gtkrc.h> */
GtkStyle* gtk_rc_get_style(GtkWidget *widget)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_add_widget_name_style(GtkRcStyle *rc_style, const gchar *pattern)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_add_widget_class_style(GtkRcStyle *rc_style, const gchar *pattern)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_add_class_style(GtkRcStyle *rc_style, const gchar *pattern)
{
}


/* <gtk/gtkrc.h> */
GtkRcStyle* gtk_rc_style_new(void)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_style_ref(GtkRcStyle *rc_style)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_style_unref(GtkRcStyle *rc_style)
{
}

/* Tell gtkrc to use a custom routine to load images specified in rc files instead of
 * the default xpm-only loader
 */

/* <gtk/gtkrc.h> */
typedef GdkPixmap* (*GtkImageLoader) (GdkWindow *window, GdkColormap *colormap, GdkBitmap **mask, GdkColor *transparent_color, const gchar *filename)
{
}

/* <gtk/gtkrc.h> */
void gtk_rc_set_image_loader(GtkImageLoader loader)
{
}


/* <gtk/gtkrc.h> */
GdkPixmap* gtk_rc_load_image(GdkColormap *colormap, GdkColor *transparent_color, const gchar *filename)
{
}

/* <gtk/gtkrc.h> */
gchar* gtk_rc_find_pixmap_in_path(GScanner *scanner, const gchar *pixmap_file)
{
}

/* <gtk/gtkrc.h> */
gchar* gtk_rc_find_module_in_path(const gchar *module_file)
{
}

/* <gtk/gtkrc.h> */
gchar* gtk_rc_get_theme_dir(void)
{
}

/* <gtk/gtkrc.h> */
gchar* gtk_rc_get_module_dir(void)
{
}

/* private functions/definitions */

/* <gtk/gtkrc.h> */
typedef enum {
	GTK_RC_TOKEN_INVALID = G_TOKEN_LAST,
	GTK_RC_TOKEN_INCLUDE,
	GTK_RC_TOKEN_NORMAL,
	GTK_RC_TOKEN_ACTIVE,
	GTK_RC_TOKEN_PRELIGHT,
	GTK_RC_TOKEN_SELECTED,
	GTK_RC_TOKEN_INSENSITIVE,
	GTK_RC_TOKEN_FG,
	GTK_RC_TOKEN_BG,
	GTK_RC_TOKEN_BASE,
	GTK_RC_TOKEN_TEXT,
	GTK_RC_TOKEN_FONT,
	GTK_RC_TOKEN_FONTSET,
	GTK_RC_TOKEN_BG_PIXMAP,
	GTK_RC_TOKEN_PIXMAP_PATH,
	GTK_RC_TOKEN_STYLE,
	GTK_RC_TOKEN_BINDING,
	GTK_RC_TOKEN_BIND,
	GTK_RC_TOKEN_WIDGET,
	GTK_RC_TOKEN_WIDGET_CLASS,
	GTK_RC_TOKEN_CLASS,
	GTK_RC_TOKEN_LOWEST,
	GTK_RC_TOKEN_GTK,
	GTK_RC_TOKEN_APPLICATION,
	GTK_RC_TOKEN_RC,
	GTK_RC_TOKEN_HIGHEST,
	GTK_RC_TOKEN_ENGINE,
	GTK_RC_TOKEN_MODULE_PATH,
	GTK_RC_TOKEN_LAST
} GtkRcTokenType;


/* <gtk/gtkrc.h> */
guint gtk_rc_parse_color(GScanner *scanner, GdkColor *color)
{
}

/* <gtk/gtkrc.h> */
guint gtk_rc_parse_state(GScanner *scanner, GtkStateType *state)
{
}

/* <gtk/gtkrc.h> */
guint gtk_rc_parse_priority(GScanner *scanner, GtkPathPriorityType *priority)
{
}
	 

/* <gtk/gtkruler.h> */
#define GTK_TYPE_RULER (gtk_ruler_get_type ())

/* <gtk/gtkruler.h> */
#define GTK_RULER(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_RULER, GtkRuler))

/* <gtk/gtkruler.h> */
#define GTK_RULER_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_RULER, GtkRulerClass))

/* <gtk/gtkruler.h> */
#define GTK_IS_RULER(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_RULER))

/* <gtk/gtkruler.h> */
#define GTK_IS_RULER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_RULER))


/* <gtk/gtkruler.h> */
typedef struct _GtkRuler
{
	GtkWidget widget;

	GdkPixmap *backing_store;
	GdkGC *non_gr_exp_gc;
	GtkRulerMetric *metric;
	gint xsrc, ysrc;
	gint slider_size;

	 /* The upper limit of the ruler (in points) */
	gfloat lower;
	 /* The lower limit of the ruler */
	gfloat upper;
	 /* The position of the mark on the ruler */
	gfloat position;
	 /* The maximum size of the ruler */
	gfloat max_size;
} GtkRuler;


/* <gtk/gtkruler.h> */
typedef struct _GtkRulerClass
{
	GtkWidgetClass parent_class;

	void (* draw_ticks) (GtkRuler *ruler);
	void (* draw_pos) (GtkRuler *ruler);
} GtkRulerClass;


/* <gtk/gtkruler.h> */
typedef struct _GtkRulerMetric
{
	gchar *metric_name;
	gchar *abbrev;
	 /* This should be points_per_unit. This is the size of the unit
 * in 1/72nd's of an inch and has nothing to do with screen pixels */
	gfloat pixels_per_unit;
	gfloat ruler_scale[10];
	gint subdivide[5];  /* five possible modes of subdivision */
} GtkRulerMetric;



/* <gtk/gtkruler.h> */
GtkType gtk_ruler_get_type(void)
{
}

/* <gtk/gtkruler.h> */
void gtk_ruler_set_metric(GtkRuler *ruler, GtkMetricType metric)
{
}

/* <gtk/gtkruler.h> */
void gtk_ruler_set_range(GtkRuler *ruler, gfloat lower, gfloat upper, gfloat position, gfloat max_size)
{
}

/* <gtk/gtkruler.h> */
void gtk_ruler_draw_ticks(GtkRuler *ruler)
{
}

/* <gtk/gtkruler.h> */
void gtk_ruler_draw_pos(GtkRuler *ruler)
{
}


/* <gtk/gtkscale.h> */
#define GTK_TYPE_SCALE (gtk_scale_get_type ())

/* <gtk/gtkscale.h> */
#define GTK_SCALE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SCALE, GtkScale))

/* <gtk/gtkscale.h> */
#define GTK_SCALE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCALE, GtkScaleClass))

/* <gtk/gtkscale.h> */
#define GTK_IS_SCALE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SCALE))

/* <gtk/gtkscale.h> */
#define GTK_IS_SCALE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCALE))



/* <gtk/gtkscale.h> */
typedef struct _GtkScale
{
	GtkRange range;

	guint draw_value : 1;
	guint value_pos : 2;
} GtkScale;


/* <gtk/gtkscale.h> */
typedef struct _GtkScaleClass
{
	GtkRangeClass parent_class;

	gint slider_length;
	gint value_spacing;
	 
	void (* draw_value) (GtkScale *scale);
} GtkScaleClass;



/* <gtk/gtkscale.h> */
GtkType gtk_scale_get_type(void)
{
}

/* <gtk/gtkscale.h> */
void gtk_scale_set_digits(GtkScale *scale, gint digits)
{
}

/* <gtk/gtkscale.h> */
void gtk_scale_set_draw_value(GtkScale *scale, gboolean draw_value)
{
}

/* <gtk/gtkscale.h> */
void gtk_scale_set_value_pos(GtkScale *scale, GtkPositionType pos)
{
}

/* <gtk/gtkscale.h> */
gint gtk_scale_get_value_width(GtkScale *scale)
{
}


/* <gtk/gtkscale.h> */
void gtk_scale_draw_value(GtkScale *scale)
{
}


/* <gtk/gtkscrollbar.h> */
#define GTK_TYPE_SCROLLBAR (gtk_scrollbar_get_type ())

/* <gtk/gtkscrollbar.h> */
#define GTK_SCROLLBAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SCROLLBAR, GtkScrollbar))

/* <gtk/gtkscrollbar.h> */
#define GTK_SCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCROLLBAR, GtkScrollbarClass))

/* <gtk/gtkscrollbar.h> */
#define GTK_IS_SCROLLBAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SCROLLBAR))

/* <gtk/gtkscrollbar.h> */
#define GTK_IS_SCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCROLLBAR))



/* <gtk/gtkscrollbar.h> */
typedef struct _GtkScrollbar
{
	GtkRange range;
} GtkScrollbar;


/* <gtk/gtkscrollbar.h> */
typedef struct _GtkScrollbarClass
{
	GtkRangeClass parent_class;
} GtkScrollbarClass;



/* <gtk/gtkscrollbar.h> */
GtkType gtk_scrollbar_get_type(void)
{
}


/* <gtk/gtkscrolledwindow.h> */
#define GTK_TYPE_SCROLLED_WINDOW (gtk_scrolled_window_get_type ())

/* <gtk/gtkscrolledwindow.h> */
#define GTK_SCROLLED_WINDOW(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SCROLLED_WINDOW, GtkScrolledWindow))

/* <gtk/gtkscrolledwindow.h> */
#define GTK_SCROLLED_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCROLLED_WINDOW, GtkScrolledWindowClass))

/* <gtk/gtkscrolledwindow.h> */
#define GTK_IS_SCROLLED_WINDOW(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SCROLLED_WINDOW))

/* <gtk/gtkscrolledwindow.h> */
#define GTK_IS_SCROLLED_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCROLLED_WINDOW))



/* <gtk/gtkscrolledwindow.h> */
typedef struct _GtkScrolledWindow
{
	GtkBin container;

	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

	guint hscrollbar_policy : 2;
	guint vscrollbar_policy : 2;
	guint hscrollbar_visible : 1;
	guint vscrollbar_visible : 1;
	guint window_placement : 2;
} GtkScrolledWindow;


/* <gtk/gtkscrolledwindow.h> */
typedef struct _GtkScrolledWindowClass
{
	GtkBinClass parent_class;
	 
	gint scrollbar_spacing;
} GtkScrolledWindowClass;



/* <gtk/gtkscrolledwindow.h> */
GtkType gtk_scrolled_window_get_type(void)
{
}

/* <gtk/gtkscrolledwindow.h> */
GtkWidget* gtk_scrolled_window_new(GtkAdjustment *hadjustment, GtkAdjustment *vadjustment)
{
}

/* <gtk/gtkscrolledwindow.h> */
void gtk_scrolled_window_set_hadjustment(GtkScrolledWindow *scrolled_window, GtkAdjustment *hadjustment)
{
}

/* <gtk/gtkscrolledwindow.h> */
void gtk_scrolled_window_set_vadjustment(GtkScrolledWindow *scrolled_window, GtkAdjustment *hadjustment)
{
}

/* <gtk/gtkscrolledwindow.h> */
GtkAdjustment* gtk_scrolled_window_get_hadjustment(GtkScrolledWindow *scrolled_window)
{
}

/* <gtk/gtkscrolledwindow.h> */
GtkAdjustment* gtk_scrolled_window_get_vadjustment(GtkScrolledWindow *scrolled_window)
{
}

/* <gtk/gtkscrolledwindow.h> */
void gtk_scrolled_window_set_policy(GtkScrolledWindow *scrolled_window, GtkPolicyType hscrollbar_policy, GtkPolicyType vscrollbar_policy)
{
}

/* <gtk/gtkscrolledwindow.h> */
void gtk_scrolled_window_set_placement(GtkScrolledWindow *scrolled_window, GtkCornerType window_placement)
{
}

/* <gtk/gtkscrolledwindow.h> */
void gtk_scrolled_window_add_with_viewport(GtkScrolledWindow *scrolled_window, GtkWidget *child)
{
}


/* <gtk/gtkselection.h> */
typedef struct _GtkTargetEntry
{
	gchar *target;
	guint flags;
	guint info;
} GtkTargetEntry;



/* <gtk/gtkselection.h> */
typedef struct _GtkTargetList
{
	GList *list;
	guint ref_count;
} GtkTargetList;


/* <gtk/gtkselection.h> */
typedef struct _GtkTargetPair
{
	GdkAtom target;
	guint flags;
	guint info;
} GtkTargetPair;


/* <gtk/gtkselection.h> */
GtkTargetList *gtk_target_list_new(const GtkTargetEntry *targets, guint ntargets)
{
}

/* <gtk/gtkselection.h> */
void gtk_target_list_ref(GtkTargetList *list)
{
}

/* <gtk/gtkselection.h> */
void gtk_target_list_unref(GtkTargetList *list)
{
}

/* <gtk/gtkselection.h> */
void gtk_target_list_add(GtkTargetList *list, GdkAtom target, guint flags, guint info)
{
}

/* <gtk/gtkselection.h> */
void gtk_target_list_add_table(GtkTargetList *list, const GtkTargetEntry *targets, guint ntargets)
{
}

/* <gtk/gtkselection.h> */
void gtk_target_list_remove(GtkTargetList *list, GdkAtom target)
{
}

/* <gtk/gtkselection.h> */
gboolean gtk_target_list_find(GtkTargetList *list, GdkAtom target, guint *info)
{
}

/* Public interface */


/* <gtk/gtkselection.h> */
gint gtk_selection_owner_set(GtkWidget *widget, GdkAtom selection, guint32 time)
{
}

/* <gtk/gtkselection.h> */
void gtk_selection_add_target(GtkWidget *widget, GdkAtom selection, GdkAtom target, guint info)
{
}

/* <gtk/gtkselection.h> */
void gtk_selection_add_targets(GtkWidget *widget, GdkAtom selection, const GtkTargetEntry *targets, guint ntargets)
{
}

/* <gtk/gtkselection.h> */
gint gtk_selection_convert(GtkWidget *widget, GdkAtom selection, GdkAtom target, guint32 time)
{
}



/* <gtk/gtkselection.h> */
void gtk_selection_data_set(GtkSelectionData *selection_data, GdkAtom type, gint format, const guchar *data, gint length)
{
}

/* Called when a widget is destroyed */


/* <gtk/gtkselection.h> */
void gtk_selection_remove_all(GtkWidget *widget)
{
}

/* Event handlers */


/* <gtk/gtkselection.h> */
gint gtk_selection_clear(GtkWidget *widget, GdkEventSelection *event)
{
}

/* <gtk/gtkselection.h> */
gint gtk_selection_request(GtkWidget *widget, GdkEventSelection *event)
{
}

/* <gtk/gtkselection.h> */
gint gtk_selection_incr_event(GdkWindow *window, GdkEventProperty *event)
{
}

/* <gtk/gtkselection.h> */
gint gtk_selection_notify(GtkWidget *widget, GdkEventSelection *event)
{
}

/* <gtk/gtkselection.h> */
gint gtk_selection_property_notify(GtkWidget *widget, GdkEventProperty *event)
{
}

/* <gtk/gtkselection.h> */
GtkSelectioData *gtk_selection_data_copy(GtkSelectionData *data)
{
}

/* <gtk/gtkselection.h> */
void gtk_selection_data_free(GtkSelectionData *data)
{
}


/* <gtk/gtkseparator.h> */
#define GTK_TYPE_SEPARATOR (gtk_separator_get_type ())

/* <gtk/gtkseparator.h> */
#define GTK_SEPARATOR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SEPARATOR, GtkSeparator))

/* <gtk/gtkseparator.h> */
#define GTK_SEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SEPARATOR, GtkSeparatorClass))

/* <gtk/gtkseparator.h> */
#define GTK_IS_SEPARATOR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SEPARATOR))

/* <gtk/gtkseparator.h> */
#define GTK_IS_SEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SEPARATOR))



/* <gtk/gtkseparator.h> */
typedef struct _GtkSeparator
{
	GtkWidget widget;
} GtkSeparator;


/* <gtk/gtkseparator.h> */
typedef struct _GtkSeparatorClass
{
	GtkWidgetClass parent_class;
} GtkSeparatorClass;



/* <gtk/gtkseparator.h> */
GtkType gtk_separator_get_type(void)
{
}


/* <gtk/gtksignal.h> */
#define GTK_SIGNAL_OFFSET(struct, field) (GTK_STRUCT_OFFSET (struct, field))
	 
	 

/* <gtk/gtksignal.h> */
typedef void(*GtkSignalMarshal) (GtkObject *object, gpointer data, guint nparams, GtkArg *args, GtkType *arg_types, GtkType return_type)
{
}

/* <gtk/gtksignal.h> */
typedef void(*GtkSignalDestroy) (gpointer data)
{
}

/* <gtk/gtksignal.h> */
typedef gboolean(*GtkEmissionHook) (GtkObject *object, guint signal_id, guint n_params, GtkArg *params, gpointer data)
{
}


/* <gtk/gtksignal.h> */
struct _GtkSignalQuery
{
	GtkType object_type;
	guint signal_id;
	const gchar *signal_name;
	guint is_user_signal : 1;
	GtkSignalRunType signal_flags;
	GtkType return_val;
	guint nparams;
	const GtkType *params;
};


/* Application-level methods */

/* <gtk/gtksignal.h> */
guint gtk_signal_lookup(const gchar *name, GtkType object_type)
{
}

/* <gtk/gtksignal.h> */
gchar* gtk_signal_name(guint signal_id)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_n_emissions(GtkObject *object, guint signal_id)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_n_emissions_by_name(GtkObject *object, const gchar *name)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emit_stop(GtkObject *object, guint signal_id)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emit_stop_by_name(GtkObject *object, const gchar *name)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_connect(GtkObject *object, const gchar *name, GtkSignalFunc func, gpointer func_data)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_connect_after(GtkObject *object, const gchar *name, GtkSignalFunc func, gpointer func_data)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_connect_object(GtkObject *object, const gchar *name, GtkSignalFunc func, GtkObject *slot_object)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_connect_object_after(GtkObject *object, const gchar *name, GtkSignalFunc func, GtkObject *slot_object)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_connect_full(GtkObject *object, const gchar *name, GtkSignalFunc func, GtkCallbackMarshal marshal, gpointer data, GtkDestroyNotify destroy_func, gint object_signal, gint after)
{
}


/* <gtk/gtksignal.h> */
void gtk_signal_connect_object_while_alive(GtkObject *object, const gchar *signal, GtkSignalFunc func, GtkObject *alive_object)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_connect_while_alive(GtkObject *object, const gchar *signal, GtkSignalFunc func, gpointer func_data, GtkObject *alive_object)
{
}


/* <gtk/gtksignal.h> */
void gtk_signal_disconnect(GtkObject *object, guint handler_id)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_disconnect_by_func(GtkObject *object, GtkSignalFunc func, gpointer data)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_disconnect_by_data(GtkObject *object, gpointer data)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_block(GtkObject *object, guint handler_id)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_block_by_func(GtkObject *object, GtkSignalFunc func, gpointer data)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_block_by_data(GtkObject *object, gpointer data)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_unblock(GtkObject *object, guint handler_id)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_unblock_by_func(GtkObject *object, GtkSignalFunc func, gpointer data)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_handler_unblock_by_data(GtkObject *object, gpointer data)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_handler_pending(GtkObject *object, guint signal_id, gboolean may_be_blocked)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_handler_pending_by_func(GtkObject *object, guint signal_id, gboolean may_be_blocked, GtkSignalFunc func, gpointer data)
{
}

/* <gtk/gtksignal.h> */
gint gtk_signal_handler_pending_by_id(GtkObject *object, guint handler_id, gboolean may_be_blocked)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_add_emission_hook(guint signal_id, GtkEmissionHook hook_func, gpointer data)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_add_emission_hook_full(guint signal_id, GtkEmissionHook hook_func, gpointer data, GDestroyNotify destroy)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_remove_emission_hook(guint signal_id, guint hook_id)
{
}

/* Report internal information about a signal. The caller has the
 * responsibility to invoke a subsequent g_free (returned_data); but
 * must not modify data pointed to by the members of GtkSignalQuery 
 */

/* <gtk/gtksignal.h> */
GtkSignalQuery* gtk_signal_query(guint signal_id)
{
}


/* Widget-level methods */

/* <gtk/gtksignal.h> */
void gtk_signal_init(void)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_new(const gchar *name, GtkSignalRunType signal_flags, GtkType object_type, guint function_offset, GtkSignalMarshaller marshaller, GtkType return_val, guint nparams, ...)
{
}

/* <gtk/gtksignal.h> */
guint gtk_signal_newv(const gchar *name, GtkSignalRunType signal_flags, GtkType object_type, guint function_offset, GtkSignalMarshaller marshaller, GtkType return_val, guint nparams, GtkType *params)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emit(GtkObject *object, guint signal_id, ...)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emit_by_name(GtkObject *object, const gchar *name, ...)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emitv(GtkObject *object, guint signal_id, GtkArg *params)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_emitv_by_name(GtkObject *object, const gchar *name, GtkArg *params)
{
}
/* Non-public methods */

/* <gtk/gtksignal.h> */
void gtk_signal_handlers_destroy(GtkObject *object)
{
}

/* <gtk/gtksignal.h> */
void gtk_signal_set_funcs(GtkSignalMarshal marshal_func, GtkSignalDestroy destroy_func)
{
}
	 

/* <gtk/gtksocket.h> */
#define GTK_SOCKET(obj) GTK_CHECK_CAST (obj, gtk_socket_get_type (), GtkSocket)

/* <gtk/gtksocket.h> */
#define GTK_SOCKET_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_socket_get_type (), GtkSocketClass)

/* <gtk/gtksocket.h> */
#define GTK_IS_SOCKET(obj) GTK_CHECK_TYPE (obj, gtk_socket_get_type ())



/* <gtk/gtksocket.h> */
typedef struct _GtkSocket
{
	GtkContainer container;

	guint16 request_width;
	guint16 request_height;
	guint16 current_width;
	guint16 current_height;
	 
	GdkWindow *plug_window;
	guint same_app : 1;
	guint focus_in : 1;
	guint have_size : 1;
	guint need_map : 1;
} GtkSocket;


/* <gtk/gtksocket.h> */
typedef struct _GtkSocketClass
{
	GtkContainerClass parent_class;
} GtkSocketClass;



/* <gtk/gtksocket.h> */
GtkWidget* gtk_socket_new(void)
{
}

/* <gtk/gtksocket.h> */
guint gtk_socket_get_type(void )
{
}

/* <gtk/gtksocket.h> */
void gtk_socket_steal(GtkSocket *socket, guint32 wid)
{
}


/* <gtk/gtkspinbutton.h> */
#define GTK_TYPE_SPIN_BUTTON (gtk_spin_button_get_type ())

/* <gtk/gtkspinbutton.h> */
#define GTK_SPIN_BUTTON(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_SPIN_BUTTON, GtkSpinButton))

/* <gtk/gtkspinbutton.h> */
#define GTK_SPIN_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_SPIN_BUTTON, GtkSpinButtonClass))

/* <gtk/gtkspinbutton.h> */
#define GTK_IS_SPIN_BUTTON(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_SPIN_BUTTON))

/* <gtk/gtkspinbutton.h> */
#define GTK_IS_SPIN_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SPIN_BUTTON))



/* <gtk/gtkspinbutton.h> */
typedef enum
{
	GTK_UPDATE_ALWAYS,
	GTK_UPDATE_IF_VALID
} GtkSpinButtonUpdatePolicy;


/* <gtk/gtkspinbutton.h> */
typedef enum
{
	GTK_SPIN_STEP_FORWARD,
	GTK_SPIN_STEP_BACKWARD,
	GTK_SPIN_PAGE_FORWARD,
	GTK_SPIN_PAGE_BACKWARD,
	GTK_SPIN_HOME,
	GTK_SPIN_END,
	GTK_SPIN_USER_DEFINED
} GtkSpinType;



/* <gtk/gtkspinbutton.h> */
typedef struct _GtkSpinButton
{
	GtkEntry entry;
	 
	GtkAdjustment *adjustment;
	 
	GdkWindow *panel;
	GtkShadowType shadow_type;
	 
	guint32 timer;
	guint32 ev_time;
	 
	gfloat climb_rate;
	gfloat timer_step;
	 
	GtkSpinButtonUpdatePolicy update_policy;
	 
	guint in_child : 2;
	guint click_child : 2;
	guint button : 2;
	guint need_timer : 1;
	guint timer_calls : 3;
	guint digits : 3;
	guint numeric : 1;
	guint wrap : 1;
	guint snap_to_ticks : 1;
} GtkSpinButton;


/* <gtk/gtkspinbutton.h> */
typedef struct _GtkSpinButtonClass
{
	GtkEntryClass parent_class;
} GtkSpinButtonClass;



/* <gtk/gtkspinbutton.h> */
GtkType gtk_spin_button_get_type(void)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_configure(GtkSpinButton *spin_button, GtkAdjustment *adjustment, gfloat climb_rate, guint digits)
{
}


/* <gtk/gtkspinbutton.h> */
GtkWidget* gtk_spin_button_new(GtkAdjustment *adjustment, gfloat climb_rate, guint digits)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_adjustment(GtkSpinButton *spin_button, GtkAdjustment *adjustment)
{
}


/* <gtk/gtkspinbutton.h> */
GtkAdjustment* gtk_spin_button_get_adjustment(GtkSpinButton *spin_button)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_digits(GtkSpinButton *spin_button, guint digits)
{
}


/* <gtk/gtkspinbutton.h> */
gfloat gtk_spin_button_get_value_as_float(GtkSpinButton *spin_button)
{
}


/* <gtk/gtkspinbutton.h> */
gint gtk_spin_button_get_value_as_int(GtkSpinButton *spin_button)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_value(GtkSpinButton *spin_button, gfloat value)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_update_policy(GtkSpinButton *spin_button, GtkSpinButtonUpdatePolicy policy)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_numeric(GtkSpinButton *spin_button, gboolean numeric)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_spin(GtkSpinButton *spin_button, GtkSpinType direction, gfloat increment)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_wrap(GtkSpinButton *spin_button, gboolean wrap)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_shadow_type(GtkSpinButton *spin_button, GtkShadowType shadow_type)
{
}


/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_set_snap_to_ticks(GtkSpinButton *spin_button, gboolean snap_to_ticks)
{
}

/* <gtk/gtkspinbutton.h> */
void gtk_spin_button_update(GtkSpinButton *spin_button)
{
}


/* <gtk/gtkstatusbar.h> */
#define GTK_STATUSBAR(obj) GTK_CHECK_CAST (obj, gtk_statusbar_get_type (), GtkStatusbar)

/* <gtk/gtkstatusbar.h> */
#define GTK_STATUSBAR_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_statusbar_get_type (), GtkStatusbarClass)

/* <gtk/gtkstatusbar.h> */
#define GTK_IS_STATUSBAR(obj) GTK_CHECK_TYPE (obj, gtk_statusbar_get_type ())


/* <gtk/gtkstatusbar.h> */
typedef struct _GtkStatusbar
{
	GtkHBox parent_widget;

	GtkWidget *frame;
	GtkWidget *label;

	GSList *messages;
	GSList *keys;

	guint seq_context_id;
	guint seq_message_id;
} GtkStatusbar;


/* <gtk/gtkstatusbar.h> */
typedef struct _GtkStatusbarClass
{
	GtkHBoxClass parent_class;

	GMemChunk *messages_mem_chunk;

	void (*text_pushed) (GtkStatusbar *statusbar, guint context_id, const gchar *text);
	void (*text_popped) (GtkStatusbar *statusbar, guint context_id, const gchar *text);
} GtkStatusbarClass;


/* <gtk/gtkstatusbar.h> */
typedef struct _GtkStatusbarMsg
{
	gchar *text;
	guint context_id;
	guint message_id;
} GtkStatusbarMsg;


/* <gtk/gtkstatusbar.h> */
guint gtk_statusbar_get_type(void)
{
}

/* <gtk/gtkstatusbar.h> */
GtkWidget* gtk_statusbar_new(void)
{
}

/* <gtk/gtkstatusbar.h> */
guint gtk_statusbar_get_context_id(GtkStatusbar *statusbar, const gchar *context_description)
{
}
/* Returns message_id used for gtk_statusbar_remove */

/* <gtk/gtkstatusbar.h> */
guint gtk_statusbar_push(GtkStatusbar *statusbar, guint context_id, const gchar *text)
{
}

/* <gtk/gtkstatusbar.h> */
void gtk_statusbar_pop(GtkStatusbar *statusbar, guint context_id)
{
}

/* <gtk/gtkstatusbar.h> */
void gtk_statusbar_remove(GtkStatusbar *statusbar, guint context_id, guint message_id)
{
}


/* <gtk/gtkstyle.h> */
#define GTK_STYLE_NUM_STYLECOLORS() (7 * 5)


/* <gtk/gtkstyle.h> */
#define GTK_STYLE_ATTACHED(style) (((GtkStyle*) (style))->attach_count > 0)


/* <gtk/gtkstyle.h> */
typedef struct _GtkStyle
{
	GtkStyleClass *klass;

	GdkColor fg[5];
	GdkColor bg[5];
	GdkColor light[5];
	GdkColor dark[5];
	GdkColor mid[5];
	GdkColor text[5];
	GdkColor base[5];
	 
	GdkColor black;
	GdkColor white;
	GdkFont *font;
	 
	GdkGC *fg_gc[5];
	GdkGC *bg_gc[5];
	GdkGC *light_gc[5];
	GdkGC *dark_gc[5];
	GdkGC *mid_gc[5];
	GdkGC *text_gc[5];
	GdkGC *base_gc[5];
	GdkGC *black_gc;
	GdkGC *white_gc;
	 
	GdkPixmap *bg_pixmap[5];
	 
	 /* private */
	 
	gint ref_count;
	gint attach_count;
	 
	gint depth;
	GdkColormap *colormap;
	 
	GtkThemeEngine *engine;
	 
	gpointer engine_data;
	 
	GtkRcStyle *rc_style; /* the Rc style from which this style
 * was created
 */
	GSList *styles;
} GtkStyle;


/* <gtk/gtkstyle.h> */
typedef struct _GtkStyleClass
{
	gint xthickness;
	gint ythickness;
	 
	void (*draw_hline) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x1, gint x2, gint y);
	void (*draw_vline) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint y1, gint y2, gint x);
	void (*draw_shadow) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_polygon) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GdkPoint *point, gint npoints, gboolean fill);
	void (*draw_arrow) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GtkArrowType arrow_type, gboolean fill, gint x, gint y, gint width, gint height);
	void (*draw_diamond) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_oval) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_string) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, const gchar *string);
	void (*draw_box) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_flat_box) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_check) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_option) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_cross) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_ramp) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GtkArrowType arrow_type, gint x, gint y, gint width, gint height);
	void (*draw_tab) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height); 
	void (*draw_shadow_gap) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width);
	void (*draw_box_gap) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width);
	void (*draw_extension) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side);
	void (*draw_focus) (GtkStyle *style, GdkWindow *window, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height);
	void (*draw_slider) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation);
	void (*draw_handle) (GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation);
} GtkStyleClass;


/* <gtk/gtkstyle.h> */
GtkStyle* gtk_style_new(void)
{
}

/* <gtk/gtkstyle.h> */
GtkStyle* gtk_style_copy(GtkStyle *style)
{
}

/* <gtk/gtkstyle.h> */
GtkStyle* gtk_style_attach(GtkStyle *style, GdkWindow *window)
{
}

/* <gtk/gtkstyle.h> */
void gtk_style_detach(GtkStyle *style)
{
}

/* <gtk/gtkstyle.h> */
GtkStyle* gtk_style_ref(GtkStyle *style)
{
}

/* <gtk/gtkstyle.h> */
void gtk_style_unref(GtkStyle *style)
{
}

/* <gtk/gtkstyle.h> */
void gtk_style_set_background(GtkStyle *style, GdkWindow *window, GtkStateType state_type)
{
}

/* <gtk/gtkstyle.h> */
void gtk_style_apply_default_background(GtkStyle *style, GdkWindow *window, gboolean set_bg, GtkStateType state_type, GdkRectangle *area, gint x, gint y, gint width, gint height)
{
}


/* <gtk/gtkstyle.h> */
void gtk_draw_hline(GtkStyle *style, GdkWindow *window, GtkStateType state_type, gint x1, gint x2, gint y)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_vline(GtkStyle *style, GdkWindow *window, GtkStateType state_type, gint y1, gint y2, gint x)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_shadow(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_polygon(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkPoint *points, gint npoints, gboolean fill)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_arrow(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GtkArrowType arrow_type, gboolean fill, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_diamond(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_oval(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_string(GtkStyle *style, GdkWindow *window, GtkStateType state_type, gint x, gint y, const gchar *string)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_box(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_flat_box(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_check(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_option(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_cross(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_ramp(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GtkArrowType arrow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_tab(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_shadow_gap(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_box_gap(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_extension(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height, GtkPositionType gap_side)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_focus(GtkStyle *style, GdkWindow *window, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_slider(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
}

/* <gtk/gtkstyle.h> */
void gtk_draw_handle(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
}


/* <gtk/gtkstyle.h> */
void gtk_paint_hline(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x1, gint x2, gint y)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_vline(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint y1, gint y2, gint x)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_shadow(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_polygon(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GdkPoint *points, gint npoints, gboolean fill)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_arrow(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GtkArrowType arrow_type, gboolean fill, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_diamond(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_oval(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_string(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, const gchar *string)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_box(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_flat_box(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_check(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_option(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_cross(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_ramp(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, GtkArrowType arrow_type, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_tab(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_shadow_gap(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_box_gap(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side, gint gap_x, gint gap_width)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_extension(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkPositionType gap_side)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_focus(GtkStyle *style, GdkWindow *window, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_slider(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
}

/* <gtk/gtkstyle.h> */
void gtk_paint_handle(GtkStyle *style, GdkWindow *window, GtkStateType state_type, GtkShadowType shadow_type, GdkRectangle *area, GtkWidget *widget, gchar *detail, gint x, gint y, gint width, gint height, GtkOrientation orientation)
{
}


/* Temporary GTK+-1.2.9 local patch for use only in theme engines.
 * Simple integer geometry properties.
 */

/* <gtk/gtkstyle.h> */
void gtk_style_set_prop_experimental(GtkStyle *style, const gchar *name, gint value)
{
}

/* <gtk/gtkstyle.h> */
gint gtk_style_get_prop_experimental(GtkStyle *style, const gchar *name, gint default_value)
{
}


/* <gtk/gtktable.h> */
#define GTK_TYPE_TABLE (gtk_table_get_type ())

/* <gtk/gtktable.h> */
#define GTK_TABLE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TABLE, GtkTable))

/* <gtk/gtktable.h> */
#define GTK_TABLE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TABLE, GtkTableClass))

/* <gtk/gtktable.h> */
#define GTK_IS_TABLE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TABLE))

/* <gtk/gtktable.h> */
#define GTK_IS_TABLE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TABLE))



/* <gtk/gtktable.h> */
typedef struct _GtkTable
{
	GtkContainer container;
	 
	GList *children;
	GtkTableRowCol *rows;
	GtkTableRowCol *cols;
	guint16 nrows;
	guint16 ncols;
	guint16 column_spacing;
	guint16 row_spacing;
	guint homogeneous : 1;
} GtkTable;


/* <gtk/gtktable.h> */
typedef struct _GtkTableClass
{
	GtkContainerClass parent_class;
} GtkTableClass;


/* <gtk/gtktable.h> */
typedef struct _GtkTableChild
{
	GtkWidget *widget;
	guint16 left_attach;
	guint16 right_attach;
	guint16 top_attach;
	guint16 bottom_attach;
	guint16 xpadding;
	guint16 ypadding;
	guint xexpand : 1;
	guint yexpand : 1;
	guint xshrink : 1;
	guint yshrink : 1;
	guint xfill : 1;
	guint yfill : 1;
} GtkTableChild;


/* <gtk/gtktable.h> */
typedef struct _GtkTableRowCol
{
	guint16 requisition;
	guint16 allocation;
	guint16 spacing;
	guint need_expand : 1;
	guint need_shrink : 1;
	guint expand : 1;
	guint shrink : 1;
	guint empty : 1;
} GtkTableRowCol;



/* <gtk/gtktable.h> */
GtkType gtk_table_get_type(void)
{
}

/* <gtk/gtktable.h> */
GtkWidget* gtk_table_new(guint rows, guint columns, gboolean homogeneous)
{
}

/* <gtk/gtktable.h> */
void gtk_table_resize(GtkTable *table, guint rows, guint columns)
{
}

/* <gtk/gtktable.h> */
void gtk_table_attach(GtkTable *table, GtkWidget *child, guint left_attach, guint right_attach, guint top_attach, guint bottom_attach, GtkAttachOptions xoptions, GtkAttachOptions yoptions, guint xpadding, guint ypadding)
{
}

/* <gtk/gtktable.h> */
void gtk_table_attach_defaults(GtkTable *table, GtkWidget *widget, guint left_attach, guint right_attach, guint top_attach, guint bottom_attach)
{
}

/* <gtk/gtktable.h> */
void gtk_table_set_row_spacing(GtkTable *table, guint row, guint spacing)
{
}

/* <gtk/gtktable.h> */
void gtk_table_set_col_spacing(GtkTable *table, guint column, guint spacing)
{
}

/* <gtk/gtktable.h> */
void gtk_table_set_row_spacings(GtkTable *table, guint spacing)
{
}

/* <gtk/gtktable.h> */
void gtk_table_set_col_spacings(GtkTable *table, guint spacing)
{
}

/* <gtk/gtktable.h> */
void gtk_table_set_homogeneous(GtkTable *table, gboolean homogeneous)
{
}


/* <gtk/gtktearoffmenuitem.h> */
#define GTK_TYPE_TEAROFF_MENU_ITEM (gtk_tearoff_menu_item_get_type ())

/* <gtk/gtktearoffmenuitem.h> */
#define GTK_TEAROFF_MENU_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TEAROFF_MENU_ITEM, GtkTearoffMenuItem))

/* <gtk/gtktearoffmenuitem.h> */
#define GTK_TEAROFF_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEAROFF_MENU_ITEM, GtkTearoffMenuItemClass))

/* <gtk/gtktearoffmenuitem.h> */
#define GTK_IS_TEAROFF_MENU_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TEAROFF_MENU_ITEM))

/* <gtk/gtktearoffmenuitem.h> */
#define GTK_IS_TEAROFF_MENU_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEAROFF_MENU_ITEM))



/* <gtk/gtktearoffmenuitem.h> */
typedef struct _GtkTearoffMenuItem
{
	GtkMenuItem menu_item;

	guint torn_off : 1;
} GtkTearoffMenuItem;


/* <gtk/gtktearoffmenuitem.h> */
typedef struct _GtkTearoffMenuItemClass
{
	GtkMenuItemClass parent_class;
} GtkTearoffMenuItemClass;



/* <gtk/gtktearoffmenuitem.h> */
GtkType gtk_tearoff_menu_item_get_type(void)
{
}

/* <gtk/gtktearoffmenuitem.h> */
GtkWidget* gtk_tearoff_menu_item_new(void)
{
}


/* <gtk/gtktext.h> */
#define GTK_TYPE_TEXT (gtk_text_get_type ())

/* <gtk/gtktext.h> */
#define GTK_TEXT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TEXT, GtkText))

/* <gtk/gtktext.h> */
#define GTK_TEXT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEXT, GtkTextClass))

/* <gtk/gtktext.h> */
#define GTK_IS_TEXT(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TEXT))

/* <gtk/gtktext.h> */
#define GTK_IS_TEXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEXT))


/* <gtk/gtktext.h> */
typedef struct _GtkPropertyMark
{
	 /* Position in list. */
	GList* property;

	 /* Offset into that property. */
	guint offset;

	 /* Current index. */
	guint index;
} GtkPropertyMark;


/* <gtk/gtktext.h> */
typedef struct _GtkText
{
	GtkEditable editable;

	GdkWindow *text_area;

	GtkAdjustment *hadj;
	GtkAdjustment *vadj;

	GdkGC *gc;

	GdkPixmap* line_wrap_bitmap;
	GdkPixmap* line_arrow_bitmap;

	 /* GAPPED TEXT SEGMENT */

	 /* The text, a single segment of text a'la emacs, with a gap
 * where insertion occurs. */
	union { GdkWChar *wc; guchar *ch; } text;
	 /* The allocated length of the text segment. */
	guint text_len;
	 /* The gap position, index into address where a char
 * should be inserted. */
	guint gap_position;
	 /* The gap size, s.t. *(text + gap_position + gap_size) is
 * the first valid character following the gap. */
	guint gap_size;
	 /* The last character position, index into address where a
 * character should be appeneded. Thus, text_end - gap_size
 * is the length of the actual data. */
	guint text_end;
	 /* LINE START CACHE */

	 /* A cache of line-start information. Data is a LineParam*. */
	GList *line_start_cache;
	 /* Index to the start of the first visible line. */
	guint first_line_start_index;
	 /* The number of pixels cut off of the top line. */
	guint first_cut_pixels;
	 /* First visible horizontal pixel. */
	guint first_onscreen_hor_pixel;
	 /* First visible vertical pixel. */
	guint first_onscreen_ver_pixel;

	 /* FLAGS */

	 /* True iff this buffer is wrapping lines, otherwise it is using a
 * horizontal scrollbar. */
	guint line_wrap : 1;
	guint word_wrap : 1;
	/* If a fontset is supplied for the widget, use_wchar become true,
 * and we use GdkWchar as the encoding of text. */
	guint use_wchar : 1;

	 /* Frozen, don't do updates. @@@ fixme */
	guint freeze_count;
	 /* TEXT PROPERTIES */

	 /* A doubly-linked-list containing TextProperty objects. */
	GList *text_properties;
	 /* The end of this list. */
	GList *text_properties_end;
	 /* The first node before or on the point along with its offset to
 * the point and the buffer's current point. This is the only
 * PropertyMark whose index is guaranteed to remain correct
 * following a buffer insertion or deletion. */
	GtkPropertyMark point;

	 /* SCRATCH AREA */

	union { GdkWChar *wc; guchar *ch; } scratch_buffer;
	guint scratch_buffer_len;

	 /* SCROLLING */

	gint last_ver_value;

	 /* CURSOR */

	gint cursor_pos_x;  /* Position of cursor. */
	gint cursor_pos_y;  /* Baseline of line cursor is drawn on. */
	GtkPropertyMark cursor_mark;  /* Where it is in the buffer. */
	GdkWChar cursor_char;  /* Character to redraw. */
	gchar cursor_char_offset; /* Distance from baseline of the font. */
	gint cursor_virtual_x;  /* Where it would be if it could be. */
	gint cursor_drawn_level; /* How many people have undrawn. */

	 /* Current Line */

	GList *current_line;

	 /* Tab Stops */

	GList *tab_stops;
	gint default_tab_width;

	GtkTextFont *current_font; /* Text font for current style */

	 /* Timer used for auto-scrolling off ends */
	gint timer;
	 
	guint button;  /* currently pressed mouse button */
	GdkGC *bg_gc;  /* gc for drawing background pixmap */
} GtkText;


/* <gtk/gtktext.h> */
typedef struct _GtkTextClass
{
	GtkEditableClass parent_class;

	void (*set_scroll_adjustments) (GtkText *text, GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);
} GtkTextClass;



/* <gtk/gtktext.h> */
GtkType gtk_text_get_type(void)
{
}

/* <gtk/gtktext.h> */
GtkWidget* gtk_text_new(GtkAdjustment *hadj, GtkAdjustment *vadj)
{
}

/* <gtk/gtktext.h> */
void gtk_text_set_editable(GtkText *text, gboolean editable)
{
}

/* <gtk/gtktext.h> */
void gtk_text_set_word_wrap(GtkText *text, gint word_wrap)
{
}

/* <gtk/gtktext.h> */
void gtk_text_set_line_wrap(GtkText *text, gint line_wrap)
{
}

/* <gtk/gtktext.h> */
void gtk_text_set_adjustments(GtkText *text, GtkAdjustment *hadj, GtkAdjustment *vadj)
{
}

/* <gtk/gtktext.h> */
void gtk_text_set_point(GtkText *text, guint index)
{
}

/* <gtk/gtktext.h> */
guint gtk_text_get_point(GtkText *text)
{
}

/* <gtk/gtktext.h> */
guint gtk_text_get_length(GtkText *text)
{
}

/* <gtk/gtktext.h> */
void gtk_text_freeze(GtkText *text)
{
}

/* <gtk/gtktext.h> */
void gtk_text_thaw(GtkText *text)
{
}

/* <gtk/gtktext.h> */
void gtk_text_insert(GtkText *text, GdkFont *font, GdkColor *fore, GdkColor *back, const char *chars, gint length)
{
}

/* <gtk/gtktext.h> */
gint gtk_text_backward_delete(GtkText *text, guint nchars)
{
}

/* <gtk/gtktext.h> */
gint gtk_text_forward_delete(GtkText *text, guint nchars)
{
}


/* <gtk/gtktext.h> */
#define GTK_TEXT_INDEX(t, index) (((t)->use_wchar) \
	? ((index) < (t)->gap_position ? (t)->text.wc[index] : \
	(t)->text.wc[(index)+(t)->gap_size]) \
	: ((index) < (t)->gap_position ? (t)->text.ch[index] : \
	(t)->text.ch[(index)+(t)->gap_size]))


/* <gtk/gtkthemes.h> */
typedef struct _GtkThemeEngine
{
	 /* Fill in engine_data pointer in a GtkRcStyle by parsing contents
 * of brackets. Returns G_TOKEN_NONE if succesfull, otherwise returns
 * the token it expected but didn't get.
 */
	guint (*parse_rc_style) (GScanner *scanner, GtkRcStyle *rc_style);
	 
	 /* Combine RC style data from src into dest. If 
 * dest->engine_data is NULL, it should be initialized to default
 * values.
 */
	void (*merge_rc_style) (GtkRcStyle *dest, GtkRcStyle *src);

	 /* Fill in style->engine_data from rc_style->engine_data */
	void (*rc_style_to_style) (GtkStyle *style, GtkRcStyle *rc_style);

	 /* Duplicate engine_data from src to dest. The engine_data will
 * not subsequently be modified except by a call to realize_style()
 * so if realize_style() does nothing, refcounting is appropriate.
 */
	void (*duplicate_style) (GtkStyle *dest, GtkStyle *src);

	 /* If style needs to initialize for a particular colormap/depth
 * combination, do it here. style->colormap/style->depth will have
 * been set at this point, and style itself initialized for 
 * the colormap
 */
	void (*realize_style) (GtkStyle *new_style);

	 /* If style needs to clean up for a particular colormap/depth
 * combination, do it here. 
 */
	void (*unrealize_style) (GtkStyle *new_style);

	 /* Clean up rc_style->engine_data before rc_style is destroyed */
	void (*destroy_rc_style) (GtkRcStyle *rc_style);

	 /* Clean up style->engine_data before style is destroyed */
	void (*destroy_style) (GtkStyle *style);

	void (*set_background) (GtkStyle *style, GdkWindow *window, GtkStateType state_type);
} GtkThemeEngine;


/* <gtk/gtkthemes.h> */
GtkThemeEngine *gtk_theme_engine_get(const gchar *name)
{
}

/* <gtk/gtkthemes.h> */
void gtk_theme_engine_ref(GtkThemeEngine *engine)
{
}

/* <gtk/gtkthemes.h> */
void gtk_theme_engine_unref(GtkThemeEngine *engine)
{
}



/* --- type macros --- */

/* <gtk/gtktipsquery.h> */
#define GTK_TYPE_TIPS_QUERY (gtk_tips_query_get_type ())

/* <gtk/gtktipsquery.h> */
#define GTK_TIPS_QUERY(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TIPS_QUERY, GtkTipsQuery))

/* <gtk/gtktipsquery.h> */
#define GTK_TIPS_QUERY_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TIPS_QUERY, GtkTipsQueryClass))

/* <gtk/gtktipsquery.h> */
#define GTK_IS_TIPS_QUERY(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TIPS_QUERY))

/* <gtk/gtktipsquery.h> */
#define GTK_IS_TIPS_QUERY_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TIPS_QUERY))


/* <gtk/gtktipsquery.h> */
struct _GtkTipsQuery
{
	GtkLabel label;

	guint emit_always : 1;
	guint in_query : 1;
	gchar *label_inactive;
	gchar *label_no_tip;

	GtkWidget *caller;
	GtkWidget *last_crossed;

	GdkCursor *query_cursor;
};


/* <gtk/gtktipsquery.h> */
struct _GtkTipsQueryClass
{
	GtkLabelClass parent_class;

	void (*start_query) (GtkTipsQuery *tips_query);
	void (*stop_query) (GtkTipsQuery *tips_query);
	void (*widget_entered) (GtkTipsQuery *tips_query, GtkWidget *widget, const gchar *tip_text, const gchar *tip_private);
	gint (*widget_selected) (GtkTipsQuery *tips_query, GtkWidget *widget, const gchar *tip_text, const gchar *tip_private, GdkEventButton *event);
};


/* --- prototypes --- */

/* <gtk/gtktipsquery.h> */
GtkType gtk_tips_query_get_type(void)
{
}

/* <gtk/gtktipsquery.h> */
GtkWidget* gtk_tips_query_new(void)
{
}

/* <gtk/gtktipsquery.h> */
void gtk_tips_query_start_query(GtkTipsQuery *tips_query)
{
}

/* <gtk/gtktipsquery.h> */
void gtk_tips_query_stop_query(GtkTipsQuery *tips_query)
{
}

/* <gtk/gtktipsquery.h> */
void gtk_tips_query_set_caller(GtkTipsQuery *tips_query, GtkWidget *caller)
{
}

/* <gtk/gtktipsquery.h> */
void gtk_tips_query_set_labels(GtkTipsQuery *tips_query, const gchar *label_inactive, const gchar *label_no_tip)
{
}
	 

/* <gtk/gtktogglebutton.h> */
#define GTK_TYPE_TOGGLE_BUTTON (gtk_toggle_button_get_type ())

/* <gtk/gtktogglebutton.h> */
#define GTK_TOGGLE_BUTTON(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TOGGLE_BUTTON, GtkToggleButton))

/* <gtk/gtktogglebutton.h> */
#define GTK_TOGGLE_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TOGGLE_BUTTON, GtkToggleButtonClass))

/* <gtk/gtktogglebutton.h> */
#define GTK_IS_TOGGLE_BUTTON(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TOGGLE_BUTTON))

/* <gtk/gtktogglebutton.h> */
#define GTK_IS_TOGGLE_BUTTON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TOGGLE_BUTTON))



/* <gtk/gtktogglebutton.h> */
typedef struct _GtkToggleButton
{
	GtkButton button;

	guint active : 1;
	guint draw_indicator : 1;
	 
	GdkWindow *event_window;
} GtkToggleButton;


/* <gtk/gtktogglebutton.h> */
typedef struct _GtkToggleButtonClass
{
	GtkButtonClass parent_class;

	void (* toggled) (GtkToggleButton *toggle_button);
} GtkToggleButtonClass;



/* <gtk/gtktogglebutton.h> */
GtkType gtk_toggle_button_get_type(void)
{
}

/* <gtk/gtktogglebutton.h> */
GtkWidget* gtk_toggle_button_new(void)
{
}

/* <gtk/gtktogglebutton.h> */
GtkWidget* gtk_toggle_button_new_with_label(const gchar *label)
{
}

/* <gtk/gtktogglebutton.h> */
void gtk_toggle_button_set_mode(GtkToggleButton *toggle_button, gboolean draw_indicator)
{
}

/* <gtk/gtktogglebutton.h> */
void gtk_toggle_button_set_active(GtkToggleButton *toggle_button, gboolean is_active)
{
}

/* <gtk/gtktogglebutton.h> */
gboolean gtk_toggle_button_get_active(GtkToggleButton *toggle_button)
{
}

/* <gtk/gtktogglebutton.h> */
void gtk_toggle_button_toggled(GtkToggleButton *toggle_button)
{
}


/* <gtk/gtktoolbar.h> */
#define GTK_TYPE_TOOLBAR (gtk_toolbar_get_type ())

/* <gtk/gtktoolbar.h> */
#define GTK_TOOLBAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TOOLBAR, GtkToolbar))

/* <gtk/gtktoolbar.h> */
#define GTK_TOOLBAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TOOLBAR, GtkToolbarClass))

/* <gtk/gtktoolbar.h> */
#define GTK_IS_TOOLBAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TOOLBAR))

/* <gtk/gtktoolbar.h> */
#define GTK_IS_TOOLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TOOLBAR))


/* <gtk/gtktoolbar.h> */
typedef enum
{
	GTK_TOOLBAR_CHILD_SPACE,
	GTK_TOOLBAR_CHILD_BUTTON,
	GTK_TOOLBAR_CHILD_TOGGLEBUTTON,
	GTK_TOOLBAR_CHILD_RADIOBUTTON,
	GTK_TOOLBAR_CHILD_WIDGET
} GtkToolbarChildType;


/* <gtk/gtktoolbar.h> */
typedef enum
{
	GTK_TOOLBAR_SPACE_EMPTY,
	GTK_TOOLBAR_SPACE_LINE
} GtkToolbarSpaceStyle;


/* <gtk/gtktoolbar.h> */
typedef struct _GtkToolbarChild
{
	GtkToolbarChildType type;
	GtkWidget *widget;
	GtkWidget *icon;
	GtkWidget *label;
} GtkToolbarChild;


/* <gtk/gtktoolbar.h> */
typedef struct _GtkToolbar
{
	GtkContainer container;

	gint num_children;
	GList *children;
	GtkOrientation orientation;
	GtkToolbarStyle style;
	gint space_size; /* big optional space between buttons */
	GtkToolbarSpaceStyle space_style;

	GtkTooltips *tooltips;

	gint button_maxw;
	gint button_maxh;
	GtkReliefStyle relief;
} GtkToolbar;


/* <gtk/gtktoolbar.h> */
typedef struct _GtkToolbarClass
{
	GtkContainerClass parent_class;

	void (* orientation_changed) (GtkToolbar *toolbar, GtkOrientation orientation);
	void (* style_changed) (GtkToolbar *toolbar, GtkToolbarStyle style);
} GtkToolbarClass;



/* <gtk/gtktoolbar.h> */
GtkType gtk_toolbar_get_type(void)
{
}

/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_new(GtkOrientation orientation, GtkToolbarStyle style)
{
}

/* Simple button items */

/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_append_item(GtkToolbar *toolbar, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data)
{
}

/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_prepend_item(GtkToolbar *toolbar, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data)
{
}

/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_insert_item(GtkToolbar *toolbar, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data, gint position)
{
}

/* Space Items */

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_append_space(GtkToolbar *toolbar)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_prepend_space(GtkToolbar *toolbar)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_insert_space(GtkToolbar *toolbar, gint position)
{
}

/* Any element type */

/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_append_element(GtkToolbar *toolbar, GtkToolbarChildType type, GtkWidget *widget, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data)
{
}


/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_prepend_element(GtkToolbar *toolbar, GtkToolbarChildType type, GtkWidget *widget, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data)
{
}


/* <gtk/gtktoolbar.h> */
GtkWidget* gtk_toolbar_insert_element(GtkToolbar *toolbar, GtkToolbarChildType type, GtkWidget *widget, const char *text, const char *tooltip_text, const char *tooltip_private_text, GtkWidget *icon, GtkSignalFunc callback, gpointer user_data, gint position)
{
}

/* Generic Widgets */

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_append_widget(GtkToolbar *toolbar, GtkWidget *widget, const char *tooltip_text, const char *tooltip_private_text)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_prepend_widget(GtkToolbar *toolbar, GtkWidget *widget, const char *tooltip_text, const char *tooltip_private_text)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_insert_widget(GtkToolbar *toolbar, GtkWidget *widget, const char *tooltip_text, const char *tooltip_private_text, gint position)
{
}

/* Style functions */

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_orientation(GtkToolbar *toolbar, GtkOrientation orientation)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_style(GtkToolbar *toolbar, GtkToolbarStyle style)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_space_size(GtkToolbar *toolbar, gint space_size)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_space_style(GtkToolbar *toolbar, GtkToolbarSpaceStyle space_style)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_tooltips(GtkToolbar *toolbar, gint enable)
{
}

/* <gtk/gtktoolbar.h> */
void gtk_toolbar_set_button_relief(GtkToolbar *toolbar, GtkReliefStyle relief)
{
}

/* <gtk/gtktoolbar.h> */
GtkReliefStyle gtk_toolbar_get_button_relief(GtkToolbar *toolbar)
{
}


/* <gtk/gtktooltips.h> */
#define GTK_TYPE_TOOLTIPS (gtk_tooltips_get_type ())

/* <gtk/gtktooltips.h> */
#define GTK_TOOLTIPS(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TOOLTIPS, GtkTooltips))

/* <gtk/gtktooltips.h> */
#define GTK_TOOLTIPS_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TOOLTIPS, GtkTooltipsClass))

/* <gtk/gtktooltips.h> */
#define GTK_IS_TOOLTIPS(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TOOLTIPS))

/* <gtk/gtktooltips.h> */
#define GTK_IS_TOOLTIPS_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TOOLTIPS))


/* <gtk/gtktooltips.h> */
typedef struct _GtkTooltipsData
{
	GtkTooltips *tooltips;
	GtkWidget *widget;
	gchar *tip_text;
	gchar *tip_private;
	GdkFont *font;
	gint width;
	GList *row;
} GtkTooltipsData;


/* <gtk/gtktooltips.h> */
typedef struct _GtkTooltips
{
	GtkData data;

	GtkWidget *tip_window;
	GtkTooltipsData *active_tips_data;
	GList *tips_data_list;

	GdkGC *gc;
	GdkColor *foreground;
	GdkColor *background;

	guint delay : 30;
	guint enabled : 1;
	gint timer_tag;
} GtkTooltips;


/* <gtk/gtktooltips.h> */
typedef struct _GtkTooltipsClass
{
	GtkDataClass parent_class;
} GtkTooltipsClass;


/* <gtk/gtktooltips.h> */
GtkType gtk_tooltips_get_type(void)
{
}

/* <gtk/gtktooltips.h> */
GtkTooltips* gtk_tooltips_new(void)
{
}


/* <gtk/gtktooltips.h> */
void gtk_tooltips_enable(GtkTooltips *tooltips)
{
}

/* <gtk/gtktooltips.h> */
void gtk_tooltips_disable(GtkTooltips *tooltips)
{
}

/* <gtk/gtktooltips.h> */
void gtk_tooltips_set_delay(GtkTooltips *tooltips, guint delay)
{
}

/* <gtk/gtktooltips.h> */
void gtk_tooltips_set_tip(GtkTooltips *tooltips, GtkWidget *widget, const gchar *tip_text, const gchar *tip_private)
{
}

/* <gtk/gtktooltips.h> */
void gtk_tooltips_set_colors(GtkTooltips *tooltips, GdkColor *background, GdkColor *foreground)
{
}

/* <gtk/gtktooltips.h> */
GtkTooltipsData* gtk_tooltips_data_get(GtkWidget *widget)
{
}

/* <gtk/gtktooltips.h> */
void gtk_tooltips_force_window(GtkTooltips *tooltips)
{
}


/* <gtk/gtktree.h> */
#define GTK_TYPE_TREE (gtk_tree_get_type ())

/* <gtk/gtktree.h> */
#define GTK_TREE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TREE, GtkTree))

/* <gtk/gtktree.h> */
#define GTK_TREE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TREE, GtkTreeClass))

/* <gtk/gtktree.h> */
#define GTK_IS_TREE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TREE))

/* <gtk/gtktree.h> */
#define GTK_IS_TREE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TREE))


/* <gtk/gtktree.h> */
#define GTK_IS_ROOT_TREE(obj) ((GtkObject*) GTK_TREE(obj)->root_tree == (GtkObject*)obj)

/* <gtk/gtktree.h> */
#define GTK_TREE_ROOT_TREE(obj) (GTK_TREE(obj)->root_tree ? GTK_TREE(obj)->root_tree : GTK_TREE(obj))

/* <gtk/gtktree.h> */
#define GTK_TREE_SELECTION(obj) (GTK_TREE_ROOT_TREE(obj)->selection)


/* <gtk/gtktree.h> */
typedef enum 
{
	GTK_TREE_VIEW_LINE,  /* default view mode */
	GTK_TREE_VIEW_ITEM
} GtkTreeViewMode;


/* <gtk/gtktree.h> */
typedef struct _GtkTree
{
	GtkContainer container;
	 
	GList *children;
	 
	GtkTree* root_tree; /* owner of selection list */
	GtkWidget* tree_owner;
	GList *selection;
	guint level;
	guint indent_value;
	guint current_indent;
	guint selection_mode : 2;
	guint view_mode : 1;
	guint view_line : 1;
} GtkTree;


/* <gtk/gtktree.h> */
typedef struct _GtkTreeClass
{
	GtkContainerClass parent_class;
	 
	void (* selection_changed) (GtkTree *tree);
	void (* select_child) (GtkTree *tree, GtkWidget *child);
	void (* unselect_child) (GtkTree *tree, GtkWidget *child);
} GtkTreeClass;



/* <gtk/gtktree.h> */
GtkType gtk_tree_get_type(void)
{
}

/* <gtk/gtktree.h> */
GtkWidget* gtk_tree_new(void)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_append(GtkTree *tree, GtkWidget *tree_item)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_prepend(GtkTree *tree, GtkWidget *tree_item)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_insert(GtkTree *tree, GtkWidget *tree_item, gint position)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_remove_items(GtkTree *tree, GList *items)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_clear_items(GtkTree *tree, gint start, gint end)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_select_item(GtkTree *tree, gint item)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_unselect_item(GtkTree *tree, gint item)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_select_child(GtkTree *tree, GtkWidget *tree_item)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_unselect_child(GtkTree *tree, GtkWidget *tree_item)
{
}

/* <gtk/gtktree.h> */
gint gtk_tree_child_position(GtkTree *tree, GtkWidget *child)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_set_selection_mode(GtkTree *tree, GtkSelectionMode mode)
{
}

/* <gtk/gtktree.h> */
void gtk_tree_set_view_mode(GtkTree *tree, GtkTreeViewMode mode)
{
} 

/* <gtk/gtktree.h> */
void gtk_tree_set_view_lines(GtkTree *tree, guint flag)
{
}

/* deprecated function, use gtk_container_remove instead.
 */

/* <gtk/gtktree.h> */
void gtk_tree_remove_item(GtkTree *tree, GtkWidget *child)
{
}


/* <gtk/gtktreeitem.h> */
#define GTK_TYPE_TREE_ITEM (gtk_tree_item_get_type ())

/* <gtk/gtktreeitem.h> */
#define GTK_TREE_ITEM(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_TREE_ITEM, GtkTreeItem))

/* <gtk/gtktreeitem.h> */
#define GTK_TREE_ITEM_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_TREE_ITEM, GtkTreeItemClass))

/* <gtk/gtktreeitem.h> */
#define GTK_IS_TREE_ITEM(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_TREE_ITEM))

/* <gtk/gtktreeitem.h> */
#define GTK_IS_TREE_ITEM_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TREE_ITEM))


/* <gtk/gtktreeitem.h> */
#define GTK_TREE_ITEM_SUBTREE(obj) (GTK_TREE_ITEM(obj)->subtree)



/* <gtk/gtktreeitem.h> */
typedef struct _GtkTreeItem
{
	GtkItem item;

	GtkWidget *subtree;
	GtkWidget *pixmaps_box;
	GtkWidget *plus_pix_widget, *minus_pix_widget;

	GList *pixmaps; 	/* pixmap node for this items color depth */

	guint expanded : 1;
} GtkTreeItem;


/* <gtk/gtktreeitem.h> */
typedef struct _GtkTreeItemClass
{
	GtkItemClass parent_class;

	void (* expand) (GtkTreeItem *tree_item);
	void (* collapse) (GtkTreeItem *tree_item);
} GtkTreeItemClass;



/* <gtk/gtktreeitem.h> */
GtkType gtk_tree_item_get_type(void)
{
}

/* <gtk/gtktreeitem.h> */
GtkWidget* gtk_tree_item_new(void)
{
}

/* <gtk/gtktreeitem.h> */
GtkWidget* gtk_tree_item_new_with_label(const gchar *label)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_set_subtree(GtkTreeItem *tree_item, GtkWidget *subtree)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_remove_subtree(GtkTreeItem *tree_item)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_select(GtkTreeItem *tree_item)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_deselect(GtkTreeItem *tree_item)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_expand(GtkTreeItem *tree_item)
{
}

/* <gtk/gtktreeitem.h> */
void gtk_tree_item_collapse(GtkTreeItem *tree_item)
{
}

/* type macros, generated by maketypes.awk */

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ACCEL_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CALENDAR_DISPLAY_OPTIONS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CELL_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CLIST_DRAG_POS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_BUTTON_ACTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CTREE_POS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CTREE_LINE_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CTREE_EXPANDER_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CTREE_EXPANSION_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_DEBUG_FLAG;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_DEST_DEFAULTS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TARGET_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ARROW_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ATTACH_OPTIONS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_BUTTON_BOX_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CURVE_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_DIRECTION_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_JUSTIFICATION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_MATCH_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_MENU_DIRECTION_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_MENU_FACTORY_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_METRIC_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ORIENTATION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CORNER_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PACK_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PATH_PRIORITY_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PATH_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_POLICY_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_POSITION_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PREVIEW_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_RELIEF_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_RESIZE_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SIGNAL_RUN_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SCROLL_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SELECTION_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SHADOW_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_STATE_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SUBMENU_DIRECTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SUBMENU_PLACEMENT;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TOOLBAR_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TROUGH_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_UPDATE_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_VISIBILITY;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_WINDOW_POSITION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_WINDOW_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SORT_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_FONT_METRIC_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_FONT_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_FONT_FILTER_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_OBJECT_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ARG_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PACKER_OPTIONS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SIDE_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ANCHOR_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PRIVATE_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PROGRESS_BAR_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_PROGRESS_BAR_ORIENTATION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_RC_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_RC_TOKEN_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SPIN_BUTTON_UPDATE_POLICY;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SPIN_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TOOLBAR_CHILD_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TOOLBAR_SPACE_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_TREE_VIEW_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_FUNDAMENTAL_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_WIDGET_FLAGS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WINDOW_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WINDOW_CLASS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_IMAGE_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_VISUAL_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FONT_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WINDOW_ATTRIBUTES_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WINDOW_HINTS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FUNCTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FILL;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FILL_RULE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_LINE_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_CAP_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_JOIN_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_CURSOR_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FILTER_RETURN;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_VISIBILITY_STATE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_EVENT_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_EVENT_MASK;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_NOTIFY_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_CROSSING_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_MODIFIER_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_SUBWINDOW_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_INPUT_CONDITION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_STATUS;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_BYTE_ORDER;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_GC_VALUES_MASK;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_SELECTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_PROPERTY_STATE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_PROP_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_INPUT_SOURCE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_INPUT_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_AXIS_USE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_TARGET;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_SELECTION_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_EXTENSION_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_IM_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_IC_ATTRIBUTES_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WM_DECORATION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WM_FUNCTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_COLOR_CONTEXT_MODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_OVERLAP_TYPE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_DRAG_ACTION;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_DRAG_PROTOCOL;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_RGB_DITHER;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_ACCEL_GROUP;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_SELECTION_DATA;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_STYLE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_CTREE_NODE;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_COLORMAP;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_VISUAL;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_FONT;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_WINDOW;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_DRAG_CONTEXT;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_EVENT;

/* <gtk/gtktypebuiltins.h> */
extern GtkType GTK_TYPE_GDK_COLOR;


/* <gtk/gtktypebuiltins.h> */
#define GTK_TYPE_NUM_BUILTINS (121)

/* Fundamental Types
 */

/* <gtk/gtktypeutils.h> */
typedef enum
{
	GTK_TYPE_INVALID,
	GTK_TYPE_NONE,
	 
	 /* flat types */
	GTK_TYPE_CHAR,
	GTK_TYPE_UCHAR,
	GTK_TYPE_BOOL,
	GTK_TYPE_INT,
	GTK_TYPE_UINT,
	GTK_TYPE_LONG,
	GTK_TYPE_ULONG,
	GTK_TYPE_FLOAT,
	GTK_TYPE_DOUBLE,
	GTK_TYPE_STRING,
	GTK_TYPE_ENUM,
	GTK_TYPE_FLAGS,
	GTK_TYPE_BOXED,
	GTK_TYPE_POINTER,
	 
	 /* structured types */
	GTK_TYPE_SIGNAL,
	GTK_TYPE_ARGS,
	GTK_TYPE_CALLBACK,
	GTK_TYPE_C_CALLBACK,
	GTK_TYPE_FOREIGN,
	 
	 /* base type node of the object system */
	GTK_TYPE_OBJECT
} GtkFundamentalType;

/* bounds definitions for type sets, these are provided to distinct
 * between fundamental types with if() statements, and to build
 * up foreign fundamentals
 */

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_FLAT_FIRST GTK_TYPE_CHAR

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_FLAT_LAST GTK_TYPE_POINTER

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_STRUCTURED_FIRST GTK_TYPE_SIGNAL

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_STRUCTURED_LAST GTK_TYPE_FOREIGN

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_FUNDAMENTAL_LAST GTK_TYPE_OBJECT

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_FUNDAMENTAL_MAX (32)


/* <gtk/gtktypeutils.h> retrieve a structure offset */
#define GTK_STRUCT_OFFSET(struct, field) ((gint) ((gchar*) &((struct*) 0)->field))


/* The debugging versions of the casting macros make sure the cast is "ok"
 * before proceeding, but they are definately slower than their less
 * careful counterparts as they involve extra ``is a'' checks.
 */

/* <gtk/gtktypeutils.h> */
#define GTK_CHECK_CAST(tobj, cast_type, cast) ((cast*) (tobj))

/* <gtk/gtktypeutils.h> */
#define GTK_CHECK_CLASS_CAST(tclass,cast_type,cast) ((cast*) (tclass))

/* Determines whether `type_object' and `type_class' are a type of `otype'.
 */

/* <gtk/gtktypeutils.h> */
#define GTK_CHECK_TYPE(type_object, otype) ( \
	((GtkTypeObject*) (type_object)) != NULL && \
	GTK_CHECK_CLASS_TYPE (((GtkTypeObject*) (type_object))->klass, (otype)) \
)

/* <gtk/gtktypeutils.h> */
#define GTK_CHECK_CLASS_TYPE(type_class, otype) ( \
	((GtkTypeClass*) (type_class)) != NULL && \
	gtk_type_is_a (((GtkTypeClass*) (type_class))->type, (otype)) \
)




/* A GtkType holds a unique type id
 */

/* <gtk/gtktypeutils.h> */
typedef guint GtkType;


/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_IDENTIFIER (gtk_identifier_get_type ())

/* <gtk/gtktypeutils.h> */
GtkType gtk_identifier_get_type(void)
{
}

/* Macros
 */

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_MAKE(parent_t, seqno) (((seqno) << 8) | GTK_FUNDAMENTAL_TYPE (parent_t))

/* <gtk/gtktypeutils.h> */
#define GTK_FUNDAMENTAL_TYPE(type) ((GtkFundamentalType) ((type) & 0xFF))

/* <gtk/gtktypeutils.h> */
#define GTK_TYPE_SEQNO(type) ((type) > 0xFF ? (type) >> 8 : (type))



/* <gtk/gtktypeutils.h> */
#define GTK_SIGNAL_FUNC(f) ((GtkSignalFunc) f)


/* <gtk/gtktypeutils.h> */
typedef void(*GtkClassInitFunc) (gpointer klass)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkObjectInitFunc) (gpointer object, gpointer klass)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkSignalFunc) ()
{
}

/* <gtk/gtktypeutils.h> */
typedef gint(*GtkFunction) (gpointer data)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkDestroyNotify) (gpointer data)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkCallbackMarshal) (GtkObject *object, gpointer data, guint n_args, GtkArg *args)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkSignalMarshaller) (GtkObject *object, GtkSignalFunc func, gpointer func_data, GtkArg *args)
{
}

/* deprecated */

/* <gtk/gtktypeutils.h> */
typedef void(*GtkArgGetFunc) (GtkObject*, GtkArg*, guint)
{
}

/* <gtk/gtktypeutils.h> */
typedef void(*GtkArgSetFunc) (GtkObject*, GtkArg*, guint)
{
}


/* A GtkTypeObject defines the minimum structure requirements
 * for type instances. Type instances returned from gtk_type_new ()
 * and initialized through a GtkObjectInitFunc need to directly inherit
 * from this structure or at least copy its fields one by one.
 */

/* <gtk/gtktypeutils.h> */
typedef struct _GtkTypeObject
{
	 /* A pointer to the objects class. This will actually point to
 * the derived objects class struct (which will be derived from
 * GtkTypeClass).
 */
	GtkTypeClass *klass;
} GtkTypeObject;


/* A GtkTypeClass defines the minimum structure requirements for
 * a types class. Classes returned from gtk_type_class () and
 * initialized through a GtkClassInitFunc need to directly inherit
 * from this structure or at least copy its fields one by one.
 */

/* <gtk/gtktypeutils.h> */
typedef struct _GtkTypeClass
{
	 /* The type identifier for the objects class. There is
 * one unique identifier per class.
 */
	GtkType type;
} GtkTypeClass;



/* <gtk/gtktypeutils.h> */
typedef struct _GtkArg
{
	GtkType type;
	gchar *name;
	 
	 /* this union only defines the required storage types for
 * the possibile values, thus there is no gint enum_data field,
 * because that would just be a mere alias for gint int_data.
 * use the GTK_VALUE_*() and GTK_RETLOC_*() macros to access
 * the discrete memebers.
 */
	union {
	 /* flat values */
	gchar char_data;
	guchar uchar_data;
	gboolean bool_data;
	gint int_data;
	guint uint_data;
	glong long_data;
	gulong ulong_data;
	gfloat float_data;
	gdouble double_data;
	gchar *string_data;
	gpointer pointer_data;
	GtkObject *object_data;
	 
	 /* structured values */
	struct {
	GtkSignalFunc f;
	gpointer d;
	} signal_data;
	struct {
	gint n_args;
	GtkArg *args;
	} args_data;
	struct {
	GtkCallbackMarshal marshal;
	gpointer data;
	GtkDestroyNotify notify;
	} callback_data;
	struct {
	GtkFunction func;
	gpointer func_data;
	} c_callback_data;
	struct {
	gpointer data;
	GtkDestroyNotify notify;
	} foreign_data;
	} d;
} GtkArg;

/* argument value access macros, these must not contain casts,
 * to allow the usage of these macros in combination with the
 * adress operator, e.g. &GTK_VALUE_CHAR (*arg)
 */

/* flat values */

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_CHAR(a) ((a).d.char_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_UCHAR(a) ((a).d.uchar_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_BOOL(a) ((a).d.bool_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_INT(a) ((a).d.int_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_UINT(a) ((a).d.uint_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_LONG(a) ((a).d.long_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_ULONG(a) ((a).d.ulong_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_FLOAT(a) ((a).d.float_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_DOUBLE(a) ((a).d.double_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_STRING(a) ((a).d.string_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_ENUM(a) ((a).d.int_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_FLAGS(a) ((a).d.uint_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_BOXED(a) ((a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_POINTER(a) ((a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_OBJECT(a) ((a).d.object_data)

/* structured values */

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_SIGNAL(a) ((a).d.signal_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_ARGS(a) ((a).d.args_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_CALLBACK(a) ((a).d.callback_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_C_CALLBACK(a) ((a).d.c_callback_data)

/* <gtk/gtktypeutils.h> */
#define GTK_VALUE_FOREIGN(a) ((a).d.foreign_data)

/* return location macros, these all narow down to
 * pointer types, because return values need to be
 * passed by reference
 */

/* flat values */

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_CHAR(a) ((gchar*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_UCHAR(a) ((guchar*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_BOOL(a) ((gboolean*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_INT(a) ((gint*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_UINT(a) ((guint*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_LONG(a) ((glong*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_ULONG(a) ((gulong*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_FLOAT(a) ((gfloat*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_DOUBLE(a) ((gdouble*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_STRING(a) ((gchar**) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_ENUM(a) ((gint*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_FLAGS(a) ((guint*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_BOXED(a) ((gpointer*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_POINTER(a) ((gpointer*) (a).d.pointer_data)

/* <gtk/gtktypeutils.h> */
#define GTK_RETLOC_OBJECT(a) ((GtkObject**) (a).d.pointer_data)


/* <gtk/gtktypeutils.h> */
typedef struct _GtkTypeInfo
{
	gchar *type_name;
	guint object_size;
	guint class_size;
	GtkClassInitFunc class_init_func;
	GtkObjectInitFunc object_init_func;
	gpointer reserved_1;
	gpointer reserved_2;
	GtkClassInitFunc base_class_init_func;
} GtkTypeInfo;


/* <gtk/gtktypeutils.h> */
typedef struct _GtkTypeQuery
{
	GtkType type;
	const gchar *type_name;
	guint object_size;
	guint class_size;
} GtkTypeQuery;


/* <gtk/gtktypeutils.h> */
typedef struct _GtkEnumValue
{
	guint value;
	gchar *value_name;
	gchar *value_nick;
} GtkEnumValue;



/* <gtk/gtktypeutils.h> */
void gtk_type_init(void)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_unique(GtkType parent_type, const GtkTypeInfo *type_info)
{
}

/* <gtk/gtktypeutils.h> */
void gtk_type_set_chunk_alloc(GtkType type, guint n_chunks)
{
}

/* <gtk/gtktypeutils.h> */
gchar* gtk_type_name(guint type)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_from_name(const gchar *name)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_parent(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
gpointer gtk_type_class(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
gpointer gtk_type_parent_class(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
GList* gtk_type_children_types(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
gpointer gtk_type_new(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
void gtk_type_free(GtkType type, gpointer mem)
{
}

/* <gtk/gtktypeutils.h> */
void gtk_type_describe_heritage(GtkType type)
{
}

/* <gtk/gtktypeutils.h> */
void gtk_type_describe_tree(GtkType type, gboolean show_size)
{
}

/* <gtk/gtktypeutils.h> */
gboolean gtk_type_is_a(GtkType type, GtkType is_a_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkTypeObject* gtk_type_check_object_cast(GtkTypeObject *type_object, GtkType cast_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkTypeClass* gtk_type_check_class_cast(GtkTypeClass *klass, GtkType cast_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_register_enum(const gchar *type_name, GtkEnumValue *values)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_register_flags(const gchar *type_name, GtkFlagValue *values)
{
}

/* <gtk/gtktypeutils.h> */
GtkEnumValue* gtk_type_enum_get_values(GtkType enum_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkFlagValue* gtk_type_flags_get_values(GtkType flags_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkEnumValue* gtk_type_enum_find_value(GtkType enum_type, const gchar *value_name)
{
}

/* <gtk/gtktypeutils.h> */
GtkFlagValue* gtk_type_flags_find_value(GtkType flag_type, const gchar *value_name)
{
}
/* set the argument collector alias for foreign fundamentals */

/* <gtk/gtktypeutils.h> */
void gtk_type_set_varargs_type(GtkType foreign_type, GtkType varargs_type)
{
}

/* <gtk/gtktypeutils.h> */
GtkType gtk_type_get_varargs_type(GtkType foreign_type)
{
}
/* Report internal information about a type. The caller has the
 * responsibility to invoke a subsequent g_free (returned_data); but
 * must not modify data pointed to by the members of GtkTypeQuery
 */

/* <gtk/gtktypeutils.h> */
GtkTypeQuery* gtk_type_query(GtkType type)
{
}



/* <gtk/gtkvbbox.h> */
#define GTK_VBUTTON_BOX(obj) GTK_CHECK_CAST (obj, gtk_vbutton_box_get_type (), GtkVButtonBox)

/* <gtk/gtkvbbox.h> */
#define GTK_VBUTTON_BOX_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_vbutton_box_get_type (), GtkVButtonBoxClass)

/* <gtk/gtkvbbox.h> */
#define GTK_IS_VBUTTON_BOX(obj) GTK_CHECK_TYPE (obj, gtk_vbutton_box_get_type ())




/* <gtk/gtkvbbox.h> */
typedef struct _GtkVButtonBox
{
	GtkButtonBox button_box;
} GtkVButtonBox;


/* <gtk/gtkvbbox.h> */
typedef struct _GtkVButtonBoxClass
{
	GtkButtonBoxClass parent_class;
} GtkVButtonBoxClass;



/* <gtk/gtkvbbox.h> */
guint gtk_vbutton_box_get_type(void)
{
}

/* <gtk/gtkvbbox.h> */
GtkWidget *gtk_vbutton_box_new(void)
{
}

/* buttons can be added by gtk_container_add() */


/* <gtk/gtkvbbox.h> */
gint gtk_vbutton_box_get_spacing_default(void)
{
}

/* <gtk/gtkvbbox.h> */
void gtk_vbutton_box_set_spacing_default(gint spacing)
{
}


/* <gtk/gtkvbbox.h> */
GtkButtonBoxStyle gtk_vbutton_box_get_layout_default(void)
{
}

/* <gtk/gtkvbbox.h> */
void gtk_vbutton_box_set_layout_default(GtkButtonBoxStyle layout)
{
}


/* <gtk/gtkvbox.h> */
#define GTK_TYPE_VBOX (gtk_vbox_get_type ())

/* <gtk/gtkvbox.h> */
#define GTK_VBOX(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_VBOX, GtkVBox))

/* <gtk/gtkvbox.h> */
#define GTK_VBOX_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_VBOX, GtkVBoxClass))

/* <gtk/gtkvbox.h> */
#define GTK_IS_VBOX(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_VBOX))

/* <gtk/gtkvbox.h> */
#define GTK_IS_VBOX_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VBOX))



/* <gtk/gtkvbox.h> */
typedef struct _GtkVBox
{
	GtkBox box;
} GtkVBox;


/* <gtk/gtkvbox.h> */
typedef struct _GtkVBoxClass
{
	GtkBoxClass parent_class;
} GtkVBoxClass;



/* <gtk/gtkvbox.h> */
GtkType gtk_vbox_get_type(void)
{
}

/* <gtk/gtkvbox.h> */
GtkWidget* gtk_vbox_new(gboolean homogeneous, gint spacing)
{
}


/* <gtk/gtkviewport.h> */
#define GTK_TYPE_VIEWPORT (gtk_viewport_get_type ())

/* <gtk/gtkviewport.h> */
#define GTK_VIEWPORT(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_VIEWPORT, GtkViewport))

/* <gtk/gtkviewport.h> */
#define GTK_VIEWPORT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_VIEWPORT, GtkViewportClass))

/* <gtk/gtkviewport.h> */
#define GTK_IS_VIEWPORT(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_VIEWPORT))

/* <gtk/gtkviewport.h> */
#define GTK_IS_VIEWPORT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VIEWPORT))



/* <gtk/gtkviewport.h> */
typedef struct _GtkViewport
{
	GtkBin bin;

	GtkShadowType shadow_type;
	GdkWindow *view_window;
	GdkWindow *bin_window;
	GtkAdjustment *hadjustment;
	GtkAdjustment *vadjustment;
} GtkViewport;


/* <gtk/gtkviewport.h> */
typedef struct _GtkViewportClass
{
	GtkBinClass parent_class;

	void (*set_scroll_adjustments) (GtkViewport *viewport, GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);
} GtkViewportClass;



/* <gtk/gtkviewport.h> */
GtkType gtk_viewport_get_type(void)
{
}

/* <gtk/gtkviewport.h> */
GtkWidget* gtk_viewport_new(GtkAdjustment *hadjustment, GtkAdjustment *vadjustment)
{
}

/* <gtk/gtkviewport.h> */
GtkAdjustment* gtk_viewport_get_hadjustment(GtkViewport *viewport)
{
}

/* <gtk/gtkviewport.h> */
GtkAdjustment* gtk_viewport_get_vadjustment(GtkViewport *viewport)
{
}

/* <gtk/gtkviewport.h> */
void gtk_viewport_set_hadjustment(GtkViewport *viewport, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkviewport.h> */
void gtk_viewport_set_vadjustment(GtkViewport *viewport, GtkAdjustment *adjustment)
{
}

/* <gtk/gtkviewport.h> */
void gtk_viewport_set_shadow_type(GtkViewport *viewport, GtkShadowType type)
{
}


/* <gtk/gtkvpaned.h> */
#define GTK_VPANED(obj) GTK_CHECK_CAST (obj, gtk_vpaned_get_type (), GtkVPaned)

/* <gtk/gtkvpaned.h> */
#define GTK_VPANED_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_vpaned_get_type (), GtkVPanedClass)

/* <gtk/gtkvpaned.h> */
#define GTK_IS_VPANED(obj) GTK_CHECK_TYPE (obj, gtk_vpaned_get_type ())



/* <gtk/gtkvpaned.h> */
typedef struct _GtkVPaned
{
	GtkPaned paned;
} GtkVPaned;


/* <gtk/gtkvpaned.h> */
typedef struct _GtkVPanedClass
{
	GtkPanedClass parent_class;
} GtkVPanedClass;



/* <gtk/gtkvpaned.h> */
guint gtk_vpaned_get_type(void)
{
}

/* <gtk/gtkvpaned.h> */
GtkWidget* gtk_vpaned_new(void)
{
}



/* <gtk/gtkvruler.h> */
#define GTK_VRULER(obj) GTK_CHECK_CAST (obj, gtk_vruler_get_type (), GtkVRuler)

/* <gtk/gtkvruler.h> */
#define GTK_VRULER_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_vruler_get_type (), GtkVRulerClass)

/* <gtk/gtkvruler.h> */
#define GTK_IS_VRULER(obj) GTK_CHECK_TYPE (obj, gtk_vruler_get_type ())



/* <gtk/gtkvruler.h> */
typedef struct _GtkVRuler
{
	GtkRuler ruler;
} GtkVRuler;


/* <gtk/gtkvruler.h> */
typedef struct _GtkVRulerClass
{
	GtkRulerClass parent_class;
} GtkVRulerClass;



/* <gtk/gtkvruler.h> */
guint gtk_vruler_get_type(void)
{
}

/* <gtk/gtkvruler.h> */
GtkWidget* gtk_vruler_new(void)
{
}


/* <gtk/gtkvscale.h> */
#define GTK_TYPE_VSCALE (gtk_vscale_get_type ())

/* <gtk/gtkvscale.h> */
#define GTK_VSCALE(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_VSCALE, GtkVScale))

/* <gtk/gtkvscale.h> */
#define GTK_VSCALE_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_VSCALE, GtkVScaleClass))

/* <gtk/gtkvscale.h> */
#define GTK_IS_VSCALE(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_VSCALE))

/* <gtk/gtkvscale.h> */
#define GTK_IS_VSCALE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VSCALE))



/* <gtk/gtkvscale.h> */
typedef struct _GtkVScale
{
	GtkScale scale;
} GtkVScale;


/* <gtk/gtkvscale.h> */
typedef struct _GtkVScaleClass
{
	GtkScaleClass parent_class;
} GtkVScaleClass;



/* <gtk/gtkvscale.h> */
GtkType gtk_vscale_get_type(void)
{
}

/* <gtk/gtkvscale.h> */
GtkWidget* gtk_vscale_new(GtkAdjustment *adjustment)
{
}


/* <gtk/gtkvscrollbar.h> */
#define GTK_TYPE_VSCROLLBAR (gtk_vscrollbar_get_type ())

/* <gtk/gtkvscrollbar.h> */
#define GTK_VSCROLLBAR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_VSCROLLBAR, GtkVScrollbar))

/* <gtk/gtkvscrollbar.h> */
#define GTK_VSCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_VSCROLLBAR, GtkVScrollbarClass))

/* <gtk/gtkvscrollbar.h> */
#define GTK_IS_VSCROLLBAR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_VSCROLLBAR))

/* <gtk/gtkvscrollbar.h> */
#define GTK_IS_VSCROLLBAR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VSCROLLBAR))



/* <gtk/gtkvscrollbar.h> */
typedef struct _GtkVScrollbar
{
	GtkScrollbar scrollbar;
} GtkVScrollbar;


/* <gtk/gtkvscrollbar.h> */
typedef struct _GtkVScrollbarClass
{
	GtkScrollbarClass parent_class;
} GtkVScrollbarClass;



/* <gtk/gtkvscrollbar.h> */
GtkType gtk_vscrollbar_get_type(void)
{
}

/* <gtk/gtkvscrollbar.h> */
GtkWidget* gtk_vscrollbar_new(GtkAdjustment *adjustment)
{
}


/* <gtk/gtkvseparator.h> */
#define GTK_TYPE_VSEPARATOR (gtk_vseparator_get_type ())

/* <gtk/gtkvseparator.h> */
#define GTK_VSEPARATOR(obj) (GTK_CHECK_CAST ((obj), GTK_TYPE_VSEPARATOR, GtkVSeparator))

/* <gtk/gtkvseparator.h> */
#define GTK_VSEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_VSEPARATOR, GtkVSeparatorClass))

/* <gtk/gtkvseparator.h> */
#define GTK_IS_VSEPARATOR(obj) (GTK_CHECK_TYPE ((obj), GTK_TYPE_VSEPARATOR))

/* <gtk/gtkvseparator.h> */
#define GTK_IS_VSEPARATOR_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_VSEPARATOR))



/* <gtk/gtkvseparator.h> */
typedef struct _GtkVSeparator
{
	GtkSeparator separator;
} GtkVSeparator;


/* <gtk/gtkvseparator.h> */
typedef struct _GtkVSeparatorClass
{
	GtkSeparatorClass parent_class;
} GtkVSeparatorClass;



/* <gtk/gtkvseparator.h> */
GtkType gtk_vseparator_get_type(void)
{
}

/* <gtk/gtkvseparator.h> */
GtkWidget* gtk_vseparator_new(void)
{
}


/* The flags that are used by GtkWidget on top of the
 * flags field of GtkObject.
 */

/* <gtk/gtkwidget.h> */
typedef enum
{
	GTK_TOPLEVEL = 1 << 4,
	GTK_NO_WINDOW = 1 << 5,
	GTK_REALIZED = 1 << 6,
	GTK_MAPPED = 1 << 7,
	GTK_VISIBLE = 1 << 8,
	GTK_SENSITIVE = 1 << 9,
	GTK_PARENT_SENSITIVE = 1 << 10,
	GTK_CAN_FOCUS = 1 << 11,
	GTK_HAS_FOCUS = 1 << 12,
	
	 /* widget is allowed to receive the default via gtk_widget_grab_default
 * and will reserve space to draw the default if possible */
	GTK_CAN_DEFAULT = 1 << 13,

	 /* the widget currently is receiving the default action and should be drawn
 * appropriately if possible */
	GTK_HAS_DEFAULT = 1 << 14,

	GTK_HAS_GRAB = 1 << 15,
	GTK_RC_STYLE = 1 << 16,
	GTK_COMPOSITE_CHILD = 1 << 17,
	GTK_NO_REPARENT = 1 << 18,
	GTK_APP_PAINTABLE = 1 << 19,
	
	 /* the widget when focused will receive the default action and have
 * HAS_DEFAULT set even if there is a different widget set as default */
	GTK_RECEIVES_DEFAULT = 1 << 20
} GtkWidgetFlags;

/* Macro for casting a pointer to a GtkWidget or GtkWidgetClass pointer.
 * Macros for testing whether `widget' or `klass' are of type GTK_TYPE_WIDGET.
 */

/* <gtk/gtkwidget.h> */
#define GTK_TYPE_WIDGET (gtk_widget_get_type ())

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET(widget) (GTK_CHECK_CAST ((widget), GTK_TYPE_WIDGET, GtkWidget))

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_WIDGET, GtkWidgetClass))

/* <gtk/gtkwidget.h> */
#define GTK_IS_WIDGET(widget) (GTK_CHECK_TYPE ((widget), GTK_TYPE_WIDGET))

/* <gtk/gtkwidget.h> */
#define GTK_IS_WIDGET_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_WIDGET))

/* Macros for extracting various fields from GtkWidget and GtkWidgetClass.
 */

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_TYPE(wid) (GTK_OBJECT_TYPE (wid))

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_STATE(wid) (GTK_WIDGET (wid)->state)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_SAVED_STATE(wid) (GTK_WIDGET (wid)->saved_state)

/* Macros for extracting the widget flags from GtkWidget.
 */

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_FLAGS(wid) (GTK_OBJECT_FLAGS (wid))

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_TOPLEVEL(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_TOPLEVEL) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_NO_WINDOW(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_NO_WINDOW) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_REALIZED(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_REALIZED) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_MAPPED(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_MAPPED) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_VISIBLE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_VISIBLE) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_DRAWABLE(wid) (GTK_WIDGET_VISIBLE (wid) && GTK_WIDGET_MAPPED (wid))

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_SENSITIVE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_SENSITIVE) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_PARENT_SENSITIVE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_PARENT_SENSITIVE) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_IS_SENSITIVE(wid) (GTK_WIDGET_SENSITIVE (wid) && \
	GTK_WIDGET_PARENT_SENSITIVE (wid))

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_CAN_FOCUS(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_CAN_FOCUS) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_HAS_FOCUS(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_HAS_FOCUS) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_CAN_DEFAULT(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_CAN_DEFAULT) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_HAS_DEFAULT(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_HAS_DEFAULT) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_HAS_GRAB(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_HAS_GRAB) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_RC_STYLE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_RC_STYLE) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_COMPOSITE_CHILD(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_COMPOSITE_CHILD) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_APP_PAINTABLE(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_APP_PAINTABLE) != 0)

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_RECEIVES_DEFAULT(wid) ((GTK_WIDGET_FLAGS (wid) & GTK_RECEIVES_DEFAULT) != 0)
	 
/* Macros for setting and clearing widget flags.
 */

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_SET_FLAGS(wid,flag) G_STMT_START{ (GTK_WIDGET_FLAGS (wid) |= (flag)); }G_STMT_END

/* <gtk/gtkwidget.h> */
#define GTK_WIDGET_UNSET_FLAGS(wid,flag) G_STMT_START{ (GTK_WIDGET_FLAGS (wid) &= ~(flag)); }G_STMT_END
	 
	 
	 

/* <gtk/gtkwidget.h> */
typedef void(*GtkCallback) (GtkWidget *widget, gpointer data)
{
}

/* A requisition is a desired amount of space which a
 * widget may request.
 */

/* <gtk/gtkwidget.h> */
typedef struct _GtkRequisition
{
	gint16 width;
	gint16 height;
} GtkRequisition;

/* An allocation is a size and position. Where a widget
 * can ask for a desired size, it is actually given
 * this amount of space at the specified position.
 */

/* <gtk/gtkwidget.h> */
typedef struct _GtkAllocation
{
	gint16 x;
	gint16 y;
	guint16 width;
	guint16 height;
} GtkAllocation;

/* The contents of a selection are returned in a GtkSelectionData
	structure. selection/target identify the request. 
	type specifies the type of the return; if length < 0, and
	the data should be ignored. This structure has object semantics -
	no fields should be modified directly, they should not be created
	directly, and pointers to them should not be stored beyond the duration of
	a callback. (If the last is changed, we'll need to add reference
	counting.) The time field gives the timestamp at which the data was sent. */


/* <gtk/gtkwidget.h> */
typedef struct _GtkSelectionData
{
	GdkAtom selection;
	GdkAtom target;
	GdkAtom type;
	gint format;
	guchar *data;
	gint length;
} GtkSelectionData;

/* The widget is the base of the tree for displayable objects.
 * (A displayable object is one which takes up some amount
 * of screen real estate). It provides a common base and interface
 * which actual widgets must adhere to.
 */

/* <gtk/gtkwidget.h> */
typedef struct _GtkWidget
{
	 /* The object structure needs to be the first
 * element in the widget structure in order for
 * the object mechanism to work correctly. This
 * allows a GtkWidget pointer to be cast to a
 * GtkObject pointer.
 */
	GtkObject object;
	 
	 /* 16 bits of internally used private flags.
 * this will be packed into the same 4 byte alignment frame that
 * state and saved_state go. we therefore don't waste any new
 * space on this.
 */
	guint16 private_flags;
	 
	 /* The state of the widget. There are actually only
 * 5 widget states (defined in "gtkenums.h").
 */
	guint8 state;
	 
	 /* The saved state of the widget. When a widgets state
 * is changed to GTK_STATE_INSENSITIVE via
 * "gtk_widget_set_state" or "gtk_widget_set_sensitive"
 * the old state is kept around in this field. The state
 * will be restored once the widget gets sensitive again.
 */
	guint8 saved_state;
	 
	 /* The widgets name. If the widget does not have a name
 * (the name is NULL), then its name (as returned by
 * "gtk_widget_get_name") is its classes name.
 * Among other things, the widget name is used to determine
 * the style to use for a widget.
 */
	gchar *name;
	 
	 /* The style for the widget. The style contains the
 * colors the widget should be drawn in for each state
 * along with graphics contexts used to draw with and
 * the font to use for text.
 */
	GtkStyle *style;
	 
	 /* The widgets desired size.
 */
	GtkRequisition requisition;
	 
	 /* The widgets allocated size.
 */
	GtkAllocation allocation;
	 
	 /* The widgets window or its parent window if it does
 * not have a window. (Which will be indicated by the
 * GTK_NO_WINDOW flag being set).
 */
	GdkWindow *window;
	 
	 /* The widgets parent.
 */
	GtkWidget *parent;
} GtkWidget;


/* <gtk/gtkwidget.h> */
typedef struct _GtkWidgetClass
{
	 /* The object class structure needs to be the first
 * element in the widget class structure in order for
 * the class mechanism to work correctly. This allows a
 * GtkWidgetClass pointer to be cast to a GtkObjectClass
 * pointer.
 */
	GtkObjectClass parent_class;
	 
	 /* The signal to emit when a widget of this class is activated,
 * gtk_widget_activate() handles the emission.
 * Implementation of this signal is optional.
 */
	guint activate_signal;

	 /* This signal is emitted when a widget of this class is added
 * to a scrolling aware parent, gtk_widget_set_scroll_adjustments()
 * handles the emission.
 * Implementation of this signal is optional.
 */
	guint set_scroll_adjustments_signal;
	 
	 /* basics */
	void (* show) (GtkWidget *widget);
	void (* show_all) (GtkWidget *widget);
	void (* hide) (GtkWidget *widget);
	void (* hide_all) (GtkWidget *widget);
	void (* map) (GtkWidget *widget);
	void (* unmap) (GtkWidget *widget);
	void (* realize) (GtkWidget *widget);
	void (* unrealize) (GtkWidget *widget);
	void (* draw) (GtkWidget *widget, GdkRectangle *area);
	void (* draw_focus) (GtkWidget *widget);
	void (* draw_default) (GtkWidget *widget);
	void (* size_request) (GtkWidget *widget, GtkRequisition *requisition);
	void (* size_allocate) (GtkWidget *widget, GtkAllocation *allocation);
	void (* state_changed) (GtkWidget *widget, GtkStateType previous_state);
	void (* parent_set) (GtkWidget *widget, GtkWidget *previous_parent);
	void (* style_set) (GtkWidget *widget, GtkStyle *previous_style);
	 
	 /* accelerators */
	gint (* add_accelerator) (GtkWidget *widget, guint accel_signal_id, GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods, GtkAccelFlags accel_flags);
	void (* remove_accelerator) (GtkWidget *widget, GtkAccelGroup *accel_group, guint accel_key, GdkModifierType accel_mods);

	 /* explicit focus */
	void (* grab_focus) (GtkWidget *widget);
	 
	 /* events */
	gint (* event) (GtkWidget *widget, GdkEvent *event);
	gint (* button_press_event) (GtkWidget *widget, GdkEventButton *event);
	gint (* button_release_event) (GtkWidget *widget, GdkEventButton *event);
	gint (* motion_notify_event) (GtkWidget *widget, GdkEventMotion *event);
	gint (* delete_event) (GtkWidget *widget, GdkEventAny *event);
	gint (* destroy_event) (GtkWidget *widget, GdkEventAny *event);
	gint (* expose_event) (GtkWidget *widget, GdkEventExpose *event);
	gint (* key_press_event) (GtkWidget *widget, GdkEventKey *event);
	gint (* key_release_event) (GtkWidget *widget, GdkEventKey *event);
	gint (* enter_notify_event) (GtkWidget *widget, GdkEventCrossing *event);
	gint (* leave_notify_event) (GtkWidget *widget, GdkEventCrossing *event);
	gint (* configure_event) (GtkWidget *widget, GdkEventConfigure *event);
	gint (* focus_in_event) (GtkWidget *widget, GdkEventFocus *event);
	gint (* focus_out_event) (GtkWidget *widget, GdkEventFocus *event);
	gint (* map_event) (GtkWidget *widget, GdkEventAny *event);
	gint (* unmap_event) (GtkWidget *widget, GdkEventAny *event);
	gint (* property_notify_event) (GtkWidget *widget, GdkEventProperty *event);
	gint (* selection_clear_event) (GtkWidget *widget, GdkEventSelection *event);
	gint (* selection_request_event) (GtkWidget *widget, GdkEventSelection *event);
	gint (* selection_notify_event) (GtkWidget *widget, GdkEventSelection *event);
	gint (* proximity_in_event) (GtkWidget *widget, GdkEventProximity *event);
	gint (* proximity_out_event) (GtkWidget *widget, GdkEventProximity *event);
	gint (* visibility_notify_event) (GtkWidget *widget, GdkEventVisibility *event);
	gint (* client_event) (GtkWidget *widget, GdkEventClient *event);
	gint (* no_expose_event) (GtkWidget *widget, GdkEventAny *event);

	 /* selection */
	void (* selection_get) (GtkWidget *widget, GtkSelectionData *selection_data, guint info, guint time);
	void (* selection_received) (GtkWidget *widget, GtkSelectionData *selection_data, guint time);

	 /* Source side drag signals */
	void (* drag_begin) (GtkWidget *widget, GdkDragContext *context);
	void (* drag_end) (GtkWidget *widget, GdkDragContext *context);
	void (* drag_data_get) (GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time);
	void (* drag_data_delete) (GtkWidget *widget, GdkDragContext *context);

	 /* Target side drag signals */
	void (* drag_leave) (GtkWidget *widget, GdkDragContext *context, guint time);
	gboolean (* drag_motion) (GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time);
	gboolean (* drag_drop) (GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time);
	void (* drag_data_received) (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint time);
	 
	 /* action signals */
	void (* debug_msg) (GtkWidget *widget, const gchar *string);

	 /* Padding for future expandsion */
	GtkFunction pad1;
	GtkFunction pad2;
	GtkFunction pad3;
	GtkFunction pad4;
} GtkWidgetClass;


/* <gtk/gtkwidget.h> */
typedef struct _GtkWidgetAuxInfo
{
	gint16 x;
	gint16 y;
	gint16 width;
	gint16 height;
} GtkWidgetAuxInfo;


/* <gtk/gtkwidget.h> */
typedef struct _GtkWidgetShapeInfo
{
	gint16 offset_x;
	gint16 offset_y;
	GdkBitmap *shape_mask;
} GtkWidgetShapeInfo;



/* <gtk/gtkwidget.h> */
GtkType gtk_widget_get_type(void)
{
}

/* <gtk/gtkwidget.h> */
GtkWidget* gtk_widget_new(GtkType type, const gchar *first_arg_name, ...)
{
}

/* <gtk/gtkwidget.h> */
GtkWidget* gtk_widget_newv(GtkType type, guint nargs, GtkArg *args)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_ref(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_unref(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_destroy(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_destroyed(GtkWidget *widget, GtkWidget **widget_pointer)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_get(GtkWidget *widget, GtkArg *arg)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_getv(GtkWidget *widget, guint nargs, GtkArg *args)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set(GtkWidget *widget, const gchar *first_arg_name, ...)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_setv(GtkWidget *widget, guint nargs, GtkArg *args)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_unparent(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_show(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_show_now(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_hide(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_show_all(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_hide_all(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_map(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_unmap(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_realize(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_unrealize(GtkWidget *widget)
{
}

/* Queuing draws */

/* <gtk/gtkwidget.h> */
void gtk_widget_queue_draw(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_queue_draw_area(GtkWidget *widget, gint x, gint y, gint width, gint height)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_queue_clear(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_queue_clear_area(GtkWidget *widget, gint x, gint y, gint width, gint height)
{
}



/* <gtk/gtkwidget.h> */
void gtk_widget_queue_resize(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_draw(GtkWidget *widget, GdkRectangle *area)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_draw_focus(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_draw_default(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_get_child_requisition(GtkWidget *widget, GtkRequisition *requisition)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_add_accelerator(GtkWidget *widget, const gchar *accel_signal, GtkAccelGroup *accel_group, guint accel_key, guint accel_mods, GtkAccelFlags accel_flags)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_remove_accelerator(GtkWidget *widget, GtkAccelGroup *accel_group, guint accel_key, guint accel_mods)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_remove_accelerators(GtkWidget *widget, const gchar *accel_signal, gboolean visible_only)
{
}

/* <gtk/gtkwidget.h> */
guint gtk_widget_accelerator_signal(GtkWidget *widget, GtkAccelGroup *accel_group, guint accel_key, guint accel_mods)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_lock_accelerators(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_unlock_accelerators(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
gboolean gtk_widget_accelerators_locked(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
gint gtk_widget_event(GtkWidget *widget, GdkEvent *event)
{
}


/* <gtk/gtkwidget.h> */
gboolean gtk_widget_activate(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
gboolean gtk_widget_set_scroll_adjustments(GtkWidget *widget, GtkAdjustment *hadjustment, GtkAdjustment *vadjustment)
{
}
	 

/* <gtk/gtkwidget.h> */
void gtk_widget_reparent(GtkWidget *widget, GtkWidget *new_parent)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_popup(GtkWidget *widget, gint x, gint y)
{
}

/* <gtk/gtkwidget.h> */
gint gtk_widget_intersect(GtkWidget *widget, GdkRectangle *area, GdkRectangle *intersection)
{
}


/* <gtk/gtkwidget.h> */
void gtk_widget_grab_focus(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_grab_default(GtkWidget *widget)
{
}


/* <gtk/gtkwidget.h> */
void gtk_widget_set_name(GtkWidget *widget, const gchar *name)
{
}

/* <gtk/gtkwidget.h> */
gchar* gtk_widget_get_name(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_state(GtkWidget *widget, GtkStateType state)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_sensitive(GtkWidget *widget, gboolean sensitive)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_app_paintable(GtkWidget *widget, gboolean app_paintable)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_parent(GtkWidget *widget, GtkWidget *parent)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_parent_window(GtkWidget *widget, GdkWindow *parent_window)
{
}

/* <gtk/gtkwidget.h> */
GdkWindow *gtk_widget_get_parent_window(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_uposition(GtkWidget *widget, gint x, gint y)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_usize(GtkWidget *widget, gint width, gint height)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_events(GtkWidget *widget, gint events)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_add_events(GtkWidget *widget, gint events)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_extension_events(GtkWidget *widget, GdkExtensionMode mode)
{
}


/* <gtk/gtkwidget.h> */
GdkExtensionMode gtk_widget_get_extension_events(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
GtkWidget* gtk_widget_get_toplevel(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
GtkWidget* gtk_widget_get_ancestor(GtkWidget *widget, GtkType widget_type)
{
}

/* <gtk/gtkwidget.h> */
GdkColormap* gtk_widget_get_colormap(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
GdkVisual* gtk_widget_get_visual(GtkWidget *widget)
{
}

/* The following functions must not be called on an already
 * realized widget. Because it is possible that somebody
 * can call get_colormap() or get_visual() and save the
 * result, these functions are probably only safe to
 * call in a widget's init() function.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_set_colormap(GtkWidget *widget, GdkColormap *colormap)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_visual(GtkWidget *widget, GdkVisual *visual)
{
}



/* <gtk/gtkwidget.h> */
gint gtk_widget_get_events(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_get_pointer(GtkWidget *widget, gint *x, gint *y)
{
}


/* <gtk/gtkwidget.h> */
gboolean gtk_widget_is_ancestor(GtkWidget *widget, GtkWidget *ancestor)
{
}

/* Hide widget and return TRUE.
 */

/* <gtk/gtkwidget.h> */
gint gtk_widget_hide_on_delete(GtkWidget *widget)
{
}

/* Widget styles.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_set_style(GtkWidget *widget, GtkStyle *style)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_rc_style(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_ensure_style(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
GtkStyle* gtk_widget_get_style(GtkWidget *widget)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_restore_default_style(GtkWidget *widget)
{
}


/* <gtk/gtkwidget.h> */
void gtk_widget_modify_style(GtkWidget *widget, GtkRcStyle *style)
{
}

/* handle composite names for GTK_COMPOSITE_CHILD widgets,
 * the returned name is newly allocated.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_set_composite_name(GtkWidget *widget, const gchar *name)
{
}

/* <gtk/gtkwidget.h> */
gchar* gtk_widget_get_composite_name(GtkWidget *widget)
{
}
	 
/* Descend recursively and set rc-style on all widgets without user styles */

/* <gtk/gtkwidget.h> */
void gtk_widget_reset_rc_styles(GtkWidget *widget)
{
}

/* Push/pop pairs, to change default values upon a widget's creation.
 * This will override the values that got set by the
 * gtk_widget_set_default_* () functions.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_push_style(GtkStyle *style)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_push_colormap(GdkColormap *cmap)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_push_visual(GdkVisual *visual)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_push_composite_child(void)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_pop_composite_child(void)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_pop_style(void)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_pop_colormap(void)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_pop_visual(void)
{
}

/* Set certain default values to be used at widget creation time.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_set_default_style(GtkStyle *style)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_default_colormap(GdkColormap *colormap)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_set_default_visual(GdkVisual *visual)
{
}

/* <gtk/gtkwidget.h> */
GtkStyle* gtk_widget_get_default_style(void)
{
}

/* <gtk/gtkwidget.h> */
GdkColormap* gtk_widget_get_default_colormap(void)
{
}

/* <gtk/gtkwidget.h> */
GdkVisual* gtk_widget_get_default_visual(void)
{
}

/* Counterpart to gdk_window_shape_combine_mask.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_shape_combine_mask(GtkWidget *widget, GdkBitmap *shape_mask, gint offset_x, gint offset_y)
{
}

/* internal function */

/* <gtk/gtkwidget.h> */
void gtk_widget_reset_shapes(GtkWidget *widget)
{
}

/* Compute a widget's path in the form "GtkWindow.MyLabel", and
 * return newly alocated strings.
 */

/* <gtk/gtkwidget.h> */
void gtk_widget_path(GtkWidget *widget, guint *path_length, gchar **path, gchar **path_reversed)
{
}

/* <gtk/gtkwidget.h> */
void gtk_widget_class_path(GtkWidget *widget, guint *path_length, gchar **path, gchar **path_reversed)
{
}



/* <gtk/gtkwindow.h> */
#define GTK_TYPE_WINDOW (gtk_window_get_type ())

/* <gtk/gtkwindow.h> */
#define GTK_WINDOW(obj) (GTK_CHECK_CAST (obj, GTK_TYPE_WINDOW, GtkWindow))

/* <gtk/gtkwindow.h> */
#define GTK_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_WINDOW, GtkWindowClass))

/* <gtk/gtkwindow.h> */
#define GTK_IS_WINDOW(obj) (GTK_CHECK_TYPE (obj, GTK_TYPE_WINDOW))

/* <gtk/gtkwindow.h> */
#define GTK_IS_WINDOW_CLASS(klass) (GTK_CHECK_CLASS_TYPE (klass, GTK_TYPE_WINDOW))



/* <gtk/gtkwindow.h> */
typedef struct _GtkWindow
{
	GtkBin bin;

	gchar *title;
	gchar *wmclass_name;
	gchar *wmclass_class;
	GtkWindowType type;

	GtkWidget *focus_widget;
	GtkWidget *default_widget;
	GtkWindow *transient_parent;

	gushort resize_count;
	guint allow_shrink : 1;
	guint allow_grow : 1;
	guint auto_shrink : 1;
	guint handling_resize : 1;
	guint position : 2;

	 /* The following flag is initially TRUE when a window is mapped.
 * and will be set to FALSE after it is first positioned.
 * It is also temporarily reset when the window's size changes.
 * 
 * When TRUE, we move the window to the position the app set.
 */
	guint use_uposition : 1;
	guint modal : 1;

	 /* Set if the window, or any descendent of it, has the focus
 */
	guint window_has_focus : 1;
	 
	 /* Set if !window_has_focus, but events are being sent to the
 * window because the pointer is in it. (Typically, no window
 * manager is running.
 */
	guint window_has_pointer_focus : 1;
} GtkWindow;


/* <gtk/gtkwindow.h> */
typedef struct _GtkWindowClass
{
	GtkBinClass parent_class;

	void (* set_focus) (GtkWindow *window, GtkWidget *focus);
} GtkWindowClass;



/* <gtk/gtkwindow.h> */
GtkType gtk_window_get_type(void)
{
}

/* <gtk/gtkwindow.h> */
GtkWidget* gtk_window_new(GtkWindowType type)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_set_title(GtkWindow *window, const gchar *title)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_set_wmclass(GtkWindow *window, const gchar *wmclass_name, const gchar *wmclass_class)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_set_policy(GtkWindow *window, gint allow_shrink, gint allow_grow, gint auto_shrink)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_add_accel_group(GtkWindow *window, GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_remove_accel_group(GtkWindow *window, GtkAccelGroup *accel_group)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_set_position(GtkWindow *window, GtkWindowPosition position)
{
}

/* <gtk/gtkwindow.h> */
gint gtk_window_activate_focus(GtkWindow *window)
{
}

/* <gtk/gtkwindow.h> */
gint gtk_window_activate_default(GtkWindow *window)
{
}


/* <gtk/gtkwindow.h> */
void gtk_window_set_transient_for(GtkWindow *window, GtkWindow *parent)
{
}

/* <gtk/gtkwindow.h> */
void gtk_window_set_geometry_hints(GtkWindow *window, GtkWidget *geometry_widget, GdkGeometry *geometry, GdkWindowHints geom_mask)
{
}
/* The following differs from gtk_widget_set_usize, in that
 * gtk_widget_set_usize() overrides the requisition, so sets a minimum
 * size, while this only sets the size requested from the WM.
 */

/* <gtk/gtkwindow.h> */
void gtk_window_set_default_size(GtkWindow *window, gint width, gint height)
{
}

/* If window is set modal, input will be grabbed when show and released when hide */

/* <gtk/gtkwindow.h> */
void gtk_window_set_modal(GtkWindow *window, gboolean modal)
{
}

