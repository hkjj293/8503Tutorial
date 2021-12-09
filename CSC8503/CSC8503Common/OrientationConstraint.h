#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class OrientationConstraint : public Constraint {
		public:
			OrientationConstraint(GameObject* a, GameObject* b, float rotate) {
				objA = a;
				objB = b;
				rotation = rotate;
			}
			~OrientationConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objA;
			GameObject* objB;

			float rotation;
		};
	}
}