#include <algorithm>
#include <boarddrivenplayer.h>
#include <boardparameters.h>
#include <bogowinplayer.h>
#include <chrono>
#include <computerplayercollection.h>
#include <datamanager.h>
#include <endgameplayer.h>
#include <enumerator.h>
#include <game.h>
#include <gameparameters.h>
#include <iostream>
#include <lexiconparameters.h>
#include <limits>
#include <map>
#include <quackleio/dictimplementation.h>
#include <quackleio/flexiblealphabet.h>
#include <quackleio/froggetopt.h>
#include <quackleio/gcgio.h>
#include <quackleio/util.h>
#include <reporter.h>
#include <resolvent.h>
#include <strategyparameters.h>

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

  const int gameCnt = 10;

  std::map<int, int> winnerInfo;
  std::vector<std::chrono::duration<double>> gameTimes;

  for (int gameIndex = 0; gameIndex < gameCnt; ++gameIndex) {
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Game: " << gameIndex << std::endl;

    Quackle::Game game;
    Quackle::PlayerList players;

    Quackle::Player speedyA(MARK_UV("Speedy A"),
                            Quackle::Player::ComputerPlayerType, 110);
    speedyA.setComputerPlayer(new Quackle::EndgamePlayer());
    players.push_back(speedyA);

    Quackle::Player boardDrivePlayer(MARK_UV("Board Driven Player"),
                                     Quackle::Player::ComputerPlayerType, 110);
    boardDrivePlayer.setComputerPlayer(new Quackle::BoardDrivenPlayer());
    players.push_back(boardDrivePlayer);

    game.setPlayers(players);
    game.associateKnownComputerPlayers();
    game.addPosition();

    const int playahead = 50;

    for (int i = 0; i < playahead; ++i) {
      if (game.currentPosition().gameOver()) {
        std::cout << "GAME OVER" << std::endl;
        Quackle::PlayerList winners(game.currentPosition().leadingPlayers());

        for (Quackle::PlayerList::const_iterator it = winners.begin();
             it != winners.end(); ++it) {
          std::cout << *it << " wins!!" << std::endl;
          winnerInfo[it->id()] += 1;
        }

        std::cout << game.currentPosition() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        gameTimes.push_back(end - start);
        break;
      }

      const Quackle::Player player(game.currentPosition().currentPlayer());
      Quackle::Move compMove(game.haveComputerPlay());
      std::cout << "with " << player.rack() << ", " << player.name()
                << " commits to " << compMove << std::endl;
    }
  }

  if (!gameTimes.empty()) {
    std::chrono::duration<double> totalDuration =
        std::chrono::duration<double>::zero();

    for (const auto &duration : gameTimes) {
      totalDuration += duration;
    }

    double averageDuration = totalDuration.count() / gameTimes.size();
    std::cout << "Average game time: " << averageDuration << " seconds"
              << std::endl;
  } else {
    std::cout << "No games played." << std::endl;
  }

  for (const auto &pair : winnerInfo) {
    std::cout << pair.first << ": " << pair.second << std::endl;
  }

  return 0;
}
