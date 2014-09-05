
#ifdef USE_EGL_CONTEXT_PROTECTION

#define PROPERTY_PROTECT_BLOCK "hwui.protect.block"

namespace android {

void egl_init_context_protection(egl_connection_t*);
bool egl_context_set_protect(egl_context_t* ctx, bool b);

} // end namespace

#endif

