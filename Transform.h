#pragma once
#include"ButiMath.h"
#include"ButiMemorySystem/ButiMemorySystem/ButiPtr.h"
#include<memory>
#include<mutex>
namespace ButiEngine {

class Transform
{
	friend class TransformGUIObject;
public:

	inline Transform() {
		localMatrix = nullptr;
		rotation = Matrix4x4();
	}
	inline Transform(const Vector3& arg_position, const Vector3& arg_rotate, const Vector3& arg_scale) {

		localPosition = arg_position;
		rotation = Matrix4x4::RollX(
			MathHelper::ToRadian(arg_rotate.x)
		) *
			Matrix4x4::RollY(
				MathHelper::ToRadian(arg_rotate.y)
			) *
			Matrix4x4::RollZ(
				MathHelper::ToRadian(arg_rotate.z)
			);
		scale = arg_scale;
		localMatrix = nullptr;
	}
	inline Transform(const Vector3& arg_position, const Vector3& arg_rotate, const float arg_scale) {

		localPosition = arg_position;
		rotation = Matrix4x4::RollX(
			MathHelper::ToRadian(arg_rotate.x)
		) *
			Matrix4x4::RollY(
				MathHelper::ToRadian(arg_rotate.y)
			) *
			Matrix4x4::RollZ(
				MathHelper::ToRadian(arg_rotate.z)
			);
		scale = Vector3(arg_scale);
		localMatrix = nullptr;
	}
	inline Transform(const Vector3& arg_position, const Matrix4x4& arg_rotate, const Vector3& arg_scale) {

		localPosition = arg_position;
		rotation = arg_rotate;
		scale = arg_scale;
		localMatrix = nullptr;
	}
	inline Transform(const Vector3& arg_position, const Matrix4x4& arg_rotate, const float arg_scale) {

		localPosition = arg_position;
		rotation = arg_rotate;
		scale = Vector3(arg_scale);
		localMatrix = nullptr;
	}
	inline Transform(const Vector3& pos) {
		localPosition = pos;
		rotation = Matrix4x4();
		localMatrix = nullptr;
	}
	inline Transform(const Transform& arg_other) {
		localPosition = arg_other.localPosition;
		scale = arg_other.scale;
		rotation = arg_other.rotation;
		baseTransform = arg_other.baseTransform;
	}
	inline ~Transform() {
		DeleteLocalMatrix();
	}
	inline Matrix4x4 ToMatrix()
	{
		return GetMatrix().GetTranspose();
	}
	inline Matrix4x4 GetMatrix()
	{
		Matrix4x4 output = GetLocalMatrix();
		std::lock_guard baseLock(mtx_baseTransform);
		if (baseTransform) {
			auto baseMatrix = baseTransform->GetMatrix();
			output = output * baseMatrix;
		}
		return output;
	}
	inline Matrix4x4 GetMatrix_WithoutScale()
	{
		std::lock_guard lock(mtx_transform);
		std::lock_guard baseLock(mtx_baseTransform);
		Matrix4x4 output = rotation;
		output._41 = localPosition.x;
		output._42 = localPosition.y;
		output._43 = localPosition.z;
		if (baseTransform) {
			auto baseMatrix = baseTransform->GetMatrix_WithoutScale();
			output = output * baseMatrix;
		}

		return output;
	}
	inline Matrix4x4 GetTranslateMatrix()
	{
		Matrix4x4 output;
		Vector3 world = GetWorldPosition();

		output._41 = world.x;
		output._42 = world.y;
		output._43 = world.z;

		return output;
	}

