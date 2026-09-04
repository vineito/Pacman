/*
 * FSMController.h
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#ifndef FSMCONTROLLER_H_
#define FSMCONTROLLER_H_

#include "Controller.h"
#include <random>
#include <chrono>
#include "FSM.h"

class ExampleStateMachine;

class FSMController: public Controller {
	std::mt19937 e;
	std::uniform_int_distribution<int> uniform_dist;
	std::shared_ptr<ExampleStateMachine> fsm;
public:
	FSMController(std::shared_ptr<Character> character);
	virtual ~FSMController();
	virtual Move getMove(const GameState& game)override;
};

class PillTransition:public FSMTransition{
	int last;
	std::shared_ptr<FSMState> _next;
public:
	PillTransition(std::shared_ptr<FSMState> next);
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

//Lo que estoy haciendo en el lab4
//Esto es para cambiar de estado segun los segundos que pasen
class TimeTransition:public FSMTransition{
	int segundos;
	//Para guardar la hora exacta
	std::chrono::time_point<std::chrono::high_resolution_clock> reloj;
	std::shared_ptr<FSMState> _next;
public:
	//el constructor que pide los segundos y el estado de destino
	TimeTransition(std::shared_ptr<FSMState> next,int _segundos);
	void resetTimer();

	//aqui en el .cpp se implementa la funcion isValid que pregunta si ya pasaron los segundos
	bool isValid(const GameState& gs)override;
	std::shared_ptr<FSMState> getNextState()override;
};

class ChaseState:public FSMState{

public:
	ChaseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~ChaseState();

};

//Lo que estoy haciendo en el lab4
//Este estado hara que el fantasma se mueva a un punto del laberinto
class ScatterState:public FSMState{
	//aqui guardare la coordenada de la esquina
	std::pair<int, int> objetivo;
public:
	//Constructor que pide el el fantasma y la coordenada de la esquina
	ScatterState(std::shared_ptr<Character> _character, std::pair<int, int> objetivo);
	//los comportamientos
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~ScatterState();
};

//Lo que estoy haciendo en el lab4
class NonFrightened:public FSMState{
	//esta variable apuntara al subestado actual, que puede ser chase o scatter
	//segun los segundos que pasen
	std::shared_ptr<FSMState> subEstadoActual;
public:
	NonFrightened(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	void onEnter(const GameState& gs) override;
	~NonFrightened();
};

class ExampleStateMachine: public FiniteStateMachine{

public:
	ExampleStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~ExampleStateMachine();

};
#endif /* FSMCONTROLLER_H_ */
