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

#include "console/consoleTypes.h"
#include "floatNode.h"
#include "floatNode_Binding.h"

namespace Materials
{
   IMPLEMENT_CONOBJECT(FloatNode);

   FloatNode::FloatNode()
   {
      type = "Float";
      mUniformName = StringTable->insert("");
      mValue = 0.0f;
   }

   void FloatNode::initPersistFields()
   {
      // Call parent.
      Parent::initPersistFields();

      addField("UniformName", TypeString, Offset(mUniformName, FloatNode), "");
      addField("Value", TypeF32, Offset(mValue, FloatNode), "");
   }

   void FloatNode::generatePixel(const MaterialGenerationSettings &settings, ReturnType refType, U32 flags)
   {
      if ( dStrlen(mUniformName) > 0 )
      {
         MaterialTemplate* matTemplate = settings.matTemplate;

         Rendering::UniformData* uniform = matTemplate->uniforms.addUniform();
         uniform->count = 1;
         uniform->uniform = Graphics::Shader::getUniform(mUniformName, bgfx::UniformType::Vec4);
         uniform->setValue(mValue);

         matTemplate->addPixelHeader("uniform vec4 %s;", mUniformName);
      }
   }

   const char* FloatNode::getPixelReference(const MaterialGenerationSettings &settings, ReturnType refType, U32 flags)
   {
      if ( dStrlen(mUniformName) > 0 )
         dSprintf(mReturnBuf, 64, "%s.x", mUniformName);
      else
         dSprintf(mReturnBuf, 64, "%f", mValue);

      return mReturnBuf;
   }
}