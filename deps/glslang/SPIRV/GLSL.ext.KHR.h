/*
** Copyright (c) 2014-2016 The Khronos Group Inc.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and/or associated documentation files (the "Materials"),
** to deal in the Materials without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Materials, and to permit persons to whom the
** Materials are furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Materials.
**
** MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS KHRONOS
** STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS SPECIFICATIONS AND
** HEADER INFORMATION ARE LOCATED AT https://www.khronos.org/registry/
**
** THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
** THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM,OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS
** IN THE MATERIALS.
*/

#ifndef GLSLextARB_H
#define GLSLextARB_H

enum BuiltIn;

static const int GLSLextARBVersion = 100;
static const int GLSLextARBRevision = 1;

// SPV_ARB_shader_ballot
static const char* const E_SPV_ARB_shader_ballot = "SPV_ARB_shader_ballot";

static const BuiltIn BuiltInSubGroupSizeARB        = static_cast<BuiltIn>(4416);
static const BuiltIn BuiltInSubGroupInvocationARB  = static_cast<BuiltIn>(4417);
static const BuiltIn BuiltInSubGroupEqMaskARB      = static_cast<BuiltIn>(4418);
static const BuiltIn BuiltInSubGroupGeMaskARB      = static_cast<BuiltIn>(4419);
static const BuiltIn BuiltInSubGroupGtMaskARB      = static_cast<BuiltIn>(4420);
static const BuiltIn BuiltInSubGroupLeMaskARB      = static_cast<BuiltIn>(4421);
static const BuiltIn BuiltInSubGroupLtMaskARB      = static_cast<BuiltIn>(4422);

enum ShaderBallotARB {
    BallotARB = 0,
    ReadInvocationARB = 1,
    ReadFirstInvocationARB = 2,

    ShaderBallotCountARB
};

#endif  // #ifndef GLSLextARB_H
