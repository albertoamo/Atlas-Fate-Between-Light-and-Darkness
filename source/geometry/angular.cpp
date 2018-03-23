#include "mcv_platform.h"

VEC3 getVectorFromYaw(float yaw) {
	return VEC3(sinf(yaw), 0.f, cosf(yaw));
}

float getYawFromVector(VEC3 front) {
	return atan2f(front.x, front.z);
}

VEC3 getVectorFromYawPitch(float yaw, float pitch) {
	return VEC3(
		  sinf(yaw) * cosf( pitch )
		,             sinf( pitch )
		, cosf(yaw) * cosf( pitch )
	);
}

void getYawPitchFromVector(VEC3 front, float* yaw, float* pitch) {
	*yaw = atan2f(front.x, front.z);
	float mdo_xz = sqrtf(front.x*front.x + front.z*front.z);
	*pitch = atan2f(front.y, mdo_xz);
}

VEC3 projectVector(const VEC3 & vector, const VEC3 & normal){

	VEC3 normal_norm = normal;
	normal_norm.Normalize();

	VEC3 proj = (vector - vector.Dot(normal_norm) * normal_norm);
	proj.Normalize();

	return proj;
}

QUAT createLookAt(const VEC3& origin, const VEC3& front, const VEC3& up)
{
	Matrix test = Matrix::CreateLookAt(origin, front, up).Transpose();
	return Quaternion::CreateFromRotationMatrix(test);
}

VEC3 rotateVectorAroundAxis(const VEC3& vecToRotate, const VEC3& axisToRotate, float deltaRotation)
{
	double cos_delta = cosf(deltaRotation);
	double sin_delta = sinf(deltaRotation);

	return (vecToRotate * cos_delta) + (axisToRotate.Cross(vecToRotate) * sin_delta) + (axisToRotate * axisToRotate.Dot(vecToRotate)) * (1 - cos_delta);
}
