#ifndef CONSTANTS_HLSL
#define CONSTANTS_HLSL

// Convention: instances named with _EXTRA may be wiped inbetween every pass

// TODO: refactor this into a more efficient system
#define CBUFFER_SLOT_VERTEX 0
#define CBUFFER_SLOT_PIXEL 1
#define CBUFFER_SLOT_COMPUTE 2
#define CBUFFER_SLOT_DOMAIN 3
#define CBUFFER_SLOT_HULL 4
#define CBUFFER_SLOT_PER_FRAME 5
#define CBUFFER_SLOT_EXTRA 6
#define CBUFFER_REGISTER_VERTEX b0
#define CBUFFER_REGISTER_PIXEL b1
#define CBUFFER_REGISTER_COMPUTE b2
#define CBUFFER_REGISTER_DOMAIN b3
#define CBUFFER_REGISTER_HULL b4
#define CBUFFER_REGISTER_PER_FRAME b5
#define CBUFFER_REGISTER_EXTRA b6

#define SAMPLER_SLOT_ANISOTROPIC 0
#define SAMPLER_SLOT_POINT 1
#define SAMPLER_SLOT_POINT_COMPARE 2
#define SAMPLER_SLOT_LINEAR 3
#define SAMPLER_SLOT_LINEAR_WRAP 4
#define SAMPLER_REGISTER_ANISOTROPIC s0
#define SAMPLER_REGISTER_POINT s1
#define SAMPLER_REGISTER_POINT_COMPARE s2
#define SAMPLER_REGISTER_LINEAR s3
#define SAMPLER_REGISTER_LINEAR_WRAP s4

#define TEXTURE_SLOT_DIFFUSE 0
#define TEXTURE_SLOT_NORMAL 1
#define TEXTURE_SLOT_DEPTH 2
#define TEXTURE_SLOT_PERLIN 3
#define TEXTURE_SLOT_EXTRA 4
#define SBUFFER_SLOT_BONE_TRANSFORMS 4
#define SBUFFER_SLOT_EXTRA 5
#define TEXTURE_REGISTER_DIFFUSE t0
#define TEXTURE_REGISTER_NORMAL t1
#define TEXTURE_REGISTER_DEPTH t2
#define TEXTURE_REGISTER_PERLIN t3
#define TEXTURE_REGISTER_EXTRA t4
#define SBUFFER_REGISTER_BONE_TRANSFORMS t4
#define SBUFFER_REGISTER_EXTRA t5

#define UAV_SLOT 0
#define UAV_REGISTER u0

#define SDSM_THREAD_GROUP_SIZE 16
#define SDSM_NUM_THREADS (SDSM_THREAD_GROUP_SIZE * SDSM_THREAD_GROUP_SIZE)

#define NUM_BONES_PER_VERTEX 4

#define LUM_MAP_WIDTH 1024
#define LUM_MAP_HEIGHT 1024

#endif