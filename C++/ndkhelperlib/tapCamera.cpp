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

//----------------------------------------------------------
//  tapCamera.cpp
//  Camera control with tap
//
//----------------------------------------------------------
#include <fstream>
#include "tapCamera.h"

using namespace M3D::Math;

#define USE_M3D_MATH 1

namespace ndk_helper
{

	const float TRANSFORM_FACTOR = 15.f;
	const float TRANSFORM_FACTORZ = 10.f;

	const float MOMENTUM_FACTOR_DECREASE = 0.85f;
	const float MOMENTUM_FACTOR_DECREASE_SHIFT = 0.9f;
	const float MOMENTUM_FACTOR = 0.8f;
	const float MOMENTUM_FACTOR_THRESHOLD = 0.001f;

	//----------------------------------------------------------
	//  Ctor
	//----------------------------------------------------------
	TapCamera::TapCamera()
		: dragging_(false)
		, momentum_(false)
		, pinching_(false)
		, ball_radius_(0.75f)
		, pinch_start_distance_SQ_(0.f)
		, camera_rotation_(0.f)
		, camera_rotation_start_(0.f)
		, camera_rotation_now_(0.f)
		, momemtum_steps_(0.f)
		, flip_z_(0.f)
	{
	    //Init offset
		InitParameters();

		vec_flip_ = Vec2(1.f, -1.f);
		m_Vec2Flip = Vector2(1.0f, -1.0f);
		
		flip_z_ = -1.f;
		vec_pinch_transform_factor_ = Vec3(1.f, 1.f, 1.f);
		m_Vec3PinchTransformFactor = Vector3(1.0f, 1.0f, 1.0f);

		vec_ball_center_ = Vec2(0, 0);
		vec_ball_now_ = Vec2(0, 0);
		vec_ball_down_ = Vec2(0, 0);
		m_Vec2BallCenter = Vector2(0, 0);
		m_Vec2BallNow = Vector2(0, 0);
		m_Vec2BallDown = Vector2(0, 0);
		
		vec_pinch_start_ = Vec2(0, 0);
		vec_pinch_start_center_ = Vec2(0, 0);
		m_Vec2PinchStart = Vector2(0, 0);
		m_Vec2PinchStartCenter = Vector2(0, 0);
		
		//vec_flip_ = Vec2(0, 0);

	}

	void TapCamera::InitParameters()
	{
	    //Init parameters
		vec_offset_ = Vec3();
		vec_offset_now_ = Vec3();
		
		m_Vec3Offset = Vector3(0, 0, 0);
		m_Vec3OffsetNow = Vector3(0, 0, 0);

		quat_ball_rot_ = Quaternion();
		quat_ball_now_ = Quaternion();
		quat_ball_now_.ToMatrix(mat_rotation_);
		
		m_QuatBallRot = M3D::Math::Quaternion(0, 0, 0, 1);
		m_QuatBallNow = M3D::Math::Quaternion(0, 0, 0, 1);
		m_QuatBallNow.ToMatrix(m_Mat4Rotation);
		
		camera_rotation_ = 0.f;

		vec_drag_delta_ = Vec2();
		vec_offset_delta_ = Vec3();
		m_Vec2DragDelta = Vector2(0, 0);
		m_Vec3OffsetDelta = Vector3(0, 0, 0);

		momentum_ = false;
	}

	//----------------------------------------------------------
	//  Dtor
	//----------------------------------------------------------
	TapCamera::~TapCamera()
	{

	}

	void TapCamera::Update()
	{
		if (momentum_)
		{
			float momenttum_steps = momemtum_steps_;

			//Momentum rotation
			Vector2 v = m_Vec2DragDelta;
			BeginDrag(Vector2(0,0)); //NOTE:This call reset _VDragDelta
			Drag(v * m_Vec2Flip);

			//Momentum shift
			m_Vec3Offset += m_Vec3OffsetDelta;

			BallUpdate();
			EndDrag();

			//Decrease deltas
			m_Vec2DragDelta = v * MOMENTUM_FACTOR_DECREASE;
			m_Vec3OffsetDelta = m_Vec3OffsetDelta * MOMENTUM_FACTOR_DECREASE_SHIFT;

			//Count steps
			momemtum_steps_ = momenttum_steps * MOMENTUM_FACTOR_DECREASE;
			if (momemtum_steps_ < MOMENTUM_FACTOR_THRESHOLD)
			{
				momentum_ = false;
			}
		}
		else
		{
			m_Vec2DragDelta *= MOMENTUM_FACTOR;
			m_Vec3OffsetDelta = m_Vec3OffsetDelta * MOMENTUM_FACTOR;
			BallUpdate();
		}

		Vector3 vec = m_Vec3Offset + m_Vec3OffsetNow;
		Vector3 vec_tmp(TRANSFORM_FACTOR, -TRANSFORM_FACTOR, TRANSFORM_FACTORZ);

		vec *= vec_tmp * m_Vec3PinchTransformFactor;

		float transform[4][4] = { 
			{ 1.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f, 0.0f },
			{ vec.x, vec.y, vec.z, 1.0f }
		};
		
		m_Mat4Transform = Matrix4x4(
			&transform[0][0]
		);//Mat4::Translation(vec);
	}