	inline void DeleteLocalMatrix() {
		std::lock_guard lock(mtx_transform);
		if (localMatrix)
			ButiMemorySystem::Allocator::deallocate( localMatrix);
		localMatrix = nullptr;
	}
	inline const Matrix4x4 GetLocalMatrix()
	{
		std::lock_guard lock(mtx_transform);
		if (localMatrix)
		{
			return *localMatrix;
		}
		localMatrix = ButiMemorySystem::Allocator::allocate<Matrix4x4>();
		{

			*localMatrix = Matrix4x4().Scale(scale) * rotation;
			localMatrix->_41 = localPosition.x;
			localMatrix->_42 = localPosition.y;
			localMatrix->_43 = localPosition.z;

		}

		return *localMatrix;

	}

	Value_ptr<Transform> Clone()const {
		auto output = ButiEngine::make_value<Transform>(localPosition, rotation, scale);
		output->SetBaseTransform(baseTransform, true);
		return output;
	}

	inline Vector3 GetWorldPosition()
	{
		auto output = GetMatrix();
		return Vector3(output._41, output._42, output._43);
	}
	inline const Matrix4x4& GetLocalRotation()const
	{
		return rotation;
	}
	inline Vector3 GetLocalRotation_Euler()const {
		return rotation.GetEulerOneValue_local().ToDegrees();
	}
	inline Vector3 GetWorldRotation_Euler() {
		return GetWorldRotation().GetEulerOneValue_local().ToDegrees();
	}

	inline Matrix4x4 GetWorldRotation()
	{
		return GetMatrix_WithoutScale().RemovePosition();
	}

	inline const Matrix4x4& SetLocalRotationIdentity() {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = Matrix4x4();
	}

	inline const Matrix4x4& SetLocalRotation(const Matrix4x4& arg_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = arg_rotation;
	}
	inline const Matrix4x4& SetWorldRotation(const Matrix4x4& arg_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		std::lock_guard baseLock(mtx_baseTransform);
		if (baseTransform) {
			rotation = arg_rotation * baseTransform->GetWorldRotation().GetInverse();
		}
		else {
			rotation = arg_rotation;
		}
		return rotation ;
	}
	inline const Matrix4x4& SetLocalRotation(const Vector3& arg_vec3_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = Matrix4x4::RollZ(
			MathHelper::ToRadian(arg_vec3_rotation.z)
		) *
			Matrix4x4::RollY(
				MathHelper::ToRadian(arg_vec3_rotation.y)
			) *
			Matrix4x4::RollX(
				MathHelper::ToRadian(arg_vec3_rotation.x)
			);
	}
	inline const Matrix4x4& SetLocalRotation_radian(const Vector3& arg_vec3_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = Matrix4x4::RollZ(
			(arg_vec3_rotation.z)
		) *
			Matrix4x4::RollY(
				(arg_vec3_rotation.y)
			) *
			Matrix4x4::RollX(
				(arg_vec3_rotation.x)
			);
	}

