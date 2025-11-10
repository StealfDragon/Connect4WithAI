#include "Connect4.h"
#include "ChessSquare.h"
#include "Bit.h"
#include <algorithm>
#include <limits>
#include <vector>

static const int COLUMNS = 7;
static const int ROWS = 6;
static const int STATE_SIZE = COLUMNS * ROWS;
static const int MAX_DEPTH = 6; // adjust for performance/strength

Connect4::Connect4() : Game() {
    _grid = new Grid(COLUMNS, ROWS);
    _redPieces = 0;
    _yellowPieces = 0;
}

Connect4::~Connect4() {
    stopGame();
    delete _grid;
}

void Connect4::setUpBoard() {
    // make sure we have the expected grid size
    // If you constructed Grid in the constructor with (7,6) this is fine.
    // If you used Grid(8,8) earlier, consider switching to Grid(7,6) in the ctor.
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;

    // Initialize all squares (this loads the sprite used for each cell)
    // Make sure "square.png" exists in your resources folder.
    _grid->initializeSquares(80, "square.png");

    // Ensure enabled only in the 7x6 region (in case grid bigger)
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        if (x >= 0 && x < 7 && y >= 0 && y < 6) {
            _grid->setEnabled(x, y, true);
        } else {
            _grid->setEnabled(x, y, false);
        }
        // clear any stray pieces / highlights
        square->destroyBit();
        square->setHighlighted(false);
    });

    // Optionally choose an AI player by default (like TicTacToe does)
    // If you don't want AI on by default, comment this out
    // setAIPlayer(AI_PLAYER);

    // mark the game as started so drawFrame() knows to render it
    startGame();
}

Bit* Connect4::createPiece(int pieceType) {
    Bit* bit = new Bit();
    bool isRed = (pieceType == RED_PIECE);
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(isRed ? RED_PLAYER : YELLOW_PLAYER));
    bit->setGameTag(pieceType);
    return bit;
}

int Connect4::getPieceType(const Bit& bit) const {
    return bit.gameTag();
}

void Connect4::getBoardPosition(BitHolder &holder, int &x, int &y) const {
    ChessSquare* sq = static_cast<ChessSquare*>(&holder);
    x = sq->getColumn();
    y = sq->getRow();
}

bool Connect4::isValidSquare(int x, int y) const {
    return _grid->isValid(x, y) && _grid->isEnabled(x, y);
}

void Connect4::clearHighlights() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y){
        square->setHighlighted(false);
    });
}

bool Connect4::actionForEmptyHolder(BitHolder &holder) {
    // When user clicks any holder in a column, drop the current player's piece
    int col, dummy;
    getBoardPosition(holder, col, dummy);

    // find lowest empty row in this column
    for (int row = ROWS - 1; row >= 0; --row) {
        ChessSquare* target = _grid->getSquare(col, row);
        if (!target) return false;
        if (!_grid->isEnabled(col, row)) return false;
        if (!target->bit()) {
            // create piece for current player
            Player* cur = getCurrentPlayer();
            if (!cur) return false;
            int playerNum = cur->playerNumber(); // expecting 0 or 1
            int pieceType = (playerNum == RED_PLAYER) ? RED_PIECE : YELLOW_PIECE;
            Bit* bit = createPiece(pieceType);
            if (!bit) return false;

            // set start position above board if you want falling animation
            ImVec2 startPos = target->getPosition();
            startPos.y = startPos.y - 80.0f * (ROWS); // start well above
            bit->setPosition(startPos);
            // place bit in holder (this takes ownership)
            target->setBit(bit);

            // animate piece to final spot if engine supports moveTo()
            // Many other classes call bit->moveTo() to animate; do the same.
            bit->moveTo(target->getPosition());

            // update counters
            if (pieceType == RED_PIECE) ++_redPieces; else ++_yellowPieces;

            // clear highlights
            clearHighlights();

            endTurn();
            return true;
        }
    }

    // column full, no action
    return false;
}

Player* Connect4::checkForWinner() {
    const int dirs[4][2] = { {1,0}, {0,1}, {1,1}, {1,-1} };

    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (!sq || !sq->bit()) continue;
            Player* owner = sq->bit()->getOwner();
            for (int d = 0; d < 4; ++d) {
                int dx = dirs[d][0], dy = dirs[d][1];
                int count = 1;
                for (int k = 1; k < 4; ++k) {
                    ChessSquare* nxt = _grid->getSquare(x + dx*k, y + dy*k);
                    if (!nxt || !nxt->bit() || nxt->bit()->getOwner() != owner) break;
                    count++;
                }
                if (count == 4) {
                    // highlight winning four
                    for (int k = 0; k < 4; ++k) {
                        ChessSquare* w = _grid->getSquare(x + dx*k, y + dy*k);
                        if (w) w->setHighlighted(true);
                    }
                    return owner;
                }
            }
        }
    }
    return nullptr;
}

