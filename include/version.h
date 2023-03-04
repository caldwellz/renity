/****************************************************
 * version.h: Project version / revision            *
 * Copyright (C) 2021-2023 Zach Caldwell            *
 ****************************************************
 * This Source Code Form is subject to the terms of *
 * the Mozilla Public License, v. 2.0. If a copy of *
 * the MPL was not distributed with this file, You  *
 * can obtain one at http://mozilla.org/MPL/2.0/.   *
 ***************************************************/

#ifndef RENITY_VERSION_H_
#define RENITY_VERSION_H_

#include <config.h>

#ifdef __cplusplus
namespace renity {
extern "C" {
#endif  //__cplusplus
extern RENITY_API const char *PUBLISHER_NAME;
extern RENITY_API const char *PRODUCT_NAME;
extern RENITY_API const char *PRODUCT_DESCRIPTION;
extern RENITY_API const char *PRODUCT_VERSION_STR;
extern RENITY_API const short PRODUCT_VERSION_MAJOR;
extern RENITY_API const short PRODUCT_VERSION_MINOR;
extern RENITY_API const short PRODUCT_VERSION_PATCH;
extern RENITY_API const short PRODUCT_VERSION_BUILD;
extern RENITY_API const char *PRODUCT_REVISION;
extern RENITY_API const char *PRODUCT_BUILD_TYPE;
extern RENITY_API const char *PRODUCT_COMPILER;
extern RENITY_API const char *PRODUCT_COMPILER_VERSION;
#ifdef __cplusplus
}
}
#endif  // __cplusplus
#endif  // RENITY_VERSION_H_
