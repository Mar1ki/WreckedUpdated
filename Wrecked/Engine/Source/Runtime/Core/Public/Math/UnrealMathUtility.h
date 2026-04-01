#pragma once
#include <cstdint>

using int8   = int8_t;
using uint8  = uint8_t;
using int16  = int16_t;
using uint16 = uint16_t;
using int32  = int32_t;
using uint32 = uint32_t;
using int64  = int64_t;
using uint64 = uint64_t;

#undef  PI
#define PI 					(3.1415926535897932f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)

#define MAX_FLT 3.402823466e+38F

#define INV_PI			(0.31830988618f)
#define HALF_PI			(1.57079632679f)

#define DELTA			(0.00001f)

#define FLOAT_NORMAL_THRESH				(0.0001f)

#define THRESH_POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.00002f)	/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
#define THRESH_UVS_ARE_SAME			    (0.0009765625f)/* Two UV are same if within this threshold (1.0f/1024f) */
/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
                                                    /* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_NORMALS_ARE_PARALLEL		(0.999845f)	/* Two unit vectors are parallel if abs(A dot B) is greater than or equal to this. This is roughly cosine(1.0 degrees). */
#define THRESH_NORMALS_ARE_ORTHOGONAL	(0.017455f)	/* Two unit vectors are orthogonal (perpendicular) if abs(A dot B) is less than or equal this. This is roughly cosine(89.0 degrees). */

#define THRESH_VECTOR_NORMALIZED		(0.01f)		/** Allowed error for a normalized vector (against squared magnitude) */
#define THRESH_QUAT_NORMALIZED			(0.01f)

struct FMath
{
    static FORCEINLINE float Abs(float Value) { return fabsf(Value); }
    static FORCEINLINE float Sin(float Value) { return sinf(Value); }
    static FORCEINLINE float Asin(float Value) { return asinf((Value < -1.0f) ? -1.0f : ((Value < 1.0f) ? Value : 1.0f)); }
    static FORCEINLINE float Sinh(float Value) { return sinhf(Value); }
    static FORCEINLINE float Cos(float Value) { return cosf(Value); }
    static FORCEINLINE float Acos(float Value) { return acosf((Value < -1.0f) ? -1.0f : ((Value < 1.0f) ? Value : 1.0f)); }
    static FORCEINLINE float Tan(float Value) { return tanf(Value); }
    static FORCEINLINE float Atan(float Value) { return atanf(Value); }
    static FORCEINLINE float Sqrt(float Value) { return sqrtf(Value); }
    static FORCEINLINE float Pow(float A, float B) { return powf(A, B); }
    static FORCEINLINE float InvSqrt(float F) { return 1.0f / sqrtf(F); }
    static FORCEINLINE float InvSqrtEst(float F) { return InvSqrt(F); }
    static FORCEINLINE float Lerp(float A, float B, float Alpha) { return A + Alpha * (B - A); }

    static FORCEINLINE bool IsNaN(float A) { return ((*(uint32*)&A) & 0x7FFFFFFF) > 0x7F800000; }
    static FORCEINLINE bool IsFinite(float A) { return ((*(uint32*)&A) & 0x7F800000) != 0x7F800000; }
    static FORCEINLINE bool IsNegativeFloat(const float& A) { return ((*(uint32*)&A) >= (uint32)0x80000000); }
    static FORCEINLINE bool IsNegativeDouble(const double& A) { return ((*(uint64*)&A) >= (uint64)0x8000000000000000); }

    static FORCEINLINE int32 Rand() { return rand(); }
    static FORCEINLINE void RandInit(int32 Seed) { srand(Seed); }
    static FORCEINLINE float FRand() { return Rand() / (float)RAND_MAX; }
    static FORCEINLINE float FRandRange(float InMin, float InMax) { return InMin + (InMax - InMin) * FRand(); }

    static FORCEINLINE int32 RandHelper(int32 A)
    {
        return A > 0 ? Min(TruncToInt(FRand() * A), A - 1) : 0;
    }

    static FORCEINLINE int32 RandRange(int32 Min, int32 Max)
    {
        const int32 Range = (Max - Min) + 1;
        return Min + RandHelper(Range);
    }

    static FORCEINLINE bool IsNearlyEqual(float A, float B, float Tolerance = KINDA_SMALL_NUMBER)
    {
        return FMath::Abs(A - B) <= Tolerance;
    }

    static FORCEINLINE bool IsNearlyZero(float Value, float Tolerance = KINDA_SMALL_NUMBER)
    {
        return FMath::Abs(Value) <= Tolerance;
    }

    template< class T = int32 >
    static FORCEINLINE T Min(const T A, const T B) { return (A <= B) ? A : B; }

    template< class T = int32 >
    static FORCEINLINE T Max(const T A, const T B) { return (A >= B) ? A : B; }

    template< class T = float >
    static FORCEINLINE T Square(const T A) { return A * A; }

    template< class T = float >
    static FORCEINLINE T Clamp(const T X, const T Min, const T Max) { return X < Min ? Min : X < Max ? X : Max; }

    static FORCEINLINE int32 TruncToInt(float F) { return (int32)F; }
    static FORCEINLINE float TruncToFloat(float F) { return (float)TruncToInt(F); }

    static FORCEINLINE int32 FloorToInt(float F) { return TruncToInt(floorf(F)); }
    static FORCEINLINE int32 RoundToInt(float F) { return FloorToInt(F + 0.5f); }

    static FORCEINLINE int32 CeilToInt(float F) { return TruncToInt(ceilf(F)); }
    static FORCEINLINE float CeilToFloat(float F) { return ceilf(F); }
};