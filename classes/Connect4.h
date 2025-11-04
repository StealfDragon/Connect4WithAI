#pragma once
#include "Game.h"

class Connect4 : public Game {
public: 
    Connect4();
    ~Connect4();

    void        setUpBoard() override;
    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    void        stopGame() override;

	void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid*       getGrid() override { return _grid; }

private:
    // Constants for piece types
    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    // Player constants
    static const int RED_PLAYER = 0;
    static const int YELLOW_PLAYER = 1;

    // Helper methods
    Bit*        createPiece(int pieceType);
    int         getPieceType(const Bit& bit) const;
    bool        isValidMove(int srcX, int srcY, int dstX, int dstY, Player* player) const;
    void        getBoardPosition(BitHolder &holder, int &x, int &y) const;
    bool        isValidSquare(int x, int y) const;

    bool        aiBoardFull(std::string& state);
    int         evaluate(std::string& state);
    int         negamax(std::string& state, int depth, int playerColor);
    int         alphaBeta();

    // Board representation
    Grid*        _grid;

    // Game state
    int         _redPieces;
    int         _yellowPieces;
};