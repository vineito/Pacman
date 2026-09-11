#include "PinkyController.h"

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

	std::pair<int, int> pinkyTarget(const GameState& game) {
		int node = game.getPacmanPos();
		const Move direction = static_cast<Move>(game.getPacmanDir());
		if (direction != PASS) {
			for (int distance = 0; distance < 4; ++distance) {
				const int next = game.getMaze().getNeighbour(node, direction);
				if (next < 0) {
					break;
				}
				node = next;
			}
		}
		return game.getMaze().getNodePos(node);
	}
}

PinkyController::PinkyController(std::shared_ptr<Character> character):
		Controller(character), root(std::make_shared<Selector>()) {
	auto frightened = std::make_shared<Filter>();
	frightened->addCondition(std::make_shared<Powerpill>());
	frightened->addAction(std::make_shared<Frightened>());
	root->addChild(frightened);

	auto scatter = std::make_shared<Filter>();
	scatter->addCondition(std::make_shared<TimeOut>());
	scatter->addAction(std::make_shared<PinkyScatter>());
	root->addChild(scatter);
	root->addChild(std::make_shared<PinkyChase>());
}

PinkyController::~PinkyController() = default;

Move PinkyController::getMove(const GameState& game) {
	Info::getInfo()->in_character = character;
	Info::getInfo()->in_gamestate = &game;
	root->tick();
	return Info::getInfo()->out_move;
}

Status PinkyScatter::update() {
	const auto character = Info::getInfo()->in_character;
	const auto game = Info::getInfo()->in_gamestate;
	const auto targets = game->getMaze().getPowerPillPositions();
	const auto target = targets.empty() ? game->getMaze().getNodePos(character->getPos())
			: targets.front();
	Info::getInfo()->out_move = moveTowards(character, *game, target);
	return BH_SUCCESS;
}

Status PinkyChase::update() {
	const auto character = Info::getInfo()->in_character;
	const auto game = Info::getInfo()->in_gamestate;
	Info::getInfo()->out_move = moveTowards(character, *game, pinkyTarget(*game));
	return BH_SUCCESS;
}