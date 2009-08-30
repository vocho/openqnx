/* Proxy classes for svn_diff.h
 * DO NOT EDIT -- AUTOMATICALLY GENERATED */

/* No default constructors for opaque structs */
#ifdef SWIGPYTHON
%nodefault svn_diff_t;
#endif

/* Callback typemaps */

/* Baton typemaps */
#ifdef SWIGPYTHON
%apply void *PY_AS_VOID {
  void *diff_baton,
  void *output_baton
};
#endif

/* Includes */
%{
#include "svn_diff.h"
%}
%include svn_diff.h

/* Structure definitions */
#ifdef SWIGPYTHON
%opaque_proxy(svn_diff_t);
%proxy(svn_diff_fns_t);
%proxy(svn_diff_output_fns_t);
%proxy(svn_diff_file_options_t);
#endif

/* Callbacks */

%inline %{
static svn_error_t * svn_diff_fns_invoke_datasource_open(
  svn_diff_fns_t * _obj, void *diff_baton, svn_diff_datasource_e datasource) {
  return (_obj->datasource_open)(diff_baton, datasource);
}

static svn_error_t * svn_diff_fns_invoke_datasource_close(
  svn_diff_fns_t * _obj, void *diff_baton, svn_diff_datasource_e datasource) {
  return (_obj->datasource_close)(diff_baton, datasource);
}

static svn_error_t * svn_diff_fns_invoke_datasource_get_next_token(
  svn_diff_fns_t * _obj, apr_uint32_t *hash, void **token, void *diff_baton, svn_diff_datasource_e datasource) {
  return (_obj->datasource_get_next_token)(hash, token, diff_baton, datasource);
}

static svn_error_t * svn_diff_fns_invoke_token_compare(
  svn_diff_fns_t * _obj, void *diff_baton, void *ltoken, void *rtoken, int *compare) {
  return (_obj->token_compare)(diff_baton, ltoken, rtoken, compare);
}

static void svn_diff_fns_invoke_token_discard(
  svn_diff_fns_t * _obj, void *diff_baton, void *token) {
  (_obj->token_discard)(diff_baton, token);
}

static void svn_diff_fns_invoke_token_discard_all(
  svn_diff_fns_t * _obj, void *diff_baton) {
  (_obj->token_discard_all)(diff_baton);
}

static svn_error_t * svn_diff_output_fns_invoke_output_common(
  svn_diff_output_fns_t * _obj, void *output_baton, apr_off_t original_start, apr_off_t original_length, apr_off_t modified_start, apr_off_t modified_length, apr_off_t latest_start, apr_off_t latest_length) {
  return (_obj->output_common)(output_baton, original_start, original_length, modified_start, modified_length, latest_start, latest_length);
}

static svn_error_t * svn_diff_output_fns_invoke_output_diff_modified(
  svn_diff_output_fns_t * _obj, void *output_baton, apr_off_t original_start, apr_off_t original_length, apr_off_t modified_start, apr_off_t modified_length, apr_off_t latest_start, apr_off_t latest_length) {
  return (_obj->output_diff_modified)(output_baton, original_start, original_length, modified_start, modified_length, latest_start, latest_length);
}

static svn_error_t * svn_diff_output_fns_invoke_output_diff_latest(
  svn_diff_output_fns_t * _obj, void *output_baton, apr_off_t original_start, apr_off_t original_length, apr_off_t modified_start, apr_off_t modified_length, apr_off_t latest_start, apr_off_t latest_length) {
  return (_obj->output_diff_latest)(output_baton, original_start, original_length, modified_start, modified_length, latest_start, latest_length);
}

static svn_error_t * svn_diff_output_fns_invoke_output_diff_common(
  svn_diff_output_fns_t * _obj, void *output_baton, apr_off_t original_start, apr_off_t original_length, apr_off_t modified_start, apr_off_t modified_length, apr_off_t latest_start, apr_off_t latest_length) {
  return (_obj->output_diff_common)(output_baton, original_start, original_length, modified_start, modified_length, latest_start, latest_length);
}

static svn_error_t * svn_diff_output_fns_invoke_output_conflict(
  svn_diff_output_fns_t * _obj, void *output_baton, apr_off_t original_start, apr_off_t original_length, apr_off_t modified_start, apr_off_t modified_length, apr_off_t latest_start, apr_off_t latest_length, svn_diff_t *resolved_diff) {
  return (_obj->output_conflict)(output_baton, original_start, original_length, modified_start, modified_length, latest_start, latest_length, resolved_diff);
}

%}

#ifdef SWIGPYTHON
%funcptr_member_proxy(svn_diff_fns_t, datasource_open, svn_diff_fns_invoke_datasource_open);
%funcptr_member_proxy(svn_diff_fns_t, datasource_close, svn_diff_fns_invoke_datasource_close);
%funcptr_member_proxy(svn_diff_fns_t, datasource_get_next_token, svn_diff_fns_invoke_datasource_get_next_token);
%funcptr_member_proxy(svn_diff_fns_t, token_compare, svn_diff_fns_invoke_token_compare);
%funcptr_member_proxy(svn_diff_fns_t, token_discard, svn_diff_fns_invoke_token_discard);
%funcptr_member_proxy(svn_diff_fns_t, token_discard_all, svn_diff_fns_invoke_token_discard_all);
%funcptr_member_proxy(svn_diff_output_fns_t, output_common, svn_diff_output_fns_invoke_output_common);
%funcptr_member_proxy(svn_diff_output_fns_t, output_diff_modified, svn_diff_output_fns_invoke_output_diff_modified);
%funcptr_member_proxy(svn_diff_output_fns_t, output_diff_latest, svn_diff_output_fns_invoke_output_diff_latest);
%funcptr_member_proxy(svn_diff_output_fns_t, output_diff_common, svn_diff_output_fns_invoke_output_diff_common);
%funcptr_member_proxy(svn_diff_output_fns_t, output_conflict, svn_diff_output_fns_invoke_output_conflict);

#endif
