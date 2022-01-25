#pragma once
#include "../../Common/Vector3.h"
#include "../CSC8503Common/CollisionDetection.h"
#include "Debug.h"
#include <list>
#include <functional>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class OctTree;

		template<class T>
		struct OctTreeEntry {
			Vector3 pos;
			Vector3 size;
			T object;

			OctTreeEntry(T obj, Vector3 pos, Vector3 size) {
				object = obj;
				this->pos = pos;
				this->size = size;
			}
		};

		template<class T>
		class OctTreeNode {
		public:
			typedef std::function<void(std::list<OctTreeEntry<T>>&)> OctTreeFunc;
		protected:
			friend class OctTree<T>;

			OctTreeNode() {}

			OctTreeNode(Vector3 pos, Vector3 size) {
				children = nullptr;
				this->position = pos;
				this->size = size;
			}

			~OctTreeNode() {
				delete[] children;
			}

			void Insert(T& object, const Vector3& objectPos, const Vector3& objectSize, int depthLeft, int maxSize) {
				if (!CollisionDetection::AABBTest(objectPos, Vector3(position.x, position.y,position.z), objectSize, Vector3(size.x, size.y, size.z))) {
					return;
				}
				if (children) { //not a leaf node , just descend the tree
					for (int i = 0; i < 8; ++i) {
						children[i].Insert(object, objectPos, objectSize, depthLeft - 1, maxSize);
					}
				}
				else { // currently a leaf node , can just expand
					contents.push_back(OctTreeEntry <T>(object, objectPos, objectSize));
					if ((int)contents.size() > maxSize && depthLeft > 0) {
						if (!children) {
							Split();
							//we need to reinsert the contents so far!
							for (const auto& i : contents) {
								for (int j = 0; j < 8; ++j) {
									auto entry = i;
									children[j].Insert(entry.object, entry.pos, entry.size, depthLeft - 1, maxSize);
								}
							}
							contents.clear(); // contents now distributed!
						}
					}
				}
			}

			void Split() {
				Vector3 halfSize = size / 2.0f;
				children = new OctTreeNode <T>[8];
				children[0] = OctTreeNode <T>(position + Vector3(-halfSize.x, halfSize.y,halfSize.z), halfSize);
				children[1] = OctTreeNode <T>(position + Vector3(halfSize.x, halfSize.y, halfSize.z), halfSize);
				children[2] = OctTreeNode <T>(position + Vector3(-halfSize.x, -halfSize.y, halfSize.z), halfSize);
				children[3] = OctTreeNode <T>(position + Vector3(halfSize.x, -halfSize.y, halfSize.z), halfSize);
				children[4] = OctTreeNode <T>(position + Vector3(-halfSize.x, halfSize.y, -halfSize.z), halfSize);
				children[5] = OctTreeNode <T>(position + Vector3(halfSize.x, halfSize.y, -halfSize.z), halfSize);
				children[6] = OctTreeNode <T>(position + Vector3(-halfSize.x, -halfSize.y, -halfSize.z), halfSize);
				children[7] = OctTreeNode <T>(position + Vector3(halfSize.x, -halfSize.y, -halfSize.z), halfSize);
			}

			void Clear() {
				if (children) {
					for (int i = 0; i < 8; ++i) {
						children[i].Clear();
					}
					delete[] children;
					children = nullptr;
				}
			}

			void SetPos(Vector3 p) {
				this->position = p;
			}

			void SetSize(Vector3 s) {
				this->size = s;
			}

			void DebugDraw() {
				if (children) {
					for (int i = 0; i < 8; ++i) {
						children[i].DebugDraw();
					}
				}
				else {//if(!contents.empty()) {
					DrawAABB(position, size);
					/*for (auto it = contents.begin(); it != contents.end(); ++it) {
						DrawAABB((*it).pos, (*it).size);
					}*/
				}

			}

			void DrawAABB(Vector3 position, Vector3 size) {
				Vector3 vertices[8]{
						Vector3(position.x - size.x, position.y + size.y, position.z + size.z),
						Vector3(position.x + size.x, position.y + size.y, position.z + size.z),
						Vector3(position.x + size.x, position.y - size.y, position.z + size.z),
						Vector3(position.x - size.x, position.y - size.y, position.z + size.z),
						Vector3(position.x - size.x, position.y + size.y, position.z - size.z),
						Vector3(position.x + size.x, position.y + size.y, position.z - size.z),
						Vector3(position.x + size.x, position.y - size.y, position.z - size.z),
						Vector3(position.x - size.x, position.y - size.y, position.z - size.z),
				};
				Debug::DrawLine(vertices[0], vertices[1], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[1], vertices[2], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[2], vertices[3], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[3], vertices[0], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[4], vertices[5], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[5], vertices[6], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[6], vertices[7], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[7], vertices[4], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[0], vertices[4], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[1], vertices[5], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[2], vertices[6], Vector4(1, 0, 0, 1), 0.05f);
				Debug::DrawLine(vertices[3], vertices[7], Vector4(1, 0, 0, 1), 0.05f);
			}

			void OperateOnContents(OctTreeFunc& func) {
				if (children) {
					for (int i = 0; i < 8; ++i) {
						children[i].OperateOnContents(func);
					}
				}
				else {
					if (!contents.empty()) {
						func(contents);
					}
				}
			}

			void OperateOnRayCastContents(Ray& r, OctTreeFunc& func) {
				RayCollision thisCollision;
				if (children) {
					for (int i = 0; i < 8; ++i) {
						children[i].OperateOnRayCastContents(r, func);
					}
				}
				else {
					//std::cout << position << CollisionDetection::AABBTest(r.GetPosition(), Vector3(position.x, 0, position.y), Vector3(1.0f, 1.0f, 1.0f), Vector3(size.x, 1000.0f, size.y)) << " " << contents.size() << " " << children;
					if (!contents.empty() &&
						(CollisionDetection::RayBoxIntersection(r, Vector3(position.x, position.y, position.z), Vector3(size.x, size.y, size.z), thisCollision) ||
							CollisionDetection::AABBTest(r.GetPosition(), Vector3(position.x, position.y, position.z), Vector3(0.1f, 0.1f, 0.1f), Vector3(size.x,size.y, size.z)))) {
						func(contents);
					}
					//std::cout << std::endl;

				}
			}

		protected:
			std::list<OctTreeEntry<T>> contents;

			Vector3 position;
			Vector3 size;

			OctTreeNode<T>* children;
		};
	}
}


namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		template<class T>
		class OctTree
		{
		public:
			OctTree(Vector3 pos, Vector3 size, int maxDepth = 6, int maxSize = 5) {
				root = OctTreeNode<T>(pos, size);
				this->maxDepth = maxDepth;
				this->maxSize = maxSize;
			}
			~OctTree() {
			}

			void SetPos(Vector3 p) {
				root.SetPos(p);
			}

			void SetSize(Vector3 s) {
				root.SetSize(s);
			}

			void SetDepth(float d) {
				maxDepth = d;
			}

			void SetMaxObj(float o) {
				maxSize = o;
			}

			void Insert(T object, const Vector3& pos, const Vector3& size) {
				root.Insert(object, pos, size, maxDepth, maxSize);
			}

			void DebugDraw() {
				root.DebugDraw();
			}

			void OperateOnContents(typename OctTreeNode<T>::OctTreeFunc  func) {
				root.OperateOnContents(func);
			}

			void OperateOnRayCastContents(Ray& r, typename OctTreeNode<T>::OctTreeFunc func) {
				root.OperateOnRayCastContents(r, func);
			}

			void Clear() {
				root.Clear();
			}

		protected:
			OctTreeNode<T> root;
			int maxDepth;
			int maxSize;
		};
	}
}