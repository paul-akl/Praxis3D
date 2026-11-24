
// Preprocessor definitions as various settings for the engine.
// Affects engine only when recompiled, to eliminate run-time checks.

// Enable multithreading
#define SETTING_MULTITHREADING_ENABLED 1
#define SETTING_ATOMIC_VARIABLES_ENABLED 1

// Use glBlitFramebuffer to copy the final buffer to the default back-buffer, instead of rendering a full-screen triangle
//#define SETTING_USE_BLIT_FRAMEBUFFER

// ImGui File Browser Dialog settings
#define okCancelButtonAlignement 1.0f
#define okButtonWidth 100.0f
#define cancelButtonWidth 100.0f
#define IMGUI_DEFINE_MATH_OPERATORS

// Physics Collision Event Component array sizes
#define NUM_DYNAMIC_COLLISION_EVENTS 100u
#define NUM_STATIC_COLLISION_EVENTS 200u

// Shadow mapping settings
#define CSM_USE_MULTILAYER_DRAW 1

// Loaders settings
#define SETTING_LOADER_RESERVE_SIZE 200