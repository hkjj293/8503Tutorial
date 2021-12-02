#pragma once
#include "CollisionVolume.h"

namespace NCL {
	class SphereVolume : CollisionVolume
	{
	public:
		SphereVolume(float sphereRadius = 1.0f, bool hollow = false, float innerRadius = 0) {
			type	= VolumeType::Sphere;
			radius	= sphereRadius;
			this->hollow = hollow;
			this->innerRadius = innerRadius;
		}
		~SphereVolume() {}

		float GetRadius() const {
			return radius;
		}
		bool GetHollow() const {
			return hollow;
		}
		float GetInnerRadius() const {
			return innerRadius;
		}
	protected:
		float radius;

		bool hollow;
		float innerRadius;
	};
}

