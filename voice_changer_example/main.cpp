#include "../voice_changer/voice_player.hpp"
void main() {
	auto player = new VoicePlayer();
	player->Open("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	player->SetEffect(Effects::UNDER_WATER);
	player->PlayAll();
	player->Loop(true);
	player->Start();
	getchar();
	player->Stop();
}