/*
 * FSMController.cpp
 *
 *  Created on: Apr 23, 2018
 *      Author: nbarriga
 */

#include "FSMController.h"
#include <iostream>

FSMController::FSMController(std::shared_ptr<Character> character):
	Controller(character),
	e(rand()),
	uniform_dist(0,3),
	fsm(std::make_shared<ExampleStateMachine>(character)) {
}

FSMController::~FSMController() {
	// TODO Auto-generated destructor stub
}

Move 
FSMController::getMove(const GameState& game){
	return fsm->update(game);
}


///////////////////////////////////PillTransition///////////////////////////////
PillTransition::PillTransition(std::shared_ptr<FSMState> next):last(0),_next(next){

}

//Pregunta cuantas pildoras quedan en el laberinto
//Si el numero de pildoras es multiplo de 20 y es distinto al ultimo que se guardo
//entonces se activa la transicion
bool PillTransition::isValid(const GameState& gs){
	int quedan=gs.getMaze().getPillPositions().size();
	if(last!=quedan && quedan%20==0){
		last =quedan;
		return true;
	}
	return false;
}
std::shared_ptr<FSMState> PillTransition::getNextState(){
	return _next;
}

////TimeTransition////
TimeTransition::TimeTransition(std::shared_ptr<FSMState> next,int _segundos):segundos(_segundos),_next(next){
	//me aseguro de que el reloj se inicializa en el momento
	resetTimer();
}

//actializo la variable reloj con la hora actual
void TimeTransition::resetTimer(){
	reloj=std::chrono::high_resolution_clock::now();
}

bool TimeTransition::isValid(const GameState&){
	//Esto es para calcular si pasaron los segundos que se le pasaron al constructor
	auto ahora=std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duracion=ahora-reloj;

	if(duracion.count()>=segundos){
		resetTimer();
		return true;
	}
	return false;
}

std::shared_ptr<FSMState> TimeTransition::getNextState(){
	return _next;
}


///////////////////////////////ChaseState///////////////////////////////////////
ChaseState::ChaseState(std::shared_ptr<Character> _character):FSMState(_character){

}
void ChaseState::onEnter(const GameState& ){
	std::cout << "Azul entro a CHASE" << std::endl;
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}
Move ChaseState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto pacmanCoord=game.getMaze().getNodePos(game.getPacmanPos());
	const auto myPos=character->getPos();
	//const auto myCoord=game.getMaze().getNodePos(myPos);

	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float min=euclid2(
		game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[0])),
			pacmanCoord);
	int minI=0;
	for(unsigned int i=1;i<moves.size();i++){
		auto dist=euclid2(
			game.getMaze().getNodePos(game.getMaze().getNeighbour(myPos,moves[i])),
			pacmanCoord);
		if(dist<min){
			min=dist;
			minI=i;
		}
	}
	return moves[minI];
}
ChaseState::~ChaseState(){

}

////ScatterState////
//El contructor recibe el fantasma y el objetivo al que se quiere ir
ScatterState::ScatterState(std::shared_ptr<Character> _character, std::pair<int, int> objetivo):FSMState(_character), objetivo(objetivo){

}

//Cuando se entra al estado se invierte la direccion del fantasma
void ScatterState::onEnter(const GameState& ){
	std::cout << "Azul entro a SCATTER" << std::endl;
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move ScatterState::onUpdate(const GameState& game){
	std::vector<Move> moves;
	const auto myPos=character->getPos();

	//Preguntamos cuales son los movimientos legales del fantasma
	if(character->getDirection()==PASS){
		moves=game.getMaze().getPossibleMoves(myPos);
	}else{
		//Si es que ya esta en movimiento
		moves=game.getMaze().getGhostLegalMoves(myPos,character->getDirection());
	}

	float minMoveDist = 10000000;
	int minI=0;

	//Se revisa cada movimiento posible
	for(unsigned int i=0;i<moves.size();i++){
		auto baldosa=game.getMaze().getNeighbour(myPos,moves[i]);
		if(baldosa<0) continue;//Si es una pared se ingnora;

		//se traduce el numero en coordenadas para poder calcular la distancia
		auto baldosaCoord=game.getMaze().getNodePos(baldosa);

		float sqDist=euclid2(baldosaCoord,objetivo);


		if(sqDist<minMoveDist){
			minMoveDist=sqDist;
			minI=i;
		}
	}
	return moves[minI];

}

ScatterState::~ScatterState(){

}

/*Codigo de pacmanDTController.cpp
	}else{
		float maxMoveDist = -1;
        
        for(Move m : moves) {
            int vecino = game.getMaze().getNeighbour(pacmanNode, m);
            if(vecino < 0) continue; 
            
            auto vecinoCoords = game.getMaze().getNodePos(vecino);
            float sqDist = euclid2(vecinoCoords, targetGhostCoords); 
            
            if(sqDist > maxMoveDist) {
                maxMoveDist = sqDist;
                bestMove = m;
			}
		}
	}
	return bestMove;	
}*/


////NonFrightened////
NonFrightened::NonFrightened(std::shared_ptr<Character> _character):FSMState(_character){

	//Creamos los subestados
	auto chase=std::make_shared<ChaseState>(character);
	auto scatter=std::make_shared<ScatterState>(character,std::make_pair(0,0));
	
	//Creamos las transiciones
	auto toChase=std::make_shared<TimeTransition>(chase, 7);
	auto toScatter=std::make_shared<TimeTransition>(scatter, 20);

	//Los conecto entre si
	scatter->addTransition(toChase);
	chase->addTransition(toScatter);

	//Defino el subestado inicial
	subEstadoActual = scatter;
}

NonFrightened::~NonFrightened(){
}

void NonFrightened::onEnter(const GameState& gs){
	//comieza a usar el estado inicial
	if(subEstadoActual!=nullptr){
		subEstadoActual->onEnter(gs);
	}
}

Move NonFrightened::onUpdate(const GameState& gs){
	if(subEstadoActual!=nullptr){
		//revisamps si algun cronometro termino
		auto t=subEstadoActual->getActiveTransition(gs);
		//cambiamos si el tiempo termino
		if(t!=nullptr){
			subEstadoActual->onExit(gs);
			t->onTransition(gs);
			subEstadoActual=t->getNextState();
			subEstadoActual->onEnter(gs);
		}
		//al estado actual le pedimos que nos diga que movimiento hacer
		return subEstadoActual->onUpdate(gs);
	}
	//si algo falla se queda quieto
	return PASS;
}

/////////////////////////////////////BlinkyStateMachine/////////////////////////////
ExampleStateMachine::ExampleStateMachine(std::shared_ptr<Character> _character):FiniteStateMachine(_character){
	initialState = std::make_shared<NonFrightened>(character);
	activeState=initialState;
	states.push_back(initialState);
	activeState->addTransition(std::make_shared<PillTransition>(activeState)); // Arreglar
}



Move ExampleStateMachine::update(const GameState& gs){
	auto t=activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState=t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


ExampleStateMachine::~ExampleStateMachine(){

}


