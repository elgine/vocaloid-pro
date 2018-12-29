#include "stdafx.h"

void main() {
	auto player = new Player();
	player->SetSource("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	player->SetEffect(Effects::FEMALE);
	player->Play(true);

	getchar();
	player->Stop();
}