#pragma once
#include "role.hpp"
#include "roles.h"
#include "alien.hpp"
#include "robot1.hpp"
#include "robot2.hpp"
#include "robot3.hpp"
#include "robot4.hpp"
#include "astronaut.hpp"
#include "uncle.hpp"
#include "female.hpp"
#include "child.hpp"
#include "male.hpp"
#include "old_male.hpp"
#include "old_female.hpp"
#include "transformer.hpp"
#include "balrog.hpp"
namespace role {
	
	Role* RoleFactory(Roles id, BaseAudioContext *ctx) {
		Role* role = nullptr;
		switch (id) {
			case Roles::ALIEN:role = new Alien(ctx); break;
			case Roles::ROBOT1:role = new Robot1(ctx); break;
			case Roles::ROBOT2:role = new Robot2(ctx); break;
			case Roles::ROBOT3: role = new Robot3(ctx); break;
			case Roles::ROBOT4: role = new Robot4(ctx); break;
			case Roles::ASTRONAUT:role = new Astronaut(ctx); break;
			case Roles::UNCLE:role = new Uncle(ctx); break;
			case Roles::FEMALE:role = new Female(ctx); break;
			case Roles::CHILD:role = new Child(ctx); break;
			case Roles::MALE:role = new Male(ctx); break;
			case Roles::OLD_MALE:role = new OldMale(ctx); break;
			case Roles::OLD_FEMALE:role = new OldFemale(ctx); break;
			case Roles::TRANSFORMER:role = new Transformer(ctx); break;
			case Roles::BALROG:role = new Balrog(ctx); break;
		}
		return role;
	}
}