/* Proxy classes for svn_version.h
 * DO NOT EDIT -- AUTOMATICALLY GENERATED */

/* No default constructors for opaque structs */
#ifdef SWIGPYTHON
#endif

/* Callback typemaps */

/* Baton typemaps */

/* Includes */
%{
#include "svn_version.h"
%}
%include svn_version.h

/* Structure definitions */
#ifdef SWIGPYTHON
%proxy(svn_version_t);
%proxy(svn_version_checklist_t);
#endif

/* Callbacks */

%inline %{
static const svn_version_t * svn_version_checklist_invoke_version_query(
  svn_version_checklist_t * _obj) {
  return (_obj->version_query)();
}

%}

#ifdef SWIGPYTHON
%funcptr_member_proxy(svn_version_checklist_t, version_query, svn_version_checklist_invoke_version_query);

#endif
