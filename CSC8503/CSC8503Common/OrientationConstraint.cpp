#include "OrientationConstraint.h"
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		void OrientationConstraint::UpdateConstraint(float dt) {
			Quaternion relativeRotation = objA->GetTransform().GetOrientation() - objB->GetTransform().GetOrientation();

			float currentDiffX = relativeRotation.ToEuler().x;
			float currentDiffY = relativeRotation.ToEuler().y;
			float currentDiffZ = relativeRotation.ToEuler().z;

			float offsetX = x - currentDiffX;
			float offsetY = y - currentDiffY;
			float offsetZ = z - currentDiffZ;

			if (offsetX < 0) {

			}
			if (offsetY < 0) {

			}
			if (offsetZ < 0) {

			}
		}
	}
}