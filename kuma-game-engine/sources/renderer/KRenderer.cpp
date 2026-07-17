#include "KRenderer.h"

#include <glad/gl.h>

namespace kuma::renderer
{

void KRenderer::Render()
{
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace kuma::renderer
