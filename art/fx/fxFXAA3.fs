#define FXAA_GREEN_AS_LUMA 0


/*============================================================================


                    NVIDIA FXAA 3.11 by TIMOTHY LOTTES


------------------------------------------------------------------------------
COPYRIGHT (C) 2010, 2011 NVIDIA CORPORATION. ALL RIGHTS RESERVED.
------------------------------------------------------------------------------
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL NVIDIA
OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR
CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR
LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION,
OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE
THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGES.

------------------------------------------------------------------------------
                           INTEGRATION CHECKLIST
------------------------------------------------------------------------------
(1.)
In the shader source, setup defines for the desired configuration.
When providing multiple shaders (for different presets),
simply setup the defines differently in multiple files.
Example,

Etc.

(2.)
Then include this file,

  #include "Fxaa3_11.h"

(3.)
Then call the FXAA pixel shader from within your desired shader.
Look at the FXAA Quality FxaaPixelShader() for docs on inputs.
As for FXAA 3.11 all inputs for all shaders are the same 
to enable easy porting between platforms.

  return FxaaPixelShader(...);

(4.)
Insure pass prior to FXAA outputs RGBL (see next section).
Or use,

  #define FXAA_GREEN_AS_LUMA 1

(5.)
Setup engine to provide the following constants
which are used in the FxaaPixelShader() inputs,

  vec2 fxaaQualityRcpFrame,
  vec4 fxaaRcpFrameOpt,
  vec4 fxaaRcpFrameOpt2,
  float fxaaEdgeSharpness,
  float fxaaEdgeThreshold,
  float fxaaEdgeThresholdMin,
  vec4 fxaa360ConstDir

Look at the FXAA Quality FxaaPixelShader() for docs on inputs.

(6.)
Have FXAA vertex shader run as a full screen triangle,
and output "pos" and "fxaaPosPos" 
such that inputs in the pixel shader provide,

  // {xy} = center of pixel
  vec2 pos,

  // {xy__} = upper left of pixel
  // {__zw} = lower right of pixel
  vec4 fxaaPosPos,

(7.)
Insure the texture sampler(s) used by FXAA are set to bilinear filtering.


------------------------------------------------------------------------------
                    INTEGRATION - RGBL AND COLORSPACE
------------------------------------------------------------------------------
FXAA3 requires RGBL as input unless the following is set, 

  #define FXAA_GREEN_AS_LUMA 1

In which case the engine uses green in place of luma,
and requires RGB input is in a non-linear colorspace.

RGB should be LDR (low dynamic range).
Specifically do FXAA after tonemapping.

RGB data as returned by a texture fetch can be non-linear,
or linear when FXAA_GREEN_AS_LUMA is not set.
Note an "sRGB format" texture counts as linear,
because the result of a texture fetch is linear data.
Regular "RGBA8" textures in the sRGB colorspace are non-linear.

If FXAA_GREEN_AS_LUMA is not set,
luma must be stored in the alpha channel prior to running FXAA.
This luma should be in a perceptual space (could be gamma 2.0).
Example pass before FXAA where output is gamma 2.0 encoded,

  color.rgb = ToneMap(color.rgb); // linear color output
  color.rgb = sqrt(color.rgb);    // gamma 2.0 color output
  return color;

To use FXAA,

  color.rgb = ToneMap(color.rgb);  // linear color output
  color.rgb = sqrt(color.rgb);     // gamma 2.0 color output
  color.a = dot(color.rgb, vec3(0.299, 0.587, 0.114)); // compute luma
  return color;

Another example where output is linear encoded,
say for instance writing to an sRGB formated render target,
where the render target does the conversion back to sRGB after blending,

  color.rgb = ToneMap(color.rgb); // linear color output
  return color;

To use FXAA,

  color.rgb = ToneMap(color.rgb); // linear color output
  color.a = sqrt(dot(color.rgb, vec3(0.299, 0.587, 0.114))); // compute luma
  return color;

Getting luma correct is required for the algorithm to work correctly.


------------------------------------------------------------------------------
                          BEING LINEARLY CORRECT?
------------------------------------------------------------------------------
Applying FXAA to a framebuffer with linear RGB color will look worse.
This is very counter intuitive, but happends to be true in this case.
The reason is because dithering artifacts will be more visiable 
in a linear colorspace.


------------------------------------------------------------------------------
                             COMPLEX INTEGRATION
------------------------------------------------------------------------------
Q. What if the engine is blending into RGB before wanting to run FXAA?

A. In the last opaque pass prior to FXAA,
   have the pass write out luma into alpha.
   Then blend into RGB only.
   FXAA should be able to run ok
   assuming the blending pass did not any add aliasing.
   This should be the common case for particles and common blending passes.

A. Or use FXAA_GREEN_AS_LUMA.

============================================================================*/


