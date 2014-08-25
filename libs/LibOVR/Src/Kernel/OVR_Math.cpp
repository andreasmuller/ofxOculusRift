/************************************************************************************

Filename    :   OVR_Math.h
Content     :   Implementation of 3D primitives such as vectors, matrices.
Created     :   September 4, 2012
Authors     :   Andrew Reisse, Michael Antonov, Anna Yershova

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Licensed under the Oculus VR SDK License Version 2.0 (the "License"); 
you may not use the Oculus VR SDK except in compliance with the License, 
which is provided at the time of installation or download, or which 
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, the Oculus VR SDK 
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*************************************************************************************/

#include "OVR_Math.h"
#include "OVR_Log.h"

#include <float.h>


namespace OVR {


//-------------------------------------------------------------------------------------
// ***** Math


// Single-precision Math constants class.
const float Math<float>::Pi      = 3.1415926f;
const float Math<float>::TwoPi   = 3.1415926f * 2;
const float Math<float>::PiOver2 = 3.1415926f / 2.0f;
const float Math<float>::PiOver4 = 3.1415926f / 4.0f;
const float Math<float>::E       = 2.7182818f;

const float Math<float>::MaxValue			= FLT_MAX;
const float Math<float>::MinPositiveValue	= FLT_MIN;

const float Math<float>::RadToDegreeFactor	= 360.0f / Math<float>::TwoPi;
const float Math<float>::DegreeToRadFactor	= Math<float>::TwoPi / 360.0f;

const float Math<float>::Tolerance			= 0.00001f;
const float Math<float>::SingularityRadius	= 0.0000001f; // Use for Gimbal lock numerical problems

// Double-precision Math constants class.
const double Math<double>::Pi      = 3.14159265358979;
const double Math<double>::TwoPi   = 3.14159265358979 * 2;
const double Math<double>::PiOver2 = 3.14159265358979 / 2.0;
const double Math<double>::PiOver4 = 3.14159265358979 / 4.0;
const double Math<double>::E       = 2.71828182845905;

const double Math<double>::MaxValue				= DBL_MAX;
const double Math<double>::MinPositiveValue		= DBL_MIN;

const double Math<double>::RadToDegreeFactor	= 360.0 / Math<double>::TwoPi;
const double Math<double>::DegreeToRadFactor	= Math<double>::TwoPi / 360.0;

const double Math<double>::Tolerance			= 0.00001;
const double Math<double>::SingularityRadius	= 0.000000000001; // Use for Gimbal lock numerical problems



//-------------------------------------------------------------------------------------
// ***** Matrix4f


Matrix4f Matrix4f::LookAtRH(const Vector3f& eye, const Vector3f& at, const Vector3f& up)
{
    Vector3f z = (eye - at).Normalized();  // Forward
    Vector3f x = up.Cross(z).Normalized(); // Right
    Vector3f y = z.Cross(x);

    Matrix4f m(x.x,  x.y,  x.z,  -(x.Dot(eye)),
               y.x,  y.y,  y.z,  -(y.Dot(eye)),
               z.x,  z.y,  z.z,  -(z.Dot(eye)),
               0,    0,    0,    1 );
    return m;
}

Matrix4f Matrix4f::LookAtLH(const Vector3f& eye, const Vector3f& at, const Vector3f& up)
{
    Vector3f z = (at - eye).Normalized();  // Forward
    Vector3f x = up.Cross(z).Normalized(); // Right
    Vector3f y = z.Cross(x);

    Matrix4f m(x.x,  x.y,  x.z,  -(x.Dot(eye)),
               y.x,  y.y,  y.z,  -(y.Dot(eye)),
               z.x,  z.y,  z.z,  -(z.Dot(eye)),
               0,    0,    0,    1 ); 
    return m;
}


Matrix4f Matrix4f::PerspectiveLH(float yfov, float aspect, float znear, float zfar)
{
    Matrix4f m;
    float    tanHalfFov = tan(yfov * 0.5f);

    m.M[0][0] = 1.0f / (aspect * tanHalfFov);
    m.M[1][1] = 1.0f / tanHalfFov;
    m.M[2][2] = zfar / (zfar - znear);
    m.M[3][2] = 1.0f;
    m.M[2][3] = (zfar * znear) / (znear - zfar);
    m.M[3][3] = 0.0f;

    // Note: Post-projection matrix result assumes Left-Handed coordinate system,
    //       with Y up, X right and Z forward. This supports positive z-buffer values.
    return m;
}


Matrix4f Matrix4f::PerspectiveRH(float yfov, float aspect, float znear, float zfar)
{
    Matrix4f m;
    float    tanHalfFov = tan(yfov * 0.5f);
  
    m.M[0][0] = 1.0f / (aspect * tanHalfFov);
    m.M[1][1] = 1.0f / tanHalfFov;
    m.M[2][2] = zfar / (znear - zfar);
   // m.M[2][2] = zfar / (zfar - znear);
    m.M[3][2] = -1.0f;
    m.M[2][3] = (zfar * znear) / (znear - zfar);
    m.M[3][3] = 0.0f;

    // Note: Post-projection matrix result assumes Left-Handed coordinate system,    
    //       with Y up, X right and Z forward. This supports positive z-buffer values.
    // This is the case even for RHS cooridnate input.       
    return m;
}

Matrix4f Matrix4f::Ortho2D(float w, float h)
{
    Matrix4f m;
    m.M[0][0] = 2.0f/w;
    m.M[1][1] = -2.0f/h;
    m.M[0][3] = -1.0;
    m.M[1][3] = 1.0;
    m.M[2][2] = 0;
    return m;
}


} // Namespace OVR
