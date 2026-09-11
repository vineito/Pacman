#pragma once


#include "BTGhostController.h"
#include "Controller.h"

class PinkyScatter : public Behavior {
public:
	Status update() override;
};

class PinkyChase : public Behavior {
public:
	Status update() override;
};

class PinkyController: public Controller {
	std::shared_ptr<Composite> root;

public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game)override;
};

