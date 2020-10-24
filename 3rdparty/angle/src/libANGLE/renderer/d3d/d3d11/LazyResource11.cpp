#include "LazyResource11.h"

namespace rx
{
    namespace d3d11
    {
        gl::Error LazyInputLayout::resolve(Renderer11 *renderer)
        {
            return resolveImpl(renderer, mInputDesc, &mByteCode, mDebugName);
        }
        
        LazyBlendState::LazyBlendState(const D3D11_BLEND_DESC &desc, const char *debugName)
            : mDesc(desc), mDebugName(debugName)
        {
        }

        gl::Error LazyBlendState::resolve(Renderer11 *renderer)
        {
            return resolveImpl(renderer, mDesc, nullptr, mDebugName);
        }
    }
}
