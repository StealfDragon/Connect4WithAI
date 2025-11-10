#include "Application.h"
#include "imgui/imgui.h"
#include "classes/TicTacToe.h"
#include "classes/Checkers.h"
#include "classes/Othello.h"
#include "classes/Connect4.h"

namespace ClassGame {
        //
        // our global variables
        //
        Game *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;
        int g_gameMode = 0;
        int g_aiSide = 0;  

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = nullptr;
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
                ImGui::DockSpaceOverViewport();

                //ImGui::ShowDemoWindow();

                ImGui::Begin("Settings");

                if (gameOver) {
                    ImGui::Text("Game Over!");
                    ImGui::Text("Winner: %d", gameWinner);
                    if (ImGui::Button("Reset Game")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                }

                ImGui::Separator();
                ImGui::Text("Game Mode:");
                ImGui::RadioButton("Two Player", &g_gameMode, 0); ImGui::SameLine();
                ImGui::RadioButton("Human vs AI", &g_gameMode, 1); ImGui::SameLine();
                ImGui::RadioButton("AI vs AI", &g_gameMode, 2);

                if (g_gameMode == 1) {
                    ImGui::Separator();
                    ImGui::Text("Choose which side AI plays:");
                    ImGui::RadioButton("AI plays Player 1 (left/top)", &g_aiSide, 0); ImGui::SameLine();
                    ImGui::RadioButton("AI plays Player 2 (right/bottom)", &g_aiSide, 1);
                }
                ImGui::Separator();

                if (!game) {
                    if (ImGui::Button("Start Tic-Tac-Toe")) {
                        game = new TicTacToe();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Checkers")) {
                        game = new Checkers();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Othello")) {
                        game = new Othello();
                        game->setUpBoard();
                    }
                    if (ImGui::Button("Start Connect 4")) {
                        game = new Connect4();

                        // Ensure the logical player count
                        game->setNumberOfPlayers(2);

                        // Build the board (this may create Player objects)
                        game->setUpBoard();

                        // Now set AI flags — do this after setUpBoard() to be safe.
                        // Default everybody to human first
                        for (int i = 0; i < 2; ++i) {
                            Player* p = game->getPlayerAt(i);
                            if (p) p->setAIPlayer(false);   // <-- use setAIPlayer, not setIsAIPlayer
                        }

                        if (g_gameMode == 1) { // Human vs AI
                            int aiIndex = (g_aiSide == 0) ? 0 : 1;
                            Player* p = game->getPlayerAt(aiIndex);
                            if (p) p->setAIPlayer(true);
                        } else if (g_gameMode == 2) { // AI vs AI
                            for (int i = 0; i < 2; ++i) {
                                Player* p = game->getPlayerAt(i);
                                if (p) p->setAIPlayer(true);
                            }
                        }
                    }
                } else {
                    ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                    ImGui::Text("Current Board State: %s", game->stateString().c_str());
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                if (game) {
                    game->drawFrame();
                    if (game->gameHasAI() && (game->getCurrentPlayer()->isAIPlayer() || game->_gameOptions.AIvsAI))
                    {
                        game->updateAI();
                    }
                }
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameOver = true;
                gameWinner = winner->playerNumber();
            }
            if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }
        }
}
