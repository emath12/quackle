#include <algorithm>
#include <iostream>
#include <limits>

#include <boardparameters.h>
#include <bogowinplayer.h>
#include <computerplayercollection.h>
#include <datamanager.h>
#include <endgameplayer.h>
#include <enumerator.h>
#include <game.h>
#include <gameparameters.h>
#include <lexiconparameters.h>
#include <reporter.h>
#include <resolvent.h>
#include <strategyparameters.h>

#include <quackleio/dictimplementation.h>
#include <quackleio/flexiblealphabet.h>
#include <quackleio/froggetopt.h>
#include <quackleio/gcgio.h>
#include <quackleio/util.h>

int main(int argc, char *argv[]) {

  Quackle::DataManager dataManager;

  dataManager.setAppDataDirectory(
      "/Users/ethanmathieu/Library/Application Support/Quackle.org/Quackle");
  dataManager.lexiconParameters()->loadDawg(
      Quackle::LexiconParameters::findDictionaryFile("twl06.dawg"));
  dataManager.lexiconParameters()->loadGaddag(
      Quackle::LexiconParameters::findDictionaryFile("twl98.gaddag"));
  dataManager.strategyParameters()->initialize("twl98");
  dataManager.setBoardParameters(new Quackle::EnglishBoard());

  std::cout << "Hello, QUACKLECLI!" << std::endl;

  Quackle::Game game;

  Quackle::PlayerList players;

  Quackle::Player bogowinA(MARK_UV("BogowinA"),
                           Quackle::Player::ComputerPlayerType, 110);

  bogowinA.setComputerPlayer(new Quackle::SmartBogowin());

  players.push_back(bogowinA);

  Quackle::Player bogowinB(MARK_UV("BogowinB"),
                           Quackle::Player::ComputerPlayerType, 110);
  bogowinB.setComputerPlayer(new Quackle::SmartBogowin());
  players.push_back(bogowinB);

  game.setPlayers(players);
  game.associateKnownComputerPlayers();

  game.addPosition();

  // const bool setupRetroPosition = false;

  // if (setupRetroPosition) {
  //   game.commitMove(Quackle::Move::createPlaceMove(
  //       MARK_UV("8c"),
  //       QUACKLE_ALPHABET_PARAMETERS->encode(MARK_UV("AMNION"))));
  //   game.currentPosition().setCurrentPlayerRack(
  //       Quackle::Rack(QUACKLE_ALPHABET_PARAMETERS->encode(MARK_UV("L"))));
  //   UVcout << "current rack: " <<
  //   game.currentPosition().currentPlayer().rack()
  //          << std::endl;
  //   game.currentPosition().kibitz(10);
  //   UVcout << "moves: " << std::endl
  //          << game.currentPosition().moves() << std::endl;
  // }

  const int playahead = 50;

  for (int i = 0; i < playahead; ++i) {
    if (game.currentPosition().gameOver()) {
      UVcout << "GAME OVER" << std::endl;
      break;
    }

    const Quackle::Player player(game.currentPosition().currentPlayer());
    Quackle::Move compMove(game.haveComputerPlay());
    UVcout << "with " << player.rack() << ", " << player.name()
           << " commits to " << compMove << std::endl;
  }

  UVcout << game.currentPosition() << std::endl;

  return 0;
}

// int main(int argc, char *argv[]) {
//   std::cout << "Hello, QuackleCLI!" << std::endl;

//   // Initialize the DataManager
//   Quackle::DataManager dm;
//   dm.setComputerPlayers(Quackle::ComputerPlayerCollection::fullCollection());

//   // Create two computer players
//   Quackle::PlayerList players;

//   Quackle::Player bogowinA(MARK_UV("BogowinA"),
//                            Quackle::Player::ComputerPlayerType, 110);
//   bogowinA.setComputerPlayer(new Quackle::SmartBogowin());
//   players.push_back(bogowinA);

//   Quackle::Player bogowinB(MARK_UV("BogowinB"),
//                            Quackle::Player::ComputerPlayerType, 110);
//   bogowinB.setComputerPlayer(new Quackle::SmartBogowin());
//   players.push_back(bogowinB);

//   // Set up the game with the players
//   Quackle::Game game;
//   game.setPlayers(players);
//   game.associateKnownComputerPlayers();
//   game.addPosition();

//   const bool setupRetroPosition = false;

//   if (setupRetroPosition) {
//     game.commitMove(Quackle::Move::createPlaceMove(
//         MARK_UV("8c"),
//         QUACKLE_ALPHABET_PARAMETERS->encode(MARK_UV("AMNION"))));
//     game.currentPosition().setCurrentPlayerRack(
//         Quackle::Rack(QUACKLE_ALPHABET_PARAMETERS->encode(MARK_UV("L"))));
//     std::cout << "Current rack: "
//               << game.currentPosition().currentPlayer().rack() << std::endl;
//     game.currentPosition().kibitz(10);
//     std::cout << "Moves: " << game.currentPosition().moves() << std::endl;
//   }

//   const int playahead = 50;

//   // Main game loop
//   for (int i = 0; i < playahead; ++i) {
//     if (game.currentPosition().gameOver()) {
//       std::cout << "GAME OVER" << std::endl;
//       break;
//     }

//     Quackle::Player currentPlayer = game.currentPosition().currentPlayer();
//     Quackle::Move compMove = game.haveComputerPlay();

//     // Output player's status and move
//     std::cout << "With " << currentPlayer.rack().toString() << ", "
//               << currentPlayer.name()
//               << " commits to move: " << compMove.debugString() << std::endl;

//     // Optionally, print the board after each move
//     std::cout << "Board:\n"
//               << game.currentPosition().board().toString() << std::endl;
//   }

//   std::cout << "Final board position:\n"
//             << game.currentPosition().board().toString() << std::endl;

//   // Optionally, generate a report or run additional tests here
//   // testGameReport(game);

//   return 0;
// }
