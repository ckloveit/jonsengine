#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 6.3.9600.16384
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// gRawAOTexture                     texture  float4          2d    3        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Position              0   xyzw        0      POS   float   xy  
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_Target                0   x           0   TARGET   float   x   
//
ps_5_0
dcl_globalFlags refactoringAllowed
dcl_resource_texture2d (float,float,float,float) t3
dcl_input_ps_siv linear noperspective v0.xy, position
dcl_output o0.x
dcl_temps 2
mov r0.zw, l(0,0,0,0)
mov r1.xy, l(0,-3,0,0)
loop 
  ilt r1.z, l(3), r1.y
  breakc_nz r1.z
  itof r1.z, r1.y
  add r1.zw, r1.zzzz, v0.xxxy
  ftou r0.xy, r1.zwzz
  ld_indexable(texture2d)(float,float,float,float) r0.x, r0.xyzw, t3.xyzw
  add r1.x, r0.x, r1.x
  iadd r1.y, r1.y, l(1)
endloop 
mul o0.x, r1.x, l(0.142857)
ret 
// Approximately 14 instruction slots used
#endif

const BYTE gBoxBlurPixelShader[] =
{
     68,  88,  66,  67,  91,  28, 
     67,  75, 241, 203,  88,  24, 
    151, 233, 155,  65, 112, 212, 
     46, 113,   1,   0,   0,   0, 
     84,   3,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
    216,   0,   0,   0,  12,   1, 
      0,   0,  64,   1,   0,   0, 
    184,   2,   0,   0,  82,  68, 
     69,  70, 156,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
    255, 255,   0,   1,   0,   0, 
    106,   0,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,   2,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   3,   0,   0,   0, 
      1,   0,   0,   0,  13,   0, 
      0,   0, 103,  82,  97, 119, 
     65,  79,  84, 101, 120, 116, 
    117, 114, 101,   0,  77, 105, 
     99, 114, 111, 115, 111, 102, 
    116,  32,  40,  82,  41,  32, 
     72,  76,  83,  76,  32,  83, 
    104,  97, 100, 101, 114,  32, 
     67, 111, 109, 112, 105, 108, 
    101, 114,  32,  54,  46,  51, 
     46,  57,  54,  48,  48,  46, 
     49,  54,  51,  56,  52,   0, 
     73,  83,  71,  78,  44,   0, 
      0,   0,   1,   0,   0,   0, 
      8,   0,   0,   0,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,   3,   0,   0,  83,  86, 
     95,  80, 111, 115, 105, 116, 
    105, 111, 110,   0,  79,  83, 
     71,  78,  44,   0,   0,   0, 
      1,   0,   0,   0,   8,   0, 
      0,   0,  32,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   1,  14, 
      0,   0,  83,  86,  95,  84, 
     97, 114, 103, 101, 116,   0, 
    171, 171,  83,  72,  69,  88, 
    112,   1,   0,   0,  80,   0, 
      0,   0,  92,   0,   0,   0, 
    106,   8,   0,   1,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      3,   0,   0,   0,  85,  85, 
      0,   0, 100,  32,   0,   4, 
     50,  16,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
    101,   0,   0,   3,  18,  32, 
     16,   0,   0,   0,   0,   0, 
    104,   0,   0,   2,   2,   0, 
      0,   0,  54,   0,   0,   8, 
    194,   0,  16,   0,   0,   0, 
      0,   0,   2,  64,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   8,  50,   0,  16,   0, 
      1,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
    253, 255, 255, 255,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     48,   0,   0,   1,  34,   0, 
      0,   7,  66,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   3,   0,   0,   0, 
     26,   0,  16,   0,   1,   0, 
      0,   0,   3,   0,   4,   3, 
     42,   0,  16,   0,   1,   0, 
      0,   0,  43,   0,   0,   5, 
     66,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   7, 194,   0,  16,   0, 
      1,   0,   0,   0, 166,  10, 
     16,   0,   1,   0,   0,   0, 
      6,  20,  16,   0,   0,   0, 
      0,   0,  28,   0,   0,   5, 
     50,   0,  16,   0,   0,   0, 
      0,   0, 230,  10,  16,   0, 
      1,   0,   0,   0,  45,   0, 
      0, 137, 194,   0,   0, 128, 
     67,  85,  21,   0,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 126,  16,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   7,  18,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  30,   0,   0,   7, 
     34,   0,  16,   0,   1,   0, 
      0,   0,  26,   0,  16,   0, 
      1,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     22,   0,   0,   1,  56,   0, 
      0,   7,  18,  32,  16,   0, 
      0,   0,   0,   0,  10,   0, 
     16,   0,   1,   0,   0,   0, 
      1,  64,   0,   0,  37,  73, 
     18,  62,  62,   0,   0,   1, 
     83,  84,  65,  84, 148,   0, 
      0,   0,  14,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      3,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0
};
