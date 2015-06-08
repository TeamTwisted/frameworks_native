
#include <cutils/log.h>
#include <cutils/properties.h>

#include <utils/CallStack.h>

#include "egl_tls.h"
#include "egldefs.h"
#include "egl_object.h"
#include "protect.h"

#ifdef USE_EGL_CONTEXT_PROTECTION

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

static gl_hooks_t gProtectedHooks[2];

static void log_call_stack() {
    char value[PROPERTY_VALUE_MAX];
    property_get("debug.egl.callstack", value, "0");
    if (atoi(value)) {
        CallStack stack(LOG_TAG);
    }
}

#define PROTECTED_GL_CALL(_api, _args, _argList, _type, _return) \
static _type Protected_ ## _api _args { \
    ALOGE("Error: Called %s function in a protected EGL context", #_api); \
    log_call_stack(); \
    egl_context_t * const c = get_context(egl_tls_t::getContext()); \
    if (c && !c->block) return c->getUnprotectedHooks()->gl._api _argList; \
    else ALOGE("Call to %s blocked", #_api); \
    _return; \
}

#define PROTECTED_GL_VOID(_api, _args, _argList) \
    PROTECTED_GL_CALL(_api, _args, _argList, void, )

#define PROTECTED_GL(_type, _api, _args, _argList) \
    PROTECTED_GL_CALL(_api, _args, _argList, _type, return 0)

#include "../protected.in"

#undef PROTECTED_GL_VOID
#undef PROTECTED_GL

void egl_init_context_protection(egl_connection_t* cnx) {
    // copy the regular hooks into gProtectedHooks
    gProtectedHooks[egl_connection_t::GLESv1_INDEX] =
        gHooks[egl_connection_t::GLESv1_INDEX];
    gProtectedHooks[egl_connection_t::GLESv2_INDEX] =
        gHooks[egl_connection_t::GLESv2_INDEX];

    cnx->protectedHooks[egl_connection_t::GLESv1_INDEX] =
        &gProtectedHooks[egl_connection_t::GLESv1_INDEX];
    cnx->protectedHooks[egl_connection_t::GLESv2_INDEX] =
        &gProtectedHooks[egl_connection_t::GLESv2_INDEX];

    gl_hooks_t* h;

#define PROTECTED_GL_VOID(_api, _args, _argList) h->gl._api = Protected_ ## _api;
#define PROTECTED_GL(_type, _api, _args, _argList) h->gl._api = Protected_ ## _api;

    // GLES 1.0 calls
    h = &gProtectedHooks[egl_connection_t::GLESv1_INDEX];
    #include "../protected.in"

    // GLES 2.0 calls
    h = &gProtectedHooks[egl_connection_t::GLESv2_INDEX];
    #include "../protected.in"

#undef PROTECTED_GL_VOID
#undef PROTECTED_GL
}

bool egl_context_set_protect(egl_context_t* ctx, bool b) {
    if (ctx == NULL) {
        ALOGE("no current context");
        return false;
    }
    if ((!b && !ctx->protect) || (b && ctx->protect)) {
        if (b)
            ALOGW("attempt to protect already protected context");
        else
            ALOGW("attempt to unprotect already unprotected context");
        char value[PROPERTY_VALUE_MAX];
        property_get("debug.egl.callstack", value, "0");
        if (atoi(value)) {
            CallStack stack(LOG_TAG);
        }
        return false;
    }
    ctx->protect = b;
    return true;
}

// ----------------------------------------------------------------------------
}; // namespace android
// ----------------------------------------------------------------------------

#endif
