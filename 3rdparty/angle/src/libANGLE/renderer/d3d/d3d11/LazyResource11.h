#pragma once

#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

namespace rx
{
    namespace d3d11
    {
        template <ResourceType ResourceT>
        class LazyResource : angle::NonCopyable
        {
        public:
            constexpr LazyResource() : mResource()
            {}
            virtual ~LazyResource()
            {}

            virtual gl::Error resolve(Renderer11 *renderer) = 0;
            void reset()
            {
                mResource.reset();
            }

            GetD3D11Type<ResourceT> *get() const
            {
                ASSERT(mResource.valid());
                return mResource.get();
            }

            const Resource11<GetD3D11Type<ResourceT>> &getObj() const
            {
                return mResource;
            }

        protected:
            LazyResource(LazyResource &&other) : mResource(std::move(other.mResource))
            {}

            gl::Error resolveImpl(Renderer11 *renderer,
                const GetDescType<ResourceT> &desc,
                GetInitDataType<ResourceT> *initData,
                const char *name)
            {
                if ( !mResource.valid() )
                {
                    ANGLE_TRY(renderer->allocateResource(desc, initData, &mResource));
                    mResource.setDebugName(name);
                }
                return gl::NoError();
            }

            Resource11<GetD3D11Type<ResourceT>> mResource;
        };

        template <typename D3D11ShaderType>
        class LazyShader final : public LazyResource<GetResourceTypeFromD3D11<D3D11ShaderType>()>
        {
        public:
            // All parameters must be constexpr. Not supported in VS2013.
            constexpr LazyShader(const BYTE *byteCode, size_t byteCodeSize, const char *name)
                : mByteCode(byteCode, byteCodeSize), mName(name)
            {}

            constexpr LazyShader(LazyShader &&shader)
                : LazyResource<GetResourceTypeFromD3D11<D3D11ShaderType>()>(std::move(shader)),
                mByteCode(std::move(shader.mByteCode)),
                mName(shader.mName)
            {}

            gl::Error resolve(Renderer11 *renderer) override
            {
                return this->resolveImpl(renderer, mByteCode, nullptr, mName);
            }

        private:
            ShaderData mByteCode;
            const char *mName;
        };

        class LazyInputLayout final : public LazyResource<ResourceType::InputLayout>
        {
        public:
            constexpr LazyInputLayout(const D3D11_INPUT_ELEMENT_DESC *inputDesc,
                size_t inputDescLen,
                const BYTE *byteCode,
                size_t byteCodeLen,
                const char *debugName)
                : mInputDesc(inputDesc, inputDescLen),
                mByteCode(byteCode, byteCodeLen),
                mDebugName(debugName)
            {}

            gl::Error resolve(Renderer11 *renderer) override;

        private:
            InputElementArray mInputDesc;
            ShaderData mByteCode;
            const char *mDebugName;
        };

        class LazyBlendState final : public LazyResource<ResourceType::BlendState>
        {
        public:
            LazyBlendState(const D3D11_BLEND_DESC &desc, const char *debugName);

            gl::Error resolve(Renderer11 *renderer) override;

        private:
            D3D11_BLEND_DESC mDesc;
            const char *mDebugName;
        };
    }
}