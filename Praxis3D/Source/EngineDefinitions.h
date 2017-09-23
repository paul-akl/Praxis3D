
// Preprocessor definitions as various settings for the engine.
// Affects engine only when recompiled, to eliminate run-time checks.

// Enable multithreading
#define SETTING_MULTITHREADING_ENABLED
#define SETTING_ATOMIC_VARIABLES_ENABLED

// Use glBlitFramebuffer to copy the final buffer to the default back-buffer, instead of rendering a full-screen triangle
//#define SETTING_USE_BLIT_FRAMEBUFFER