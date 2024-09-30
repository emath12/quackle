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
#include <mutex>
#include <quackleio/dictimplementation.h>
#include <quackleio/flexiblealphabet.h>
#include <quackleio/froggetopt.h>
#include <quackleio/gcgio.h>
#include <quackleio/util.h>
#include <reporter.h>
#include <resolvent.h>
#include <strategyparameters.h>
#include <thread>
#include <vector>

std::mutex data_mutex;

// Function to simulate a game and log results
void simulateGames(int startGameIndex, int numGames,
                   Quackle::DataManager &dataManager,
                   std::map<int, int> &winnerInfo,
                   std::vector<std::chrono::duration<double>> &gameTimes) {
  for (int gameIndex = startGameIndex; gameIndex < startGameIndex + numGames;
       ++gameIndex) {
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Simulating Game: " << gameIndex << std::endl;

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
        std::cout << "GAME OVER in Game: " << gameIndex << std::endl;
        Quackle::PlayerList winners(game.currentPosition().leadingPlayers());

        // Logging game outcome
        for (Quackle::PlayerList::const_iterator it = winners.begin();
             it != winners.end(); ++it) {
          std::cout << "Player " << *it << " wins!!" << std::endl;

          // Safely update winner information
          {
            std::lock_guard<std::mutex> lock(data_mutex);
            winnerInfo[it->id()] += 1;
          }
        }

        std::cout << game.currentPosition() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        {
          std::lock_guard<std::mutex> lock(data_mutex);
          gameTimes.push_back(end - start);
        }

        break;
      }

      const Quackle::Player player(game.currentPosition().currentPlayer());
      Quackle::Move compMove(game.haveComputerPlay());
      std::cout << "With rack " << player.rack() << ", player " << player.name()
                << " commits to move " << compMove << std::endl;
    }

    // Record game time if the game doesn't end
    if (!game.currentPosition().gameOver()) {
      auto end = std::chrono::high_resolution_clock::now();
      std::lock_guard<std::mutex> lock(data_mutex);
      gameTimes.push_back(end - start);
    }
  }
}

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
  const int numThreads =
      8; // Start with one thread to rule out concurrency issues

  std::map<int, int> winnerInfo;
  std::vector<std::chrono::duration<double>> gameTimes;

  std::vector<std::thread> threads;

  // Launch threads to simulate games
  for (int t = 0; t < numThreads; ++t) {
    int startGameIndex = t * (gameCnt / numThreads);
    threads.push_back(std::thread(simulateGames, startGameIndex,
                                  gameCnt / numThreads, std::ref(dataManager),
                                  std::ref(winnerInfo), std::ref(gameTimes)));
  }

  // Join all threads
  for (auto &th : threads) {
    th.join();
  }

  // Print statistics after all threads finish
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

  // Print winner statistics
  for (const auto &pair : winnerInfo) {
    std::cout << "Player " << pair.first << ": " << pair.second << " wins"
              << std::endl;
  }

  return 0;
}
