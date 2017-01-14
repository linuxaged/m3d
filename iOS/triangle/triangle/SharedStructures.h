//
//  SharedStructures.h
//  triangle
//
//  Created by Tracy on 2017/1/15.
//  Copyright (c) 2017年 __MyCompanyName__. All rights reserved.
//

#ifndef SharedStructures_h
#define SharedStructures_h

#include <simd/simd.h>

typedef struct
{
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
} uniforms_t;

#endif /* SharedStructures_h */

