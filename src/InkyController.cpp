#include "InkyController.h"

#include <limits>

namespace {
	std::vector<Move> legalMoves(const std::shared_ptr<Character>& character,
			const GameState& game) {
		if (character->getDirection() == PASS) {
			return game.getMaze().getPossibleMoves(character->getPos());
		}
		return game.getMaze().getGhostLegalMoves(character->getPos(),
				character->getDirection());
	}

	Move moveTowards(const std::shared_ptr<Character>& character,
			const GameState& game, const std::pair<int, int>& target) {
		const auto moves = legalMoves(character, game);
		Move bestMove = PASS;
		float bestDistance = std::numeric_limits<float>::max();

		for (Move move : moves) {
			if (move == PASS) {
				continue;
			}
			const int neighbour = game.getMaze().getNeighbour(character->getPos(), move);
			if (neighbour < 0) {
				continue;
			}
			const float distance = euclid2(target, game.getMaze().getNodePos(neighbour));
			if (distance < bestDistance) {
				bestDistance = distance;
				bestMove = move;
			}
		}
		return bestMove;
	}
}

InkyController::InkyController(std::shared_ptr<Character> character):
		Controller(character), root(std::make_shared<Selector>()) {
	auto frightened = std::make_shared<Filter>();
	frightened->addCondition(std::make_shared<Powerpill>());
	frightened->addAction(std::make_shared<Frightened>());
	root->addChild(frightened);

	auto paparazzi = std::make_shared<Filter>();
	paparazzi->addCondition(std::make_shared<PaparazziTimeOut>());
	paparazzi->addAction(std::make_shared<InkyPaparazzi>());
	root->addChild(paparazzi);
	root->addChild(std::make_shared<InkyScatter>());
}

InkyController::~InkyController() = default;

Move InkyController::getMove(const GameState& game) {
	Info::getInfo()->in_character = character;
	Info::getInfo()->in_gamestate = &game;
	root->tick();
	return Info::getInfo()->out_move;
}

Status InkyScatter::update() {
	const auto character = Info::getInfo()->in_character;
	const auto game = Info::getInfo()->in_gamestate;
	const auto targets = game->getMaze().getPowerPillPositions();
	const auto target = targets.empty() ? game->getMaze().getNodePos(character->getPos())
			: targets.front();
	Info::getInfo()->out_move = moveTowards(character, *game, target);
	return BH_SUCCESS;
}

PaparazziTimeOut::PaparazziTimeOut() : Behavior() {
	lastTime = std::chrono::high_resolution_clock::now();
}

Status PaparazziTimeOut::update(){
	std::chrono::duration<float> timeStamp = std::chrono::high_resolution_clock::now() - lastTime;
	if( (int)timeStamp.count() % 47 < 40){
		return BH_SUCCESS;
	}else{
		return BH_FAILURE;
	}

}

Status InkyPaparazzi::update() {
	const auto character = Info::getInfo()->in_character;
	const auto game = Info::getInfo()->in_gamestate;

	int pacmanNode = game->getPacmanPos();
	auto pacmanCoords = game->getMaze().getNodePos(pacmanNode);
	auto inkyCoords = game->getMaze().getNodePos(character->getPos());

	float currentDist = euclid2(inkyCoords, pacmanCoords);

	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = game->getMaze().getPossibleMoves(character->getPos());
	} else {
		moves = game->getMaze().getGhostLegalMoves(character->getPos(),
				character->getDirection());
	}
	Move bestMove = PASS;

	if(currentDist > 4.0f){
		float minDistance = 10000000;
		for (Move move : moves) {
			if (move == PASS) continue;
			int neighbour = game->getMaze().getNeighbour(character->getPos(), move);
			if (neighbour < 0) continue;
			float distance = euclid2(pacmanCoords, game->getMaze().getNodePos(neighbour));
			if (distance < minDistance) {
				minDistance = distance;
				bestMove = move;
			}
		}
	}
	else{
		float maxDistance = -1;
		for (Move move : moves) {
			if (move == PASS) continue;
			int neighbour = game->getMaze().getNeighbour(character->getPos(), move);
			if (neighbour < 0) continue;
			float distance = euclid2(pacmanCoords, game->getMaze().getNodePos(neighbour));
			if (distance > maxDistance) {
				maxDistance = distance;
				bestMove = move;
			}
		}
	}

	Info::getInfo()->out_move = bestMove;
	return BH_SUCCESS;
}
