#pragma once
#include "StateMachine.h"

namespace NCL {
	namespace CSC8503 {


		class GameStateMachine : public StateMachine {
		public:
			GameStateMachine();
			~GameStateMachine();

			float GetTimeLapse() { return timeLapse; }
			void SetTimeLapse(float timeLapse) { this->timeLapse = timeLapse; }

			int GetLevel() { return level; }
			void SetLevel(int level) { this->level = level; }

			int GetMainChoice() { return mainChoice; }
			void SetMainChoice(int mainChoice) { this->mainChoice = mainChoice; }

			int GetPauseChoice() { return pauseChoice; }
			void SetPauseChoice(int pauseChoice) { this->pauseChoice = pauseChoice; }

			int GetScore() { return score; }
			void SetScore(int score) { this->score = score; }

		protected:
			int level = 0;
			float timeLapse = 0;
			int mainChoice = 0;
			int pauseChoice = 0;
			int score = 0;
		};
	}
}