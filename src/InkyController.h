#pragma once

#include "BTGhostController.h"
#include "Controller.h"

class InkyScatter : public Behavior {
public:
	Status update() override;
};

class InkyPaparazzi : public Behavior {
public:
	Status update() override;
};

class PaparazziTimeOut : public Behavior {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
public:
	Status update() override;
	PaparazziTimeOut();
};

class InkyController: public Controller {
	std::shared_ptr<Composite> root;
public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game)override;
};

