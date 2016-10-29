#version 330 core

//uniform vec3 cameraPosVec;
uniform vec3 cameraTargetVec;
uniform float elapsedTime;
uniform vec4 testVec;

//in vec2 texCoord;

out vec4 fragColor;

// Geometry buffers
layout(location = 0) out vec4 positionBuffer;
layout(location = 1) out vec4 diffuseBuffer;
layout(location = 2) out vec4 normalBuffer;
layout(location = 3) out vec4 emissiveBuffer;

// Variables from vertex shader
in vec3 worldPos;
in vec2 texCoord;
in vec3 normal;
in vec3 eyeDir;
in mat3 TBN;
in vec3 viewDir;
in vec3 position;

//void main() 
//{
//   fragColor = vec4(0.0, texCoord, 1.0);
//}

vec3      iResolution = vec3(1600, 900, 0);           // viewport resolution (in pixels)
float     iGlobalTime = elapsedTime;           // shader playback time (in seconds)
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
vec4      iMouse = vec4(0.0, 0.0, 0.0, 0.0);                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D iChannel0;			 // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;			 // input channel. XX = 2D/Cube
uniform sampler2D iChannel2;			 // input channel. XX = 2D/Cube
uniform sampler2D iChannel3;			 // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)


// "Wavescape" by dr2 - 2015
// License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License

// Water waves, including what the fish sees.

// Acknowledgments: thanks for the following -
//  Dave_H's multilayered clouds with nimitz's variable layer spacing
//    (but depends on elevation rather than distance).
//  Wave shapes from TDM; they seem a little more "energetic" than TekF's.
//  Raymarching with binary subdivision, as used by Dave_H for mountains;
//    TekF and TDM use one or the other, not both.
//  Buoy based on TekF's, but raymarched for generality; shows aging effects
//    below waterline.
//  Foam idea from TekF.
//  Noise functions from iq.

const float pi = 3.14159;
const vec4 cHashA4 = vec4 (0., 1., 57., 58.);
const vec3 cHashA3 = vec3 (1., 57., 113.);
const float cHashM = 43758.54;

vec2 Hashv2f (float p)
{
  return fract (sin (p + cHashA4.xy) * cHashM);
}

vec4 Hashv4f (float p)
{
  return fract (sin (p + cHashA4) * cHashM);
}

vec4 Hashv4v3 (vec3 p)
{
  const vec3 cHashVA3 = vec3 (37.1, 61.7, 12.4);
  const vec3 e = vec3 (1., 0., 0.);
  return fract (sin (vec4 (dot (p + e.yyy, cHashVA3), dot (p + e.xyy, cHashVA3),
     dot (p + e.yxy, cHashVA3), dot (p + e.xxy, cHashVA3))) * cHashM);
}

float Noiseff (float p)
{
  float i, f;
  i = floor (p);  f = fract (p);
  f = f * f * (3. - 2. * f);
  vec2 t = Hashv2f (i);
  return mix (t.x, t.y, f);
}

float Noisefv2 (vec2 p)
{
  vec2 i, f;
  i = floor (p);  f = fract (p);
  f = f * f * (3. - 2. * f);
  vec4 t = Hashv4f (dot (i, cHashA3.xy));
  return mix (mix (t.x, t.y, f.x), mix (t.z, t.w, f.x), f.y);
}

float Noisefv3a (vec3 p)
{
  vec3 i, f;
  i = floor (p);  f = fract (p);
  f *= f * (3. - 2. * f);
  vec4 t1 = Hashv4v3 (i);
  vec4 t2 = Hashv4v3 (i + vec3 (0., 0., 1.));
  return mix (mix (mix (t1.x, t1.y, f.x), mix (t1.z, t1.w, f.x), f.y),
              mix (mix (t2.x, t2.y, f.x), mix (t2.z, t2.w, f.x), f.y), f.z);
}

float Fbm1 (float p)
{
  float f, a;
  f = 0.;  a = 1.;
  for (int i = 0; i < 5; i ++) {
    f += a * Noiseff (p);
    a *= 0.5;  p *= 2.;
  }
  return f;
}

float Fbm3 (vec3 p)
{
  const mat3 mr = mat3 (0., 0.8, 0.6, -0.8, 0.36, -0.48, -0.6, -0.48, 0.64);
  float f, a, am, ap;
  f = 0.;  a = 0.5;
  am = 0.5;  ap = 4.;
  p *= 0.5;
  for (int i = 0; i < 6; i ++) {
    f += a * Noisefv3a (p);
    p *= mr * ap;  a *= am;
  }
  return f;
}

