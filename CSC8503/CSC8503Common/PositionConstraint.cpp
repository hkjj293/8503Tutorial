#include "PositionConstraint.h"
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		void PositionConstraint::UpdateConstraint(float dt) {
			Vector3 relativePos = objA->GetTransform().GetPosition() - objB->GetTransform().GetPosition();

			float currentDistance = relativePos.Length();

			float offset = distance - currentDistance;

			if (currentDistance > distance) {
				Vector3 offsetDir = relativePos.Normalised();

				PhysicsObject* physA = objA->GetPhysicsObject();
				PhysicsObject* physB = objB->GetPhysicsObject();

				Vector3 relativeVelocity = physA->GetLinearVelocity() - physB->GetLinearVelocity();

				float constraintMass = physA->GetInverseMass() + physB->GetInverseMass();

				if (constraintMass > 0.0f) {
					float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);
					float biasFactor = 0.1f;
					float bias = -(biasFactor / dt) * offset;

					float lambda = -(velocityDot + bias) / constraintMass;

					Vector3 aImpulse = offsetDir * lambda;
					Vector3 bImpulse = -offsetDir * lambda;

					physA->ApplyLinearImpulse(aImpulse);
					physB->ApplyLinearImpulse(bImpulse); 
				}
			}
		}
	}
}