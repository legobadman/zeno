#pragma once

#include <vector_types.h>

ushort2 toHalf(const float2& in);
ushort3 toHalf(const float3& in);
ushort3 toHalf(const float4& in);

float3  toFloat(ushort3 in);
float  toFloat(ushort1 in);