float Fbmn (vec3 p, vec3 n)
{
  vec3 f = vec3 (0.);
  float a = 1.;
  for (int i = 0; i < 5; i ++) {
    f += a * vec3 (Noisefv2 (p.yz), Noisefv2 (p.zx), Noisefv2 (p.xy));
    a *= 0.5;  p *= 2.;
  }
  return dot (f, abs (n));
}

vec3 VaryNf (vec3 p, vec3 n, float f)
{
  vec3 e = vec3 (0.2, 0., 0.);
  float s = Fbmn (p, n);
  vec3 g = vec3 (Fbmn (p + e.xyy, n) - s,
     Fbmn (p + e.yxy, n) - s, Fbmn (p + e.yyx, n) - s);
  return normalize (n + f * (g - n * dot (n, g)));
}

float SmoothBump (float lo, float hi, float w, float x)
{
  return (1. - smoothstep (hi - w, hi + w, x)) * smoothstep (lo - w, lo + w, x);
}

float PrSphDf (vec3 p, float s)
{
  return length (p) - s;
}

float PrCylDf (vec3 p, float r, float h)
{
  return max (length (p.xy) - r, abs (p.z) - h);
}

int idObj;
mat3 ballMat;
vec3 qHit, ballPos, sunCol, sunDir, cloudDisp, waterDisp;
float tCur, fCloud;
const float dstFar = 300.;

vec3 SkyGrndCol (vec3 ro, vec3 rd)
{
  vec3 p, q, cSun, skyBg, clCol, col;
  float colSum, attSum, s, att, a, dDotS, ds;
  const vec3 cCol1 = 0.5 * vec3 (0.15, 0.2, 0.4),
     cCol2 = 0.5 * vec3 (0.25, 0.5, 0.7), gCol = 1.3 * vec3 (0.05, 0.08, 0.05);
  const float cloudLo = 100., cloudRngI = 1./50., atFac = 0.06;
  const int nLay = 30;
  if (rd.y < 0.015 * Fbm1 (16. * rd.x)) col = gCol * (0.5 + 0.5 *
     Noisefv2 (1000. * vec2 (5. * atan (rd.x, rd.z), rd.y)));
  else {
    fCloud = clamp (fCloud, 0., 1.);
    dDotS = max (dot (rd, sunDir), 0.);
    ro += cloudDisp;
    p = ro;
    p.xz += (cloudLo - p.y) * rd.xz / rd.y;
    p.y = cloudLo;
    ds = 1. / (cloudRngI * rd.y * (2. - rd.y) * float (nLay));
    colSum = 0.;  attSum = 0.;
    s = 0.;  att = 0.;
    for (int j = 0; j < nLay; j ++) {
      q = p + rd * s;
      q.z *= 0.7;
      att += atFac * max (fCloud - Fbm3 (0.02 * q), 0.);
      a = (1. - attSum) * att;
      colSum += a * (q.y - cloudLo) * cloudRngI;
      attSum += a;  s += ds;
      if (attSum >= 1.) break;
    }
    colSum += 0.5 * min ((1. - attSum) * pow (dDotS, 3.), 1.);
    clCol = vec3 (1.) * colSum + 0.05 * sunCol;
    cSun = sunCol * clamp ((min (pow (dDotS, 1500.) * 2., 1.) +
       min (pow (dDotS, 10.) * 0.75, 1.)), 0., 1.);
    skyBg = mix (cCol1, cCol2, 1. - rd.y);
    col = clamp (mix (skyBg + cSun, 1.6 * clCol, attSum), 0., 1.);
  }
  return col;
}

vec3 SeaFloorCol (vec3 rd)
{
  vec2 p;
  float w, f;
  p = 5. * rd.xz / rd.y;
  w = 1.;
  f = 0.;
  for (int j = 0; j < 4; j ++) {
    f += w * Noisefv2 (p);
    w *= 0.5;  p *= 2.;
  }
  return mix (vec3 (0.01, 0.04, 0.02), vec3 (0, 0.05, 0.05), 
     smoothstep (0.4, 0.7, f));
}

