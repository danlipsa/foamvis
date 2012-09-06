#version 120
struct Rect
{
    vec2 m_low;
    vec2 m_high;
};
// each fragmet receives the object coordinates of the fragment.
varying vec2 v_objectCoordA;
// object coordinates for screen
uniform Rect u_enclosingRect;
// expressed in (e1, e2) axes
uniform vec2 u_gridTranslationE;
// cell length in object coordinates
uniform float u_cellLength;
// line width in object coordinates
uniform float u_lineWidth;
uniform float u_noiseStart;
uniform float u_noiseFrequency;
uniform float u_noiseAmplitude;
uniform float u_sizeRatio;
uniform bool u_sameSize;
uniform bool u_colorMapped;
// deformation tensors are stored here.
uniform sampler2D u_tensorAverageTexUnit;
// scalar averages are stored here (sum, count, min, max). We use count.
uniform sampler2D u_scalarAverageTexUnit;
uniform float u_minValue;
uniform float u_maxValue;
uniform sampler1D u_overlayBarTexUnit;
uniform bool u_gridShown;
uniform bool u_gridCellCenterShown;
uniform bool u_clampingShown;
uniform float u_onePixelInObjectSpace;
//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}
vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}
float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187, // (3.0-sqrt(3.0))/6.0
   0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
   -0.577350269189626, // -1.0 + 2.0 * C.x
   0.024390243902439); // 1.0 / 41.0
// First corner
    vec2 i = floor(v + dot(v, C.yy) );
    vec2 x0 = v - i + dot(i, C.xx);
// Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
// Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
        + i.x + vec3(0.0, i1.x, 1.0 ));
    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;
// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
// Compute final noise value at P
    vec3 g;
    g.x = a0.x * x0.x + h.x * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}
// Local Variables:
// mode: c++
// End:
vec2 snoise2 (vec2 x)
{
    return vec2 (snoise (x), snoise (x) + 43);
}
// x is in [0, 1)
bool isOnGrid (vec2 x)
{
    vec2 isBackground;
    float perc = (u_cellLength - u_onePixelInObjectSpace) / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    isBackground = step (x, percentage);
    return isBackground.x * isBackground.y == 0.;
}
// x is in [0, 1)
bool isOnGridCellCenter (vec2 x, vec2 f)
{
    float empty = (u_cellLength - 2 * u_onePixelInObjectSpace) / u_cellLength;
    float halfFull = (1 - empty) / 2;
    vec2 percentage = vec2 (empty, empty);
    x = fract (
 vec2 (0.5, 0.5) + x -
 halfFull * (vec2(1,1) +
      u_noiseAmplitude * snoise2 (
   u_noiseStart + f / u_noiseFrequency)));
    vec2 isOnLine = step (percentage, x);
    return isOnLine.x * isOnLine.y == 1.0;
}
// x is in (-inf, +inf)
bool isOnGridCenter (vec2 x)
{
    x = abs (x);
    float perc = 2*u_onePixelInObjectSpace / u_cellLength;
    vec2 percentage = vec2 (perc, perc);
    vec2 isOnLine = step (x, percentage);
    return isOnLine.x * isOnLine.y == 1.0;
}
// See doc/TensorDisplay.pdf
void getCoordinates (out vec2 gridCoord, out vec2 texCoordCenter)
{
    vec2 gridTranslationA = mat2 (
 gl_ModelViewMatrix[0].xy,
 gl_ModelViewMatrix[1].xy) * u_gridTranslationE;
    // expressed in (a1, a2) coordinate system
    gridCoord = (v_objectCoordA - gridTranslationA) / u_cellLength;
    vec2 gridCoordFloor = floor (gridCoord);
    vec2 cellCenterA = u_cellLength * (gridCoordFloor + vec2 (.5, .5));
    // express in  (e1, e2) coordinates.
    vec2 cellCenterE = mat2 (gl_ModelViewMatrixInverse[0].xy,
         gl_ModelViewMatrixInverse[1].xy) * cellCenterA;
    texCoordCenter =
 (cellCenterE + u_gridTranslationE - u_enclosingRect.m_low) /
 (u_enclosingRect.m_high - u_enclosingRect.m_low);
}
// VectorTensor.frag
// Local Variables:
// mode: c++
// End:
/**
 * @file   TensorDisplay.frag
 * @author Dan R. Lipsa
 *
 * Displays deformation ellipses
 */
// see Practical Linear Algebra, ch. 9
// BUG: Mac OS X 10.6.8 OpenGL driver does't like structures 
//      that end in a mat2.
struct Ellipse
{
    // r*d*transpose(r)
    mat2 m_a;
    // ellipse cMin, cMax which determine the size of the ellipse and the 
    // width of the line
    vec2 m_c;
};
const float sqrt2 = 1.41421356237;
const mat2 tensor45 = mat2 (3., 1., 1. / sqrt2, 1./ sqrt2);
const mat2 tensor_b264 = mat2 (
    0.00099301, 0.000340428,
    0.783351, 0.621579);
