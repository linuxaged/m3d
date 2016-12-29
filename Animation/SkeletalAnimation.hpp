#pragma once

struct Joint {
	Matrix4x3 invBindPose;
	const char *name;
	uint8_t parent;
}

struct Skeleton
{
	uint32_t jointCount;
	Joint *joint;
};

struct JointPose
{
	Quaternion rotation;
	Vector3f translation;
	float scale;
};

struct SkeletonPose
{
	Skeleton *pSkeleton;
	JointPose *pLocalPose;
};

struct SkinnedVertex
{
	float position[3];
	float normal[3];
	float u,v;
	uint8_t jointIndex[4];
	float jointWeight[3];
};