float WaveHt (vec3 p)
{
  const mat2 qRot = mat2 (1.6, -1.2, 1.2, 1.6);
  vec4 t4, ta4, v4;
  vec2 q2, t2, v2;
  float wFreq, wAmp, pRough, ht;
  wFreq = 0.16;  wAmp = 0.6;  pRough = 5.;
  q2 = p.xz + waterDisp.xz;
  ht = 0.;
  for (int j = 0; j < 5; j ++) {
    t2 = 1.1 * tCur * vec2 (1., -1.);
    t4 = vec4 (q2 + t2.xx, q2 + t2.yy) * wFreq;
    t2 = vec2 (Noisefv2 (t4.xy), Noisefv2 (t4.zw));
    t4 += 2. * vec4 (t2.xx, t2.yy) - 1.;
    ta4 = abs (sin (t4));
    v4 = (1. - ta4) * (ta4 + abs (cos (t4)));
    v2 = pow (1. - pow (v4.xz * v4.yw, vec2 (0.65)), vec2 (pRough));
    ht += (v2.x + v2.y) * wAmp;
    q2 *= qRot;  wFreq *= 1.9;  wAmp *= 0.22;
    pRough = 0.8 * pRough + 0.2;
  }
  return ht;
}

float WaveRay (vec3 ro, vec3 rd)
{
  vec3 p;
  float dHit, h, s, sLo, sHi;
  s = 0.;
  sLo = 0.;
  dHit = dstFar;
  for (int j = 0; j < 150; j ++) {
    p = ro + s * rd;
    h = p.y - WaveHt (p);
    if (h < 0.) break;
    sLo = s;
    s += max (0.2, h) + 0.005 * s;
    if (s > dstFar) break;
  }
  if (h < 0.) {
    sHi = s;
    for (int j = 0; j < 7; j ++) {
      s = 0.5 * (sLo + sHi);
      p = ro + s * rd;
      h = step (0., p.y - WaveHt (p));
      sLo += h * (s - sLo);
      sHi += (1. - h) * (s - sHi);
    }
    dHit = sHi;
  }
  return dHit;
}

float WaveOutRay (vec3 ro, vec3 rd)
{
  vec3 p;
  float dHit, h, s, sLo, sHi;
  s = 0.;
  sLo = 0.;
  dHit = dstFar;
  ro.y *= -1.;
  rd.y *= -1.;
  for (int j = 0; j < 150; j ++) {
    p = ro + s * rd;
    h = p.y + WaveHt (p);
    if (h < 0.) break;
    sLo = s;
    s += max (0.2, h) + 0.005 * s;
    if (s > dstFar) break;
  }
  if (h < 0.) {
    sHi = s;
    for (int j = 0; j < 7; j ++) {
      s = 0.5 * (sLo + sHi);
      p = ro + s * rd;
      h = step (0., p.y + WaveHt (p));
      sLo += h * (s - sLo);
      sHi += (1. - h) * (s - sHi);
    }
    dHit = sHi;
  }
  return dHit;
}

vec3 WaveNf (vec3 p, float d)
{
  vec2 e = vec2 (max (0.1, 5e-5 * d * d), 0.);
  float h = WaveHt (p);
  return normalize (vec3 (h - WaveHt (p + e.xyy), e.x, h - WaveHt (p + e.yyx)));
}

vec3 ShowScene (vec3 ro, vec3 rd)
{
  vec3 col, vn, rdd, refCol, uwatCol;
  float dstHit, dstWat, dif, bk, sh, foamFac;
  const float eta = 0.75, att = 0.5;
  int idObjT;
  bool doReflect;
  if (ro.y > WaveHt (ro)) {
    dstWat = WaveRay (ro, rd);
    idObj = -1;
    if (idObj < 0) dstHit = dstFar;
    doReflect = (dstWat < dstFar && dstWat < dstHit);
    if (doReflect) {
      ro += rd * dstWat;
      vn = WaveNf (ro, dstWat);
      rdd = rd;
      rd = reflect (rd, vn);
      idObj = -1;
      if (idObj < 0) dstHit = dstFar;
    }
    col = SkyGrndCol (ro, rd);
    if (doReflect) {
      refCol = col;
      rd = refract (rdd, vn, eta);
      idObj = -1;
      if (idObj < 0) dstHit = dstFar;
      col = SeaFloorCol (rd);
      col = mix (col, 0.8 * refCol, pow (1. - abs (dot (rdd, vn)), 5.));
      foamFac = pow (clamp (WaveHt (ro) +
         0.004 * Fbm3 (256. * ro) - 0.65, 0., 1.), 8.);
      col = mix (col, vec3 (1.), foamFac);
    }
  } else {
    uwatCol = vec3 (0., 0.05, 0.05) +
       step (0.4, Fbm1 (20. * tCur)) * vec3 (0.02, 0.02, 0.03);
    col = uwatCol;
    dstWat = WaveOutRay (ro, rd);
    idObj = -1;
    if (idObj < 0) dstHit = dstFar;
    if (dstWat < dstFar && dstWat < dstHit) {
      ro += rd * dstWat;
      vn = - WaveNf (ro, dstWat);
      rdd = refract (rd, vn, 1. / eta);
      if (length (rdd) > 0.) rd = rdd;
      else rd = reflect (rd, vn);
      idObj = -1;
      if (idObj < 0) dstHit = dstFar;
      if (rd.y > 0.) col = mix (uwatCol, 0.9 * SkyGrndCol (ro, rd),
         exp (- 0.07 * att * dstWat));
    } 
  }
  return col;
}

