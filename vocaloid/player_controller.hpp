#pragma once
#include "player.h"
namespace vocaloid {
	namespace io {
		class PlayerController {
		private:

			Player *player_;

			int64_t bufferd_size_;

			// Has played
			int64_t played_size_;
		public:

			explicit PlayerController(Player* player) {
				player_ = player;
			}


		};
	}
}