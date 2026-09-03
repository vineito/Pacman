#include "PacmanDTController.h"
#include "GameState.h"
#include "Ghost.h"
#include <iostream>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanDTController::~PacmanDTController() {

}

Move PacmanDTController::getMove(const GameState& game){

    if(character->getDirection() == PASS){
        return LEFT;
    }

    int pacmanNode = game.getPacmanPos();
    auto pacmanCoords = game.getMaze().getNodePos(pacmanNode);

    float minDistToGhost = 10000000;
    int targetGhostNode = -1;
    bool targetGhostEdible = false;

    for(int i = 0; i < 4; i++){

        int ghostNode = game.getGhostsPos(i);
        auto ghostCoords = game.getMaze().getNodePos(ghostNode);

        float dist = euclid2(pacmanCoords, ghostCoords); 

        if(dist < minDistToGhost) {
            minDistToGhost = dist;
            targetGhostNode = ghostNode;
            targetGhostEdible = game.isGhostEdible(i);
        }
    }

    if(targetGhostNode == -1){
        return character-> getDirection();
    }

    std::vector<Move> moves = game.getMaze().getGhostLegalMoves(pacmanNode, character->getDirection());
    Move bestMove = character->getDirection();

    auto targetGhostCoords = game.getMaze().getNodePos(targetGhostNode);

	if(targetGhostEdible){
		float minMoveDist=10000000;

		for(Move m:moves){
			int vecino = game.getMaze().getNeighbour(pacmanNode, m);
            if(vecino < 0) continue;
            
            auto vecinoCoords = game.getMaze().getNodePos(vecino);
            float sqDist = euclid2(vecinoCoords, targetGhostCoords);
            
            if(sqDist < minMoveDist) {
                minMoveDist = sqDist;
                bestMove = m;
            }
		}

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
}