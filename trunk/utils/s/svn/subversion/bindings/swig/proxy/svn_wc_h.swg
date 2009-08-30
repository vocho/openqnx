/* Proxy classes for svn_wc.h
 * DO NOT EDIT -- AUTOMATICALLY GENERATED */

/* No default constructors for opaque structs */
#ifdef SWIGPYTHON
%nodefault svn_wc_adm_access_t;
%nodefault svn_wc_traversal_info_t;
%nodefault svn_wc_committed_queue_t;
#endif

/* Callback typemaps */
#ifdef SWIGPYTHON
%apply CALLABLE_CALLBACK {
  svn_wc_notify_func2_t,
  svn_wc_notify_func_t,
  svn_wc_get_file_t,
  svn_wc_conflict_resolver_func_t,
  svn_wc_status_func2_t,
  svn_wc_status_func_t,
  svn_wc_canonicalize_svn_prop_get_file_t,
  svn_wc_relocation_validator3_t,
  svn_wc_relocation_validator2_t,
  svn_wc_relocation_validator_t
};
%apply CALLABLE_CALLBACK * {
  svn_wc_notify_func2_t *,
  svn_wc_notify_func_t *,
  svn_wc_get_file_t *,
  svn_wc_conflict_resolver_func_t *,
  svn_wc_status_func2_t *,
  svn_wc_status_func_t *,
  svn_wc_canonicalize_svn_prop_get_file_t *,
  svn_wc_relocation_validator3_t *,
  svn_wc_relocation_validator2_t *,
  svn_wc_relocation_validator_t *
};
#endif

/* Baton typemaps */
#ifdef SWIGPYTHON
%apply void *PY_AS_VOID {
  void *cancel_baton,
  void *baton,
  void *diff_baton,
  void *walk_baton,
  void *status_baton,
  void *set_locks_baton,
  void *notify_baton,
  void *report_baton,
  void *conflict_baton,
  void *fetch_baton,
  void *getter_baton,
  void *callback_baton,
  void *validator_baton,
  void *file_baton
};
#endif

/* Includes */
%{
#include "svn_wc.h"
%}
%include svn_wc.h

/* Structure definitions */
#ifdef SWIGPYTHON
%opaque_proxy(svn_wc_adm_access_t);
%opaque_proxy(svn_wc_traversal_info_t);
%proxy(svn_wc_external_item2_t);
%proxy(svn_wc_external_item_t);
%proxy(svn_wc_notify_t);
%proxy(svn_wc_conflict_description_t);
%proxy(svn_wc_conflict_result_t);
%proxy(svn_wc_diff_callbacks2_t);
%proxy(svn_wc_diff_callbacks_t);
%proxy(svn_wc_entry_t);
%proxy(svn_wc_entry_callbacks2_t);
%proxy(svn_wc_entry_callbacks_t);
%proxy(svn_wc_status2_t);
%proxy(svn_wc_status_t);
%opaque_proxy(svn_wc_committed_queue_t);
%proxy(svn_wc_revision_status_t);
#endif

/* Callbacks */

