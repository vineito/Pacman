#pragma once //Si algun otro archivo ya leyo esta parte, no se leera de nuevo, esto es para evitar errores
//como que muchos archivos se conecten entre si

#include "Controller.h"//Con esto llamamos a controller.h y tenemos acceso a la formula que calcula la distancia

//Aqui declaramos el nombre de mi clase
class PacmanDTController: public Controller {

public:
	PacmanDTController(std::shared_ptr<Character> character);
	virtual ~PacmanDTController();
	virtual Move getMove(const GameState& game)override;
};