	inline const Matrix4x4& SetLocalRotationX(const float rotate) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		Vector3 euler = rotation.GetEulerOneValue();
		return rotation = Matrix4x4::RollZ(euler.z) *
			Matrix4x4::RollY(euler.y) *
			Matrix4x4::RollX(rotate);
	}
	inline const Matrix4x4& SetLocalRotationY(const float rotate) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		Vector3 euler = rotation.GetEulerOneValue();
		return rotation = Matrix4x4::RollZ(euler.z) *
			Matrix4x4::RollY(rotate) *
			Matrix4x4::RollX(euler.x);
	}
	inline const Matrix4x4& SetLocalRotationZ(const float rotate) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		Vector3 euler = rotation.GetEulerOneValue();
		return rotation = Matrix4x4::RollZ(rotate) *
			Matrix4x4::RollY(euler.y) *
			Matrix4x4::RollX(euler.x);
	}
	inline const Matrix4x4& SetLocalRotationX_Degrees(const float rotate) {
		return SetLocalRotationX(MathHelper::ToRadian(rotate));
	}
	inline const Matrix4x4& SetLocalRotationY_Degrees(const float rotate) {
		return SetLocalRotationY(MathHelper::ToRadian(rotate));
	}
	inline const Matrix4x4& SetLocalRotationZ_Degrees(const float rotate) {
		return SetLocalRotationZ(MathHelper::ToRadian(rotate));
	}

	inline const Matrix4x4& RollLocalRotationX_Degrees(const float arg_x) {
		return RollLocalRotationX_Radian(MathHelper::ToRadian(arg_x));

	}
	inline const Matrix4x4& RollLocalRotationX_Radian(const float arg_x) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return  rotation = Matrix4x4::RollX(
			arg_x
		) * rotation;

	}
	inline const Matrix4x4& RollWorldRotationX_Degrees(const float arg_x) {

		return RollWorldRotationX_Radian(MathHelper::ToRadian(arg_x));

	}
	inline const Matrix4x4& RollWorldRotationX_Radian(const float arg_x) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = rotation * Matrix4x4::RollX(
			arg_x
		);

	}

	inline const Matrix4x4& RollLocalRotationY_Degrees(const float arg_y) {
		return RollLocalRotationY_Radian(MathHelper::ToRadian(arg_y));

	}
	inline const Matrix4x4& RollLocalRotationY_Radian(const float arg_y) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return  rotation = Matrix4x4::RollY(
			arg_y
		) * rotation;

	}
	inline const Matrix4x4& RollWorldRotationY_Degrees(const float arg_y) {
		return RollWorldRotationY_Radian(MathHelper::ToRadian(arg_y));

	}
	inline const Matrix4x4& RollWorldRotationY_Radian(const float arg_y) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = rotation * Matrix4x4::RollY(
			arg_y
		);

	}

	inline const Matrix4x4& RollLocalRotationZ_Degrees(const float arg_z) {
		return RollLocalRotationZ_Radian(MathHelper::ToRadian(arg_z));

	}
	inline const Matrix4x4& RollLocalRotationZ_Radian(const float arg_z) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return  rotation = Matrix4x4::RollZ(
			arg_z
		) * rotation;

	}
	inline const Matrix4x4& RollWorldRotationZ_Degrees(const float arg_z) {

		return RollWorldRotationZ_Radian(MathHelper::ToRadian(arg_z));

	}
	inline const Matrix4x4& RollWorldRotationZ_Radian(const float arg_z) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = rotation * Matrix4x4::RollZ(
			arg_z
		);

	}
	inline const Matrix4x4& RollLocalRotation(const Vector3& arg_vec3_rotation)
	{
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = Matrix4x4::RollX(
			MathHelper::ToRadian(arg_vec3_rotation.x)
		) *
			Matrix4x4::RollY(
				MathHelper::ToRadian(arg_vec3_rotation.y)
			) *
			Matrix4x4::RollZ(
				MathHelper::ToRadian(arg_vec3_rotation.z)
			) * rotation;
	}
	inline const Matrix4x4& RollWorldBase(const Vector3& arg_vec3_rotation)
	{
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return rotation = rotation * Matrix4x4::RollX(
			MathHelper::ToRadian(arg_vec3_rotation.x)
		) *
			Matrix4x4::RollY(
				MathHelper::ToRadian(arg_vec3_rotation.y)
			) *
			Matrix4x4::RollZ(
				MathHelper::ToRadian(arg_vec3_rotation.z)
			);
	}
	inline const Matrix4x4& RollWorldRotation(const Quat& arg_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		std::lock_guard baseLock(mtx_baseTransform);
		if (baseTransform) {
			rotation *= arg_rotation.ToMatrix() * baseTransform->GetWorldRotation().GetInverse();
		}
		else
			rotation *= arg_rotation.ToMatrix();

		return rotation;
	}

	inline void RollIdentity() {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		rotation.Identity();
	}

	inline const Vector3& Translate(const Vector3& arg_velocity) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_41 += arg_velocity.x;
			localMatrix->_42 += arg_velocity.y;
			localMatrix->_43 += arg_velocity.z;
		}

		return  localPosition += arg_velocity;
	}
	inline const Vector3& TranslateX(const float arg_moveX) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_41 += arg_moveX;
		}
		localPosition.x += arg_moveX;
		return  localPosition;
	}

	inline const Vector3& TranslateY(const float arg_moveY) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_42 += arg_moveY;
		}
		localPosition.y += arg_moveY;
		return  localPosition;
	}

	inline const Vector3& TranslateZ(const float arg_moveZ) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_43 += arg_moveZ;
		}
		localPosition.z += arg_moveZ;
		return  localPosition;
	}
	inline const Vector3& Scaling(const Vector3& arg_scale) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		return	scale *= arg_scale;
	}
	inline const Vector3& ScalingX(const float arg_scaleX) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		scale.x *= arg_scaleX;
		return  scale;
	}

	inline const Vector3& ScalingY(const float arg_scaleY) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		scale.y *= arg_scaleY;
		return  scale;
	}

	inline const Vector3& ScalingZ(const float arg_scaleZ) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		scale.z *= arg_scaleZ;
		return  scale;
	}

	inline float GetLocalX()const {
		return localPosition.x;
	}

	inline float GetLocalY()const {
		return localPosition.y;
	}

	inline float GetLocalZ()const {
		return localPosition.z;
	}

	inline float GetWorldX() {
		return GetWorldPosition().x;
	}

	inline float GetLocalY() {
		return GetWorldPosition().y;
	}

	inline float GetWorldZ() {
		return GetWorldPosition().z;
	}

	inline void SetLocalPositionX(const float arg_value) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_41 = arg_value;
		}
		localPosition.x = arg_value;
	}
	inline void SetLocalPositionY(const float arg_value) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_42 = arg_value;
		}
		localPosition.y = arg_value;
	}
	inline void SetLocalPositionZ(const float arg_value) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_43 = arg_value;
		}
		localPosition.z = arg_value;
	}

	inline void SetWorldPostionX(const float arg_value) {
		auto currentPos = GetWorldPosition();
		currentPos.x = arg_value;
		SetWorldPosition(currentPos);
	}
	inline void SetWorldPostionY(const float arg_value) {
		auto currentPos = GetWorldPosition();
		currentPos.y = arg_value;
		SetWorldPosition(currentPos);
	}
	inline void SetWorldPostionZ(const float arg_value) {
		auto currentPos = GetWorldPosition();
		currentPos.z = arg_value;
		SetWorldPosition(currentPos);
	}

	inline const Matrix4x4& RollLocalRotation(const Matrix4x4& arg_rotation) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		rotation = rotation * arg_rotation;
		return rotation;
	}

	inline const Matrix4x4& SetLookAtRotation(const Vector3& arg_targetPos, const Vector3& arg_upAxis) {
		DeleteLocalMatrix();
		Vector3 z = ((Vector3)(arg_targetPos - GetWorldPosition())).GetNormalize();
		Vector3 x = arg_upAxis.GetCross(z).GetNormalize();
		Vector3 y = z.GetCross(x).GetNormalize();

		Matrix4x4 worldRotation;
		worldRotation._11 = x.x; worldRotation._12 = x.y; worldRotation._13 = x.z;
		worldRotation._21 = y.x; worldRotation._22 = y.y; worldRotation._23 = y.z;
		worldRotation._31 = z.x; worldRotation._32 = z.y; worldRotation._33 = z.z;
		SetWorldRotation(worldRotation);
		std::lock_guard lock(mtx_transform);
		return rotation;
	}
	inline const Matrix4x4& SetLookAtRotation(const Vector3& arg_targetPos) {
		return SetLookAtRotation(arg_targetPos, Vector3Const::YAxis);
	}
	inline  Matrix4x4 GetLookAtRotation(const Vector3& arg_targetPos, const Vector3& arg_upAxis) {
		Vector3 z = ((Vector3)(arg_targetPos - GetWorldPosition())).GetNormalize();
		Vector3 x = arg_upAxis.GetCross(z).GetNormalize();
		Vector3 y = z.GetCross(x).GetNormalize();

		auto out = Matrix4x4();
		out._11 = x.x; out._12 = x.y; out._13 = x.z;
		out._21 = y.x; out._22 = y.y; out._23 = y.z;
		out._31 = z.x; out._32 = z.y; out._33 = z.z;


		return out;
	}
	inline  Matrix4x4 GetLookAtRotation_local(const Vector3& arg_targetPos, const Vector3& arg_upAxis) {
		Vector3 z = ((Vector3)(arg_targetPos - localPosition)).GetNormalize();
		Vector3 x = (arg_upAxis * GetLocalMatrix()).GetCross(z).GetNormalize();
		Vector3 y = z.GetCross(x).GetNormalize();

		auto out = Matrix4x4();
		out._11 = x.x; out._12 = x.y; out._13 = x.z;
		out._21 = y.x; out._22 = y.y; out._23 = y.z;
		out._31 = z.x; out._32 = z.y; out._33 = z.z;


		return out;
	}
	inline const Vector3& GetLocalPosition()const
	{
		return localPosition;
	}
	inline const Vector3& SetLocalPosition(const Vector3& arg_position) {
		std::lock_guard lock(mtx_transform);
		if (localMatrix) {
			localMatrix->_41 = arg_position.x;
			localMatrix->_42 = arg_position.y;
			localMatrix->_43 = arg_position.z;
		}
		localPosition = arg_position;
		return  localPosition;
	}
	inline const Vector3& SetWorldPosition(const Vector3& arg_position) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		std::lock_guard baseLock(mtx_baseTransform);
		if (!baseTransform) {
			localPosition = arg_position;
		}
		else {
			localPosition = arg_position * baseTransform->GetMatrix().Inverse();
		}

		return localPosition;
	}
	inline const Vector3& GetLocalScale() const
	{
		return scale;
	}
	inline  Matrix4x4 GetLocalScaleMatrix() const {
		return Matrix4x4::Scale(scale);
	}
	inline Vector3 GetWorldScale()
	{
		std::lock_guard lock(mtx_transform);
		auto out = scale;
		std::lock_guard baseLock(mtx_baseTransform);
		if (baseTransform) {
			out *= baseTransform->GetWorldScale();
		}
		return out;
	}

	inline const Vector3& SetLocalScale(const Vector3& arg_scale) {
		DeleteLocalMatrix();
		std::lock_guard lock(mtx_transform);
		scale = arg_scale;
		return  scale;
	}
	inline Vector3 GetFront() {
		return Vector3Const::ZAxis * GetWorldRotation();
	}

	inline Vector3 GetRight() {
		return  Vector3Const::XAxis * GetWorldRotation();

	}

	inline Vector3 GetUp() {
		return  Vector3Const::YAxis * GetWorldRotation();
	}

	inline void GetRotatedVector(Vector3& arg_vector3) {
		arg_vector3 *= GetWorldRotation();
	}
	inline void SetBaseTransform(Value_ptr<Transform> arg_Parent, const bool arg_isKeepLocalPosition = false)
	{
		{
			std::lock_guard baseLock(mtx_baseTransform);
			if (arg_Parent == baseTransform) {
				return;
			}
		}
		if (!arg_Parent) {
			if (arg_isKeepLocalPosition) {
				std::lock_guard baseLock(mtx_baseTransform);
				baseTransform = arg_Parent;
				return;
			}
			auto keepPos = GetWorldPosition();
			auto keepRotate = GetWorldRotation();
			DeleteLocalMatrix();
			rotation = keepRotate;
			localPosition = keepPos;
			std::lock_guard baseLock(mtx_baseTransform);
			baseTransform = arg_Parent;
			return;
		}
		if (!arg_isKeepLocalPosition) {
			auto keepPos = GetWorldPosition();
			auto keepRotate = GetWorldRotation();
			{
				std::lock_guard baseLock(mtx_baseTransform);
				baseTransform = arg_Parent;
			}
			SetWorldPosition(keepPos);
			SetWorldRotation(keepRotate);
			if (localMatrix) {
				localMatrix->_41 = localPosition.x;
				localMatrix->_42 = localPosition.y;
				localMatrix->_43 = localPosition.z;
			}
		}
		else {
			std::lock_guard baseLock(mtx_baseTransform);
			baseTransform = arg_Parent;
		}
	}
	inline Value_ptr<Transform> GetBaseTransform()
	{
		return baseTransform;
	}


	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(localPosition);
		archive(scale);
		archive(rotation);
		archive(baseTransform);
	}
