#pragma once
#include "Constraint.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class SpringConstraint : public Constraint {
		public:
			SpringConstraint(GameObject* a, GameObject* b, float d) {
				objA = a;
				objB = b;
				distance = d;
			}
			~SpringConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objA;
			GameObject* objB;

			float distance;
		};
	}
}