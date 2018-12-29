#include "stdafx.h"

void main() {
	auto player = new Player();
	player->SetSource("G:\\Projects\\VSC++\\vocaloid\\samples\\female-voice.wav");
	player->SetEffect(Effects::TRANSFORMER);
	player->Play(true);

	getchar();
	player->Stop();
}