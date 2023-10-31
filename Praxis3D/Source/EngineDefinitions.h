
// Preprocessor definitions as various settings for the engine.
// Affects engine only when recompiled, to eliminate run-time checks.

// Enable multithreading
#define SETTING_MULTITHREADING_ENABLED
#define SETTING_ATOMIC_VARIABLES_ENABLED

// Use glBlitFramebuffer to copy the final buffer to the default back-buffer, instead of rendering a full-screen triangle
//#define SETTING_USE_BLIT_FRAMEBUFFER

// ImGui File Browser Dialog settings
#define okCancelButtonAlignement 1.0f
#define okButtonWidth 100.0f
#define cancelButtonWidth 100.0f
#define IMGUI_DEFINE_MATH_OPERATORS

// Physics Collision Event Component array sizes
#define NUM_DYNAMIC_COLLISION_EVENTS 10u
#define NUM_STATIC_COLLISION_EVENTS 20u