float FxaaLuma(vec4 rgba) { return rgba.a; }

vec4 FxaaTexTop(sampler2D uSampler, vec2 pos) {
    vec4 color;
    color.rgb = texture2D(uSampler, pos).rgb;
    color.a = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    return color;
}


/*============================================================================

                             FXAA3 QUALITY - PC CONSOLE

============================================================================*/

/*--------------------------------------------------------------------------*/
vec4 FxaaPixelShader(
    //
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy} = center of pixel
    vec2 pos,
    //
    // Use noperspective interpolation here (turn off perspective interpolation).
    // {xy__} = upper left of pixel
    // {__zw} = lower right of pixel
    vec4 fxaaPosPos,
    //
    // Input color texture.
    // {rgb_} = color in linear or perceptual color space
    // if (FXAA_GREEN_AS_LUMA == 0)
    //     {___a} = luma in perceptual color space (not linear)
    sampler2D tex,
    //
    // This must be from a constant/uniform.
    // This effects sub-pixel AA quality and inversely sharpness.
    //   Where N ranges between,
    //     N = 0.50 (default)
    //     N = 0.33 (sharper)
    // {x_} =  N/screenWidthInPixels  
    // {_y} =  N/screenHeightInPixels 
    vec2 fxaaRcpFrameOpt,
    //
    // Not used on 360, but used on PS3 and PC.
    // This must be from a constant/uniform.
    // {x_} =  2.0/screenWidthInPixels  
    // {_y} =  2.0/screenHeightInPixels 
    vec2 fxaaRcpFrameOpt2,
    // 
    // This used to be the FXAA_CONSOLE__EDGE_SHARPNESS define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_SHARPNESS for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only three safe values here: 2 and 4 and 8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // For all other platforms can be a non-power of two.
    //   8.0 is sharper (default!!!)
    //   4.0 is softer
    //   2.0 is really soft (good only for vector graphics inputs)
    float fxaaEdgeSharpness,
    //
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD define.
    // It is here now to allow easier tuning.
    // This does not effect PS3, as this needs to be compiled in.
    //   Use FXAA_CONSOLE__PS3_EDGE_THRESHOLD for PS3.
    //   Due to the PS3 being ALU bound,
    //   there are only two safe values here: 1/4 and 1/8.
    //   These options use the shaders ability to a free *|/ by 2|4|8.
    // The console setting has a different mapping than the quality setting.
    // Other platforms can use other values.
    //   0.125 leaves less aliasing, but is softer (default!!!)
    //   0.25 leaves more aliasing, and is sharper
    float fxaaEdgeThreshold,
    //
    // This used to be the FXAA_CONSOLE__EDGE_THRESHOLD_MIN define.
    // It is here now to allow easier tuning.
    // Trims the algorithm from processing darks.
    // The console setting has a different mapping than the quality setting.
    //   0.06 - faster but more aliasing in darks
    //   0.05 - default
    //   0.04 - slower and less aliasing in darks
    // Special notes when using FXAA_GREEN_AS_LUMA,
    //   Likely want to set this to zero.
    //   As colors that are mostly not-green
    //   will appear very dark in the green channel!
    //   Tune by looking at mostly non-green content,
    //   then start at zero and increase until aliasing is a problem.
    float fxaaEdgeThresholdMin
) {
/*--------------------------------------------------------------------------*/
    float lumaNw = FxaaLuma(FxaaTexTop(tex, fxaaPosPos.xy));
    float lumaSw = FxaaLuma(FxaaTexTop(tex, fxaaPosPos.xw));
    float lumaNe = FxaaLuma(FxaaTexTop(tex, fxaaPosPos.zy));
    float lumaSe = FxaaLuma(FxaaTexTop(tex, fxaaPosPos.zw));
/*--------------------------------------------------------------------------*/
    vec4 rgbyM = FxaaTexTop(tex, pos.xy);
    #if (FXAA_GREEN_AS_LUMA == 0)
        float lumaM = rgbyM.w;
    #else
        float lumaM = rgbyM.y;
    #endif
/*--------------------------------------------------------------------------*/
    float lumaMaxNwSw = max(lumaNw, lumaSw);
    lumaNe += 1.0/384.0;
    float lumaMinNwSw = min(lumaNw, lumaSw);
/*--------------------------------------------------------------------------*/
    float lumaMaxNeSe = max(lumaNe, lumaSe);
    float lumaMinNeSe = min(lumaNe, lumaSe);
/*--------------------------------------------------------------------------*/
    float lumaMax = max(lumaMaxNeSe, lumaMaxNwSw);
    float lumaMin = min(lumaMinNeSe, lumaMinNwSw);
/*--------------------------------------------------------------------------*/
    float lumaMaxScaled = lumaMax * fxaaEdgeThreshold;
/*--------------------------------------------------------------------------*/
    float lumaMinM = min(lumaMin, lumaM);
    float lumaMaxScaledClamped = max(fxaaEdgeThresholdMin, lumaMaxScaled);
    float lumaMaxM = max(lumaMax, lumaM);
    float dirSwMinusNe = lumaSw - lumaNe;
    float lumaMaxSubMinM = lumaMaxM - lumaMinM;
    float dirSeMinusNw = lumaSe - lumaNw;
    if(lumaMaxSubMinM < lumaMaxScaledClamped) return rgbyM;
/*--------------------------------------------------------------------------*/
    vec2 dir;
    dir.x = dirSwMinusNe + dirSeMinusNw;
    dir.y = dirSwMinusNe - dirSeMinusNw;
/*--------------------------------------------------------------------------*/
    vec2 dir1 = normalize(dir.xy);
    vec4 rgbyN1 = FxaaTexTop(tex, pos.xy - dir1 * fxaaRcpFrameOpt);
    vec4 rgbyP1 = FxaaTexTop(tex, pos.xy + dir1 * fxaaRcpFrameOpt);
/*--------------------------------------------------------------------------*/
    float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * fxaaEdgeSharpness;
    vec2 dir2 = clamp(dir1.xy / dirAbsMinTimesC, -2.0, 2.0);
/*--------------------------------------------------------------------------*/
    vec4 rgbyN2 = FxaaTexTop(tex, pos.xy - dir2 * fxaaRcpFrameOpt2);
    vec4 rgbyP2 = FxaaTexTop(tex, pos.xy + dir2 * fxaaRcpFrameOpt2);
/*--------------------------------------------------------------------------*/
    vec4 rgbyA = rgbyN1 + rgbyP1;
    vec4 rgbyB = ((rgbyN2 + rgbyP2) * 0.25) + (rgbyA * 0.25);
/*--------------------------------------------------------------------------*/
    #if (FXAA_GREEN_AS_LUMA == 0)
        bool twoTap = (rgbyB.w < lumaMin) || (rgbyB.w > lumaMax);
    #else
        bool twoTap = (rgbyB.y < lumaMin) || (rgbyB.y > lumaMax);
    #endif
    if(twoTap) rgbyB.xyz = rgbyA.xyz * 0.5;
    return rgbyB; }