%inline %{
static svn_error_t * svn_wc_diff_callbacks2_invoke_file_changed(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *contentstate, svn_wc_notify_state_t *propstate, const char *path, const char *tmpfile1, const char *tmpfile2, svn_revnum_t rev1, svn_revnum_t rev2, const char *mimetype1, const char *mimetype2, const apr_array_header_t *propchanges, apr_hash_t *originalprops, void *diff_baton) {
  return (_obj->file_changed)(adm_access, contentstate, propstate, path, tmpfile1, tmpfile2, rev1, rev2, mimetype1, mimetype2, propchanges, originalprops, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks2_invoke_file_added(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *contentstate, svn_wc_notify_state_t *propstate, const char *path, const char *tmpfile1, const char *tmpfile2, svn_revnum_t rev1, svn_revnum_t rev2, const char *mimetype1, const char *mimetype2, const apr_array_header_t *propchanges, apr_hash_t *originalprops, void *diff_baton) {
  return (_obj->file_added)(adm_access, contentstate, propstate, path, tmpfile1, tmpfile2, rev1, rev2, mimetype1, mimetype2, propchanges, originalprops, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks2_invoke_file_deleted(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const char *tmpfile1, const char *tmpfile2, const char *mimetype1, const char *mimetype2, apr_hash_t *originalprops, void *diff_baton) {
  return (_obj->file_deleted)(adm_access, state, path, tmpfile1, tmpfile2, mimetype1, mimetype2, originalprops, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks2_invoke_dir_added(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, svn_revnum_t rev, void *diff_baton) {
  return (_obj->dir_added)(adm_access, state, path, rev, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks2_invoke_dir_deleted(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, void *diff_baton) {
  return (_obj->dir_deleted)(adm_access, state, path, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks2_invoke_dir_props_changed(
  svn_wc_diff_callbacks2_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const apr_array_header_t *propchanges, apr_hash_t *original_props, void *diff_baton) {
  return (_obj->dir_props_changed)(adm_access, state, path, propchanges, original_props, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_file_changed(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const char *tmpfile1, const char *tmpfile2, svn_revnum_t rev1, svn_revnum_t rev2, const char *mimetype1, const char *mimetype2, void *diff_baton) {
  return (_obj->file_changed)(adm_access, state, path, tmpfile1, tmpfile2, rev1, rev2, mimetype1, mimetype2, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_file_added(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const char *tmpfile1, const char *tmpfile2, svn_revnum_t rev1, svn_revnum_t rev2, const char *mimetype1, const char *mimetype2, void *diff_baton) {
  return (_obj->file_added)(adm_access, state, path, tmpfile1, tmpfile2, rev1, rev2, mimetype1, mimetype2, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_file_deleted(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const char *tmpfile1, const char *tmpfile2, const char *mimetype1, const char *mimetype2, void *diff_baton) {
  return (_obj->file_deleted)(adm_access, state, path, tmpfile1, tmpfile2, mimetype1, mimetype2, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_dir_added(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, svn_revnum_t rev, void *diff_baton) {
  return (_obj->dir_added)(adm_access, state, path, rev, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_dir_deleted(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, void *diff_baton) {
  return (_obj->dir_deleted)(adm_access, state, path, diff_baton);
}

static svn_error_t * svn_wc_diff_callbacks_invoke_props_changed(
  svn_wc_diff_callbacks_t * _obj, svn_wc_adm_access_t *adm_access, svn_wc_notify_state_t *state, const char *path, const apr_array_header_t *propchanges, apr_hash_t *original_props, void *diff_baton) {
  return (_obj->props_changed)(adm_access, state, path, propchanges, original_props, diff_baton);
}

static svn_error_t * svn_wc_entry_callbacks2_invoke_found_entry(
  svn_wc_entry_callbacks2_t * _obj, const char *path, const svn_wc_entry_t *entry, void *walk_baton, apr_pool_t *pool) {
  return (_obj->found_entry)(path, entry, walk_baton, pool);
}

static svn_error_t * svn_wc_entry_callbacks2_invoke_handle_error(
  svn_wc_entry_callbacks2_t * _obj, const char *path, svn_error_t *err, void *walk_baton, apr_pool_t *pool) {
  return (_obj->handle_error)(path, err, walk_baton, pool);
}

static svn_error_t * svn_wc_entry_callbacks_invoke_found_entry(
  svn_wc_entry_callbacks_t * _obj, const char *path, const svn_wc_entry_t *entry, void *walk_baton, apr_pool_t *pool) {
  return (_obj->found_entry)(path, entry, walk_baton, pool);
}

static void svn_wc_invoke_notify_func2(
  svn_wc_notify_func2_t _obj, void *baton, const svn_wc_notify_t *notify, apr_pool_t *pool) {
  _obj(baton, notify, pool);
}

static void svn_wc_invoke_notify_func(
  svn_wc_notify_func_t _obj, void *baton, const char *path, svn_wc_notify_action_t action, svn_node_kind_t kind, const char *mime_type, svn_wc_notify_state_t content_state, svn_wc_notify_state_t prop_state, svn_revnum_t revision) {
  _obj(baton, path, action, kind, mime_type, content_state, prop_state, revision);
}

static svn_error_t * svn_wc_invoke_get_file(
  svn_wc_get_file_t _obj, void *baton, const char *path, svn_revnum_t revision, svn_stream_t *stream, svn_revnum_t *fetched_rev, apr_hash_t **props, apr_pool_t *pool) {
  return _obj(baton, path, revision, stream, fetched_rev, props, pool);
}

static svn_error_t * svn_wc_invoke_conflict_resolver_func(
  svn_wc_conflict_resolver_func_t _obj, svn_wc_conflict_result_t **result, const svn_wc_conflict_description_t *description, void *baton, apr_pool_t *pool) {
  return _obj(result, description, baton, pool);
}

static void svn_wc_invoke_status_func2(
  svn_wc_status_func2_t _obj, void *baton, const char *path, svn_wc_status2_t *status) {
  _obj(baton, path, status);
}

static void svn_wc_invoke_status_func(
  svn_wc_status_func_t _obj, void *baton, const char *path, svn_wc_status_t *status) {
  _obj(baton, path, status);
}

static svn_error_t * svn_wc_invoke_canonicalize_svn_prop_get_file(
  svn_wc_canonicalize_svn_prop_get_file_t _obj, const svn_string_t **mime_type, svn_stream_t *stream, void *baton, apr_pool_t *pool) {
  return _obj(mime_type, stream, baton, pool);
}

static svn_error_t * svn_wc_invoke_relocation_validator3(
  svn_wc_relocation_validator3_t _obj, void *baton, const char *uuid, const char *url, const char *root_url, apr_pool_t *pool) {
  return _obj(baton, uuid, url, root_url, pool);
}

static svn_error_t * svn_wc_invoke_relocation_validator2(
  svn_wc_relocation_validator2_t _obj, void *baton, const char *uuid, const char *url, svn_boolean_t root, apr_pool_t *pool) {
  return _obj(baton, uuid, url, root, pool);
}

static svn_error_t * svn_wc_invoke_relocation_validator(
  svn_wc_relocation_validator_t _obj, void *baton, const char *uuid, const char *url) {
  return _obj(baton, uuid, url);
}

%}

#ifdef SWIGPYTHON
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, file_changed, svn_wc_diff_callbacks2_invoke_file_changed);
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, file_added, svn_wc_diff_callbacks2_invoke_file_added);
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, file_deleted, svn_wc_diff_callbacks2_invoke_file_deleted);
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, dir_added, svn_wc_diff_callbacks2_invoke_dir_added);
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, dir_deleted, svn_wc_diff_callbacks2_invoke_dir_deleted);
%funcptr_member_proxy(svn_wc_diff_callbacks2_t, dir_props_changed, svn_wc_diff_callbacks2_invoke_dir_props_changed);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, file_changed, svn_wc_diff_callbacks_invoke_file_changed);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, file_added, svn_wc_diff_callbacks_invoke_file_added);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, file_deleted, svn_wc_diff_callbacks_invoke_file_deleted);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, dir_added, svn_wc_diff_callbacks_invoke_dir_added);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, dir_deleted, svn_wc_diff_callbacks_invoke_dir_deleted);
%funcptr_member_proxy(svn_wc_diff_callbacks_t, props_changed, svn_wc_diff_callbacks_invoke_props_changed);
%funcptr_member_proxy(svn_wc_entry_callbacks2_t, found_entry, svn_wc_entry_callbacks2_invoke_found_entry);
%funcptr_member_proxy(svn_wc_entry_callbacks2_t, handle_error, svn_wc_entry_callbacks2_invoke_handle_error);
%funcptr_member_proxy(svn_wc_entry_callbacks_t, found_entry, svn_wc_entry_callbacks_invoke_found_entry);
%funcptr_proxy(svn_wc_notify_func2_t, svn_wc_invoke_notify_func2);
%funcptr_proxy(svn_wc_notify_func_t, svn_wc_invoke_notify_func);
%funcptr_proxy(svn_wc_get_file_t, svn_wc_invoke_get_file);
%funcptr_proxy(svn_wc_conflict_resolver_func_t, svn_wc_invoke_conflict_resolver_func);
%funcptr_proxy(svn_wc_status_func2_t, svn_wc_invoke_status_func2);
%funcptr_proxy(svn_wc_status_func_t, svn_wc_invoke_status_func);
%funcptr_proxy(svn_wc_canonicalize_svn_prop_get_file_t, svn_wc_invoke_canonicalize_svn_prop_get_file);
%funcptr_proxy(svn_wc_relocation_validator3_t, svn_wc_invoke_relocation_validator3);
%funcptr_proxy(svn_wc_relocation_validator2_t, svn_wc_invoke_relocation_validator2);
%funcptr_proxy(svn_wc_relocation_validator_t, svn_wc_invoke_relocation_validator);

#endif