void BallPM ()
{
  const vec3 e = vec3 (1., 0., 0.);
  float h[5], b;
  ballPos = vec3 (0., 0., 0.);
  h[0] = WaveHt (ballPos);
  h[1] = WaveHt (ballPos + e.yyx);  h[2] = WaveHt (ballPos - e.yyx);
  h[3] = WaveHt (ballPos + e);  h[4] = WaveHt (ballPos - e);
  ballPos.y = 0.5 + (2. * h[0] + h[1] + h[2] + h[3] + h[4]) / 9.;
  b = (h[1] - h[2]) / (4. * e.x);
  ballMat[2] = normalize (vec3 (0., b, 1.));
  b = (h[3] - h[4]) / (4. * e.x);
  ballMat[1] = normalize (cross (ballMat[2], vec3 (1., b, 0.)));
  ballMat[0] = cross (ballMat[1], ballMat[2]);
}

void main()
{
  vec2 uv = 2. * gl_FragCoord.xy / iResolution.xy - 1.;
  uv.x *= iResolution.x / iResolution.y;
  tCur = iGlobalTime;
  vec4 mPtr = iMouse;
  mPtr.xy = mPtr.xy / iResolution.xy - 0.5;
  mat3 vuMat;
  vec3 col, ro, rd;
  vec2 vEl, vAz;
  float el, az, zmFac, a, tPer, tSeq;
  cloudDisp = 10. * tCur * vec3 (1., 0., 1.);
  waterDisp = 0.5 * tCur * vec3 (-1., 0., 1.);
  sunDir = normalize (vec3 (0.2, 0.5, 0.5));
  sunCol = vec3 (1., 0.4, 0.3) + vec3 (0., 0.5, 0.2) * sunDir.y;
  fCloud = 0.5 + 0.2 * sin (0.022 * 2. * pi * tCur);
  zmFac = 3.5;
  tPer = 35.;
  if (mPtr.z <= 0.) {
    az = 0.01 * tCur;
    el = 0.2 * pi;
    tSeq = mod (tCur, tPer);
    if (mod (floor (tCur / tPer), 2.) == 0.) {
      a = SmoothBump (10., 30., 5., tSeq);
      zmFac -= 0.1 * a;
      el -= 0.19 * pi * a;
    } else {
      a = SmoothBump (8., 26., 8., tSeq);
      zmFac -= 0.05 * a;
      el -= 0.55 * pi * a;
    }
  } else {
    az = 1.1 * pi * mPtr.x;
    el = 0.02 * pi - 0.7 * pi * mPtr.y;
	
	//az = cameraTargetVec.x;
	//el = cameraTargetVec.y;
	
  }
  vEl = vec2 (cos (el), sin (el));
  vAz = vec2 (cos (az), sin (az));
  rd = normalize (vec3 (uv, zmFac));
  vuMat = mat3 (1., 0., 0., 0., vEl.x, - vEl.y, 0., vEl.y, vEl.x) *
     mat3 (vAz.x, 0., vAz.y, 0., 1., 0., - vAz.y, 0., vAz.x);
  rd = rd * vuMat;
  ro = vec3 (0., 0., -20.) * vuMat;
  ro.y += 2.;
  BallPM ();
  col = ShowScene (ro, rd);
  diffuseBuffer = vec4 (sqrt (clamp (col, 0., 1.)), 1.);
}