protected:
	Vector3 localPosition = Vector3(0.0f, 0.0f, 0.0f);
	Matrix4x4 rotation;
	Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
	Matrix4x4* localMatrix = nullptr;
	std::mutex mtx_transform,mtx_baseTransform;
	Value_ptr<Transform> baseTransform = nullptr;
	virtual void PolymophicDummy() {}
};

class BoneTransform :public Transform {
	friend class TransformGUIObject;
public:
	inline BoneTransform() :Transform() {
	}
	inline BoneTransform(const Vector3& arg_position, const Vector3& arg_rotate, const Vector3& arg_scale)
		: Transform(arg_position, arg_rotate, arg_scale) {

	}
	inline BoneTransform(const Vector3& arg_position, const Matrix4x4& arg_rotate, const Vector3& arg_scale)
		: Transform(arg_position, arg_rotate, arg_scale) {

	}
	inline BoneTransform(const Vector3& arg_pos)
		: Transform(arg_pos) {
	}
	inline BoneTransform(const BoneTransform& arg_other) {
		parentBoneTransform = arg_other.parentBoneTransform;
		baseTransform = arg_other.baseTransform;
		rotation = arg_other.rotation;
		localPosition = arg_other.localPosition;
		scale = arg_other.scale;
	}

	inline Matrix4x4 GetBoneMatrix()
	{
		Matrix4x4 output = GetLocalMatrix();
		if (parentBoneTransform != nullptr) {
			auto baseMatrix = parentBoneTransform->GetBoneMatrix();
			output = output * baseMatrix;
		}
		return output;
	}