bool Connect4::checkForDraw() {
    bool full = true;
    _grid->forEachSquare([&full](ChessSquare* square, int x, int y){
        if (!square->bit()) full = false;
    });
    return full;
}

std::string Connect4::initialStateString() {
    return std::string(STATE_SIZE, '0');
}

std::string Connect4::stateString() {
    std::string s;
    s.reserve(STATE_SIZE);
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (sq && sq->bit()) {
                int tag = sq->bit()->gameTag();
                if (tag == RED_PIECE) s.push_back('1');
                else if (tag == YELLOW_PIECE) s.push_back('2');
                else s.push_back('0');
            } else {
                s.push_back('0');
            }
        }
    }
    return s;
}

void Connect4::setStateString(const std::string &s) {
    if ((int)s.length() != STATE_SIZE) return;

    _redPieces = 0;
    _yellowPieces = 0;

    int idx = 0;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            ChessSquare* sq = _grid->getSquare(x, y);
            if (!sq) { ++idx; continue; }
            // remove any existing piece
            sq->destroyBit();
            char c = s[idx++];
            if (c == '1' || c == '2') {
                int pieceType = (c == '1') ? RED_PIECE : YELLOW_PIECE;
                Bit* b = createPiece(pieceType);
                b->setPosition(sq->getPosition());
                sq->setBit(b);
                if (pieceType == RED_PIECE) ++_redPieces; else ++_yellowPieces;
            }
        }
    }
}

void Connect4::stopGame() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y){
        square->destroyBit();
        square->setHighlighted(false);
    });
    _redPieces = 0;
    _yellowPieces = 0;
}

bool Connect4::canBitMoveFrom(Bit &bit, BitHolder &src) {
    (void)bit; (void)src; // avoid unused warnings
    return false;
}

bool Connect4::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    (void)bit; (void)src; (void)dst;
    return false;
}

//
// -------------------- AI implementation --------------------
//

bool Connect4::aiBoardFull(const std::string& state) const {
    return state.find('0') == std::string::npos;
}

bool Connect4::columnFull(const std::string& state, int col) const {
    // top row (y=0) index = 0 * COLUMNS + col
    return state[col] != '0';
}

void Connect4::applyMoveToState(std::string& state, int col, char pieceChar) const {
    // drop from bottom (y = ROWS-1) upward
    for (int y = ROWS - 1; y >= 0; --y) {
        int idx = y * COLUMNS + col;
        if (state[idx] == '0') {
            state[idx] = pieceChar;
            return;
        }
    }
    // column full -> no change
}

int Connect4::evaluate(const std::string& state, int aiPieceChar) const {
    const int OPP = (aiPieceChar == 1) ? 2 : 1;
    const int WIN_SCORE = 100000;
    const int THREE_OPEN = 1000;
    const int TWO_OPEN = 50;

    auto scoreWindow = [&](int x0, int y0, int dx, int dy) -> int {
        int aiCount = 0, oppCount = 0, emptyCount = 0;
        for (int k = 0; k < 4; ++k) {
            int x = x0 + dx * k;
            int y = y0 + dy * k;
            if (x < 0 || x >= COLUMNS || y < 0 || y >= ROWS) return 0; // invalid window
            char c = state[y * COLUMNS + x];
            if (c == '0') ++emptyCount;
            else if (c - '0' == aiPieceChar) ++aiCount;
            else ++oppCount;
        }
        if (aiCount == 4) return WIN_SCORE;
        if (oppCount == 4) return -WIN_SCORE;

        int val = 0;
        if (aiCount == 3 && emptyCount == 1) val += THREE_OPEN;
        else if (aiCount == 2 && emptyCount == 2) val += TWO_OPEN;
        if (oppCount == 3 && emptyCount == 1) val -= (THREE_OPEN - 200); // slightly prioritize blocking
        else if (oppCount == 2 && emptyCount == 2) val -= TWO_OPEN;
        return val;
    };

    int total = 0;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            total += scoreWindow(x, y, 1, 0);  // horizontal
            total += scoreWindow(x, y, 0, 1);  // vertical
            total += scoreWindow(x, y, 1, 1);  // diag down-right
            total += scoreWindow(x, y, 1, -1); // diag up-right
        }
    }
    return total;
}

