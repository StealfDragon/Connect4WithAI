#include "Connect4.h"

Connect4::Connect4() : Game() {
    _grid = new Grid(8, 8);
    _redPieces = 0;
    _yellowPieces = 0;
}

Connect4::Connect4() {
    delete _grid;
}

void Connect4::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;

    _grid->initializeSquares(80, "square.png");
    for(int y = 0; y < 6; y++) {
        for(int x = 0; x < 7; x++) {
            _grid->setEnabled(x, y, true);
        }
    }
}