/*==========================================================================*/

void main()
{
    vec2 uResolution = textureSize(iChannel0,0);

    vec2 fxaaRcpFrameOpt = vec2(0.5 / uResolution.x, 0.5 / uResolution.y);
    vec2 fxaaRcpFrameOpt2 = vec2(2.0 / uResolution.x, 2.0 / uResolution.y);
    vec4 frameSize = vec4(uResolution.x, uResolution.y, 1.0 / uResolution.x, 1.0 / uResolution.y);    

    vec4 fxaaPosPos;
    fxaaPosPos.xy = TEXCOORD.xy * frameSize.xy - 1.0;
    fxaaPosPos.zw = fxaaPosPos.xy + 2.0;
    fxaaPosPos *= frameSize.zwzw;

    float fxaaEdgeSharpness = 8.0;
    float fxaaEdgeThreshold = 0.125;
    float fxaaEdgeThresholdMin = 0.04; //0.0625; // 0.05;

    vec4 outColor = FxaaPixelShader(TEXCOORD.xy, fxaaPosPos, iChannel0,
                    fxaaRcpFrameOpt, fxaaRcpFrameOpt2,
                    fxaaEdgeSharpness, fxaaEdgeThreshold, fxaaEdgeThresholdMin);

    //outColor.a = 0.9;
    //if (outColor.a != 1) discard;
    FRAGCOLOR = outColor;
}
