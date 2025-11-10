#pragma once

#include "Game.h"
#include "Grid.h"
#include <string>

class Connect4 : public Game {
public:
    Connect4();
    ~Connect4();

    // Game lifecycle
    void        setUpBoard() override;
    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder &holder) override;
    void        stopGame() override;

    bool        canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool        canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;

    // AI
    void        updateAI() override;
    bool        gameHasAI() override { return true; }
    Grid*       getGrid() override { return _grid; }

private:
    // piece constants (gametag values)
    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    // player indices
    static const int RED_PLAYER = 0;
    static const int YELLOW_PLAYER = 1;

    // helpers
    Bit*        createPiece(int pieceType);
    int         getPieceType(const Bit& bit) const;
    void        getBoardPosition(BitHolder &holder, int &x, int &y) const;
    bool        isValidSquare(int x, int y) const;
    void        clearHighlights();

    // AI helpers
    bool        aiBoardFull(const std::string& state) const;
    int         evaluate(const std::string& state, int aiPieceChar) const;
    int         negamax(std::string& state, int depth, int currentPlayerChar, int aiPieceChar, int alpha, int beta);
    void        applyMoveToState(std::string& state, int col, char pieceChar) const;
    bool        columnFull(const std::string& state, int col) const;
    void bestPlayColumnAndReturn(int bestCol, int aiChar);

    // board
    Grid*       _grid;

    // counts (not strictly required but kept)
    int         _redPieces;
    int         _yellowPieces;
};