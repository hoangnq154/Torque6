//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------


#ifndef _RENDER_CAMERA_H_
#define _RENDER_CAMERA_H_

#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif

#ifndef _RENDERING_H_
#include "rendering/rendering.h"
#endif

#ifndef BGFX_H_HEADER_GUARD
#include <bgfx/bgfx.h>
#endif

#ifndef _TRANSPARENCY_H_
#include "rendering/transparency.h"
#endif

namespace Rendering 
{
   class DLL_PUBLIC RenderCamera;
   class DLL_PUBLIC RenderPath;

   // ----------------------------------------
   //  RenderPostProcess : Post Processing effect specific to the camera it's assigned to.
   // ----------------------------------------
   // Post Process
   struct DLL_PUBLIC RenderPostProcess
   {
      RenderCamera*  mCamera;
      S16            mPriority;

      RenderPostProcess() : mCamera(NULL), mPriority(0) { }

      virtual void onAddToCamera()        { }
      virtual void onRemoveFromCamera()   { }

      virtual void process() { }
   };

   // ----------------------------------------
   //  Render Cameras
   // ----------------------------------------
   class DLL_PUBLIC RenderCamera
   {
      protected:
         struct CommonUniforms
         {
            bgfx::UniformHandle camPos;
            bgfx::UniformHandle time;
            bgfx::UniformHandle sceneViewMat;
            bgfx::UniformHandle sceneInvViewMat;
            bgfx::UniformHandle sceneProjMat;
            bgfx::UniformHandle sceneInvProjMat;
            bgfx::UniformHandle sceneViewProjMat;
            bgfx::UniformHandle sceneInvViewProjMat;
         } mCommonUniforms;

         StringTableEntry           mName;
         S16                        mPriority;
         bool                       mInitialized;

         bool                       mBeginEnabled;
         Graphics::Shader*          mBeginShader;
         Graphics::ViewTableEntry*  mBeginView;

         bool                       mFinishEnabled;
         Graphics::Shader*          mFinishShader;
         Graphics::ViewTableEntry*  mFinishView;
         bgfx::FrameBufferHandle    mFinishBuffer;

         StringTableEntry           mRenderTextureName;
         Vector<RenderPostProcess*> mRenderPostProcessList;

         void initBuffers();
         void destroyBuffers();
         void setCommonUniforms();

      public:
         F32 nearPlane;
         F32 farPlane;
         F32 viewMatrix[16];
         F32 projectionMatrix[16];
         F32 projectionWidth;
         F32 projectionHeight;
         Point3F position;

         RenderPath*    mRenderPath;
         Transparency*  mTransparency;

         RenderCamera();
         ~RenderCamera();

         // Post Processing
         U32                        mPostBufferIdx;
         bgfx::FrameBufferHandle    mPostBuffers[2];
         bgfx::FrameBufferHandle    getPostSource();
         bgfx::FrameBufferHandle    getPostTarget();
         Graphics::ViewTableEntry*  overrideBegin();
         Graphics::ViewTableEntry*  overrideFinish();
         void flipPostBuffers();
         void freeBegin();
         void freeFinish();
         void addRenderPostProcess(RenderPostProcess* postProcess);
         bool removeRenderPostProcess(RenderPostProcess* postProcess);
         Vector<RenderPostProcess*>* getRenderPostProcessList();

         // Render Targets
         bgfx::FrameBufferHandle getBackBuffer();
         bgfx::TextureHandle     getColorTexture();
         bgfx::TextureHandle     getDepthTexture();
         bgfx::TextureHandle     getDepthTextureRead();
         bgfx::TextureHandle     getNormalTexture();
         bgfx::TextureHandle     getMatInfoTexture();

         StringTableEntry getName();
         void setName(StringTableEntry name);
         StringTableEntry getRenderTextureName();
         void setRenderTextureName(StringTableEntry name);
         S16 getRenderPriority();
         void setRenderPriority(S16 priority);

         virtual void render();
         virtual void postProcess();
   };

   // ----------------------------------------
   //  RenderPath : Implementations of rendering methods such as Forward, Deferred Shading, etc.
   // ----------------------------------------
   class DLL_PUBLIC RenderPath
   {
      protected:
         bool           mInitialized;
         RenderCamera*  mCamera;

      public:
         RenderPath(RenderCamera* camera)
            : mInitialized(false),
              mCamera(camera)
         {
            //
         }

         virtual void preRender()   { }
         virtual void render()      { }
         virtual void postRender()  { }
         virtual void resize()      { }

         // Render Targets
         virtual bgfx::FrameBufferHandle getBackBuffer()       { bgfx::FrameBufferHandle fbh; fbh.idx = bgfx::invalidHandle; return fbh; }
         virtual bgfx::TextureHandle     getColorTexture()     { bgfx::TextureHandle th; th.idx = bgfx::invalidHandle; return th; }
         virtual bgfx::TextureHandle     getDepthTexture()     { bgfx::TextureHandle th; th.idx = bgfx::invalidHandle; return th; }
         virtual bgfx::TextureHandle     getDepthTextureRead() { bgfx::TextureHandle th; th.idx = bgfx::invalidHandle; return th; }
         virtual bgfx::TextureHandle     getNormalTexture()    { bgfx::TextureHandle th; th.idx = bgfx::invalidHandle; return th; }
         virtual bgfx::TextureHandle     getMatInfoTexture()   { bgfx::TextureHandle th; th.idx = bgfx::invalidHandle; return th; }
   };

   typedef Rendering::RenderPath* (*_CreateRenderPathFunc_)(Rendering::RenderCamera*);
   void registerRenderPath(const char* renderPathName, _CreateRenderPathFunc_ createFuncPtr);

   class DLL_PUBLIC _RenderPathRegister_
   {
      public:
         _RenderPathRegister_(const char* renderPathName, _CreateRenderPathFunc_ createFuncPtr)
         {
            Rendering::registerRenderPath(renderPathName, createFuncPtr);
         }
   };

   // Creates an instance of a RenderPath class registered with renderPathName.
   Rendering::RenderPath* getRenderPathInstance(const char* renderPathName, Rendering::RenderCamera* camera);
}

// ----------------------------------------
//  Render Path Macros : used to register a class as a render path.
// ----------------------------------------

// The Render Path Macros below create a static function in each RenderPath class that creates an instance of itself and returns it.
// A static _RenderPathRegister_ class is created as well which registers the create function pointer in a hashmap. We can then use
// Rendering::getRenderPathInstance() with an arbitrary string to fetch an instance of a RenderPath for a RenderCamera to use.

#define IMPLEMENT_RENDER_PATH(renderPathName, renderPathClass)                                                                                     \
    Rendering::RenderPath* renderPathClass::renderPathClass##CreateFunc(Rendering::RenderCamera* camera) { return new renderPathClass(camera); }   \
    Rendering::_RenderPathRegister_ renderPathClass::renderPathClass##Register(renderPathName, &renderPathClass::renderPathClass##CreateFunc)

#define DECLARE_RENDER_PATH(renderPathName, renderPathClass)                              \
   static Rendering::RenderPath* renderPathClass##CreateFunc(Rendering::RenderCamera*);   \
   static Rendering::_RenderPathRegister_ renderPathClass##Register

#endif