/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Maths.h"
#include "Plane.h"
#include "../Common/Vector2.h"
#include "../Common/Vector3.h"

namespace NCL {
	namespace Maths {
		void ScreenBoxOfTri(const Vector3& v0, const Vector3& v1, const Vector3& v2, Vector2& topLeft, Vector2& bottomRight) {
			topLeft.x = std::min(v0.x, std::min(v1.x, v2.x));
			topLeft.y = std::min(v0.y, std::min(v1.y, v2.y));

			bottomRight.x = std::max(v0.x, std::max(v1.x, v2.x));
			bottomRight.y = std::max(v0.y, std::max(v1.y, v2.y));
		}

		int ScreenAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			int area =(int) (((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y)));
			return (area >> 1);
		}

		float FloatAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			float area = ((a.x * b.y) + (b.x * c.y) + (c.x * a.y)) -
				((b.x * a.y) + (c.x * b.y) + (a.x * c.y));
			return (area * 0.5f);
		}

		float CrossAreaOfTri(const Vector3 &a, const Vector3 &b, const Vector3 & c) {
			Vector3 area = Vector3::Cross(a - b, a - c);
			return area.Length() * 0.5f;
		}
	

		Vector3 Clamp(const Vector3& a, const Vector3&mins, const Vector3& maxs) {
			return Vector3(
				Clamp(a.x, mins.x, maxs.x),
				Clamp(a.y, mins.y, maxs.y),
				Clamp(a.z, mins.z, maxs.z)
			);
		}

		Vector3 PointProjOnLineSegment(const Vector3& lStart, const Vector3& lEnd, const Vector3& point) {
			Vector3 v1 = lEnd - lStart;
			Vector3 p = point - lStart;

			if (v1.Length() == 0) return lStart;

			Vector3 dir = v1.Normalised();
			float l = Vector3::Dot(p, dir);
			Vector3 closestPoint = (dir * l) + lStart;
			if (l >= v1.Length()) {
				closestPoint = lEnd;
			}
			else if (l <= 0) {
				closestPoint = lStart;
			}
			return closestPoint;
		}

		Vector3 LineSegClosestPointAOnB(const Vector3& lStartA, const Vector3& lEndA, const Vector3& lStartB, const Vector3& lEndB) {
			Vector3 v1 = lEndA - lStartA;
			Vector3 v2 = lEndB - lStartB;

			if (abs(Vector3::Dot(v1.Normalised(), v2.Normalised())) == 1.0f) {
				return (v2 * 0.5) + lStartB;
			}

			if (v2.Length() == 0) {
				return lStartB;
			}

			if (v1.Length() == 0) {
				Vector3 point = PointProjOnLineSegment(lStartB, lEndB, lStartA);
				return point;
			}

			Plane projPlane(v1.Normalised(), Vector3::Dot(-lStartA, v1.Normalised()));
			Vector3 topBProj = projPlane.ProjectPointOntoPlane(lEndB);
			Vector3 bottomBProj = projPlane.ProjectPointOntoPlane(lStartB);

			Vector3 closestPointOnPlane = PointProjOnLineSegment(bottomBProj, topBProj, lStartA);
			float ratio = (closestPointOnPlane - bottomBProj).Length() / (topBProj - bottomBProj).Length();

			Vector3 closestPointAOnB = (v2 * ratio) + lStartB;
			return closestPointAOnB;
		}

		Vector3 LineSegClosestPointAOnB(const Vector3& lStartA, const Vector3& lEndA, const Vector3& lStartB, const Vector3& lEndB, Vector3& distance) {
			Vector3 closestPointAOnB = LineSegClosestPointAOnB(lStartA, lEndA, lStartB, lEndB);
			distance = PointProjOnLineSegment(lStartA, lEndA, closestPointAOnB) - closestPointAOnB;
			return closestPointAOnB;
		}
	}
}