int Connect4::negamax(std::string& state, int depth, int currentPlayerChar, int aiPieceChar, int alpha, int beta) {
    // currentPlayerChar: 1 or 2 (which player is about to move)
    // aiPieceChar: the char representing AI (1 or 2)
    int evalScore = evaluate(state, aiPieceChar);

    // terminal checks
    if (std::abs(evalScore) >= 100000) {
        // someone has 4 in a row in state
        return evalScore;
    }
    if (aiBoardFull(state) || depth >= MAX_DEPTH) {
        return evalScore;
    }

    int best = -10000000;
    // order columns: prefer center for heuristic
    const int colOrder[7] = {3, 2, 4, 1, 5, 0, 6};
    for (int ic = 0; ic < COLUMNS; ++ic) {
        int col = colOrder[ic];
        if (columnFull(state, col)) continue;
        std::string next = state;
        char ch = (currentPlayerChar == 1) ? '1' : '2';
        applyMoveToState(next, col, ch);
        int nextPlayer = (currentPlayerChar == 1) ? 2 : 1;
        int val = -negamax(next, depth + 1, nextPlayer, aiPieceChar, -beta, -alpha);
        if (val > best) best = val;
        alpha = std::max(alpha, val);
        if (alpha >= beta) break; // alpha-beta prune
    }
    return best;
}

void Connect4::updateAI() {
    Player* cur = getCurrentPlayer();
    if (!cur) return;
    int aiIndex = cur->playerNumber();
    int aiChar = (aiIndex == RED_PLAYER) ? 1 : 2;
    int oppChar = (aiChar == 1) ? 2 : 1;

    std::string state = stateString();

    // 1) Immediate win
    for (int col = 0; col < COLUMNS; ++col) {
        if (columnFull(state, col)) continue;
        std::string tmp = state;
        applyMoveToState(tmp, col, char('0' + aiChar));
        if (evaluate(tmp, aiChar) >= 100000) {
            // play this winning move
            bestPlayColumnAndReturn(col, aiChar);
            return;
        }
    }

    // 2) Immediate block of opponent's win
    for (int col = 0; col < COLUMNS; ++col) {
        if (columnFull(state, col)) continue;
        std::string tmp = state;
        applyMoveToState(tmp, col, char('0' + oppChar));
        if (evaluate(tmp, oppChar) >= 100000) {
            // must block by playing this column
            bestPlayColumnAndReturn(col, aiChar);
            return;
        }
    }

    // 3) Negamax search with alpha-beta and center-first ordering
    int bestCol = -1;
    int bestScore = -100000000;
    const int colOrder[7] = {3, 2, 4, 1, 5, 0, 6};
    for (int i = 0; i < COLUMNS; ++i) {
        int col = colOrder[i];
        if (columnFull(state, col)) continue;
        std::string s2 = state;
        applyMoveToState(s2, col, char('0' + aiChar));
        int nextPlayer = oppChar;
        int score = -negamax(s2, 1, nextPlayer, aiChar, -100000000, 100000000);
        if (score > bestScore) {
            bestScore = score;
            bestCol = col;
        }
    }

    if (bestCol == -1) {
        for (int c = 0; c < COLUMNS; ++c) if (!columnFull(state, c)) { bestCol = c; break; }
    }

    if (bestCol >= 0) {
        bestPlayColumnAndReturn(bestCol, aiChar);
    }
}

void Connect4::bestPlayColumnAndReturn(int bestCol, int aiChar) {
    for (int row = ROWS - 1; row >= 0; --row) {
        ChessSquare* sq = _grid->getSquare(bestCol, row);
        if (!sq) continue;
        if (!sq->bit()) {
            int pieceType = (aiChar == 1) ? RED_PIECE : YELLOW_PIECE;
            Bit* bit = createPiece(pieceType);
            if (!bit) return;
            ImVec2 startPos = sq->getPosition();
            startPos.y = startPos.y - 80.0f * (ROWS);
            bit->setPosition(startPos);
            sq->setBit(bit);
            bit->moveTo(sq->getPosition());
            if (pieceType == RED_PIECE) ++_redPieces; else ++_yellowPieces;
            clearHighlights();
            endTurn();
            return;
        }
    }
}