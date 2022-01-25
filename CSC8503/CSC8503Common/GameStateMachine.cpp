#include "GameStateMachine.h"

namespace NCL {
	namespace CSC8503 {
		GameStateMachine::GameStateMachine() : StateMachine() {
			timeLapse = 0;
			level = 0;
		}

		GameStateMachine::~GameStateMachine() {

		}
	}
}