#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Matrix3.h"

using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			void SetLinearResistance(Vector3 linearResist) {
				linearResistance = linearResist;
			}

			Vector3 GetLinearResistance() const {
				return linearResistance;
			}

			void SetAngularResistance(Vector3 angularResist) {
				angularResistance = angularResist;
			}

			Vector3 GetAngularResistance() const {
				return angularResistance;
			}

			void SetElasticity(float elasticity) {
				this->elasticity = elasticity;
			}

			float GetElasticity() const {
				return elasticity;
			}

			void SetFriction(float friction) {
				this->friction = friction;
			}

			float GetFriction() const {
				return friction;
			}

			void SetResolve(bool resolve) {
				this->isResolve = resolve;
			}

			bool IsResolve() const {
				return isResolve;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);


			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			void InitCubeInertia();
			void InitSphereInertia(bool hollow = false, float innerRadius = 0);

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			float inverseMass;
			float elasticity;
			float friction;

			bool isResolve;

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;
			Vector3 linearResistance;
			

			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
			Vector3 angularResistance;
		};
	}
}

