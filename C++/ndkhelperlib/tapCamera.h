/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <vector>
#include <string>
#include <GLES2/gl2.h>

#include "JNIHelper.h"
//#include "vecmath.h"
#include "interpolator.h"

#include "../Math/Matrix.h"
#include "../Math/Quaternion.h"

namespace ndk_helper
{

/******************************************************************
 * Camera control helper class with a tap gesture
 * This class is mainly used for 3D space camera control in samples.
 *
 */
class TapCamera
{
private:
    //Trackball
	M3D::Math::Vector2 m_Vec2BallCenter;
	
    float ball_radius_;

	M3D::Math::Quaternion m_QuatBallNow;
	M3D::Math::Quaternion m_QuatBallDown;

	M3D::Math::Vector2 m_Vec2BallNow;
	M3D::Math::Vector2 m_Vec2BallDown;
	M3D::Math::Quaternion m_QuatBallRot;

    bool dragging_;
    bool pinching_;

    //Pinch related info
	M3D::Math::Vector2 m_Vec2PinchStart;
	M3D::Math::Vector2 m_Vec2PinchStartCenter;
    float pinch_start_distance_SQ_;

    //Camera shift
	M3D::Math::Vector3 m_Vec3Offset;
	M3D::Math::Vector3 m_Vec3OffsetNow;

    //Camera Rotation
    float camera_rotation_;
    float camera_rotation_start_;
    float camera_rotation_now_;

    //Momentum support
    bool momentum_;

	M3D::Math::Vector2 m_Vec2DragDelta;
	M3D::Math::Vector2 m_Vec2LastInput;
	M3D::Math::Vector3 m_Vec3OffsetLast;
	M3D::Math::Vector3 m_Vec3OffsetDelta;
	
    float momemtum_steps_;

	M3D::Math::Vector2 m_Vec2Flip;
    float flip_z_;

	M3D::Math::Matrix4x4 m_Mat4Rotation;
	M3D::Math::Matrix4x4 m_Mat4Transform;

	M3D::Math::Vector3 m_Vec3PinchTransformFactor;

	M3D::Math::Vector3 PointOnSphere(M3D::Math::Vector2& point);
	
    void BallUpdate();
    void InitParameters();
public:
    TapCamera();
    virtual ~TapCamera();
    void BeginDrag( const M3D::Math::Vector2& vec );
    void EndDrag();
    void Drag( const M3D::Math::Vector2& vec );
    void Update();

	M3D::Math::Matrix4x4& GetRotation();
	M3D::Math::Matrix4x4& GetTransform();

    void BeginPinch( const M3D::Math::Vector2& v1, const M3D::Math::Vector2& v2 );
    void EndPinch();
	void Pinch(const M3D::Math::Vector2& v1, const M3D::Math::Vector2& v2);
	
    void SetFlip( const float x, const float y, const float z )
    {
	    m_Vec2Flip = M3D::Math::Vector2(x, y);
        flip_z_ = z;
    }

    void SetPinchTransformFactor( const float x, const float y, const float z )
    {
	    m_Vec3PinchTransformFactor = M3D::Math::Vector3(x,y,z);
    }

    void Reset( const bool bAnimate );

};

} //namespace ndkHelper
