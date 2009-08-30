/* gdkstub.c */

/* This file contains datatypes and stub functions for the Gdk library.
 * It is derived the the Gdk header files, which bear the following
 * copyright notice...
 */

/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/* <gdk/gdk.h> */
#define GDK_PRIORITY_EVENTS(G_PRIORITY_DEFAULT)

/* <gdk/gdk.h> */
void gdk_init(gint *argc, gchar ***argv)
{
}

/* <gdk/gdk.h> */
gboolean gdk_init_check(gint *argc, gchar ***argv)
{
}

/* <gdk/gdk.h> */
void gdk_exit(gint error_code)
{
}

/* <gdk/gdk.h> */
gchar* gdk_set_locale(void)
{
}

/* <gdk/gdk.h> */
void gdk_error_trap_push(void)
{
}

/* <gdk/gdk.h> */
gint gdk_error_trap_pop(void)
{
}

/* <gdk/gdk.h> */
gboolean gdk_events_pending(void)
{
}

/* <gdk/gdk.h> */
GdkEvent* gdk_event_get(void)
{
}

/* <gdk/gdk.h> */
GdkEvent* gdk_event_peek(void)
{
}

/* <gdk/gdk.h> */
GdkEvent* gdk_event_get_graphics_expose(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_event_put(GdkEvent *event)
{
}

/* <gdk/gdk.h> */
GdkEvent* gdk_event_copy(GdkEvent *event)
{
}

/* <gdk/gdk.h> */
void gdk_event_free(GdkEvent *event)
{
}

/* <gdk/gdk.h> */
guint32 gdk_event_get_time(GdkEvent *event)
{
}

/* <gdk/gdk.h> */
void gdk_event_handler_set(GdkEventFunc func, gpointer data, GDestroyNotify notify)
{
}

/* <gdk/gdk.h> */
void gdk_set_show_events(gboolean show_events)
{
}

/* <gdk/gdk.h> */
void gdk_set_use_xshm(gboolean use_xshm)
{
}

/* <gdk/gdk.h> */
gboolean gdk_get_show_events(void)
{
}

/* <gdk/gdk.h> */
gboolean gdk_get_use_xshm(void)
{
}

/* <gdk/gdk.h> */
gchar* gdk_get_display(void)
{
}

/* <gdk/gdk.h> */
guint32 gdk_time_get(void)
{
}

/* <gdk/gdk.h> */
guint32 gdk_timer_get(void)
{
}

/* <gdk/gdk.h> */
void gdk_timer_set(guint32 milliseconds)
{
}

/* <gdk/gdk.h> */
void gdk_timer_enable(void)
{
}

/* <gdk/gdk.h> */
void gdk_timer_disable(void)
{
}

/* <gdk/gdk.h> */
gint gdk_input_add_full(gint source, GdkInputCondition condition, GdkInputFunction function, gpointer data, GdkDestroyNotify destroy)
{
}

/* <gdk/gdk.h> */
gint gdk_input_add(gint source, GdkInputCondition condition, GdkInputFunction function, gpointer data)
{
}

/* <gdk/gdk.h> */
void gdk_input_remove(gint tag)
{
}

/* <gdk/gdk.h> */
gint gdk_pointer_grab(GdkWindow *window, gint owner_events, GdkEventMask event_mask, GdkWindow *confine_to, GdkCursor *cursor, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_pointer_ungrab(guint32 time)
{
}

/* <gdk/gdk.h> */
gint gdk_keyboard_grab(GdkWindow *window, gboolean owner_events, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_keyboard_ungrab(guint32 time)
{
}

/* <gdk/gdk.h> */
gboolean gdk_pointer_is_grabbed(void)
{
}

/* <gdk/gdk.h> */
gint gdk_screen_width(void)
{
}

/* <gdk/gdk.h> */
gint gdk_screen_height(void)
{
}

/* <gdk/gdk.h> */
gint gdk_screen_width_mm(void)
{
}

/* <gdk/gdk.h> */
gint gdk_screen_height_mm(void)
{
}

/* <gdk/gdk.h> */
void gdk_flush(void)
{
}

/* <gdk/gdk.h> */
void gdk_beep(void)
{
}

/* <gdk/gdk.h> */
void gdk_key_repeat_disable(void)
{
}

/* <gdk/gdk.h> */
void gdk_key_repeat_restore(void)
{
}

/* <gdk/gdk.h> */
gint gdk_visual_get_best_depth(void)
{
}

/* <gdk/gdk.h> */
GdkVisualType gdk_visual_get_best_type(void)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_get_system(void)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_get_best(void)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_get_best_with_depth(gint depth)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_get_best_with_type(GdkVisualType visual_type)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_get_best_with_both(gint depth, GdkVisualType visual_type)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_visual_ref(GdkVisual *visual)
{
}

/* <gdk/gdk.h> */
void gdk_visual_unref(GdkVisual *visual)
{
}

/* <gdk/gdk.h> */
void gdk_query_depths(gint **depths, gint *count)
{
}

/* <gdk/gdk.h> */
void gdk_query_visual_types(GdkVisualType **visual_types, gint *count)
{
}

/* <gdk/gdk.h> */
GList* gdk_list_visuals(void)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_new(GdkWindow *parent, GdkWindowAttr *attributes, gint attributes_mask)
{
}

/* <gdk/gdk.h> */
void gdk_window_destroy(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_ref(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_unref(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_at_pointer(gint *win_x, gint *win_y)
{
}

/* <gdk/gdk.h> */
void gdk_window_show(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_hide(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_withdraw(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_move(GdkWindow *window, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_window_resize(GdkWindow *window, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_window_move_resize(GdkWindow *window, gint x, gint y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_window_reparent(GdkWindow *window, GdkWindow *new_parent, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_window_clear(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_clear_area(GdkWindow *window, gint x, gint y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_window_clear_area_e(GdkWindow *window, gint x, gint y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_window_copy_area(GdkWindow *window, GdkGC *gc, gint x, gint y, GdkWindow *source_window, gint source_x, gint source_y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_window_raise(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_lower(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_user_data(GdkWindow *window, gpointer user_data)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_override_redirect(GdkWindow *window, gboolean override_redirect)
{
}

/* <gdk/gdk.h> */
void gdk_window_add_filter(GdkWindow *window, GdkFilterFunc function, gpointer data)
{
}

/* <gdk/gdk.h> */
void gdk_window_remove_filter(GdkWindow *window, GdkFilterFunc function, gpointer data)
{
}

/* <gdk/gdk.h> */
void gdk_window_shape_combine_mask(GdkWindow *window, GdkBitmap *shape_mask, gint offset_x, gint offset_y)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_child_shapes(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_merge_child_shapes(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
gboolean gdk_window_is_visible(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
gboolean gdk_window_is_viewable(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
gboolean gdk_window_set_static_gravities(GdkWindow *window, gboolean use_static)
{
}

/* <gdk/gdk.h> */
void gdk_add_client_message_filter(GdkAtom message_type, GdkFilterFunc func, gpointer data)
{
}

/* <gdk/gdk.h> */
GdkDragContext * gdk_drag_context_new(void)
{
}

/* <gdk/gdk.h> */
void gdk_drag_context_ref(GdkDragContext *context)
{
}

/* <gdk/gdk.h> */
void gdk_drag_context_unref(GdkDragContext *context)
{
}

/* <gdk/gdk.h> */
void gdk_drag_status(GdkDragContext *context, GdkDragAction action, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_drop_reply(GdkDragContext *context, gboolean ok, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_drop_finish(GdkDragContext *context, gboolean success, guint32 time)
{
}

/* <gdk/gdk.h> */
GdkAtom gdk_drag_get_selection(GdkDragContext *context)
{
}

/* <gdk/gdk.h> */
GdkDragContext * gdk_drag_begin(GdkWindow *window, GList *targets)
{
}

/* <gdk/gdk.h> */
guint32 gdk_drag_get_protocol(guint32 xid, GdkDragProtocol *protocol)
{
}

/* <gdk/gdk.h> */
void gdk_drag_find_window(GdkDragContext *context, GdkWindow *drag_window, gint x_root, gint y_root, GdkWindow **dest_window, GdkDragProtocol *protocol)
{
}

/* <gdk/gdk.h> */
gboolean gdk_drag_motion(GdkDragContext *context, GdkWindow *dest_window, GdkDragProtocol protocol, gint x_root, gint y_root, GdkDragAction suggested_action, GdkDragAction possible_actions, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_drag_drop(GdkDragContext *context, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_drag_abort(GdkDragContext *context, guint32 time)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_hints(GdkWindow *window, gint x, gint y, gint min_width, gint min_height, gint max_width, gint max_height, gint flags)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_geometry_hints(GdkWindow *window, GdkGeometry *geometry, GdkWindowHints flags)
{
}

/* <gdk/gdk.h> */
void gdk_set_sm_client_id(const gchar *sm_client_id)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_title(GdkWindow *window, const gchar *title)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_role(GdkWindow *window, const gchar *role)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_transient_for(GdkWindow *window, GdkWindow *leader)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_background(GdkWindow *window, GdkColor *color)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_back_pixmap(GdkWindow *window, GdkPixmap *pixmap, gboolean parent_relative)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_cursor(GdkWindow *window, GdkCursor *cursor)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_colormap(GdkWindow *window, GdkColormap *colormap)
{
}

/* <gdk/gdk.h> */
void gdk_window_get_user_data(GdkWindow *window, gpointer *data)
{
}

/* <gdk/gdk.h> */
void gdk_window_get_geometry(GdkWindow *window, gint *x, gint *y, gint *width, gint *height, gint *depth)
{
}

/* <gdk/gdk.h> */
void gdk_window_get_position(GdkWindow *window, gint *x, gint *y)
{
}

/* <gdk/gdk.h> */
void gdk_window_get_size(GdkWindow *window, gint *width, gint *height)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_window_get_visual(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkColormap* gdk_window_get_colormap(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkWindowType gdk_window_get_type(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
gint gdk_window_get_origin(GdkWindow *window, gint *x, gint *y)
{
}

/* <gdk/gdk.h> */
gboolean gdk_window_get_deskrelative_origin(GdkWindow *window, gint *x, gint *y)
{
}

/* <gdk/gdk.h> */
void gdk_window_get_root_origin(GdkWindow *window, gint *x, gint *y)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_get_pointer(GdkWindow *window, gint *x, gint *y, GdkModifierType *mask)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_get_parent(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_window_get_toplevel(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GList* gdk_window_get_children(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkEventMask gdk_window_get_events(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_events(GdkWindow *window, GdkEventMask event_mask)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_icon(GdkWindow *window, GdkWindow *icon_window, GdkPixmap *pixmap, GdkBitmap *mask)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_icon_name(GdkWindow *window, const gchar *name)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_group(GdkWindow *window, GdkWindow *leader)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_decorations(GdkWindow *window, GdkWMDecoration decorations)
{
}

/* <gdk/gdk.h> */
void gdk_window_set_functions(GdkWindow *window, GdkWMFunction functions)
{
}

/* <gdk/gdk.h> */
GList * gdk_window_get_toplevels(void)
{
}

/* <gdk/gdk.h> */
void gdk_window_register_dnd(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_drawable_set_data(GdkDrawable *drawable, const gchar *key, gpointer data, GDestroyNotify destroy_func)
{
}

/* <gdk/gdk.h> */
GdkCursor* gdk_cursor_new(GdkCursorType cursor_type)
{
}

/* <gdk/gdk.h> */
GdkCursor* gdk_cursor_new_from_pixmap(GdkPixmap *source, GdkPixmap *mask, GdkColor *fg, GdkColor *bg, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_cursor_destroy(GdkCursor *cursor)
{
}

/* <gdk/gdk.h> */
GdkGC* gdk_gc_new(GdkWindow *window)
{
}

/* <gdk/gdk.h> */
GdkGC* gdk_gc_new_with_values(GdkWindow *window, GdkGCValues *values, GdkGCValuesMask values_mask)
{
}

/* <gdk/gdk.h> */
GdkGC* gdk_gc_ref(GdkGC *gc)
{
}

/* <gdk/gdk.h> */
void gdk_gc_unref(GdkGC *gc)
{
}

/* <gdk/gdk.h> */
void gdk_gc_destroy(GdkGC *gc)
{
}

/* <gdk/gdk.h> */
void gdk_gc_get_values(GdkGC *gc, GdkGCValues *values)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_foreground(GdkGC *gc, GdkColor *color)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_background(GdkGC *gc, GdkColor *color)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_font(GdkGC *gc, GdkFont *font)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_function(GdkGC *gc, GdkFunction function)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_fill(GdkGC *gc, GdkFill fill)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_tile(GdkGC *gc, GdkPixmap *tile)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_stipple(GdkGC *gc, GdkPixmap *stipple)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_ts_origin(GdkGC *gc, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_clip_origin(GdkGC *gc, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_clip_mask(GdkGC *gc, GdkBitmap *mask)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_clip_rectangle(GdkGC *gc, GdkRectangle *rectangle)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_clip_region(GdkGC *gc, GdkRegion *region)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_subwindow(GdkGC *gc, GdkSubwindowMode mode)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_exposures(GdkGC *gc, gboolean exposures)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_line_attributes(GdkGC *gc, gint line_width, GdkLineStyle line_style, GdkCapStyle cap_style, GdkJoinStyle join_style)
{
}

/* <gdk/gdk.h> */
void gdk_gc_set_dashes(GdkGC *gc, gint dash_offset, gint8 dash_list[], gint n)
{
}

/* <gdk/gdk.h> */
void gdk_gc_copy(GdkGC *dst_gc, GdkGC *src_gc)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_new(GdkWindow *window, gint width, gint height, gint depth)
{
}

/* <gdk/gdk.h> */
GdkBitmap* gdk_bitmap_create_from_data(GdkWindow *window, const gchar *data, gint width, gint height)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_create_from_data(GdkWindow *window, const gchar *data, gint width, gint height, gint depth, GdkColor *fg, GdkColor *bg)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_create_from_xpm(GdkWindow *window, GdkBitmap **mask, GdkColor *transparent_color, const gchar *filename)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_colormap_create_from_xpm( GdkWindow *window, GdkColormap *colormap, GdkBitmap **mask, GdkColor *transparent_color, const gchar *filename)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_create_from_xpm_d(GdkWindow *window, GdkBitmap **mask, GdkColor *transparent_color, gchar **data)
{
}

/* <gdk/gdk.h> */
GdkPixmap* gdk_pixmap_colormap_create_from_xpm_d( GdkWindow *window, GdkColormap *colormap, GdkBitmap **mask, GdkColor *transparent_color, gchar **data)
{
}

/* <gdk/gdk.h> */
GdkPixmap *gdk_pixmap_ref(GdkPixmap *pixmap)
{
}

/* <gdk/gdk.h> */
void gdk_pixmap_unref(GdkPixmap *pixmap)
{
}

/* <gdk/gdk.h> */
GdkBitmap *gdk_bitmap_ref(GdkBitmap *pixmap)
{
}

/* <gdk/gdk.h> */
void gdk_bitmap_unref(GdkBitmap *pixmap)
{
}

/* <gdk/gdk.h> */
GdkImage* gdk_image_new_bitmap(GdkVisual *visual, gpointer data, gint width, gint height)
{
}

/* <gdk/gdk.h> */
GdkImage* gdk_image_new(GdkImageType type, GdkVisual *visual, gint width, gint height)
{
}

/* <gdk/gdk.h> */
GdkImage* gdk_image_get(GdkWindow *window, gint x, gint y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_image_put_pixel(GdkImage *image, gint x, gint y, guint32 pixel)
{
}

/* <gdk/gdk.h> */
guint32 gdk_image_get_pixel(GdkImage *image, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_image_destroy(GdkImage *image)
{
}

/* <gdk/gdk.h> */
GdkColormap* gdk_colormap_new(GdkVisual *visual, gboolean allocate)
{
}

/* <gdk/gdk.h> */
GdkColormap* gdk_colormap_ref(GdkColormap *cmap)
{
}

/* <gdk/gdk.h> */
void gdk_colormap_unref(GdkColormap *cmap)
{
}

/* <gdk/gdk.h> */
GdkColormap* gdk_colormap_get_system(void)
{
}

/* <gdk/gdk.h> */
gint gdk_colormap_get_system_size(void)
{
}

/* <gdk/gdk.h> */
void gdk_colormap_change(GdkColormap *colormap, gint ncolors)
{
}

/* <gdk/gdk.h> */
void gdk_colormap_sync(GdkColormap *colormap, gboolean force)
{
}

/* <gdk/gdk.h> */
gint gdk_colormap_alloc_colors(GdkColormap *colormap, GdkColor *colors, gint ncolors, gboolean writeable, gboolean best_match, gboolean *success)
{
}

/* <gdk/gdk.h> */
gboolean gdk_colormap_alloc_color(GdkColormap *colormap, GdkColor *color, gboolean writeable, gboolean best_match)
{
}

/* <gdk/gdk.h> */
void gdk_colormap_free_colors(GdkColormap *colormap, GdkColor *colors, gint ncolors)
{
}

/* <gdk/gdk.h> */
GdkVisual* gdk_colormap_get_visual(GdkColormap *colormap)
{
}

/* <gdk/gdk.h> */
GdkColor* gdk_color_copy(const GdkColor *color)
{
}

/* <gdk/gdk.h> */
void gdk_color_free(GdkColor *color)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_parse(const gchar *spec, GdkColor *color)
{
}

/* <gdk/gdk.h> */
guint gdk_color_hash(const GdkColor *colora, const GdkColor *colorb)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_equal(const GdkColor *colora, const GdkColor *colorb)
{
}

/* <gdk/gdk.h> */
void gdk_colors_store(GdkColormap *colormap, GdkColor *colors, gint ncolors)
{
}

/* <gdk/gdk.h> */
gboolean gdk_colors_alloc(GdkColormap *colormap, gboolean contiguous, gulong *planes, gint nplanes, gulong *pixels, gint npixels)
{
}

/* <gdk/gdk.h> */
void gdk_colors_free(GdkColormap *colormap, gulong *pixels, gint npixels, gulong planes)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_white(GdkColormap *colormap, GdkColor *color)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_black(GdkColormap *colormap, GdkColor *color)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_alloc(GdkColormap *colormap, GdkColor *color)
{
}

/* <gdk/gdk.h> */
gboolean gdk_color_change(GdkColormap *colormap, GdkColor *color)
{
}

/* <gdk/gdk.h> */
GdkFont* gdk_font_load(const gchar *font_name)
{
}

/* <gdk/gdk.h> */
GdkFont* gdk_fontset_load(const gchar *fontset_name)
{
}

/* <gdk/gdk.h> */
GdkFont* gdk_font_ref(GdkFont *font)
{
}

/* <gdk/gdk.h> */
void gdk_font_unref(GdkFont *font)
{
}

/* <gdk/gdk.h> */
gint gdk_font_id(const GdkFont *font)
{
}

/* <gdk/gdk.h> */
gboolean gdk_font_equal(const GdkFont *fonta, const GdkFont *fontb)
{
}

/* <gdk/gdk.h> */
gint gdk_string_width(GdkFont *font, const gchar *string)
{
}

/* <gdk/gdk.h> */
gint gdk_text_width(GdkFont *font, const gchar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
gint gdk_text_width_wc(GdkFont *font, const GdkWChar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
gint gdk_char_width(GdkFont *font, gchar character)
{
}

/* <gdk/gdk.h> */
gint gdk_char_width_wc(GdkFont *font, GdkWChar character)
{
}

/* <gdk/gdk.h> */
gint gdk_string_measure(GdkFont *font, const gchar *string)
{
}

/* <gdk/gdk.h> */
gint gdk_text_measure(GdkFont *font, const gchar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
gint gdk_char_measure(GdkFont *font, gchar character)
{
}

/* <gdk/gdk.h> */
gint gdk_string_height(GdkFont *font, const gchar *string)
{
}

/* <gdk/gdk.h> */
gint gdk_text_height(GdkFont *font, const gchar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
gint gdk_char_height(GdkFont *font, gchar character)
{
}

/* <gdk/gdk.h> */
void gdk_text_extents(GdkFont *font, const gchar *text, gint text_length, gint *lbearing, gint *rbearing, gint *width, gint *ascent, gint *descent)
{
}

/* <gdk/gdk.h> */
void gdk_text_extents_wc(GdkFont *font, const GdkWChar *text, gint text_length, gint *lbearing, gint *rbearing, gint *width, gint *ascent, gint *descent)
{
}

/* <gdk/gdk.h> */
void gdk_string_extents(GdkFont *font, const gchar *string, gint *lbearing, gint *rbearing, gint *width, gint *ascent, gint *descent)
{
}

/* <gdk/gdk.h> */
void gdk_draw_point(GdkDrawable *drawable, GdkGC *gc, gint x, gint y)
{
}

/* <gdk/gdk.h> */
void gdk_draw_line(GdkDrawable *drawable, GdkGC *gc, gint x1, gint y1, gint x2, gint y2)
{
}

/* <gdk/gdk.h> */
void gdk_draw_rectangle(GdkDrawable *drawable, GdkGC *gc, gint filled, gint x, gint y, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_draw_arc(GdkDrawable *drawable, GdkGC *gc, gint filled, gint x, gint y, gint width, gint height, gint angle1, gint angle2)
{
}

/* <gdk/gdk.h> */
void gdk_draw_polygon(GdkDrawable *drawable, GdkGC *gc, gint filled, GdkPoint *points, gint npoints)
{
}

/* <gdk/gdk.h> */
void gdk_draw_string(GdkDrawable *drawable, GdkFont *font, GdkGC *gc, gint x, gint y, const gchar *string)
{
}

/* <gdk/gdk.h> */
void gdk_draw_text(GdkDrawable *drawable, GdkFont *font, GdkGC *gc, gint x, gint y, const gchar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
void gdk_draw_text_wc(GdkDrawable *drawable, GdkFont *font, GdkGC *gc, gint x, gint y, const GdkWChar *text, gint text_length)
{
}

/* <gdk/gdk.h> */
void gdk_draw_pixmap(GdkDrawable *drawable, GdkGC *gc, GdkDrawable *src, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_draw_image(GdkDrawable *drawable, GdkGC *gc, GdkImage *image, gint xsrc, gint ysrc, gint xdest, gint ydest, gint width, gint height)
{
}

/* <gdk/gdk.h> */
void gdk_draw_points(GdkDrawable *drawable, GdkGC *gc, GdkPoint *points, gint npoints)
{
}

/* <gdk/gdk.h> */
void gdk_draw_segments(GdkDrawable *drawable, GdkGC *gc, GdkSegment *segs, gint nsegs)
{
}

/* <gdk/gdk.h> */
void gdk_draw_lines(GdkDrawable *drawable, GdkGC *gc, GdkPoint *points, gint npoints)
{
}

/* <gdk/gdk.h> */
gboolean gdk_selection_owner_set(GdkWindow *owner, GdkAtom selection, guint32 time, gint send_event)
{
}

/* <gdk/gdk.h> */
GdkWindow* gdk_selection_owner_get(GdkAtom selection)
{
}

/* <gdk/gdk.h> */
void gdk_selection_convert(GdkWindow *requestor, GdkAtom selection, GdkAtom target, guint32 time)
{
}

/* <gdk/gdk.h> */
gboolean gdk_selection_property_get(GdkWindow *requestor, guchar **data, GdkAtom *prop_type, gint *prop_format)
{
}

/* <gdk/gdk.h> */
void gdk_selection_send_notify(guint32 requestor, GdkAtom selection, GdkAtom target, GdkAtom property, guint32 time)
{
}

/* <gdk/gdk.h> */
gint gdk_text_property_to_text_list(GdkAtom encoding, gint format, guchar *text, gint length, gchar ***list)
{
}

/* <gdk/gdk.h> */
void gdk_free_text_list(gchar **list)
{
}

/* <gdk/gdk.h> */
gint gdk_string_to_compound_text(const gchar *str, GdkAtom *encoding, gint *format, guchar **ctext, gint *length)
{
}

/* <gdk/gdk.h> */
void gdk_free_compound_text(guchar *ctext)
{
}

/* <gdk/gdk.h> */
GdkAtom gdk_atom_intern(const gchar *atom_name, gint only_if_exists)
{
}

/* <gdk/gdk.h> */
gchar* gdk_atom_name(GdkAtom atom)
{
}

/* <gdk/gdk.h> */
gboolean gdk_property_get(GdkWindow *window, GdkAtom property, GdkAtom type, gulong offset, gulong length, gint pdelete, GdkAtom *actual_property_type, gint *actual_format, gint *actual_length, guchar **data)
{
}

/* <gdk/gdk.h> */
void gdk_property_change(GdkWindow *window, GdkAtom property, GdkAtom type, gint format, GdkPropMode mode, guchar *data, gint nelements)
{
}

/* <gdk/gdk.h> */
void gdk_property_delete(GdkWindow *window, GdkAtom property)
{
}

/* <gdk/gdk.h> */
gboolean gdk_rectangle_intersect(GdkRectangle *src1, GdkRectangle *src2, GdkRectangle *dest)
{
}

/* <gdk/gdk.h> */
void gdk_rectangle_union(GdkRectangle *src1, GdkRectangle *src2, GdkRectangle *dest)
{
}

/* <gdk/gdk.h> */
void gdk_input_init(void)
{
}

/* <gdk/gdk.h> */
void gdk_input_exit(void)
{
}

/* <gdk/gdk.h> */
GList * gdk_input_list_devices(void)
{
}

/* <gdk/gdk.h> */
void gdk_input_set_extension_events(GdkWindow *window, gint mask, GdkExtensionMode mode)
{
}

/* <gdk/gdk.h> */
void gdk_input_set_source(guint32 deviceid, GdkInputSource source)
{
}

/* <gdk/gdk.h> */
gboolean gdk_input_set_mode(guint32 deviceid, GdkInputMode mode)
{
}

/* <gdk/gdk.h> */
void gdk_input_set_axes(guint32 deviceid, GdkAxisUse *axes)
{
}

/* <gdk/gdk.h> */
void gdk_input_set_key(guint32 deviceid, guint index, guint keyval, GdkModifierType modifiers)
{
}

/* <gdk/gdk.h> */
void gdk_input_window_get_pointer(GdkWindow *window, guint32 deviceid, gdouble *x, gdouble *y, gdouble *pressure, gdouble *xtilt, gdouble *ytilt, GdkModifierType *mask)
{
}

/* <gdk/gdk.h> */
GdkTimeCoord *gdk_input_motion_events(GdkWindow *window, guint32 deviceid, guint32 start, guint32 stop, gint *nevents_return)
{
}

/* <gdk/gdk.h> */
gboolean gdk_im_ready(void)
{
}

/* <gdk/gdk.h> */
void gdk_im_begin(GdkIC *ic, GdkWindow *window)
{
}

/* <gdk/gdk.h> */
void gdk_im_end(void)
{
}

/* <gdk/gdk.h> */
GdkIMStyle gdk_im_decide_style(GdkIMStyle supported_style)
{
}

/* <gdk/gdk.h> */
GdkIMStyle gdk_im_set_best_style(GdkIMStyle best_allowed_style)
{
}

/* <gdk/gdk.h> */
GdkIC* gdk_ic_new(GdkICAttr *attr, GdkICAttributesType mask)
{
}

/* <gdk/gdk.h> */
void gdk_ic_destroy(GdkIC *ic)
{
}

/* <gdk/gdk.h> */
GdkIMStyle gdk_ic_get_style(GdkIC *ic)
{
}

/* <gdk/gdk.h> */
GdkEventMask gdk_ic_get_events(GdkIC *ic)
{
}

/* <gdk/gdk.h> */
GdkICAttr* gdk_ic_attr_new(void)
{
}

/* <gdk/gdk.h> */
void gdk_ic_attr_destroy(GdkICAttr *attr)
{
}

/* <gdk/gdk.h> */
GdkICAttributesType gdk_ic_set_attr(GdkIC *ic,  GdkICAttr *attr, GdkICAttributesType mask)
{
}

/* <gdk/gdk.h> */
GdkICAttributesType gdk_ic_get_attr(GdkIC *ic, GdkICAttr *attr, GdkICAttributesType mask)
{
}

/* <gdk/gdk.h> */
gchar *gdk_wcstombs(const GdkWChar *src)
{
}

/* <gdk/gdk.h> */
gint gdk_mbstowcs(GdkWChar *dest, const gchar *src, gint dest_max)
{
}

/* <gdk/gdk.h> */
GdkColorContext *gdk_color_context_new(GdkVisual *visual, GdkColormap *colormap)
{
}

/* <gdk/gdk.h> */
GdkColorContext *gdk_color_context_new_mono(GdkVisual *visual, GdkColormap *colormap)
{
}

/* <gdk/gdk.h> */
void gdk_color_context_free(GdkColorContext *cc)
{
}

/* <gdk/gdk.h> */
gulong gdk_color_context_get_pixel(GdkColorContext *cc, gushort red, gushort green, gushort blue, gint *failed)
{
}

/* <gdk/gdk.h> */
void gdk_color_context_get_pixels(GdkColorContext *cc, gushort *reds, gushort *greens, gushort *blues, gint ncolors, gulong *colors, gint *nallocated)
{
}

/* <gdk/gdk.h> */
void gdk_color_context_get_pixels_incremental(GdkColorContext *cc, gushort *reds, gushort *greens, gushort *blues, gint ncolors, gint *used, gulong *colors, gint *nallocated)
{
}

/* <gdk/gdk.h> */
gint gdk_color_context_query_color(GdkColorContext *cc, GdkColor *color)
{
}

/* <gdk/gdk.h> */
gint gdk_color_context_query_colors(GdkColorContext *cc, GdkColor *colors, gint num_colors)
{
}

/* <gdk/gdk.h> */
gint gdk_color_context_add_palette(GdkColorContext *cc, GdkColor *palette, gint num_palette)
{
}

/* <gdk/gdk.h> */
void gdk_color_context_init_dither(GdkColorContext *cc)
{
}

/* <gdk/gdk.h> */
void gdk_color_context_free_dither(GdkColorContext *cc)
{
}

/* <gdk/gdk.h> */
gulong gdk_color_context_get_pixel_from_palette(GdkColorContext *cc, gushort *red, gushort *green, gushort *blue, gint *failed)
{
}

/* <gdk/gdk.h> */
guchar gdk_color_context_get_index_from_palette(GdkColorContext *cc, gint *red, gint *green, gint *blue, gint *failed)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_region_new(void)
{
}

/* <gdk/gdk.h> */
void gdk_region_destroy(GdkRegion *region)
{
}

/* <gdk/gdk.h> */
void gdk_region_get_clipbox(GdkRegion *region, GdkRectangle *rectangle)
{
}

/* <gdk/gdk.h> */
gboolean gdk_region_empty(GdkRegion *region)
{
}

/* <gdk/gdk.h> */
gboolean gdk_region_equal(GdkRegion *region1, GdkRegion *region2)
{
}

/* <gdk/gdk.h> */
gboolean gdk_region_point_in(GdkRegion *region, int x, int y)
{
}

/* <gdk/gdk.h> */
GdkOverlapType gdk_region_rect_in(GdkRegion *region, GdkRectangle *rect)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_region_polygon(GdkPoint *points, gint npoints, GdkFillRule fill_rule)
{
}

/* <gdk/gdk.h> */
void gdk_region_offset(GdkRegion *region, gint dx, gint dy)
{
}

/* <gdk/gdk.h> */
void gdk_region_shrink(GdkRegion *region, gint dx, gint dy)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_region_union_with_rect(GdkRegion *region, GdkRectangle *rect)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_regions_intersect(GdkRegion *source1, GdkRegion *source2)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_regions_union(GdkRegion *source1, GdkRegion *source2)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_regions_subtract(GdkRegion *source1, GdkRegion *source2)
{
}

/* <gdk/gdk.h> */
GdkRegion* gdk_regions_xor(GdkRegion *source1, GdkRegion *source2)
{
}

/* <gdk/gdk.h> */
void gdk_event_send_clientmessage_toall(GdkEvent *event)
{
}

/* <gdk/gdk.h> */
gboolean gdk_event_send_client_message(GdkEvent *event, guint32 xid)
{
}

/* <gdk/gdk.h> */
gchar* gdk_keyval_name(guint keyval)
{
}

/* <gdk/gdk.h> */
guint gdk_keyval_from_name(const gchar *keyval_name)
{
}

/* <gdk/gdk.h> */
guint gdk_keyval_to_upper(guint keyval)
{
}

/* <gdk/gdk.h> */
guint gdk_keyval_to_lower(guint keyval)
{
}

/* <gdk/gdk.h> */
gboolean gdk_keyval_is_upper(guint keyval)
{
}

/* <gdk/gdk.h> */
gboolean gdk_keyval_is_lower(guint keyval)
{
}

/* <gdk/gdk.h> */
extern GMutex *gdk_threads_mutex;

/* <gdk/gdk.h> */
void gdk_threads_enter(void)
{
}

/* <gdk/gdk.h> */
void gdk_threads_leave(void)
{
}

/* <gdk/gdk.h> */
#define GDK_THREADS_ENTER()	/* lock gdk_threads_mutex */

/* <gdk/gdk.h> */
#define GDK_THREADS_LEAVE()	/* unlock gdk_threads_mutex */

#include <gdk/gdkrgb.h>

/* <gdk/gdki18n.h> */
# define gdk_iswalnum(c) iswalnum(c)

/* <gdk/gdki18n.h> */
# define gdk_iswspace(c) iswspace(c)

/* <gdk/gdkkeysyms.h> */
#define GDK_VoidSymbol 0xFFFFFF

/* <gdk/gdkkeysyms.h> */
#define GDK_BackSpace 0xFF08

/* <gdk/gdkkeysyms.h> */
#define GDK_Tab 0xFF09

/* <gdk/gdkkeysyms.h> */
#define GDK_Linefeed 0xFF0A

/* <gdk/gdkkeysyms.h> */
#define GDK_Clear 0xFF0B

/* <gdk/gdkkeysyms.h> */
#define GDK_Return 0xFF0D

/* <gdk/gdkkeysyms.h> */
#define GDK_Pause 0xFF13

/* <gdk/gdkkeysyms.h> */
#define GDK_Scroll_Lock 0xFF14

/* <gdk/gdkkeysyms.h> */
#define GDK_Sys_Req 0xFF15

/* <gdk/gdkkeysyms.h> */
#define GDK_Escape 0xFF1B

/* <gdk/gdkkeysyms.h> */
#define GDK_Delete 0xFFFF

/* <gdk/gdkkeysyms.h> */
#define GDK_Multi_key 0xFF20

/* <gdk/gdkkeysyms.h> */
#define GDK_SingleCandidate 0xFF3C

/* <gdk/gdkkeysyms.h> */
#define GDK_MultipleCandidate 0xFF3D

/* <gdk/gdkkeysyms.h> */
#define GDK_PreviousCandidate 0xFF3E

/* <gdk/gdkkeysyms.h> */
#define GDK_Kanji 0xFF21

/* <gdk/gdkkeysyms.h> */
#define GDK_Muhenkan 0xFF22

/* <gdk/gdkkeysyms.h> */
#define GDK_Henkan_Mode 0xFF23

/* <gdk/gdkkeysyms.h> */
#define GDK_Henkan 0xFF23

/* <gdk/gdkkeysyms.h> */
#define GDK_Romaji 0xFF24

/* <gdk/gdkkeysyms.h> */
#define GDK_Hiragana 0xFF25

/* <gdk/gdkkeysyms.h> */
#define GDK_Katakana 0xFF26

/* <gdk/gdkkeysyms.h> */
#define GDK_Hiragana_Katakana 0xFF27

/* <gdk/gdkkeysyms.h> */
#define GDK_Zenkaku 0xFF28

/* <gdk/gdkkeysyms.h> */
#define GDK_Hankaku 0xFF29

/* <gdk/gdkkeysyms.h> */
#define GDK_Zenkaku_Hankaku 0xFF2A

/* <gdk/gdkkeysyms.h> */
#define GDK_Touroku 0xFF2B

/* <gdk/gdkkeysyms.h> */
#define GDK_Massyo 0xFF2C

/* <gdk/gdkkeysyms.h> */
#define GDK_Kana_Lock 0xFF2D

/* <gdk/gdkkeysyms.h> */
#define GDK_Kana_Shift 0xFF2E

/* <gdk/gdkkeysyms.h> */
#define GDK_Eisu_Shift 0xFF2F

/* <gdk/gdkkeysyms.h> */
#define GDK_Eisu_toggle 0xFF30

/* <gdk/gdkkeysyms.h> */
#define GDK_Zen_Koho 0xFF3D

/* <gdk/gdkkeysyms.h> */
#define GDK_Mae_Koho 0xFF3E

/* <gdk/gdkkeysyms.h> */
#define GDK_Home 0xFF50

/* <gdk/gdkkeysyms.h> */
#define GDK_Left 0xFF51

/* <gdk/gdkkeysyms.h> */
#define GDK_Up 0xFF52

/* <gdk/gdkkeysyms.h> */
#define GDK_Right 0xFF53

/* <gdk/gdkkeysyms.h> */
#define GDK_Down 0xFF54

/* <gdk/gdkkeysyms.h> */
#define GDK_Prior 0xFF55

/* <gdk/gdkkeysyms.h> */
#define GDK_Page_Up 0xFF55

/* <gdk/gdkkeysyms.h> */
#define GDK_Next 0xFF56

/* <gdk/gdkkeysyms.h> */
#define GDK_Page_Down 0xFF56

/* <gdk/gdkkeysyms.h> */
#define GDK_End 0xFF57

/* <gdk/gdkkeysyms.h> */
#define GDK_Begin 0xFF58

/* <gdk/gdkkeysyms.h> */
#define GDK_Select 0xFF60

/* <gdk/gdkkeysyms.h> */
#define GDK_Print 0xFF61

/* <gdk/gdkkeysyms.h> */
#define GDK_Execute 0xFF62

/* <gdk/gdkkeysyms.h> */
#define GDK_Insert 0xFF63

/* <gdk/gdkkeysyms.h> */
#define GDK_Undo 0xFF65

/* <gdk/gdkkeysyms.h> */
#define GDK_Redo 0xFF66

/* <gdk/gdkkeysyms.h> */
#define GDK_Menu 0xFF67

/* <gdk/gdkkeysyms.h> */
#define GDK_Find 0xFF68

/* <gdk/gdkkeysyms.h> */
#define GDK_Cancel 0xFF69

/* <gdk/gdkkeysyms.h> */
#define GDK_Help 0xFF6A

/* <gdk/gdkkeysyms.h> */
#define GDK_Break 0xFF6B

/* <gdk/gdkkeysyms.h> */
#define GDK_Mode_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_script_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_Num_Lock 0xFF7F

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Space 0xFF80

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Tab 0xFF89

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Enter 0xFF8D

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_F1 0xFF91

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_F2 0xFF92

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_F3 0xFF93

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_F4 0xFF94

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Home 0xFF95

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Left 0xFF96

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Up 0xFF97

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Right 0xFF98

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Down 0xFF99

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Prior 0xFF9A

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Page_Up 0xFF9A

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Next 0xFF9B

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Page_Down 0xFF9B

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_End 0xFF9C

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Begin 0xFF9D

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Insert 0xFF9E

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Delete 0xFF9F

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Equal 0xFFBD

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Multiply 0xFFAA

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Add 0xFFAB

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Separator 0xFFAC

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Subtract 0xFFAD

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Decimal 0xFFAE

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_Divide 0xFFAF

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_0 0xFFB0

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_1 0xFFB1

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_2 0xFFB2

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_3 0xFFB3

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_4 0xFFB4

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_5 0xFFB5

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_6 0xFFB6

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_7 0xFFB7

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_8 0xFFB8

/* <gdk/gdkkeysyms.h> */
#define GDK_KP_9 0xFFB9

/* <gdk/gdkkeysyms.h> */
#define GDK_F1 0xFFBE

/* <gdk/gdkkeysyms.h> */
#define GDK_F2 0xFFBF

/* <gdk/gdkkeysyms.h> */
#define GDK_F3 0xFFC0

/* <gdk/gdkkeysyms.h> */
#define GDK_F4 0xFFC1

/* <gdk/gdkkeysyms.h> */
#define GDK_F5 0xFFC2

/* <gdk/gdkkeysyms.h> */
#define GDK_F6 0xFFC3

/* <gdk/gdkkeysyms.h> */
#define GDK_F7 0xFFC4

/* <gdk/gdkkeysyms.h> */
#define GDK_F8 0xFFC5

/* <gdk/gdkkeysyms.h> */
#define GDK_F9 0xFFC6

/* <gdk/gdkkeysyms.h> */
#define GDK_F10 0xFFC7

/* <gdk/gdkkeysyms.h> */
#define GDK_F11 0xFFC8

/* <gdk/gdkkeysyms.h> */
#define GDK_L1 0xFFC8

/* <gdk/gdkkeysyms.h> */
#define GDK_F12 0xFFC9

/* <gdk/gdkkeysyms.h> */
#define GDK_L2 0xFFC9

/* <gdk/gdkkeysyms.h> */
#define GDK_F13 0xFFCA

/* <gdk/gdkkeysyms.h> */
#define GDK_L3 0xFFCA

/* <gdk/gdkkeysyms.h> */
#define GDK_F14 0xFFCB

/* <gdk/gdkkeysyms.h> */
#define GDK_L4 0xFFCB

/* <gdk/gdkkeysyms.h> */
#define GDK_F15 0xFFCC

/* <gdk/gdkkeysyms.h> */
#define GDK_L5 0xFFCC

/* <gdk/gdkkeysyms.h> */
#define GDK_F16 0xFFCD

/* <gdk/gdkkeysyms.h> */
#define GDK_L6 0xFFCD

/* <gdk/gdkkeysyms.h> */
#define GDK_F17 0xFFCE

/* <gdk/gdkkeysyms.h> */
#define GDK_L7 0xFFCE

/* <gdk/gdkkeysyms.h> */
#define GDK_F18 0xFFCF

/* <gdk/gdkkeysyms.h> */
#define GDK_L8 0xFFCF

/* <gdk/gdkkeysyms.h> */
#define GDK_F19 0xFFD0

/* <gdk/gdkkeysyms.h> */
#define GDK_L9 0xFFD0

/* <gdk/gdkkeysyms.h> */
#define GDK_F20 0xFFD1

/* <gdk/gdkkeysyms.h> */
#define GDK_L10 0xFFD1

/* <gdk/gdkkeysyms.h> */
#define GDK_F21 0xFFD2

/* <gdk/gdkkeysyms.h> */
#define GDK_R1 0xFFD2

/* <gdk/gdkkeysyms.h> */
#define GDK_F22 0xFFD3

/* <gdk/gdkkeysyms.h> */
#define GDK_R2 0xFFD3

/* <gdk/gdkkeysyms.h> */
#define GDK_F23 0xFFD4

/* <gdk/gdkkeysyms.h> */
#define GDK_R3 0xFFD4

/* <gdk/gdkkeysyms.h> */
#define GDK_F24 0xFFD5

/* <gdk/gdkkeysyms.h> */
#define GDK_R4 0xFFD5

/* <gdk/gdkkeysyms.h> */
#define GDK_F25 0xFFD6

/* <gdk/gdkkeysyms.h> */
#define GDK_R5 0xFFD6

/* <gdk/gdkkeysyms.h> */
#define GDK_F26 0xFFD7

/* <gdk/gdkkeysyms.h> */
#define GDK_R6 0xFFD7

/* <gdk/gdkkeysyms.h> */
#define GDK_F27 0xFFD8

/* <gdk/gdkkeysyms.h> */
#define GDK_R7 0xFFD8

/* <gdk/gdkkeysyms.h> */
#define GDK_F28 0xFFD9

/* <gdk/gdkkeysyms.h> */
#define GDK_R8 0xFFD9

/* <gdk/gdkkeysyms.h> */
#define GDK_F29 0xFFDA

/* <gdk/gdkkeysyms.h> */
#define GDK_R9 0xFFDA

/* <gdk/gdkkeysyms.h> */
#define GDK_F30 0xFFDB

/* <gdk/gdkkeysyms.h> */
#define GDK_R10 0xFFDB

/* <gdk/gdkkeysyms.h> */
#define GDK_F31 0xFFDC

/* <gdk/gdkkeysyms.h> */
#define GDK_R11 0xFFDC

/* <gdk/gdkkeysyms.h> */
#define GDK_F32 0xFFDD

/* <gdk/gdkkeysyms.h> */
#define GDK_R12 0xFFDD

/* <gdk/gdkkeysyms.h> */
#define GDK_F33 0xFFDE

/* <gdk/gdkkeysyms.h> */
#define GDK_R13 0xFFDE

/* <gdk/gdkkeysyms.h> */
#define GDK_F34 0xFFDF

/* <gdk/gdkkeysyms.h> */
#define GDK_R14 0xFFDF

/* <gdk/gdkkeysyms.h> */
#define GDK_F35 0xFFE0

/* <gdk/gdkkeysyms.h> */
#define GDK_R15 0xFFE0

/* <gdk/gdkkeysyms.h> */
#define GDK_Shift_L 0xFFE1

/* <gdk/gdkkeysyms.h> */
#define GDK_Shift_R 0xFFE2

/* <gdk/gdkkeysyms.h> */
#define GDK_Control_L 0xFFE3

/* <gdk/gdkkeysyms.h> */
#define GDK_Control_R 0xFFE4

/* <gdk/gdkkeysyms.h> */
#define GDK_Caps_Lock 0xFFE5

/* <gdk/gdkkeysyms.h> */
#define GDK_Shift_Lock 0xFFE6

/* <gdk/gdkkeysyms.h> */
#define GDK_Meta_L 0xFFE7

/* <gdk/gdkkeysyms.h> */
#define GDK_Meta_R 0xFFE8

/* <gdk/gdkkeysyms.h> */
#define GDK_Alt_L 0xFFE9

/* <gdk/gdkkeysyms.h> */
#define GDK_Alt_R 0xFFEA

/* <gdk/gdkkeysyms.h> */
#define GDK_Super_L 0xFFEB

/* <gdk/gdkkeysyms.h> */
#define GDK_Super_R 0xFFEC

/* <gdk/gdkkeysyms.h> */
#define GDK_Hyper_L 0xFFED

/* <gdk/gdkkeysyms.h> */
#define GDK_Hyper_R 0xFFEE

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Lock 0xFE01

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Level2_Latch 0xFE02

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Level3_Shift 0xFE03

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Level3_Latch 0xFE04

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Level3_Lock 0xFE05

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Group_Shift 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Group_Latch 0xFE06

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Group_Lock 0xFE07

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Next_Group 0xFE08

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Next_Group_Lock 0xFE09

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Prev_Group 0xFE0A

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Prev_Group_Lock 0xFE0B

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_First_Group 0xFE0C

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_First_Group_Lock 0xFE0D

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Last_Group 0xFE0E

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Last_Group_Lock 0xFE0F

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Left_Tab 0xFE20

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Move_Line_Up 0xFE21

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Move_Line_Down 0xFE22

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Partial_Line_Up 0xFE23

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Partial_Line_Down 0xFE24

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Partial_Space_Left 0xFE25

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Partial_Space_Right 0xFE26

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Set_Margin_Left 0xFE27

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Set_Margin_Right 0xFE28

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Release_Margin_Left 0xFE29

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Release_Margin_Right 0xFE2A

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Release_Both_Margins 0xFE2B

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Fast_Cursor_Left 0xFE2C

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Fast_Cursor_Right 0xFE2D

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Fast_Cursor_Up 0xFE2E

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Fast_Cursor_Down 0xFE2F

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Continuous_Underline 0xFE30

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Discontinuous_Underline 0xFE31

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Emphasize 0xFE32

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Center_Object 0xFE33

/* <gdk/gdkkeysyms.h> */
#define GDK_ISO_Enter 0xFE34

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_grave 0xFE50

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_acute 0xFE51

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_circumflex 0xFE52

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_tilde 0xFE53

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_macron 0xFE54

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_breve 0xFE55

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_abovedot 0xFE56

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_diaeresis 0xFE57

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_abovering 0xFE58

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_doubleacute 0xFE59

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_caron 0xFE5A

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_cedilla 0xFE5B

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_ogonek 0xFE5C

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_iota 0xFE5D

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_voiced_sound 0xFE5E

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_semivoiced_sound 0xFE5F

/* <gdk/gdkkeysyms.h> */
#define GDK_dead_belowdot 0xFE60

/* <gdk/gdkkeysyms.h> */
#define GDK_First_Virtual_Screen 0xFED0

/* <gdk/gdkkeysyms.h> */
#define GDK_Prev_Virtual_Screen 0xFED1

/* <gdk/gdkkeysyms.h> */
#define GDK_Next_Virtual_Screen 0xFED2

/* <gdk/gdkkeysyms.h> */
#define GDK_Last_Virtual_Screen 0xFED4

/* <gdk/gdkkeysyms.h> */
#define GDK_Terminate_Server 0xFED5

/* <gdk/gdkkeysyms.h> */
#define GDK_AccessX_Enable 0xFE70

/* <gdk/gdkkeysyms.h> */
#define GDK_AccessX_Feedback_Enable 0xFE71

/* <gdk/gdkkeysyms.h> */
#define GDK_RepeatKeys_Enable 0xFE72

/* <gdk/gdkkeysyms.h> */
#define GDK_SlowKeys_Enable 0xFE73

/* <gdk/gdkkeysyms.h> */
#define GDK_BounceKeys_Enable 0xFE74

/* <gdk/gdkkeysyms.h> */
#define GDK_StickyKeys_Enable 0xFE75

/* <gdk/gdkkeysyms.h> */
#define GDK_MouseKeys_Enable 0xFE76

/* <gdk/gdkkeysyms.h> */
#define GDK_MouseKeys_Accel_Enable 0xFE77

/* <gdk/gdkkeysyms.h> */
#define GDK_Overlay1_Enable 0xFE78

/* <gdk/gdkkeysyms.h> */
#define GDK_Overlay2_Enable 0xFE79

/* <gdk/gdkkeysyms.h> */
#define GDK_AudibleBell_Enable 0xFE7A

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Left 0xFEE0

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Right 0xFEE1

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Up 0xFEE2

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Down 0xFEE3

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_UpLeft 0xFEE4

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_UpRight 0xFEE5

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DownLeft 0xFEE6

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DownRight 0xFEE7

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button_Dflt 0xFEE8

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button1 0xFEE9

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button2 0xFEEA

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button3 0xFEEB

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button4 0xFEEC

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Button5 0xFEED

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick_Dflt 0xFEEE

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick1 0xFEEF

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick2 0xFEF0

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick3 0xFEF1

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick4 0xFEF2

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DblClick5 0xFEF3

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag_Dflt 0xFEF4

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag1 0xFEF5

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag2 0xFEF6

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag3 0xFEF7

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag4 0xFEF8

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Drag5 0xFEFD

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_EnableKeys 0xFEF9

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_Accelerate 0xFEFA

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DfltBtnNext 0xFEFB

/* <gdk/gdkkeysyms.h> */
#define GDK_Pointer_DfltBtnPrev 0xFEFC

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Duplicate 0xFD01

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_FieldMark 0xFD02

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Right2 0xFD03

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Left2 0xFD04

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_BackTab 0xFD05

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_EraseEOF 0xFD06

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_EraseInput 0xFD07

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Reset 0xFD08

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Quit 0xFD09

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_PA1 0xFD0A

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_PA2 0xFD0B

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_PA3 0xFD0C

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Test 0xFD0D

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Attn 0xFD0E

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_CursorBlink 0xFD0F

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_AltCursor 0xFD10

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_KeyClick 0xFD11

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Jump 0xFD12

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Ident 0xFD13

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Rule 0xFD14

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Copy 0xFD15

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Play 0xFD16

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Setup 0xFD17

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Record 0xFD18

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_ChangeScreen 0xFD19

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_DeleteWord 0xFD1A

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_ExSelect 0xFD1B

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_CursorSelect 0xFD1C

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_PrintScreen 0xFD1D

/* <gdk/gdkkeysyms.h> */
#define GDK_3270_Enter 0xFD1E

/* <gdk/gdkkeysyms.h> */
#define GDK_space 0x020

/* <gdk/gdkkeysyms.h> */
#define GDK_exclam 0x021

/* <gdk/gdkkeysyms.h> */
#define GDK_quotedbl 0x022

/* <gdk/gdkkeysyms.h> */
#define GDK_numbersign 0x023

/* <gdk/gdkkeysyms.h> */
#define GDK_dollar 0x024

/* <gdk/gdkkeysyms.h> */
#define GDK_percent 0x025

/* <gdk/gdkkeysyms.h> */
#define GDK_ampersand 0x026

/* <gdk/gdkkeysyms.h> */
#define GDK_apostrophe 0x027

/* <gdk/gdkkeysyms.h> */
#define GDK_quoteright 0x027

/* <gdk/gdkkeysyms.h> */
#define GDK_parenleft 0x028

/* <gdk/gdkkeysyms.h> */
#define GDK_parenright 0x029

/* <gdk/gdkkeysyms.h> */
#define GDK_asterisk 0x02a

/* <gdk/gdkkeysyms.h> */
#define GDK_plus 0x02b

/* <gdk/gdkkeysyms.h> */
#define GDK_comma 0x02c

/* <gdk/gdkkeysyms.h> */
#define GDK_minus 0x02d

/* <gdk/gdkkeysyms.h> */
#define GDK_period 0x02e

/* <gdk/gdkkeysyms.h> */
#define GDK_slash 0x02f

/* <gdk/gdkkeysyms.h> */
#define GDK_0 0x030

/* <gdk/gdkkeysyms.h> */
#define GDK_1 0x031

/* <gdk/gdkkeysyms.h> */
#define GDK_2 0x032

/* <gdk/gdkkeysyms.h> */
#define GDK_3 0x033

/* <gdk/gdkkeysyms.h> */
#define GDK_4 0x034

/* <gdk/gdkkeysyms.h> */
#define GDK_5 0x035

/* <gdk/gdkkeysyms.h> */
#define GDK_6 0x036

/* <gdk/gdkkeysyms.h> */
#define GDK_7 0x037

/* <gdk/gdkkeysyms.h> */
#define GDK_8 0x038

/* <gdk/gdkkeysyms.h> */
#define GDK_9 0x039

/* <gdk/gdkkeysyms.h> */
#define GDK_colon 0x03a

/* <gdk/gdkkeysyms.h> */
#define GDK_semicolon 0x03b

/* <gdk/gdkkeysyms.h> */
#define GDK_less 0x03c

/* <gdk/gdkkeysyms.h> */
#define GDK_equal 0x03d

/* <gdk/gdkkeysyms.h> */
#define GDK_greater 0x03e

/* <gdk/gdkkeysyms.h> */
#define GDK_question 0x03f

/* <gdk/gdkkeysyms.h> */
#define GDK_at 0x040

/* <gdk/gdkkeysyms.h> */
#define GDK_A 0x041

/* <gdk/gdkkeysyms.h> */
#define GDK_B 0x042

/* <gdk/gdkkeysyms.h> */
#define GDK_C 0x043

/* <gdk/gdkkeysyms.h> */
#define GDK_D 0x044

/* <gdk/gdkkeysyms.h> */
#define GDK_E 0x045

/* <gdk/gdkkeysyms.h> */
#define GDK_F 0x046

/* <gdk/gdkkeysyms.h> */
#define GDK_G 0x047

/* <gdk/gdkkeysyms.h> */
#define GDK_H 0x048

/* <gdk/gdkkeysyms.h> */
#define GDK_I 0x049

/* <gdk/gdkkeysyms.h> */
#define GDK_J 0x04a

/* <gdk/gdkkeysyms.h> */
#define GDK_K 0x04b

/* <gdk/gdkkeysyms.h> */
#define GDK_L 0x04c

/* <gdk/gdkkeysyms.h> */
#define GDK_M 0x04d

/* <gdk/gdkkeysyms.h> */
#define GDK_N 0x04e

/* <gdk/gdkkeysyms.h> */
#define GDK_O 0x04f

/* <gdk/gdkkeysyms.h> */
#define GDK_P 0x050

/* <gdk/gdkkeysyms.h> */
#define GDK_Q 0x051

/* <gdk/gdkkeysyms.h> */
#define GDK_R 0x052

/* <gdk/gdkkeysyms.h> */
#define GDK_S 0x053

/* <gdk/gdkkeysyms.h> */
#define GDK_T 0x054

/* <gdk/gdkkeysyms.h> */
#define GDK_U 0x055

/* <gdk/gdkkeysyms.h> */
#define GDK_V 0x056

/* <gdk/gdkkeysyms.h> */
#define GDK_W 0x057

/* <gdk/gdkkeysyms.h> */
#define GDK_X 0x058

/* <gdk/gdkkeysyms.h> */
#define GDK_Y 0x059

/* <gdk/gdkkeysyms.h> */
#define GDK_Z 0x05a

/* <gdk/gdkkeysyms.h> */
#define GDK_bracketleft 0x05b

/* <gdk/gdkkeysyms.h> */
#define GDK_backslash 0x05c

/* <gdk/gdkkeysyms.h> */
#define GDK_bracketright 0x05d

/* <gdk/gdkkeysyms.h> */
#define GDK_asciicircum 0x05e

/* <gdk/gdkkeysyms.h> */
#define GDK_underscore 0x05f

/* <gdk/gdkkeysyms.h> */
#define GDK_grave 0x060

/* <gdk/gdkkeysyms.h> */
#define GDK_quoteleft 0x060

/* <gdk/gdkkeysyms.h> */
#define GDK_a 0x061

/* <gdk/gdkkeysyms.h> */
#define GDK_b 0x062

/* <gdk/gdkkeysyms.h> */
#define GDK_c 0x063

/* <gdk/gdkkeysyms.h> */
#define GDK_d 0x064

/* <gdk/gdkkeysyms.h> */
#define GDK_e 0x065

/* <gdk/gdkkeysyms.h> */
#define GDK_f 0x066

/* <gdk/gdkkeysyms.h> */
#define GDK_g 0x067

/* <gdk/gdkkeysyms.h> */
#define GDK_h 0x068

/* <gdk/gdkkeysyms.h> */
#define GDK_i 0x069

/* <gdk/gdkkeysyms.h> */
#define GDK_j 0x06a

/* <gdk/gdkkeysyms.h> */
#define GDK_k 0x06b

/* <gdk/gdkkeysyms.h> */
#define GDK_l 0x06c

/* <gdk/gdkkeysyms.h> */
#define GDK_m 0x06d

/* <gdk/gdkkeysyms.h> */
#define GDK_n 0x06e

/* <gdk/gdkkeysyms.h> */
#define GDK_o 0x06f

/* <gdk/gdkkeysyms.h> */
#define GDK_p 0x070

/* <gdk/gdkkeysyms.h> */
#define GDK_q 0x071

/* <gdk/gdkkeysyms.h> */
#define GDK_r 0x072

/* <gdk/gdkkeysyms.h> */
#define GDK_s 0x073

/* <gdk/gdkkeysyms.h> */
#define GDK_t 0x074

/* <gdk/gdkkeysyms.h> */
#define GDK_u 0x075

/* <gdk/gdkkeysyms.h> */
#define GDK_v 0x076

/* <gdk/gdkkeysyms.h> */
#define GDK_w 0x077

/* <gdk/gdkkeysyms.h> */
#define GDK_x 0x078

/* <gdk/gdkkeysyms.h> */
#define GDK_y 0x079

/* <gdk/gdkkeysyms.h> */
#define GDK_z 0x07a

/* <gdk/gdkkeysyms.h> */
#define GDK_braceleft 0x07b

/* <gdk/gdkkeysyms.h> */
#define GDK_bar 0x07c

/* <gdk/gdkkeysyms.h> */
#define GDK_braceright 0x07d

/* <gdk/gdkkeysyms.h> */
#define GDK_asciitilde 0x07e

/* <gdk/gdkkeysyms.h> */
#define GDK_nobreakspace 0x0a0

/* <gdk/gdkkeysyms.h> */
#define GDK_exclamdown 0x0a1

/* <gdk/gdkkeysyms.h> */
#define GDK_cent 0x0a2

/* <gdk/gdkkeysyms.h> */
#define GDK_sterling 0x0a3

/* <gdk/gdkkeysyms.h> */
#define GDK_currency 0x0a4

/* <gdk/gdkkeysyms.h> */
#define GDK_yen 0x0a5

/* <gdk/gdkkeysyms.h> */
#define GDK_brokenbar 0x0a6

/* <gdk/gdkkeysyms.h> */
#define GDK_section 0x0a7

/* <gdk/gdkkeysyms.h> */
#define GDK_diaeresis 0x0a8

/* <gdk/gdkkeysyms.h> */
#define GDK_copyright 0x0a9

/* <gdk/gdkkeysyms.h> */
#define GDK_ordfeminine 0x0aa

/* <gdk/gdkkeysyms.h> */
#define GDK_guillemotleft 0x0ab

/* <gdk/gdkkeysyms.h> */
#define GDK_notsign 0x0ac

/* <gdk/gdkkeysyms.h> */
#define GDK_hyphen 0x0ad

/* <gdk/gdkkeysyms.h> */
#define GDK_registered 0x0ae

/* <gdk/gdkkeysyms.h> */
#define GDK_macron 0x0af

/* <gdk/gdkkeysyms.h> */
#define GDK_degree 0x0b0

/* <gdk/gdkkeysyms.h> */
#define GDK_plusminus 0x0b1

/* <gdk/gdkkeysyms.h> */
#define GDK_twosuperior 0x0b2

/* <gdk/gdkkeysyms.h> */
#define GDK_threesuperior 0x0b3

/* <gdk/gdkkeysyms.h> */
#define GDK_acute 0x0b4

/* <gdk/gdkkeysyms.h> */
#define GDK_mu 0x0b5

/* <gdk/gdkkeysyms.h> */
#define GDK_paragraph 0x0b6

/* <gdk/gdkkeysyms.h> */
#define GDK_periodcentered 0x0b7

/* <gdk/gdkkeysyms.h> */
#define GDK_cedilla 0x0b8

/* <gdk/gdkkeysyms.h> */
#define GDK_onesuperior 0x0b9

/* <gdk/gdkkeysyms.h> */
#define GDK_masculine 0x0ba

/* <gdk/gdkkeysyms.h> */
#define GDK_guillemotright 0x0bb

/* <gdk/gdkkeysyms.h> */
#define GDK_onequarter 0x0bc

/* <gdk/gdkkeysyms.h> */
#define GDK_onehalf 0x0bd

/* <gdk/gdkkeysyms.h> */
#define GDK_threequarters 0x0be

/* <gdk/gdkkeysyms.h> */
#define GDK_questiondown 0x0bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Agrave 0x0c0

/* <gdk/gdkkeysyms.h> */
#define GDK_Aacute 0x0c1

/* <gdk/gdkkeysyms.h> */
#define GDK_Acircumflex 0x0c2

/* <gdk/gdkkeysyms.h> */
#define GDK_Atilde 0x0c3

/* <gdk/gdkkeysyms.h> */
#define GDK_Adiaeresis 0x0c4

/* <gdk/gdkkeysyms.h> */
#define GDK_Aring 0x0c5

/* <gdk/gdkkeysyms.h> */
#define GDK_AE 0x0c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ccedilla 0x0c7

/* <gdk/gdkkeysyms.h> */
#define GDK_Egrave 0x0c8

/* <gdk/gdkkeysyms.h> */
#define GDK_Eacute 0x0c9

/* <gdk/gdkkeysyms.h> */
#define GDK_Ecircumflex 0x0ca

/* <gdk/gdkkeysyms.h> */
#define GDK_Ediaeresis 0x0cb

/* <gdk/gdkkeysyms.h> */
#define GDK_Igrave 0x0cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Iacute 0x0cd

/* <gdk/gdkkeysyms.h> */
#define GDK_Icircumflex 0x0ce

/* <gdk/gdkkeysyms.h> */
#define GDK_Idiaeresis 0x0cf

/* <gdk/gdkkeysyms.h> */
#define GDK_ETH 0x0d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Eth 0x0d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Ntilde 0x0d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Ograve 0x0d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Oacute 0x0d3

/* <gdk/gdkkeysyms.h> */
#define GDK_Ocircumflex 0x0d4

/* <gdk/gdkkeysyms.h> */
#define GDK_Otilde 0x0d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Odiaeresis 0x0d6

/* <gdk/gdkkeysyms.h> */
#define GDK_multiply 0x0d7

/* <gdk/gdkkeysyms.h> */
#define GDK_Ooblique 0x0d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Ugrave 0x0d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Uacute 0x0da

/* <gdk/gdkkeysyms.h> */
#define GDK_Ucircumflex 0x0db

/* <gdk/gdkkeysyms.h> */
#define GDK_Udiaeresis 0x0dc

/* <gdk/gdkkeysyms.h> */
#define GDK_Yacute 0x0dd

/* <gdk/gdkkeysyms.h> */
#define GDK_THORN 0x0de

/* <gdk/gdkkeysyms.h> */
#define GDK_Thorn 0x0de

/* <gdk/gdkkeysyms.h> */
#define GDK_ssharp 0x0df

/* <gdk/gdkkeysyms.h> */
#define GDK_agrave 0x0e0

/* <gdk/gdkkeysyms.h> */
#define GDK_aacute 0x0e1

/* <gdk/gdkkeysyms.h> */
#define GDK_acircumflex 0x0e2

/* <gdk/gdkkeysyms.h> */
#define GDK_atilde 0x0e3

/* <gdk/gdkkeysyms.h> */
#define GDK_adiaeresis 0x0e4

/* <gdk/gdkkeysyms.h> */
#define GDK_aring 0x0e5

/* <gdk/gdkkeysyms.h> */
#define GDK_ae 0x0e6

/* <gdk/gdkkeysyms.h> */
#define GDK_ccedilla 0x0e7

/* <gdk/gdkkeysyms.h> */
#define GDK_egrave 0x0e8

/* <gdk/gdkkeysyms.h> */
#define GDK_eacute 0x0e9

/* <gdk/gdkkeysyms.h> */
#define GDK_ecircumflex 0x0ea

/* <gdk/gdkkeysyms.h> */
#define GDK_ediaeresis 0x0eb

/* <gdk/gdkkeysyms.h> */
#define GDK_igrave 0x0ec

/* <gdk/gdkkeysyms.h> */
#define GDK_iacute 0x0ed

/* <gdk/gdkkeysyms.h> */
#define GDK_icircumflex 0x0ee

/* <gdk/gdkkeysyms.h> */
#define GDK_idiaeresis 0x0ef

/* <gdk/gdkkeysyms.h> */
#define GDK_eth 0x0f0

/* <gdk/gdkkeysyms.h> */
#define GDK_ntilde 0x0f1

/* <gdk/gdkkeysyms.h> */
#define GDK_ograve 0x0f2

/* <gdk/gdkkeysyms.h> */
#define GDK_oacute 0x0f3

/* <gdk/gdkkeysyms.h> */
#define GDK_ocircumflex 0x0f4

/* <gdk/gdkkeysyms.h> */
#define GDK_otilde 0x0f5

/* <gdk/gdkkeysyms.h> */
#define GDK_odiaeresis 0x0f6

/* <gdk/gdkkeysyms.h> */
#define GDK_division 0x0f7

/* <gdk/gdkkeysyms.h> */
#define GDK_oslash 0x0f8

/* <gdk/gdkkeysyms.h> */
#define GDK_ugrave 0x0f9

/* <gdk/gdkkeysyms.h> */
#define GDK_uacute 0x0fa

/* <gdk/gdkkeysyms.h> */
#define GDK_ucircumflex 0x0fb

/* <gdk/gdkkeysyms.h> */
#define GDK_udiaeresis 0x0fc

/* <gdk/gdkkeysyms.h> */
#define GDK_yacute 0x0fd

/* <gdk/gdkkeysyms.h> */
#define GDK_thorn 0x0fe

/* <gdk/gdkkeysyms.h> */
#define GDK_ydiaeresis 0x0ff

/* <gdk/gdkkeysyms.h> */
#define GDK_Aogonek 0x1a1

/* <gdk/gdkkeysyms.h> */
#define GDK_breve 0x1a2

/* <gdk/gdkkeysyms.h> */
#define GDK_Lstroke 0x1a3

/* <gdk/gdkkeysyms.h> */
#define GDK_Lcaron 0x1a5

/* <gdk/gdkkeysyms.h> */
#define GDK_Sacute 0x1a6

/* <gdk/gdkkeysyms.h> */
#define GDK_Scaron 0x1a9

/* <gdk/gdkkeysyms.h> */
#define GDK_Scedilla 0x1aa

/* <gdk/gdkkeysyms.h> */
#define GDK_Tcaron 0x1ab

/* <gdk/gdkkeysyms.h> */
#define GDK_Zacute 0x1ac

/* <gdk/gdkkeysyms.h> */
#define GDK_Zcaron 0x1ae

/* <gdk/gdkkeysyms.h> */
#define GDK_Zabovedot 0x1af

/* <gdk/gdkkeysyms.h> */
#define GDK_aogonek 0x1b1

/* <gdk/gdkkeysyms.h> */
#define GDK_ogonek 0x1b2

/* <gdk/gdkkeysyms.h> */
#define GDK_lstroke 0x1b3

/* <gdk/gdkkeysyms.h> */
#define GDK_lcaron 0x1b5

/* <gdk/gdkkeysyms.h> */
#define GDK_sacute 0x1b6

/* <gdk/gdkkeysyms.h> */
#define GDK_caron 0x1b7

/* <gdk/gdkkeysyms.h> */
#define GDK_scaron 0x1b9

/* <gdk/gdkkeysyms.h> */
#define GDK_scedilla 0x1ba

/* <gdk/gdkkeysyms.h> */
#define GDK_tcaron 0x1bb

/* <gdk/gdkkeysyms.h> */
#define GDK_zacute 0x1bc

/* <gdk/gdkkeysyms.h> */
#define GDK_doubleacute 0x1bd

/* <gdk/gdkkeysyms.h> */
#define GDK_zcaron 0x1be

/* <gdk/gdkkeysyms.h> */
#define GDK_zabovedot 0x1bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Racute 0x1c0

/* <gdk/gdkkeysyms.h> */
#define GDK_Abreve 0x1c3

/* <gdk/gdkkeysyms.h> */
#define GDK_Lacute 0x1c5

/* <gdk/gdkkeysyms.h> */
#define GDK_Cacute 0x1c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ccaron 0x1c8

/* <gdk/gdkkeysyms.h> */
#define GDK_Eogonek 0x1ca

/* <gdk/gdkkeysyms.h> */
#define GDK_Ecaron 0x1cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Dcaron 0x1cf

/* <gdk/gdkkeysyms.h> */
#define GDK_Dstroke 0x1d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Nacute 0x1d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Ncaron 0x1d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Odoubleacute 0x1d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Rcaron 0x1d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Uring 0x1d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Udoubleacute 0x1db

/* <gdk/gdkkeysyms.h> */
#define GDK_Tcedilla 0x1de

/* <gdk/gdkkeysyms.h> */
#define GDK_racute 0x1e0

/* <gdk/gdkkeysyms.h> */
#define GDK_abreve 0x1e3

/* <gdk/gdkkeysyms.h> */
#define GDK_lacute 0x1e5

/* <gdk/gdkkeysyms.h> */
#define GDK_cacute 0x1e6

/* <gdk/gdkkeysyms.h> */
#define GDK_ccaron 0x1e8

/* <gdk/gdkkeysyms.h> */
#define GDK_eogonek 0x1ea

/* <gdk/gdkkeysyms.h> */
#define GDK_ecaron 0x1ec

/* <gdk/gdkkeysyms.h> */
#define GDK_dcaron 0x1ef

/* <gdk/gdkkeysyms.h> */
#define GDK_dstroke 0x1f0

/* <gdk/gdkkeysyms.h> */
#define GDK_nacute 0x1f1

/* <gdk/gdkkeysyms.h> */
#define GDK_ncaron 0x1f2

/* <gdk/gdkkeysyms.h> */
#define GDK_odoubleacute 0x1f5

/* <gdk/gdkkeysyms.h> */
#define GDK_udoubleacute 0x1fb

/* <gdk/gdkkeysyms.h> */
#define GDK_rcaron 0x1f8

/* <gdk/gdkkeysyms.h> */
#define GDK_uring 0x1f9

/* <gdk/gdkkeysyms.h> */
#define GDK_tcedilla 0x1fe

/* <gdk/gdkkeysyms.h> */
#define GDK_abovedot 0x1ff

/* <gdk/gdkkeysyms.h> */
#define GDK_Hstroke 0x2a1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hcircumflex 0x2a6

/* <gdk/gdkkeysyms.h> */
#define GDK_Iabovedot 0x2a9

/* <gdk/gdkkeysyms.h> */
#define GDK_Gbreve 0x2ab

/* <gdk/gdkkeysyms.h> */
#define GDK_Jcircumflex 0x2ac

/* <gdk/gdkkeysyms.h> */
#define GDK_hstroke 0x2b1

/* <gdk/gdkkeysyms.h> */
#define GDK_hcircumflex 0x2b6

/* <gdk/gdkkeysyms.h> */
#define GDK_idotless 0x2b9

/* <gdk/gdkkeysyms.h> */
#define GDK_gbreve 0x2bb

/* <gdk/gdkkeysyms.h> */
#define GDK_jcircumflex 0x2bc

/* <gdk/gdkkeysyms.h> */
#define GDK_Cabovedot 0x2c5

/* <gdk/gdkkeysyms.h> */
#define GDK_Ccircumflex 0x2c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Gabovedot 0x2d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Gcircumflex 0x2d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Ubreve 0x2dd

/* <gdk/gdkkeysyms.h> */
#define GDK_Scircumflex 0x2de

/* <gdk/gdkkeysyms.h> */
#define GDK_cabovedot 0x2e5

/* <gdk/gdkkeysyms.h> */
#define GDK_ccircumflex 0x2e6

/* <gdk/gdkkeysyms.h> */
#define GDK_gabovedot 0x2f5

/* <gdk/gdkkeysyms.h> */
#define GDK_gcircumflex 0x2f8

/* <gdk/gdkkeysyms.h> */
#define GDK_ubreve 0x2fd

/* <gdk/gdkkeysyms.h> */
#define GDK_scircumflex 0x2fe

/* <gdk/gdkkeysyms.h> */
#define GDK_kra 0x3a2

/* <gdk/gdkkeysyms.h> */
#define GDK_kappa 0x3a2

/* <gdk/gdkkeysyms.h> */
#define GDK_Rcedilla 0x3a3

/* <gdk/gdkkeysyms.h> */
#define GDK_Itilde 0x3a5

/* <gdk/gdkkeysyms.h> */
#define GDK_Lcedilla 0x3a6

/* <gdk/gdkkeysyms.h> */
#define GDK_Emacron 0x3aa

/* <gdk/gdkkeysyms.h> */
#define GDK_Gcedilla 0x3ab

/* <gdk/gdkkeysyms.h> */
#define GDK_Tslash 0x3ac

/* <gdk/gdkkeysyms.h> */
#define GDK_rcedilla 0x3b3

/* <gdk/gdkkeysyms.h> */
#define GDK_itilde 0x3b5

/* <gdk/gdkkeysyms.h> */
#define GDK_lcedilla 0x3b6

/* <gdk/gdkkeysyms.h> */
#define GDK_emacron 0x3ba

/* <gdk/gdkkeysyms.h> */
#define GDK_gcedilla 0x3bb

/* <gdk/gdkkeysyms.h> */
#define GDK_tslash 0x3bc

/* <gdk/gdkkeysyms.h> */
#define GDK_ENG 0x3bd

/* <gdk/gdkkeysyms.h> */
#define GDK_eng 0x3bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Amacron 0x3c0

/* <gdk/gdkkeysyms.h> */
#define GDK_Iogonek 0x3c7

/* <gdk/gdkkeysyms.h> */
#define GDK_Eabovedot 0x3cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Imacron 0x3cf

/* <gdk/gdkkeysyms.h> */
#define GDK_Ncedilla 0x3d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Omacron 0x3d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Kcedilla 0x3d3

/* <gdk/gdkkeysyms.h> */
#define GDK_Uogonek 0x3d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Utilde 0x3dd

/* <gdk/gdkkeysyms.h> */
#define GDK_Umacron 0x3de

/* <gdk/gdkkeysyms.h> */
#define GDK_amacron 0x3e0

/* <gdk/gdkkeysyms.h> */
#define GDK_iogonek 0x3e7

/* <gdk/gdkkeysyms.h> */
#define GDK_eabovedot 0x3ec

/* <gdk/gdkkeysyms.h> */
#define GDK_imacron 0x3ef

/* <gdk/gdkkeysyms.h> */
#define GDK_ncedilla 0x3f1

/* <gdk/gdkkeysyms.h> */
#define GDK_omacron 0x3f2

/* <gdk/gdkkeysyms.h> */
#define GDK_kcedilla 0x3f3

/* <gdk/gdkkeysyms.h> */
#define GDK_uogonek 0x3f9

/* <gdk/gdkkeysyms.h> */
#define GDK_utilde 0x3fd

/* <gdk/gdkkeysyms.h> */
#define GDK_umacron 0x3fe

/* <gdk/gdkkeysyms.h> */
#define GDK_overline 0x47e

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_fullstop 0x4a1

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_openingbracket 0x4a2

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_closingbracket 0x4a3

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_comma 0x4a4

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_conjunctive 0x4a5

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_middledot 0x4a5

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_WO 0x4a6

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_a 0x4a7

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_i 0x4a8

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_u 0x4a9

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_e 0x4aa

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_o 0x4ab

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_ya 0x4ac

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_yu 0x4ad

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_yo 0x4ae

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_tsu 0x4af

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_tu 0x4af

/* <gdk/gdkkeysyms.h> */
#define GDK_prolongedsound 0x4b0

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_A 0x4b1

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_I 0x4b2

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_U 0x4b3

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_E 0x4b4

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_O 0x4b5

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_KA 0x4b6

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_KI 0x4b7

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_KU 0x4b8

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_KE 0x4b9

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_KO 0x4ba

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_SA 0x4bb

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_SHI 0x4bc

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_SU 0x4bd

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_SE 0x4be

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_SO 0x4bf

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TA 0x4c0

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_CHI 0x4c1

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TI 0x4c1

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TSU 0x4c2

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TU 0x4c2

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TE 0x4c3

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_TO 0x4c4

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_NA 0x4c5

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_NI 0x4c6

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_NU 0x4c7

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_NE 0x4c8

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_NO 0x4c9

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_HA 0x4ca

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_HI 0x4cb

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_FU 0x4cc

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_HU 0x4cc

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_HE 0x4cd

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_HO 0x4ce

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_MA 0x4cf

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_MI 0x4d0

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_MU 0x4d1

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_ME 0x4d2

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_MO 0x4d3

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_YA 0x4d4

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_YU 0x4d5

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_YO 0x4d6

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_RA 0x4d7

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_RI 0x4d8

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_RU 0x4d9

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_RE 0x4da

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_RO 0x4db

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_WA 0x4dc

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_N 0x4dd

/* <gdk/gdkkeysyms.h> */
#define GDK_voicedsound 0x4de

/* <gdk/gdkkeysyms.h> */
#define GDK_semivoicedsound 0x4df

/* <gdk/gdkkeysyms.h> */
#define GDK_kana_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_comma 0x5ac

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_semicolon 0x5bb

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_question_mark 0x5bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hamza 0x5c1

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_maddaonalef 0x5c2

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hamzaonalef 0x5c3

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hamzaonwaw 0x5c4

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hamzaunderalef 0x5c5

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hamzaonyeh 0x5c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_alef 0x5c7

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_beh 0x5c8

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_tehmarbuta 0x5c9

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_teh 0x5ca

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_theh 0x5cb

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_jeem 0x5cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_hah 0x5cd

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_khah 0x5ce

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_dal 0x5cf

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_thal 0x5d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_ra 0x5d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_zain 0x5d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_seen 0x5d3

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_sheen 0x5d4

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_sad 0x5d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_dad 0x5d6

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_tah 0x5d7

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_zah 0x5d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_ain 0x5d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_ghain 0x5da

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_tatweel 0x5e0

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_feh 0x5e1

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_qaf 0x5e2

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_kaf 0x5e3

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_lam 0x5e4

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_meem 0x5e5

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_noon 0x5e6

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_ha 0x5e7

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_heh 0x5e7

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_waw 0x5e8

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_alefmaksura 0x5e9

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_yeh 0x5ea

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_fathatan 0x5eb

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_dammatan 0x5ec

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_kasratan 0x5ed

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_fatha 0x5ee

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_damma 0x5ef

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_kasra 0x5f0

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_shadda 0x5f1

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_sukun 0x5f2

/* <gdk/gdkkeysyms.h> */
#define GDK_Arabic_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_dje 0x6a1

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_gje 0x6a2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_io 0x6a3

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_ie 0x6a4

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_je 0x6a4

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_dse 0x6a5

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_i 0x6a6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_i 0x6a6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_yi 0x6a7

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_yi 0x6a7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_je 0x6a8

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_je 0x6a8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_lje 0x6a9

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_lje 0x6a9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_nje 0x6aa

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_nje 0x6aa

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_tshe 0x6ab

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_kje 0x6ac

/* <gdk/gdkkeysyms.h> */
#define GDK_Byelorussian_shortu 0x6ae

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_dzhe 0x6af

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_dze 0x6af

/* <gdk/gdkkeysyms.h> */
#define GDK_numerosign 0x6b0

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_DJE 0x6b1

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_GJE 0x6b2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_IO 0x6b3

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_IE 0x6b4

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_JE 0x6b4

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_DSE 0x6b5

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_I 0x6b6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_I 0x6b6

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukrainian_YI 0x6b7

/* <gdk/gdkkeysyms.h> */
#define GDK_Ukranian_YI 0x6b7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_JE 0x6b8

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_JE 0x6b8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_LJE 0x6b9

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_LJE 0x6b9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_NJE 0x6ba

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_NJE 0x6ba

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_TSHE 0x6bb

/* <gdk/gdkkeysyms.h> */
#define GDK_Macedonia_KJE 0x6bc

/* <gdk/gdkkeysyms.h> */
#define GDK_Byelorussian_SHORTU 0x6be

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_DZHE 0x6bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Serbian_DZE 0x6bf

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_yu 0x6c0

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_a 0x6c1

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_be 0x6c2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_tse 0x6c3

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_de 0x6c4

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ie 0x6c5

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ef 0x6c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ghe 0x6c7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ha 0x6c8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_i 0x6c9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_shorti 0x6ca

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ka 0x6cb

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_el 0x6cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_em 0x6cd

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_en 0x6ce

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_o 0x6cf

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_pe 0x6d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ya 0x6d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_er 0x6d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_es 0x6d3

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_te 0x6d4

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_u 0x6d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_zhe 0x6d6

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ve 0x6d7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_softsign 0x6d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_yeru 0x6d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ze 0x6da

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_sha 0x6db

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_e 0x6dc

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_shcha 0x6dd

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_che 0x6de

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_hardsign 0x6df

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_YU 0x6e0

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_A 0x6e1

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_BE 0x6e2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_TSE 0x6e3

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_DE 0x6e4

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_IE 0x6e5

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_EF 0x6e6

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_GHE 0x6e7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_HA 0x6e8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_I 0x6e9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_SHORTI 0x6ea

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_KA 0x6eb

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_EL 0x6ec

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_EM 0x6ed

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_EN 0x6ee

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_O 0x6ef

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_PE 0x6f0

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_YA 0x6f1

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ER 0x6f2

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ES 0x6f3

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_TE 0x6f4

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_U 0x6f5

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ZHE 0x6f6

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_VE 0x6f7

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_SOFTSIGN 0x6f8

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_YERU 0x6f9

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_ZE 0x6fa

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_SHA 0x6fb

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_E 0x6fc

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_SHCHA 0x6fd

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_CHE 0x6fe

/* <gdk/gdkkeysyms.h> */
#define GDK_Cyrillic_HARDSIGN 0x6ff

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_ALPHAaccent 0x7a1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_EPSILONaccent 0x7a2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_ETAaccent 0x7a3

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_IOTAaccent 0x7a4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_IOTAdiaeresis 0x7a5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_OMICRONaccent 0x7a7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_UPSILONaccent 0x7a8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_UPSILONdieresis 0x7a9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_OMEGAaccent 0x7ab

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_accentdieresis 0x7ae

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_horizbar 0x7af

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_alphaaccent 0x7b1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_epsilonaccent 0x7b2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_etaaccent 0x7b3

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_iotaaccent 0x7b4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_iotadieresis 0x7b5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_iotaaccentdieresis 0x7b6

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_omicronaccent 0x7b7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_upsilonaccent 0x7b8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_upsilondieresis 0x7b9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_upsilonaccentdieresis 0x7ba

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_omegaaccent 0x7bb

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_ALPHA 0x7c1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_BETA 0x7c2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_GAMMA 0x7c3

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_DELTA 0x7c4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_EPSILON 0x7c5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_ZETA 0x7c6

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_ETA 0x7c7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_THETA 0x7c8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_IOTA 0x7c9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_KAPPA 0x7ca

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_LAMDA 0x7cb

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_LAMBDA 0x7cb

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_MU 0x7cc

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_NU 0x7cd

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_XI 0x7ce

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_OMICRON 0x7cf

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_PI 0x7d0

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_RHO 0x7d1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_SIGMA 0x7d2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_TAU 0x7d4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_UPSILON 0x7d5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_PHI 0x7d6

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_CHI 0x7d7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_PSI 0x7d8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_OMEGA 0x7d9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_alpha 0x7e1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_beta 0x7e2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_gamma 0x7e3

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_delta 0x7e4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_epsilon 0x7e5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_zeta 0x7e6

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_eta 0x7e7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_theta 0x7e8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_iota 0x7e9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_kappa 0x7ea

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_lamda 0x7eb

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_lambda 0x7eb

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_mu 0x7ec

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_nu 0x7ed

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_xi 0x7ee

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_omicron 0x7ef

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_pi 0x7f0

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_rho 0x7f1

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_sigma 0x7f2

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_finalsmallsigma 0x7f3

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_tau 0x7f4

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_upsilon 0x7f5

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_phi 0x7f6

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_chi 0x7f7

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_psi 0x7f8

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_omega 0x7f9

/* <gdk/gdkkeysyms.h> */
#define GDK_Greek_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_leftradical 0x8a1

/* <gdk/gdkkeysyms.h> */
#define GDK_topleftradical 0x8a2

/* <gdk/gdkkeysyms.h> */
#define GDK_horizconnector 0x8a3

/* <gdk/gdkkeysyms.h> */
#define GDK_topintegral 0x8a4

/* <gdk/gdkkeysyms.h> */
#define GDK_botintegral 0x8a5

/* <gdk/gdkkeysyms.h> */
#define GDK_vertconnector 0x8a6

/* <gdk/gdkkeysyms.h> */
#define GDK_topleftsqbracket 0x8a7

/* <gdk/gdkkeysyms.h> */
#define GDK_botleftsqbracket 0x8a8

/* <gdk/gdkkeysyms.h> */
#define GDK_toprightsqbracket 0x8a9

/* <gdk/gdkkeysyms.h> */
#define GDK_botrightsqbracket 0x8aa

/* <gdk/gdkkeysyms.h> */
#define GDK_topleftparens 0x8ab

/* <gdk/gdkkeysyms.h> */
#define GDK_botleftparens 0x8ac

/* <gdk/gdkkeysyms.h> */
#define GDK_toprightparens 0x8ad

/* <gdk/gdkkeysyms.h> */
#define GDK_botrightparens 0x8ae

/* <gdk/gdkkeysyms.h> */
#define GDK_leftmiddlecurlybrace 0x8af

/* <gdk/gdkkeysyms.h> */
#define GDK_rightmiddlecurlybrace 0x8b0

/* <gdk/gdkkeysyms.h> */
#define GDK_topleftsummation 0x8b1

/* <gdk/gdkkeysyms.h> */
#define GDK_botleftsummation 0x8b2

/* <gdk/gdkkeysyms.h> */
#define GDK_topvertsummationconnector 0x8b3

/* <gdk/gdkkeysyms.h> */
#define GDK_botvertsummationconnector 0x8b4

/* <gdk/gdkkeysyms.h> */
#define GDK_toprightsummation 0x8b5

/* <gdk/gdkkeysyms.h> */
#define GDK_botrightsummation 0x8b6

/* <gdk/gdkkeysyms.h> */
#define GDK_rightmiddlesummation 0x8b7

/* <gdk/gdkkeysyms.h> */
#define GDK_lessthanequal 0x8bc

/* <gdk/gdkkeysyms.h> */
#define GDK_notequal 0x8bd

/* <gdk/gdkkeysyms.h> */
#define GDK_greaterthanequal 0x8be

/* <gdk/gdkkeysyms.h> */
#define GDK_integral 0x8bf

/* <gdk/gdkkeysyms.h> */
#define GDK_therefore 0x8c0

/* <gdk/gdkkeysyms.h> */
#define GDK_variation 0x8c1

/* <gdk/gdkkeysyms.h> */
#define GDK_infinity 0x8c2

/* <gdk/gdkkeysyms.h> */
#define GDK_nabla 0x8c5

/* <gdk/gdkkeysyms.h> */
#define GDK_approximate 0x8c8

/* <gdk/gdkkeysyms.h> */
#define GDK_similarequal 0x8c9

/* <gdk/gdkkeysyms.h> */
#define GDK_ifonlyif 0x8cd

/* <gdk/gdkkeysyms.h> */
#define GDK_implies 0x8ce

/* <gdk/gdkkeysyms.h> */
#define GDK_identical 0x8cf

/* <gdk/gdkkeysyms.h> */
#define GDK_radical 0x8d6

/* <gdk/gdkkeysyms.h> */
#define GDK_includedin 0x8da

/* <gdk/gdkkeysyms.h> */
#define GDK_includes 0x8db

/* <gdk/gdkkeysyms.h> */
#define GDK_intersection 0x8dc

/* <gdk/gdkkeysyms.h> */
#define GDK_union 0x8dd

/* <gdk/gdkkeysyms.h> */
#define GDK_logicaland 0x8de

/* <gdk/gdkkeysyms.h> */
#define GDK_logicalor 0x8df

/* <gdk/gdkkeysyms.h> */
#define GDK_partialderivative 0x8ef

/* <gdk/gdkkeysyms.h> */
#define GDK_function 0x8f6

/* <gdk/gdkkeysyms.h> */
#define GDK_leftarrow 0x8fb

/* <gdk/gdkkeysyms.h> */
#define GDK_uparrow 0x8fc

/* <gdk/gdkkeysyms.h> */
#define GDK_rightarrow 0x8fd

/* <gdk/gdkkeysyms.h> */
#define GDK_downarrow 0x8fe

/* <gdk/gdkkeysyms.h> */
#define GDK_blank 0x9df

/* <gdk/gdkkeysyms.h> */
#define GDK_soliddiamond 0x9e0

/* <gdk/gdkkeysyms.h> */
#define GDK_checkerboard 0x9e1

/* <gdk/gdkkeysyms.h> */
#define GDK_ht 0x9e2

/* <gdk/gdkkeysyms.h> */
#define GDK_ff 0x9e3

/* <gdk/gdkkeysyms.h> */
#define GDK_cr 0x9e4

/* <gdk/gdkkeysyms.h> */
#define GDK_lf 0x9e5

/* <gdk/gdkkeysyms.h> */
#define GDK_nl 0x9e8

/* <gdk/gdkkeysyms.h> */
#define GDK_vt 0x9e9

/* <gdk/gdkkeysyms.h> */
#define GDK_lowrightcorner 0x9ea

/* <gdk/gdkkeysyms.h> */
#define GDK_uprightcorner 0x9eb

/* <gdk/gdkkeysyms.h> */
#define GDK_upleftcorner 0x9ec

/* <gdk/gdkkeysyms.h> */
#define GDK_lowleftcorner 0x9ed

/* <gdk/gdkkeysyms.h> */
#define GDK_crossinglines 0x9ee

/* <gdk/gdkkeysyms.h> */
#define GDK_horizlinescan1 0x9ef

/* <gdk/gdkkeysyms.h> */
#define GDK_horizlinescan3 0x9f0

/* <gdk/gdkkeysyms.h> */
#define GDK_horizlinescan5 0x9f1

/* <gdk/gdkkeysyms.h> */
#define GDK_horizlinescan7 0x9f2

/* <gdk/gdkkeysyms.h> */
#define GDK_horizlinescan9 0x9f3

/* <gdk/gdkkeysyms.h> */
#define GDK_leftt 0x9f4

/* <gdk/gdkkeysyms.h> */
#define GDK_rightt 0x9f5

/* <gdk/gdkkeysyms.h> */
#define GDK_bott 0x9f6

/* <gdk/gdkkeysyms.h> */
#define GDK_topt 0x9f7

/* <gdk/gdkkeysyms.h> */
#define GDK_vertbar 0x9f8

/* <gdk/gdkkeysyms.h> */
#define GDK_emspace 0xaa1

/* <gdk/gdkkeysyms.h> */
#define GDK_enspace 0xaa2

/* <gdk/gdkkeysyms.h> */
#define GDK_em3space 0xaa3

/* <gdk/gdkkeysyms.h> */
#define GDK_em4space 0xaa4

/* <gdk/gdkkeysyms.h> */
#define GDK_digitspace 0xaa5

/* <gdk/gdkkeysyms.h> */
#define GDK_punctspace 0xaa6

/* <gdk/gdkkeysyms.h> */
#define GDK_thinspace 0xaa7

/* <gdk/gdkkeysyms.h> */
#define GDK_hairspace 0xaa8

/* <gdk/gdkkeysyms.h> */
#define GDK_emdash 0xaa9

/* <gdk/gdkkeysyms.h> */
#define GDK_endash 0xaaa

/* <gdk/gdkkeysyms.h> */
#define GDK_signifblank 0xaac

/* <gdk/gdkkeysyms.h> */
#define GDK_ellipsis 0xaae

/* <gdk/gdkkeysyms.h> */
#define GDK_doubbaselinedot 0xaaf

/* <gdk/gdkkeysyms.h> */
#define GDK_onethird 0xab0

/* <gdk/gdkkeysyms.h> */
#define GDK_twothirds 0xab1

/* <gdk/gdkkeysyms.h> */
#define GDK_onefifth 0xab2

/* <gdk/gdkkeysyms.h> */
#define GDK_twofifths 0xab3

/* <gdk/gdkkeysyms.h> */
#define GDK_threefifths 0xab4

/* <gdk/gdkkeysyms.h> */
#define GDK_fourfifths 0xab5

/* <gdk/gdkkeysyms.h> */
#define GDK_onesixth 0xab6

/* <gdk/gdkkeysyms.h> */
#define GDK_fivesixths 0xab7

/* <gdk/gdkkeysyms.h> */
#define GDK_careof 0xab8

/* <gdk/gdkkeysyms.h> */
#define GDK_figdash 0xabb

/* <gdk/gdkkeysyms.h> */
#define GDK_leftanglebracket 0xabc

/* <gdk/gdkkeysyms.h> */
#define GDK_decimalpoint 0xabd

/* <gdk/gdkkeysyms.h> */
#define GDK_rightanglebracket 0xabe

/* <gdk/gdkkeysyms.h> */
#define GDK_marker 0xabf

/* <gdk/gdkkeysyms.h> */
#define GDK_oneeighth 0xac3

/* <gdk/gdkkeysyms.h> */
#define GDK_threeeighths 0xac4

/* <gdk/gdkkeysyms.h> */
#define GDK_fiveeighths 0xac5

/* <gdk/gdkkeysyms.h> */
#define GDK_seveneighths 0xac6

/* <gdk/gdkkeysyms.h> */
#define GDK_trademark 0xac9

/* <gdk/gdkkeysyms.h> */
#define GDK_signaturemark 0xaca

/* <gdk/gdkkeysyms.h> */
#define GDK_trademarkincircle 0xacb

/* <gdk/gdkkeysyms.h> */
#define GDK_leftopentriangle 0xacc

/* <gdk/gdkkeysyms.h> */
#define GDK_rightopentriangle 0xacd

/* <gdk/gdkkeysyms.h> */
#define GDK_emopencircle 0xace

/* <gdk/gdkkeysyms.h> */
#define GDK_emopenrectangle 0xacf

/* <gdk/gdkkeysyms.h> */
#define GDK_leftsinglequotemark 0xad0

/* <gdk/gdkkeysyms.h> */
#define GDK_rightsinglequotemark 0xad1

/* <gdk/gdkkeysyms.h> */
#define GDK_leftdoublequotemark 0xad2

/* <gdk/gdkkeysyms.h> */
#define GDK_rightdoublequotemark 0xad3

/* <gdk/gdkkeysyms.h> */
#define GDK_prescription 0xad4

/* <gdk/gdkkeysyms.h> */
#define GDK_minutes 0xad6

/* <gdk/gdkkeysyms.h> */
#define GDK_seconds 0xad7

/* <gdk/gdkkeysyms.h> */
#define GDK_latincross 0xad9

/* <gdk/gdkkeysyms.h> */
#define GDK_hexagram 0xada

/* <gdk/gdkkeysyms.h> */
#define GDK_filledrectbullet 0xadb

/* <gdk/gdkkeysyms.h> */
#define GDK_filledlefttribullet 0xadc

/* <gdk/gdkkeysyms.h> */
#define GDK_filledrighttribullet 0xadd

/* <gdk/gdkkeysyms.h> */
#define GDK_emfilledcircle 0xade

/* <gdk/gdkkeysyms.h> */
#define GDK_emfilledrect 0xadf

/* <gdk/gdkkeysyms.h> */
#define GDK_enopencircbullet 0xae0

/* <gdk/gdkkeysyms.h> */
#define GDK_enopensquarebullet 0xae1

/* <gdk/gdkkeysyms.h> */
#define GDK_openrectbullet 0xae2

/* <gdk/gdkkeysyms.h> */
#define GDK_opentribulletup 0xae3

/* <gdk/gdkkeysyms.h> */
#define GDK_opentribulletdown 0xae4

/* <gdk/gdkkeysyms.h> */
#define GDK_openstar 0xae5

/* <gdk/gdkkeysyms.h> */
#define GDK_enfilledcircbullet 0xae6

/* <gdk/gdkkeysyms.h> */
#define GDK_enfilledsqbullet 0xae7

/* <gdk/gdkkeysyms.h> */
#define GDK_filledtribulletup 0xae8

/* <gdk/gdkkeysyms.h> */
#define GDK_filledtribulletdown 0xae9

/* <gdk/gdkkeysyms.h> */
#define GDK_leftpointer 0xaea

/* <gdk/gdkkeysyms.h> */
#define GDK_rightpointer 0xaeb

/* <gdk/gdkkeysyms.h> */
#define GDK_club 0xaec

/* <gdk/gdkkeysyms.h> */
#define GDK_diamond 0xaed

/* <gdk/gdkkeysyms.h> */
#define GDK_heart 0xaee

/* <gdk/gdkkeysyms.h> */
#define GDK_maltesecross 0xaf0

/* <gdk/gdkkeysyms.h> */
#define GDK_dagger 0xaf1

/* <gdk/gdkkeysyms.h> */
#define GDK_doubledagger 0xaf2

/* <gdk/gdkkeysyms.h> */
#define GDK_checkmark 0xaf3

/* <gdk/gdkkeysyms.h> */
#define GDK_ballotcross 0xaf4

/* <gdk/gdkkeysyms.h> */
#define GDK_musicalsharp 0xaf5

/* <gdk/gdkkeysyms.h> */
#define GDK_musicalflat 0xaf6

/* <gdk/gdkkeysyms.h> */
#define GDK_malesymbol 0xaf7

/* <gdk/gdkkeysyms.h> */
#define GDK_femalesymbol 0xaf8

/* <gdk/gdkkeysyms.h> */
#define GDK_telephone 0xaf9

/* <gdk/gdkkeysyms.h> */
#define GDK_telephonerecorder 0xafa

/* <gdk/gdkkeysyms.h> */
#define GDK_phonographcopyright 0xafb

/* <gdk/gdkkeysyms.h> */
#define GDK_caret 0xafc

/* <gdk/gdkkeysyms.h> */
#define GDK_singlelowquotemark 0xafd

/* <gdk/gdkkeysyms.h> */
#define GDK_doublelowquotemark 0xafe

/* <gdk/gdkkeysyms.h> */
#define GDK_cursor 0xaff

/* <gdk/gdkkeysyms.h> */
#define GDK_leftcaret 0xba3

/* <gdk/gdkkeysyms.h> */
#define GDK_rightcaret 0xba6

/* <gdk/gdkkeysyms.h> */
#define GDK_downcaret 0xba8

/* <gdk/gdkkeysyms.h> */
#define GDK_upcaret 0xba9

/* <gdk/gdkkeysyms.h> */
#define GDK_overbar 0xbc0

/* <gdk/gdkkeysyms.h> */
#define GDK_downtack 0xbc2

/* <gdk/gdkkeysyms.h> */
#define GDK_upshoe 0xbc3

/* <gdk/gdkkeysyms.h> */
#define GDK_downstile 0xbc4

/* <gdk/gdkkeysyms.h> */
#define GDK_underbar 0xbc6

/* <gdk/gdkkeysyms.h> */
#define GDK_jot 0xbca

/* <gdk/gdkkeysyms.h> */
#define GDK_quad 0xbcc

/* <gdk/gdkkeysyms.h> */
#define GDK_uptack 0xbce

/* <gdk/gdkkeysyms.h> */
#define GDK_circle 0xbcf

/* <gdk/gdkkeysyms.h> */
#define GDK_upstile 0xbd3

/* <gdk/gdkkeysyms.h> */
#define GDK_downshoe 0xbd6

/* <gdk/gdkkeysyms.h> */
#define GDK_rightshoe 0xbd8

/* <gdk/gdkkeysyms.h> */
#define GDK_leftshoe 0xbda

/* <gdk/gdkkeysyms.h> */
#define GDK_lefttack 0xbdc

/* <gdk/gdkkeysyms.h> */
#define GDK_righttack 0xbfc

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_doublelowline 0xcdf

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_aleph 0xce0

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_bet 0xce1

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_beth 0xce1

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_gimel 0xce2

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_gimmel 0xce2

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_dalet 0xce3

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_daleth 0xce3

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_he 0xce4

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_waw 0xce5

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_zain 0xce6

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_zayin 0xce6

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_chet 0xce7

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_het 0xce7

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_tet 0xce8

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_teth 0xce8

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_yod 0xce9

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalkaph 0xcea

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_kaph 0xceb

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_lamed 0xcec

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalmem 0xced

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_mem 0xcee

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalnun 0xcef

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_nun 0xcf0

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_samech 0xcf1

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_samekh 0xcf1

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_ayin 0xcf2

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalpe 0xcf3

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_pe 0xcf4

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalzade 0xcf5

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_finalzadi 0xcf5

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_zade 0xcf6

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_zadi 0xcf6

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_qoph 0xcf7

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_kuf 0xcf7

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_resh 0xcf8

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_shin 0xcf9

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_taw 0xcfa

/* <gdk/gdkkeysyms.h> */
#define GDK_hebrew_taf 0xcfa

/* <gdk/gdkkeysyms.h> */
#define GDK_Hebrew_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_kokai 0xda1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_khokhai 0xda2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_khokhuat 0xda3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_khokhwai 0xda4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_khokhon 0xda5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_khorakhang 0xda6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_ngongu 0xda7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_chochan 0xda8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_choching 0xda9

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_chochang 0xdaa

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_soso 0xdab

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_chochoe 0xdac

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_yoying 0xdad

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_dochada 0xdae

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_topatak 0xdaf

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thothan 0xdb0

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thonangmontho 0xdb1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thophuthao 0xdb2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_nonen 0xdb3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_dodek 0xdb4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_totao 0xdb5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thothung 0xdb6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thothahan 0xdb7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thothong 0xdb8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_nonu 0xdb9

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_bobaimai 0xdba

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_popla 0xdbb

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_phophung 0xdbc

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_fofa 0xdbd

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_phophan 0xdbe

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_fofan 0xdbf

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_phosamphao 0xdc0

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_moma 0xdc1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_yoyak 0xdc2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_rorua 0xdc3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_ru 0xdc4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_loling 0xdc5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lu 0xdc6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_wowaen 0xdc7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sosala 0xdc8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sorusi 0xdc9

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sosua 0xdca

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_hohip 0xdcb

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lochula 0xdcc

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_oang 0xdcd

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_honokhuk 0xdce

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_paiyannoi 0xdcf

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraa 0xdd0

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maihanakat 0xdd1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraaa 0xdd2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraam 0xdd3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarai 0xdd4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraii 0xdd5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraue 0xdd6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarauee 0xdd7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarau 0xdd8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarauu 0xdd9

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_phinthu 0xdda

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maihanakat_maitho 0xdde

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_baht 0xddf

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarae 0xde0

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraae 0xde1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_sarao 0xde2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraaimaimuan 0xde3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_saraaimaimalai 0xde4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lakkhangyao 0xde5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maiyamok 0xde6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maitaikhu 0xde7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maiek 0xde8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maitho 0xde9

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maitri 0xdea

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_maichattawa 0xdeb

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_thanthakhat 0xdec

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_nikhahit 0xded

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_leksun 0xdf0

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_leknung 0xdf1

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_leksong 0xdf2

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_leksam 0xdf3

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_leksi 0xdf4

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lekha 0xdf5

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lekhok 0xdf6

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lekchet 0xdf7

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lekpaet 0xdf8

/* <gdk/gdkkeysyms.h> */
#define GDK_Thai_lekkao 0xdf9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul 0xff31

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Start 0xff32

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_End 0xff33

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Hanja 0xff34

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Jamo 0xff35

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Romaja 0xff36

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Codeinput 0xff37

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Jeonja 0xff38

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Banja 0xff39

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_PreHanja 0xff3a

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_PostHanja 0xff3b

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SingleCandidate 0xff3c

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_MultipleCandidate 0xff3d

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_PreviousCandidate 0xff3e

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Special 0xff3f

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_switch 0xFF7E

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Kiyeog 0xea1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SsangKiyeog 0xea2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_KiyeogSios 0xea3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Nieun 0xea4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_NieunJieuj 0xea5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_NieunHieuh 0xea6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Dikeud 0xea7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SsangDikeud 0xea8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Rieul 0xea9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulKiyeog 0xeaa

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulMieum 0xeab

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulPieub 0xeac

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulSios 0xead

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulTieut 0xeae

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulPhieuf 0xeaf

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulHieuh 0xeb0

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Mieum 0xeb1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Pieub 0xeb2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SsangPieub 0xeb3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_PieubSios 0xeb4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Sios 0xeb5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SsangSios 0xeb6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Ieung 0xeb7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Jieuj 0xeb8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SsangJieuj 0xeb9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Cieuc 0xeba

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Khieuq 0xebb

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Tieut 0xebc

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Phieuf 0xebd

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_Hieuh 0xebe

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_A 0xebf

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_AE 0xec0

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YA 0xec1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YAE 0xec2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_EO 0xec3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_E 0xec4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YEO 0xec5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YE 0xec6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_O 0xec7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_WA 0xec8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_WAE 0xec9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_OE 0xeca

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YO 0xecb

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_U 0xecc

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_WEO 0xecd

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_WE 0xece

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_WI 0xecf

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YU 0xed0

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_EU 0xed1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YI 0xed2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_I 0xed3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Kiyeog 0xed4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_SsangKiyeog 0xed5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_KiyeogSios 0xed6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Nieun 0xed7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_NieunJieuj 0xed8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_NieunHieuh 0xed9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Dikeud 0xeda

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Rieul 0xedb

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulKiyeog 0xedc

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulMieum 0xedd

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulPieub 0xede

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulSios 0xedf

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulTieut 0xee0

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulPhieuf 0xee1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_RieulHieuh 0xee2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Mieum 0xee3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Pieub 0xee4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_PieubSios 0xee5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Sios 0xee6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_SsangSios 0xee7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Ieung 0xee8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Jieuj 0xee9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Cieuc 0xeea

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Khieuq 0xeeb

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Tieut 0xeec

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Phieuf 0xeed

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_Hieuh 0xeee

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_RieulYeorinHieuh 0xeef

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SunkyeongeumMieum 0xef0

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SunkyeongeumPieub 0xef1

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_PanSios 0xef2

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_KkogjiDalrinIeung 0xef3

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_SunkyeongeumPhieuf 0xef4

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_YeorinHieuh 0xef5

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_AraeA 0xef6

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_AraeAE 0xef7

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_PanSios 0xef8

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_KkogjiDalrinIeung 0xef9

/* <gdk/gdkkeysyms.h> */
#define GDK_Hangul_J_YeorinHieuh 0xefa

/* <gdk/gdkkeysyms.h> */
#define GDK_Korean_Won 0xeff

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
typedef struct _GdkRgbCmap {
	guint32 colors[256];
	guchar lut[256]; /* for 8-bit modes */
} GdkRgbCmap;;

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_RGB_DITHER_NONE,
	GDK_RGB_DITHER_NORMAL,
	GDK_RGB_DITHER_MAX
} GdkRgbDither;

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_init(void)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
gulong gdk_rgb_xpixel_from_rgb(guint32 rgb)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_gc_set_foreground(GdkGC *gc, guint32 rgb)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_gc_set_background(GdkGC *gc, guint32 rgb)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_draw_rgb_image(GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint width, gint height, GdkRgbDither dith, guchar *rgb_buf, gint rowstride)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_draw_rgb_image_dithalign(GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint width, gint height, GdkRgbDither dith, guchar *rgb_buf, gint rowstride, gint xdith, gint ydith)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_draw_rgb_32_image(GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint width, gint height, GdkRgbDither dith, guchar *buf, gint rowstride)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_draw_gray_image(GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint width, gint height, GdkRgbDither dith, guchar *buf, gint rowstride)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
GdkRgbCmap * gdk_rgb_cmap_new(guint32 *colors, gint n_colors)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_cmap_free(GdkRgbCmap *cmap)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_draw_indexed_image(GdkDrawable *drawable, GdkGC *gc, gint x, gint y, gint width, gint height, GdkRgbDither dith, guchar *buf, gint rowstride, GdkRgbCmap *cmap)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
gboolean gdk_rgb_ditherable(void)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_set_verbose(gboolean verbose)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_set_install(gboolean install)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
void gdk_rgb_set_min_colors(gint min_colors)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
GdkColormap * gdk_rgb_get_cmap(void)
{
}

/* <gdk/gdkrgb.h> or <gdk/gdk.h> */
GdkVisual * gdk_rgb_get_visual(void)
{
}

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
#define GDK_NONE 0L

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
#define GDK_CURRENT_TIME 0L

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
#define GDK_PARENT_RELATIVE 1L

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
#define GDK_CORE_POINTER 0xfedc

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef gulong GdkAtom;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef void (*GdkEventFunc) (GdkEvent *event, gpointer	data);

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef guint32 GdkWChar;
	
/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_WINDOW_ROOT,	/* the X server's root window */
	GDK_WINDOW_TOPLEVEL,	/* interacts with window manager */
	GDK_WINDOW_CHILD,	/* child of some other type of window */
	GDK_WINDOW_DIALOG,	/* special transient kind of toplevel window */
	GDK_WINDOW_TEMP,	/* other transient windows */
	GDK_WINDOW_PIXMAP,	/* non-window drawable */
	GDK_WINDOW_FOREIGN	/* window belonging to other program */
} GdkWindowType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_INPUT_OUTPUT, GDK_INPUT_ONLY
} GdkWindowClass;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_IMAGE_NORMAL, GDK_IMAGE_SHARED, GDK_IMAGE_FASTEST
} GdkImageType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_VISUAL_STATIC_GRAY, GDK_VISUAL_GRAYSCALE, GDK_VISUAL_STATIC_COLOR,
	GDK_VISUAL_PSEUDO_COLOR, GDK_VISUAL_TRUE_COLOR, GDK_VISUAL_DIRECT_COLOR
} GdkVisualType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_FONT_FONT,	 /* is an XFontStruct */
	GDK_FONT_FONTSET /* is an XFontSet used for i18n */
} GdkFontType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_WA_TITLE = 1 << 1,
	GDK_WA_X = 1 << 2,
	GDK_WA_Y = 1 << 3,
	GDK_WA_CURSOR = 1 << 4,
	GDK_WA_COLORMAP = 1 << 5,
	GDK_WA_VISUAL = 1 << 6,
	GDK_WA_WMCLASS = 1 << 7,
	GDK_WA_NOREDIR = 1 << 8
} GdkWindowAttributesType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_HINT_POS = 1 << 0,
	GDK_HINT_MIN_SIZE = 1 << 1,
	GDK_HINT_MAX_SIZE = 1 << 2,
	GDK_HINT_BASE_SIZE = 1 << 3,
	GDK_HINT_ASPECT = 1 << 4,
	GDK_HINT_RESIZE_INC = 1 << 5
} GdkWindowHints;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_COPY, GDK_INVERT, GDK_XOR, GDK_CLEAR, GDK_AND, GDK_AND_REVERSE,
	GDK_AND_INVERT, GDK_NOOP, GDK_OR, GDK_EQUIV, GDK_OR_REVERSE,
	GDK_COPY_INVERT, GDK_OR_INVERT, GDK_NAND, GDK_SET
} GdkFunction;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_SOLID, GDK_TILED, GDK_STIPPLED, GDK_OPAQUE_STIPPLED
} GdkFill;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_EVEN_ODD_RULE, GDK_WINDING_RULE
} GdkFillRule;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_LINE_SOLID, GDK_LINE_ON_OFF_DASH, GDK_LINE_DOUBLE_DASH
} GdkLineStyle;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_CAP_NOT_LAST, GDK_CAP_BUTT, GDK_CAP_ROUND, GDK_CAP_PROJECTING
} GdkCapStyle;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_JOIN_MITER, GDK_JOIN_ROUND, GDK_JOIN_BEVEL
} GdkJoinStyle;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_NUM_GLYPHS = 154,

	GDK_X_CURSOR, GDK_ARROW, GDK_BASED_ARROW_DOWN, GDK_BASED_ARROW_UP,
	GDK_BOAT, GDK_BOGOSITY, GDK_BOTTOM_LEFT_CORNER, GDK_BOTTOM_RIGHT_CORNER,
	GDK_BOTTOM_SIDE, GDK_BOTTOM_TEE, GDK_BOX_SPIRAL, GDK_CENTER_PTR,
	GDK_CIRCLE, GDK_CLOCK, GDK_COFFEE_MUG, GDK_CROSS, GDK_CROSS_REVERSE,
	GDK_CROSSHAIR, GDK_DIAMOND_CROSS, GDK_DOT, GDK_DOTBOX, GDK_DOUBLE_ARROW,
	GDK_DRAFT_LARGE, GDK_DRAFT_SMALL, GDK_DRAPED_BOX, GDK_EXCHANGE,
	GDK_FLEUR, GDK_GOBBLER, GDK_GUMBY, GDK_HAND1, GDK_HAND2, GDK_HEART,
	GDK_ICON, GDK_IRON_CROSS, GDK_LEFT_PTR, GDK_LEFT_SIDE, GDK_LEFT_TEE,
	GDK_LEFTBUTTON, GDK_LL_ANGLE, GDK_LR_ANGLE, GDK_MAN, GDK_MIDDLEBUTTON,
	GDK_MOUSE, GDK_PENCIL, GDK_PIRATE, GDK_PLUS, GDK_QUESTION_ARROW,
	GDK_RIGHT_PTR, GDK_RIGHT_SIDE, GDK_RIGHT_TEE, GDK_RIGHTBUTTON,
	GDK_RTL_LOGO, GDK_SAILBOAT, GDK_SB_DOWN_ARROW, GDK_SB_H_DOUBLE_ARROW,
	GDK_SB_LEFT_ARROW, GDK_SB_RIGHT_ARROW, GDK_SB_UP_ARROW,
	GDK_SB_V_DOUBLE_ARROW, GDK_SHUTTLE, GDK_SIZING, GDK_SPIDER,
	GDK_SPRAYCAN, GDK_STAR, GDK_TARGET, GDK_TCROSS, GDK_TOP_LEFT_ARROW,
	GDK_TOP_LEFT_CORNER, GDK_TOP_RIGHT_CORNER, GDK_TOP_SIDE, GDK_TOP_TEE,
	GDK_TREK, GDK_UL_ANGLE, GDK_UMBRELLA, GDK_UR_ANGLE, GDK_WATCH,
	GDK_XTERM,

	GDK_LAST_CURSOR,
	GDK_CURSOR_IS_PIXMAP = -1
} GdkCursorType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum {
	GDK_FILTER_CONTINUE,  /* Event not handled, continue processesing */
	GDK_FILTER_TRANSLATE,  /* Translated event stored */
	GDK_FILTER_REMOVE  /* Terminate processing, removing event */
} GdkFilterReturn;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum {
	GDK_VISIBILITY_UNOBSCURED,
	GDK_VISIBILITY_PARTIAL,
	GDK_VISIBILITY_FULLY_OBSCURED
} GdkVisibilityState;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_NOTHING = -1,
	GDK_DELETE = 0,
	GDK_DESTROY = 1,
	GDK_EXPOSE = 2,
	GDK_MOTION_NOTIFY	= 3,
	GDK_BUTTON_PRESS	= 4,
	GDK_2BUTTON_PRESS	= 5,
	GDK_3BUTTON_PRESS	= 6,
	GDK_BUTTON_RELEASE	= 7,
	GDK_KEY_PRESS = 8,
	GDK_KEY_RELEASE	= 9,
	GDK_ENTER_NOTIFY	= 10,
	GDK_LEAVE_NOTIFY	= 11,
	GDK_FOCUS_CHANGE	= 12,
	GDK_CONFIGURE = 13,
	GDK_MAP = 14,
	GDK_UNMAP = 15,
	GDK_PROPERTY_NOTIFY	= 16,
	GDK_SELECTION_CLEAR	= 17,
	GDK_SELECTION_REQUEST = 18,
	GDK_SELECTION_NOTIFY	= 19,
	GDK_PROXIMITY_IN	= 20,
	GDK_PROXIMITY_OUT	= 21,
	GDK_DRAG_ENTER = 22,
	GDK_DRAG_LEAVE = 23,
	GDK_DRAG_MOTION = 24,
	GDK_DRAG_STATUS = 25,
	GDK_DROP_START = 26,
	GDK_DROP_FINISHED = 27,
	GDK_CLIENT_EVENT	= 28,
	GDK_VISIBILITY_NOTIFY = 29,
	GDK_NO_EXPOSE = 30
} GdkEventType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_EXPOSURE_MASK = 1 << 1,
	GDK_POINTER_MOTION_MASK	= 1 << 2,
	GDK_POINTER_MOTION_HINT_MASK	= 1 << 3,
	GDK_BUTTON_MOTION_MASK	= 1 << 4,
	GDK_BUTTON1_MOTION_MASK	= 1 << 5,
	GDK_BUTTON2_MOTION_MASK	= 1 << 6,
	GDK_BUTTON3_MOTION_MASK	= 1 << 7,
	GDK_BUTTON_PRESS_MASK = 1 << 8,
	GDK_BUTTON_RELEASE_MASK	= 1 << 9,
	GDK_KEY_PRESS_MASK = 1 << 10,
	GDK_KEY_RELEASE_MASK = 1 << 11,
	GDK_ENTER_NOTIFY_MASK = 1 << 12,
	GDK_LEAVE_NOTIFY_MASK = 1 << 13,
	GDK_FOCUS_CHANGE_MASK = 1 << 14,
	GDK_STRUCTURE_MASK = 1 << 15,
	GDK_PROPERTY_CHANGE_MASK	= 1 << 16,
	GDK_VISIBILITY_NOTIFY_MASK	= 1 << 17,
	GDK_PROXIMITY_IN_MASK = 1 << 18,
	GDK_PROXIMITY_OUT_MASK	= 1 << 19,
	GDK_SUBSTRUCTURE_MASK = 1 << 20,
	GDK_ALL_EVENTS_MASK = 0x0FFFFF
} GdkEventMask;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_NOTIFY_ANCESTOR = 0,
	GDK_NOTIFY_VIRTUAL = 1,
	GDK_NOTIFY_INFERIOR = 2,
	GDK_NOTIFY_NONLINEAR = 3,
	GDK_NOTIFY_NONLINEAR_VIRTUAL	= 4,
	GDK_NOTIFY_UNKNOWN = 5
} GdkNotifyType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_CROSSING_NORMAL,
	GDK_CROSSING_GRAB,
	GDK_CROSSING_UNGRAB
} GdkCrossingMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_SHIFT_MASK = 1 << 0,
	GDK_LOCK_MASK = 1 << 1,
	GDK_CONTROL_MASK = 1 << 2,
	GDK_MOD1_MASK = 1 << 3,
	GDK_MOD2_MASK = 1 << 4,
	GDK_MOD3_MASK = 1 << 5,
	GDK_MOD4_MASK = 1 << 6,
	GDK_MOD5_MASK = 1 << 7,
	GDK_BUTTON1_MASK = 1 << 8,
	GDK_BUTTON2_MASK = 1 << 9,
	GDK_BUTTON3_MASK = 1 << 10,
	GDK_BUTTON4_MASK = 1 << 11,
	GDK_BUTTON5_MASK = 1 << 12,
	GDK_RELEASE_MASK = 1 << 13,
	GDK_MODIFIER_MASK = 0x3fff
} GdkModifierType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_CLIP_BY_CHILDREN, GDK_INCLUDE_INFERIORS
} GdkSubwindowMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_INPUT_READ = 1 << 0,
	GDK_INPUT_WRITE = 1 << 1,
	GDK_INPUT_EXCEPTION = 1 << 2
} GdkInputCondition;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_OK = 0,
	GDK_ERROR = -1,
	GDK_ERROR_PARAM = -2,
	GDK_ERROR_FILE = -3,
	GDK_ERROR_MEM = -4
} GdkStatus;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_LSB_FIRST, GDK_MSB_FIRST
} GdkByteOrder;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_GC_FOREGROUND = 1 << 0,
	GDK_GC_BACKGROUND = 1 << 1,
	GDK_GC_FONT = 1 << 2,
	GDK_GC_FUNCTION = 1 << 3,
	GDK_GC_FILL = 1 << 4,
	GDK_GC_TILE = 1 << 5,
	GDK_GC_STIPPLE = 1 << 6,
	GDK_GC_CLIP_MASK = 1 << 7,
	GDK_GC_SUBWINDOW = 1 << 8,
	GDK_GC_TS_X_ORIGIN = 1 << 9,
	GDK_GC_TS_Y_ORIGIN = 1 << 10,
	GDK_GC_CLIP_X_ORIGIN = 1 << 11,
	GDK_GC_CLIP_Y_ORIGIN = 1 << 12,
	GDK_GC_EXPOSURES = 1 << 13,
	GDK_GC_LINE_WIDTH = 1 << 14,
	GDK_GC_LINE_STYLE = 1 << 15,
	GDK_GC_CAP_STYLE = 1 << 16,
	GDK_GC_JOIN_STYLE = 1 << 17
} GdkGCValuesMask;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_SELECTION_PRIMARY = 1,
	GDK_SELECTION_SECONDARY = 2
} GdkSelection;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_PROPERTY_NEW_VALUE, GDK_PROPERTY_DELETE
} GdkPropertyState;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_PROP_MODE_REPLACE, GDK_PROP_MODE_PREPEND, GDK_PROP_MODE_APPEND
} GdkPropMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_SOURCE_MOUSE, GDK_SOURCE_PEN, GDK_SOURCE_ERASER, GDK_SOURCE_CURSOR
} GdkInputSource;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_MODE_DISABLED, GDK_MODE_SCREEN, GDK_MODE_WINDOW
} GdkInputMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_AXIS_IGNORE, GDK_AXIS_X, GDK_AXIS_Y, GDK_AXIS_PRESSURE,
	GDK_AXIS_XTILT, GDK_AXIS_YTILT, GDK_AXIS_LAST
} GdkAxisUse;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_TARGET_BITMAP = 5,
	GDK_TARGET_COLORMAP = 7,
	GDK_TARGET_DRAWABLE = 17,
	GDK_TARGET_PIXMAP = 20,
	GDK_TARGET_STRING = 31
} GdkTarget;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_SELECTION_TYPE_ATOM = 4,
	GDK_SELECTION_TYPE_BITMAP = 5,
	GDK_SELECTION_TYPE_COLORMAP = 7,
	GDK_SELECTION_TYPE_DRAWABLE = 17,
	GDK_SELECTION_TYPE_INTEGER = 19,
	GDK_SELECTION_TYPE_PIXMAP = 20,
	GDK_SELECTION_TYPE_WINDOW = 33,
	GDK_SELECTION_TYPE_STRING = 31
} GdkSelectionType;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_EXTENSION_EVENTS_NONE,
	GDK_EXTENSION_EVENTS_ALL,
	GDK_EXTENSION_EVENTS_CURSOR
} GdkExtensionMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h>  Mask values, may be OR'ed together */
typedef enum
{
	GDK_IM_PREEDIT_AREA, GDK_IM_PREEDIT_CALLBACKS, GDK_IM_PREEDIT_POSITION,
	GDK_IM_PREEDIT_NOTHING, GDK_IM_PREEDIT_NONE, GDK_IM_PREEDIT_MASK,
	GDK_IM_STATUS_AREA, GDK_IM_STATUS_CALLBACKS, GDK_IM_STATUS_NOTHING,
	GDK_IM_STATUS_NONE, GDK_IM_STATUS_MASK
} GdkIMStyle;

/* <gdk/gtktypes.h> or <gdk/gdk.h>  Mask values, may be OR'ed together */
typedef enum
{
	GDK_IC_STYLE, GDK_IC_CLIENT_WINDOW, GDK_IC_FOCUS_WINDOW,
	GDK_IC_FILTER_EVENTS, GDK_IC_SPOT_LOCATION, GDK_IC_LINE_SPACING,
	GDK_IC_CURSOR, GDK_IC_PREEDIT_FONTSET, GDK_IC_PREEDIT_AREA,
	GDK_IC_PREEDIT_AREA_NEEDED, GDK_IC_PREEDIT_FOREGROUND,
	GDK_IC_PREEDIT_BACKGROUND, GDK_IC_PREEDIT_PIXMAP,
	GDK_IC_PREEDIT_COLORMAP, GDK_IC_STATUS_FONTSET, GDK_IC_STATUS_AREA,
	GDK_IC_STATUS_AREA_NEEDED, GDK_IC_STATUS_FOREGROUND,
	GDK_IC_STATUS_BACKGROUND, GDK_IC_STATUS_PIXMAP, GDK_IC_STATUS_COLORMAP,
	GDK_IC_ALL_REQ, GDK_IC_PREEDIT_AREA_REQ, GDK_IC_PREEDIT_POSITION_REQ,
	GDK_IC_STATUS_AREA_REQ
} GdkICAttributesType;

/* <gdk/gtktypes.h> or <gdk/gdk.h>  Mask values, may be OR'ed together */
typedef enum
{
	GDK_DECOR_ALL, GDK_DECOR_BORDER, GDK_DECOR_RESIZEH, GDK_DECOR_TITLE,
	GDK_DECOR_MENU, GDK_DECOR_MINIMIZE, GDK_DECOR_MAXIMIZE
} GdkWMDecoration;

/* <gdk/gtktypes.h> or <gdk/gdk.h>  Mask values, may be OR'ed together */
typedef enum
{
	GDK_FUNC_ALL, GDK_FUNC_RESIZE, GDK_FUNC_MOVE, GDK_FUNC_MINIMIZE,
	GDK_FUNC_MAXIMIZE, GDK_FUNC_CLOS	  
} GdkWMFunction;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef void (*GdkInputFunction) (gpointer data, gint source, GdkInputCondition condition);

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef void (*GdkDestroyNotify) (gpointer data);

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_CC_MODE_UNDEFINED, GDK_CC_MODE_BW, GDK_CC_MODE_STD_CMAP,
	GDK_CC_MODE_TRUE, GDK_CC_MODE_MY_GRAY, GDK_CC_MODE_PALETTE
} GdkColorContextMode;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum
{
	GDK_OVERLAP_RECTANGLE_IN, GDK_OVERLAP_RECTANGLE_OUT,
	GDK_OVERLAP_RECTANGLE_PART
} GdkOverlapType;

/* <gdk/gtktypes.h> or <gdk/gdk.h>  Mask values, may be OR'ed together */
typedef enum {
	GDK_ACTION_DEFAULT, GDK_ACTION_COPY, GDK_ACTION_MOVE, GDK_ACTION_LINK,
	GDK_ACTION_PRIVATE, GDK_ACTION_ASK
} GdkDragAction;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef enum {
	GDK_DRAG_PROTO_MOTIF, GDK_DRAG_PROTO_XDND, GDK_DRAG_PROTO_ROOTWIN,
	GDK_DRAG_PROTO_NONE
} GdkDragProtocol;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkColor
{
	gulong pixel;
	gushort red;
	gushort green;
	gushort blue;
} GdkColor;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkColormap
{
	gint size;
	GdkColor *colors;
} GdkColormap;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkVisual
{
	GdkVisualType type;
	gint depth;
	GdkByteOrder byte_order;
	gint colormap_size;
	gint bits_per_rgb;

	guint32 red_mask;
	gint red_shift;
	gint red_prec;

	guint32 green_mask;
	gint green_shift;
	gint green_prec;

	guint32 blue_mask;
	gint blue_shift;
	gint blue_prec;
} GdkVisual;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkWindowAttr
{
	gchar *title;
	gint event_mask;
	gint16 x, y;
	gint16 width;
	gint16 height;
	GdkWindowClass wclass;
	GdkVisual *visual;
	GdkColormap *colormap;
	GdkWindowType window_type;
	GdkCursor *cursor;
	gchar *wmclass_name;
	gchar *wmclass_class;
	gboolean override_redirect;
} GdkWindowAttr;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkWindow
{
	gpointer user_data;
} GdkWindow;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkGeometry {
	gint min_width;
	gint min_height;
	gint max_width;
	gint max_height;
	gint base_width;
	gint base_height;
	gint width_inc;
	gint height_inc;
	gdouble min_aspect;
	gdouble max_aspect;
	/* GdkGravity gravity; */
} GdkGeometry;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkImage
{
	GdkImageType	type;
	GdkVisual	*visual; 
	GdkByteOrder	byte_order;
	guint16		width;
	guint16		height;
	guint16		depth;
	guint16		bpp;  /* bytes per pixel */
	guint16		bpl;  /* bytes per line */
	gpointer	mem;
} GdkImage;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkGCValues
{
	GdkColor foreground;
	GdkColor background;
	GdkFont *font;
	GdkFunction function;
	GdkFill fill;
	GdkPixmap *tile;
	GdkPixmap *stipple;
	GdkPixmap *clip_mask;
	GdkSubwindowMode subwindow_mode;
	gint ts_x_origin;
	gint ts_y_origin;
	gint clip_x_origin;
	gint clip_y_origin;
	gint graphics_exposures;
	gint line_width;
	GdkLineStyle line_style;
	GdkCapStyle cap_style;
	GdkJoinStyle join_style;
} GdkGCValues;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkGC
{
	gint dummy_var;
} GdkGC;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkPoint
{
	gint16 x;
	gint16 y;
} GdkPoint;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkRectangle
{
	gint16 x;
	gint16 y;
	guint16 width;
	guint16 height;
} GdkRectangle;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkSegment
{
	gint16 x1;
	gint16 y1;
	gint16 x2;
	gint16 y2;
} GdkSegment;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkFont
{
	GdkFontType type;
	gint ascent;
	gint descent;
} GdkFont;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkCursor
{
	GdkCursorType type;
} GdkCursor;


/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkColorContextDither
{
	gint fast_rgb[32][32][32]; /* quick look-up table for faster rendering */
	gint fast_err[32][32][32]; /* internal RGB error information */
	gint fast_erg[32][32][32];
	gint fast_erb[32][32][32];
} GdkColorContextDither;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkColorContext
{
	GdkVisual	*visual;
	GdkColormap	*colormap;
	gint		num_colors;	/* available # of colors in colormap */
	gint		max_colors;	/* maximum no. of colors */
	gint		num_allocated;	/* no. of allocated colors */
	GdkColorContextMode mode;
	gint		need_to_free_colormap;
	GdkAtom		std_cmap_atom;
	gulong		*clut;		/* color look-up table */
	GdkColor	*cmap;		/* colormap */
	GHashTable	*color_hash;	/* hash table of allocated colors */
	GdkColor	*palette;	/* preallocated palette */
	gint	num_palette;		/* size of palette */
	GdkColorContextDither	*fast_dither;	/* fast dither matrix */
	struct { gint red, green, blue; } shifts;
	struct { gulong red, green, blue; } masks;
	struct { gint red, green, blue; } bits;
	gulong		max_entry;
	gulong		black_pixel;
	gulong		white_pixel;
} GdkColorContext;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkDeviceKey
{
	guint keyval;
	GdkModifierType modifiers;
} GdkDeviceKey;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkDeviceInfo
{
	guint32		deviceid;
	gchar		*name;
	GdkInputSource	source;
	GdkInputMode	mode;
	gint		has_cursor; /* TRUE if device moves cursor */
	gint		num_axes;
	GdkAxisUse	*axes;  /* Specifies use for each axis */
	gint		num_keys;
	GdkDeviceKey	*keys;
} GdkDeviceInfo;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkTimeCoord
{
	guint32 time;
	gdouble x;
	gdouble y;
	gdouble pressure;
	gdouble xtilt;
	gdouble ytilt;
} GdkTimeCoord;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkDragContext
{
	GdkDragProtocol protocol;
	gboolean is_source;
	GdkWindow *source_window;
	GdkWindow *dest_window;
	GList *targets;
	GdkDragAction actions;
	GdkDragAction suggested_action;
	GdkDragAction action; 
	guint32 start_time;
} GdkDragContext;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef void GdkXEvent;  /* Can be cast to XEvent */

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef GdkFilterReturn (*GdkFilterFunc) (GdkXEvent *xevent, GdkEvent *event, gpointer data);

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventAny
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
} GdkEventAny;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventExpose
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkRectangle area;
	gint count; /* If non-zero, how many more events follow. */
} GdkEventExpose;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventNoExpose
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
} GdkEventNoExpose;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventVisibility
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkVisibilityState state;
} GdkEventVisibility;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventMotion
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	guint32 time;
	gdouble x;
	gdouble y;
	gdouble pressure;
	gdouble xtilt;
	gdouble ytilt;
	guint state;
	gint16 is_hint;
	GdkInputSource source;
	guint32 deviceid;
	gdouble x_root, y_root;
} GdkEventMotion;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventButton
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	guint32 time;
	gdouble x;
	gdouble y;
	gdouble pressure;
	gdouble xtilt;
	gdouble ytilt;
	guint state;
	guint button;
	GdkInputSource source;
	guint32 deviceid;
	gdouble x_root, y_root;
} GdkEventButton;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventKey
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	guint32 time;
	guint state;
	guint keyval;
	gint length;
	gchar *string;
} GdkEventKey;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventCrossing
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkWindow *subwindow;
	guint32 time;
	gdouble x;
	gdouble y;
	gdouble x_root;
	gdouble y_root;
	GdkCrossingMode mode;
	GdkNotifyType detail;
	gboolean focus;
	guint state;
} GdkEventCrossing;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventFocus
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	gint16 in;
} GdkEventFocus;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventConfigure
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	gint16 x, y;
	gint16 width;
	gint16 height;
} GdkEventConfigure;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventProperty
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkAtom atom;
	guint32 time;
	guint state;
} GdkEventProperty;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventSelection
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkAtom selection;
	GdkAtom target;
	GdkAtom property;
	guint32 requestor;
	guint32 time;
} GdkEventSelection;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventProximity
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	guint32 time;
	GdkInputSource source;
	guint32 deviceid;
} GdkEventProximity;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventClient
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkAtom message_type;
	gushort data_format;
	union {
	 char b[20];
	 short s[10];
	 long l[5];
	} data;
} GdkEventClient;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkEventDND
{
	GdkEventType type;
	GdkWindow *window;
	gint8 send_event;
	GdkDragContext *context;
	guint32 time;
	gshort x_root, y_root;
} GdkEventDND;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef union _GdkEvent
{
	GdkEventType type;
	GdkEventAny any;
	GdkEventExpose expose;
	GdkEventNoExpose no_expose;
	GdkEventVisibility visibility;
	GdkEventMotion motion;
	GdkEventButton button;
	GdkEventKey key;
	GdkEventCrossing crossing;
	GdkEventFocus focus_change;
	GdkEventConfigure configure;
	GdkEventProperty property;
	GdkEventSelection selection;
	GdkEventProximity proximity;
	GdkEventClient client;
	GdkEventDND dnd;
} GdkEvent;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkRegion
{
	gpointer user_data;
} GdkRegion;