const mat2 transform45 = mat2 (2., 1., 1., 2.);
const mat2 transform_b264 = mat2 (
    7.4088e-04, 3.1775e-04, 3.1775e-04, 5.9256e-04);
void swap (inout float first, inout float second)
{
    float temp = first;
    first = second;
    second = temp;
}
vec2 getOrthogonal (vec2 v)
{
    return vec2 (v[1], -v[0]);
}
// see Numerical Recipes, ch 5.6
vec2 getQuadraticRoots (float a, float b, float c)
{
    float r = b * b - 4. * a * c;
    if (r < 0.0 || a == 0.0)
 discard;
    r = sqrt (r);
    float q = (-1.0 / 2.0) * (b + sign (b) * r);
    float first = q / a;
    float second = c / q;
    if (first < second)
 swap (first, second);
    return vec2 (first, second);
}
vec2 getEigenVector (float l, mat2 a)
{
    a[0][0] -= l;
    a[1][1] -= l;
    int i = (abs(a[0][0]) < abs(a[1][0])) ? 1 : 0;
    vec2 r = vec2 (-a[i][1] / a[i][0], 1.0);
    r = normalize (r);
    return r;
}
/**
 * returns true if the transform matrix is valid
 */
bool getTensor (vec2 texCoordCenter, out mat2 a)
{
    float count = texture2D (u_scalarAverageTexUnit, texCoordCenter).y;
    if (count == 0.0)
 return false;
    //debug
    //if (count == 1.0)
    //texCoordCenter = vec2 (0.5, 0.5);
    vec4 ta = texture2D (u_tensorAverageTexUnit, texCoordCenter);
    a = mat2 (ta[0], ta[1], ta[2], ta[3]);
    a = a / count;
    return true;
}
void clampTensorSize (inout vec2 eigenVal, float max)
{
    if (eigenVal[0] > max)
    {
 float ratio = max / eigenVal[0];
 eigenVal = ratio * eigenVal;
    }
}
vec2 calculateC (vec2 eigen)
{
    float middle = u_sizeRatio / u_cellLength;
    float width = u_lineWidth / eigen[1] / u_cellLength / 2;
    return vec2 (middle - width, middle + width);
}
Ellipse fromEigen (mat2 t)
{
    mat2 r = mat2 (t[1], getOrthogonal (t[1]));
    vec2 eigenVal = t[0];
    vec2 c = calculateC (eigenVal);
    clampTensorSize (eigenVal, 0.5 / c[1]);
    c = calculateC (eigenVal);
    vec2 l = vec2 (1. / (eigenVal[0] * eigenVal[0]),
     1. / (eigenVal[1] * eigenVal[1]));
    mat2 d = mat2 (l[0], 0., 0., l[1]);
    mat2 a = r * d * transpose (r);
    Ellipse e = Ellipse (a, vec2 (c[0] * c[0], c[1] * c[1]));
    // debug
    //e = Ellipse (mat2 (1./25., 0., 0., 1./9.), vec2 (1./25., 1./9.));
    return e;
}
Ellipse fromTensor (mat2 a)
{
    vec2 eVal = getQuadraticRoots (1.0, -a[0][0] - a[1][1],
     a[0][0] * a[1][1] - a[0][1] * a[1][0]);
    vec2 eVec = getEigenVector (eVal[0], a);
    return fromEigen (mat2 (eVal, eVec));
}
void rotateTensor (inout mat2 a)
{
    mat2 r = mat2 (gl_ModelViewMatrix[0].xy,
     gl_ModelViewMatrix[1].xy);
    mat2 rInv = mat2 (gl_ModelViewMatrixInverse[0].xy,
        gl_ModelViewMatrixInverse[1].xy);
    a = r * a * rInv;
}
// x is in [0, 1)
bool isOnEllipse (vec2 x, vec2 texCoordCenter)
{
    mat2 a;
    if (getTensor (texCoordCenter, a))
    {
 //Ellipse t = fromEigen (tensor_b264);
 //Ellipse t = fromTensor (transform45);
 rotateTensor (a);
 Ellipse t = fromTensor (a);
 vec2 v = vec2 (0.5, 0.5);
 float value = dot (x - v, t.m_a * (x - v));
 return (t.m_c[0] <= value && value <= t.m_c[1]);
 //debug
 //return true;
    }
    else
 return false;
}
void main (void)
{
    const vec4 inkColor = vec4 (0., 0., 0., 1.);
    vec2 gridCoord, texCoordCenter;
    getCoordinates (gridCoord, texCoordCenter);
    vec2 gridCoordFract = fract (gridCoord);
    vec2 gridCoordFloor = floor (gridCoord);
    if (isOnEllipse (gridCoordFract, texCoordCenter) ||
 (u_gridShown && isOnGrid (gridCoordFract)) ||
 (u_gridCellCenterShown &&
  isOnGridCellCenter (gridCoordFract, gridCoordFloor))
 ||
 isOnGridCenter (gridCoord))
 gl_FragColor = inkColor;
    else
 discard;
}
// Local Variables:
// mode: c++
// End:
