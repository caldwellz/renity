/****************************************************
 * GL_VertShader.h: GL vertex shader loader         *
 * Copyright (C) 2023 by Zach Caldwell              *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/
#pragma once

#include "GL_Shader.h"

namespace renity {
class RENITY_API GL_VertShader : public GL_Shader {
 public:
  inline GL_VertShader() : GL_Shader(GL_VERTEX_SHADER) {}
};
using GL_VertShaderPtr = SharedPtr<GL_VertShader>;
}  // namespace renity