/* <gdk/gtktypes.h> or <gdk/gdk.h> */
typedef struct _GdkICAttr
{
	GdkIMStyle style;
	GdkWindow *client_window;
	GdkWindow *focus_window;
	GdkEventMask filter_events;
	GdkPoint spot_location;
	gint line_spacing;
	GdkCursor *cursor;

	GdkFont *preedit_fontset;
	GdkRectangle preedit_area;
	GdkRectangle preedit_area_needed; 
	GdkColor preedit_foreground;
	GdkColor preedit_background;
	GdkPixmap *preedit_pixmap;
	GdkColormap *preedit_colormap;

	GdkFont *status_fontset;
	GdkRectangle status_area;
	GdkRectangle status_area_needed; 
	GdkColor status_foreground;
	GdkColor status_background;
	GdkPixmap *status_pixmap;
	GdkColormap *status_colormap;
} GdkICAttr;

/* <gdk/gdkx.h> */
#define GDK_ROOT_WINDOW() gdk_root_window

/* <gdk/gdkx.h> */
#define GDK_ROOT_PARENT() ((GdkWindow *)&gdk_root_parent)

/* <gdk/gdkx.h> */
#define GDK_DISPLAY() gdk_display

/* <gdk/gdkx.h> */
#define GDK_WINDOW_XDISPLAY(win) (((GdkWindowPrivate*) win)->xdisplay)

