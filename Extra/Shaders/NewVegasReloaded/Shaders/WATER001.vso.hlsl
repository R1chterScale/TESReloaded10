//
// Generated by Microsoft (R) HLSL Shader Compiler 9.23.949.2378
//
// Parameters:

row_major float4x4 ModelViewProj : register(c0);
row_major float4x4 WorldMat : register(c4);
float TexScale : register(c8);
float4 QPosAdjust : register(c9);
float ObjectUV : register(c10);

// Registers:
//
//   Name          Reg   Size
//   ------------- ----- ----
//   ModelViewProj[0] const_0        1
//   ModelViewProj[1] const_1        1
//   ModelViewProj[2] const_2        1
//   ModelViewProj[3] const_3        1
//   WorldMat      const_4       3
//   TexScale      const_8       1
//   QPosAdjust    const_9       1
//   ObjectUV      const_10      1
//


// Structures:

struct VS_INPUT {
    float4 LPOSITION : POSITION;
    float4 LCOLOR_0 : COLOR0;
    float4 LTEXCOORD_0 : TEXCOORD0;
};

struct VS_OUTPUT {
    float4 position : POSITION;
    float4 texcoord_0 : TEXCOORD0;
    float4 texcoord_1 : TEXCOORD1;
    float4 texcoord_2 : TEXCOORD2;
    float4 texcoord_3 : TEXCOORD3;
    float4 texcoord_4 : TEXCOORD4;
    float4 texcoord_5 : TEXCOORD5;
    float4 texcoord_6 : TEXCOORD6;
    float4 texcoord_7 : TEXCOORD7;
};

// Code:

VS_OUTPUT main(VS_INPUT IN) {
    VS_OUTPUT OUT;

#define	weight(v)		dot(v, 1)
#define	sqr(v)			((v) * (v))

    const float4 const_7 = {0.5, 0.001, 0, 0};

    float4 mdl0;
    float2 q4;
    float2 r0;
    float2 r1;

    OUT.position.xyzw = mul(ModelViewProj, IN.LPOSITION.xyzw);
    r1.y = dot(WorldMat[1].xyzw, IN.LPOSITION.xyzw);
    r1.x = dot(WorldMat[0].xyzw, IN.LPOSITION.xyzw);
    OUT.texcoord_0.w = length(WorldMat[0].xyz);
    OUT.texcoord_0.z = dot(WorldMat[2].xyzw, IN.LPOSITION.xyzw);
    OUT.texcoord_0.xy = r1.xy;
    r0.xy = const_7.xy;
    mdl0.xyzw = r0.x * ModelViewProj[3].xyzw;
    OUT.texcoord_1.xyzw = IN.LPOSITION.xyzw;
    q4.xy = (r1.xy + QPosAdjust.xy) / TexScale.x;
    OUT.texcoord_2.xyzw = (r0.x * ModelViewProj[0].xyzw) + mdl0.xyzw;
    OUT.texcoord_3.xyzw = (r0.x * ModelViewProj[1].xyzw) + mdl0.xyzw;
    OUT.texcoord_4.xyzw = (r0.x * ModelViewProj[2].xyzw) + mdl0.xyzw;
    OUT.texcoord_5.xyzw = ModelViewProj[3].xyzw;
    OUT.texcoord_6.w = IN.LCOLOR_0.w;
    OUT.texcoord_6.xyz = IN.LTEXCOORD_0.xyz;
    r0.x = (-(sqr(ObjectUV.x)) < sqr(ObjectUV.x) ? 1.0 : 0.0);
    OUT.texcoord_7.zw = 0;
    OUT.texcoord_7.xy = (r0.x * ((IN.LTEXCOORD_0.xy / (r0.y * TexScale.x)) - q4.xy)) + q4.xy;
    return OUT;
	
};