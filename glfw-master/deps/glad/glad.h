/* ==========================================================================
 * glad1 -> glad2 compatibility shim
 *
 * LearnOpenGL and many books use the glad1 API:
 *     #include <glad/glad.h>
 *     gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
 *
 * This project uses glad2 (header-only, deps/glad/gl.h). This shim lets that
 * older code compile unchanged by forwarding to the glad2 API.
 * New code should prefer glad2 directly:
 *     #include <glad/gl.h>
 *     if (!gladLoadGL(glfwGetProcAddress)) { /-* error *-/ }
 * ========================================================================== */
#ifndef GLAD_GLAD_H_
#define GLAD_GLAD_H_

#include <glad/gl.h>

typedef GLADloadfunc GLADloadproc;

/* glad1 returned non-zero on success; glad2 returns the loaded GL version
 * (0 on failure) - the comparison preserves glad1 semantics. */
static int gladLoadGLLoader(GLADloadproc load)
{
    return gladLoadGL(load) != 0;
}

#endif /* GLAD_GLAD_H_ */