	Mat4& TapCamera::GetRotationMatrix()
	{
		return mat_rotation_;
	}

	Mat4& TapCamera::GetTransformMatrix()
	{
		return mat_transform_;
	}
	
	Matrix4x4& TapCamera::GetRotation()
	{
		return m_Mat4Rotation;
	}
	
	Matrix4x4& TapCamera::GetTransform()
	{
		return m_Mat4Transform;
	}

	void TapCamera::Reset(const bool bAnimate)
	{
		InitParameters();
		Update();

	}

	//----------------------------------------------------------
	//Drag control
	//----------------------------------------------------------
	void TapCamera::BeginDrag(const Vector2& v)
	{
		if (dragging_)
			EndDrag();

		if (pinching_)
			EndPinch();

		Vector2 vec = v * m_Vec2Flip;
		m_Vec2BallNow = vec;
		m_Vec2BallDown = m_Vec2BallNow;

		dragging_ = true;
		momentum_ = false;
		m_Vec2LastInput = vec;
		m_Vec2DragDelta = Vector2(0,0);
	}

	void TapCamera::EndDrag()
	{
		m_QuatBallDown = m_QuatBallNow;
		m_QuatBallRot = M3D::Math::Quaternion(0,0,0,1);

		dragging_ = false;
		momentum_ = true;
		momemtum_steps_ = 1.0f;
	}

	void TapCamera::Drag(const Vector2& v)
	{
		if (!dragging_)
			return;

		Vector2 vec = v * m_Vec2Flip;
		m_Vec2BallNow = vec;

		m_Vec2DragDelta = m_Vec2DragDelta * MOMENTUM_FACTOR + (vec - m_Vec2LastInput);
		m_Vec2LastInput = vec;
	}

	//----------------------------------------------------------
	//Pinch controll
	//----------------------------------------------------------
	void TapCamera::BeginPinch(const Vector2& v1, const Vector2& v2)
	{
		if (dragging_)
			EndDrag();

		if (pinching_)
			EndPinch();

		BeginDrag(Vector2(0,0));

		m_Vec2PinchStartCenter = (v1 + v2) / 2.f;

		Vector2 vec = v1 - v2;
		float x_diff;
		float y_diff;
		// vec.Value(x_diff, y_diff);
		x_diff = vec.x;
		y_diff = vec.y;

		pinch_start_distance_SQ_ = x_diff * x_diff + y_diff * y_diff;
		camera_rotation_start_ = atan2f(y_diff, x_diff);
		camera_rotation_now_ = 0;

		pinching_ = true;
		momentum_ = false;

		    //Init momentum factors
		m_Vec3OffsetDelta = Vector3(0,0,0);
	}
#if USE_M3D_MATH
	void TapCamera::EndPinch()
	{
		pinching_ = false;
		momentum_ = true;
		momemtum_steps_ = 1.f;
//		vec_offset_ += vec_offset_now_;
		m_Vec3Offset += m_Vec3OffsetNow;
		
		camera_rotation_ += camera_rotation_now_;
		
//		vec_offset_now_ = Vec3();
		m_Vec3OffsetNow = Vector3(0, 0, 0);

		camera_rotation_now_ = 0;

		EndDrag();
	}
#else
	void TapCamera::EndPinch()
	{
		pinching_ = false;
		momentum_ = true;
		momemtum_steps_ = 1.f;
		vec_offset_ += vec_offset_now_;
		camera_rotation_ += camera_rotation_now_;
		vec_offset_now_ = Vec3();

		camera_rotation_now_ = 0;

		EndDrag();
	}
#endif
	