/* <gdk/gdkx.h> */
#define GDK_WINDOW_XWINDOW(win) (((GdkWindowPrivate*) win)->xwindow)

/* <gdk/gdkx.h> */
#define GDK_IMAGE_XDISPLAY(image) (((GdkImagePrivate*) image)->xdisplay)

/* <gdk/gdkx.h> */
#define GDK_IMAGE_XIMAGE(image) (((GdkImagePrivate*) image)->ximage)

/* <gdk/gdkx.h> */
#define GDK_GC_XDISPLAY(gc) (((GdkGCPrivate*) gc)->xdisplay)

/* <gdk/gdkx.h> */
#define GDK_GC_XGC(gc) (((GdkGCPrivate*) gc)->xgc)

/* <gdk/gdkx.h> */
#define GDK_COLORMAP_XDISPLAY(cmap) (((GdkColormapPrivate*) cmap)->xdisplay)

/* <gdk/gdkx.h> */
#define GDK_COLORMAP_XCOLORMAP(cmap) (((GdkColormapPrivate*) cmap)->xcolormap)

/* <gdk/gdkx.h> */
#define GDK_VISUAL_XVISUAL(vis) (((GdkVisualPrivate*) vis)->xvisual)

/* <gdk/gdkx.h> */
#define GDK_FONT_XDISPLAY(font) (((GdkFontPrivate*) font)->xdisplay)

/* <gdk/gdkx.h> */
#define GDK_FONT_XFONT(font) (((GdkFontPrivate*) font)->xfont)

/* <gdk/gdkx.h> */
GdkVisual* gdkx_visual_get (VisualID xvisualid);

/* <gdk/gdkx.h>  Do not use this function until it is fixed. An X Colormap
 * is useless unless we also have the visual. */
GdkColormap* gdkx_colormap_get (Colormap xcolormap);

/* <gdk/gdkx.h> What does this do? */
Window gdk_get_client_window (Display *dpy, Window win);

/* <gdk/gdkx.h> Create a GdkPixmap using a given X pixmap id */
GdkPixmap *gdk_pixmap_foreign_new (guint32 anid);

/* <gdk/gdkx.h> Create a GdkWindow using a given X window id */
GdkWindow *gdk_window_foreign_new (guint32 anid);