	inline Vector3 GetBonePosition()
	{
		Matrix4x4 output = GetLocalMatrix();
		if (parentBoneTransform != nullptr) {
			auto baseMatrix = parentBoneTransform->GetBoneMatrix();
			output = output * baseMatrix;
		}
		return Vector3(output._41, output._42, output._43);
	}
	inline void SetParentTransform(Value_ptr<BoneTransform> arg_Parent, const bool arg_isKeepLocalPosition = false)
	{
		parentBoneTransform = arg_Parent;
		if (!arg_isKeepLocalPosition) {

			localPosition = localPosition - parentBoneTransform->GetWorldPosition();
			rotation = rotation * parentBoneTransform->GetWorldRotation().Inverse();
			if (localMatrix) {
				localMatrix->_41 = localPosition.x;
				localMatrix->_42 = localPosition.y;
				localMatrix->_43 = localPosition.z;
			}
		}
	}

	Value_ptr<BoneTransform> Clone_BoneTransform()const {
		auto output = ButiEngine::make_value<BoneTransform>(localPosition, rotation, scale);
		output->SetBaseTransform(baseTransform, true);
		output->SetParentTransform(parentBoneTransform, true);
		return output;
	}
	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(localPosition);
		archive(scale);
		archive(rotation);
		archive(baseTransform);
		archive(parentBoneTransform);
	}
private:
	Value_ptr<BoneTransform> parentBoneTransform = nullptr;
};
}

