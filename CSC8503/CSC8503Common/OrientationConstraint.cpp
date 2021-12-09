#include "OrientationConstraint.h"
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		void OrientationConstraint::UpdateConstraint(float dt) {
			Quaternion relativeOrientation = objA->GetTransform().GetOrientation() - objB->GetTransform().GetOrientation();

			Vector3 currentDiff = relativeOrientation.ToEuler();

		}
	}
}