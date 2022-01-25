#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>
#include <functional>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			GameObject(string name, std::function<void(float,GameObject* a)> updatefunc);
			~GameObject();

			void UpdateGlobalTransform();

			void Update(float dt) {
				updateFunc(dt,this);
				for (auto it = children.begin(); it != children.end(); ++it) {
					(*it)->Update(dt);
				}

			}

			void changeOrigin(Vector3 t);

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			Transform& GetTransform() {
				return transform;
			}

			Transform& GetLocalTransform() {
				return localTransform;
			}

			Vector3 GetOffset() {
				return offset;
			}

			Vector3 GetLocalOffset() {
				return LocalOffset;
			}

			void SetLocalOffset(Vector3 LocalOffset) {
				this->LocalOffset = LocalOffset;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}

			void SetName(string name){
				this->name = name;
			}

			const int GetLayer() const {
				return layer;
			}

			void SetLayer(int layer){
				this->layer = layer;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				if (this->GetName() == "player" && otherObject->GetName() == "finalPlate") {
					this->name = "Win";
				}
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

			GameObject* SetParent(GameObject* p) {
				parent = p;
				return parent;
			}

			GameObject* GetParent() {
				return parent;
			}

			GameObject* Pop();

			GameObject* AddChild(GameObject* newChild);
			GameObject* PopChild(string name);
			GameObject* PopChild(int id);
			GameObject* PopChild(GameObject* ptr);
			GameObject* FindChild(string name);
			GameObject* FindChild(int id);
			GameObject* FindChild(GameObject* ptr);
			std::vector<GameObject*> GetChildren() {
				return children;
			}
			std::vector<GameObject*>::iterator GetChilrenBegin() {
				return children.begin();
			}
			std::vector<GameObject*>::iterator GetChilrenEnd() {
				return children.end();
			}
			int GetChildrenSize() const {
				return children.size();
			}

			void OnSpreadChild(std::function<void(GameObject*)> f) {
				if (GetChildrenSize() > 0) {
					for(GameObject* i : children) {
						f(i);
						i->OnSpreadChild(f);
					}
				}
			}

			std::function<void(float, GameObject* a)> GetupdateFunc() {
				return updateFunc;
			}

			void SetupdateFunc(std::function<void(float, GameObject* a)> func) {
				updateFunc = func;
			}

		protected:
			Transform			transform;  // Auto calculate
			Transform			localTransform;
			Vector3			offset; // Auto calculate
			Vector3			LocalOffset;
			std::function<void(float, GameObject* a)> updateFunc;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;

			bool	isActive;
			int		worldID;
			int		layer;
			string	name;

			Vector3 broadphaseAABB;
			GameObject* parent;
			std::vector<GameObject*> children;
		};
	}
}