	void TapCamera::Pinch(const Vector2& v1, const Vector2& v2)
	{
		if (!pinching_)
			return;

			    //Update momentum factor
//		vec_offset_last_ = vec_offset_now_;
		m_Vec3OffsetLast = m_Vec3OffsetNow;

		float x_diff, y_diff;
		Vector2 vec = v1 - v2;
//		vec.Value(x_diff, y_diff);
		x_diff = vec.x;
		y_diff = vec.y;

		float fDistanceSQ = x_diff * x_diff + y_diff * y_diff;

		float f = pinch_start_distance_SQ_ / fDistanceSQ;
		if (f < 1.f)
			f = -1.f / f + 1.0f;
		else
			f = f - 1.f;
		if (isnan(f))
			f = 0.f;

//		vec = (v1 + v2) / 2.f - vec_pinch_start_center_;
		vec = (v1 + v2) / 2.f - m_Vec2PinchStartCenter;
//		vec_offset_now_ = Vec3(vec, flip_z_ * f);
		m_Vec3OffsetNow = Vector3(vec, flip_z_ * f);

		    //Update momentum factor
//		vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR + (vec_offset_now_ - vec_offset_last_);
		m_Vec3OffsetDelta = m_Vec3OffsetDelta * MOMENTUM_FACTOR + m_Vec3OffsetNow - m_Vec3OffsetLast;
		            //
		            //Update ration quaternion
		float fRotation = atan2f(y_diff, x_diff);
		camera_rotation_now_ = fRotation - camera_rotation_start_;

		    //Trackball rotation
//		quat_ball_rot_ = Quaternion( 0.f,
//			0.f,
//			sinf(-camera_rotation_now_ * 0.5f),
//			cosf(-camera_rotation_now_ * 0.5f));
		
		m_QuatBallRot = M3D::Math::Quaternion(0,
			0,
			sinf(-camera_rotation_now_ * 0.5f),
			cosf(-camera_rotation_now_ * 0.5f));
	}
	
	void TapCamera::Pinch(const Vec2& v1, const Vec2& v2)
	{
		if (!pinching_)
			return;

			    //Update momentum factor
		vec_offset_last_ = vec_offset_now_;

		float x_diff, y_diff;
		Vec2 vec = v1 - v2;
		vec.Value(x_diff, y_diff);

		float fDistanceSQ = x_diff * x_diff + y_diff * y_diff;

		float f = pinch_start_distance_SQ_ / fDistanceSQ;
		if (f < 1.f)
			f = -1.f / f + 1.0f;
		else
			f = f - 1.f;
		if (isnan(f))
			f = 0.f;

		vec = (v1 + v2) / 2.f - vec_pinch_start_center_;
		vec_offset_now_ = Vec3(vec, flip_z_ * f);

		    //Update momentum factor
		vec_offset_delta_ = vec_offset_delta_ * MOMENTUM_FACTOR
		        + (vec_offset_now_ - vec_offset_last_);

		            //
		            //Update ration quaternion
		float fRotation = atan2f(y_diff, x_diff);
		camera_rotation_now_ = fRotation - camera_rotation_start_;

		    //Trackball rotation
		quat_ball_rot_ = Quaternion( 0.f,
			0.f,
			sinf(-camera_rotation_now_ * 0.5f),
			cosf(-camera_rotation_now_ * 0.5f));
	}

	//----------------------------------------------------------
	//Trackball controll
	//----------------------------------------------------------
#if USE_M3D_MATH
	void TapCamera::BallUpdate()
	{
		if (dragging_)
		{
			Vector3 vec_from = PointOnSphere(m_Vec2BallDown);
			Vector3 vec_to = PointOnSphere(m_Vec2BallNow);

			Vector3 vec = vec_from ^ vec_to;
			float w = vec_from | vec_to;

			M3D::Math::Quaternion qDrag = M3D::Math::Quaternion(vec.x, vec.y, vec.z, w);
			qDrag = qDrag * m_QuatBallDown;
			m_QuatBallNow = m_QuatBallRot * qDrag;
		}
		m_QuatBallNow.ToMatrix(m_Mat4Rotation);
	}
#else
	void TapCamera::BallUpdate()
	{
		if (dragging_)
		{
			Vec3 vec_from = PointOnSphere(vec_ball_down_);
			Vec3 vec_to = PointOnSphere(vec_ball_now_);

			Vec3 vec = vec_from.Cross(vec_to);
			float w = vec_from.Dot(vec_to);

			Quaternion qDrag = Quaternion(vec, w);
			qDrag = qDrag * quat_ball_down_;
			quat_ball_now_ = quat_ball_rot_ * qDrag;
		}
		quat_ball_now_.ToMatrix(mat_rotation_);
	}
#endif

	Vector3 TapCamera::PointOnSphere(Vector2& point)
	{
		Vector3 ball_mouse;
		float mag;
		Vector2 vec = (point - m_Vec2BallCenter) / ball_radius_;
		mag = vec | vec;
		if (mag > 1.f)
		{
			float scale = 1.f / sqrtf(mag);
			vec = vec * scale;
			ball_mouse = Vector3(vec, 0.f);
		}
		else
		{
			ball_mouse = Vector3(vec, sqrtf(1.f - mag));
		}
		return ball_mouse;
	}

	Vec3 TapCamera::PointOnSphere(Vec2& point)
	{
		Vec3 ball_mouse;
		float mag;
		Vec2 vec = (point - vec_ball_center_) / ball_radius_;
		mag = vec.Dot(vec);
		if (mag > 1.f)
		{
			float scale = 1.f / sqrtf(mag);
			vec = vec * scale;
			ball_mouse = Vec3(vec, 0.f);
		}
		else
		{
			ball_mouse = Vec3(vec, sqrtf(1.f - mag));
		}
		return ball_mouse;
	}

} //namespace ndkHelper
