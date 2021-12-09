#pragma once
#include "Constraint.h"
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class OrientationConstraint : public Constraint {
		public:
			OrientationConstraint(GameObject* a, GameObject* b, float x, float y, float z) {
				objA = a;
				objB = b;
				this->x = x;
				this->y = y;
				this->z = z;
			}
			~OrientationConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objA;
			GameObject* objB;
			float x;
			float y;
			float z;
		};
	}
}