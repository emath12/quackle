#include <algorithm>
#include <boarddrivenplayer.h>
#include <boardparameters.h>
#include <bogowinplayer.h>
#include <chrono>
#include <cmath>
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
#include <numeric>
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

struct TurnDurationInfo {
  double totalTurnLength = 0.0;
  int turnCount = 0;
};

void simulateGames(int startGameIndex, int numGames,
                   Quackle::DataManager &dataManager,
                   std::map<int, int> &winnerInfo,
                   std::vector<std::chrono::duration<double>> &gameTimes,
                   std::map<int, TurnDurationInfo> &turnDurationInfo) {
  for (int gameIndex = startGameIndex; gameIndex < startGameIndex + numGames;
       ++gameIndex) {
    auto start = std::chrono::high_resolution_clock::now();

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
        Quackle::PlayerList winners(game.currentPosition().leadingPlayers());
        for (Quackle::PlayerList::const_iterator it = winners.begin();
             it != winners.end(); ++it) {
          {
            std::lock_guard<std::mutex> lock(data_mutex);
            winnerInfo[it->id()] += 1;
          }
        }

        auto end = std::chrono::high_resolution_clock::now();
        {
          std::lock_guard<std::mutex> lock(data_mutex);
          gameTimes.push_back(end - start);
        }
        break;
      }

      Quackle::Player player(game.currentPosition().currentPlayer());
      auto turnStart = std::chrono::high_resolution_clock::now();
      Quackle::Move compMove(game.haveComputerPlay());
      auto turnEnd = std::chrono::high_resolution_clock::now();

      double turnDuration =
          std::chrono::duration<double>(turnEnd - turnStart).count();

      {
        std::lock_guard<std::mutex> lock(data_mutex);
        turnDurationInfo[player.id()].totalTurnLength += turnDuration;
        turnDurationInfo[player.id()].turnCount += 1;
      }
    }

    if (!game.currentPosition().gameOver()) {
      auto end = std::chrono::high_resolution_clock::now();
      std::lock_guard<std::mutex> lock(data_mutex);
      gameTimes.push_back(end - start);
    }
  }
}

double calculatePValue(int winsPlayer1, int winsPlayer2, int totalGames) {
  double mean1 = static_cast<double>(winsPlayer1) / totalGames;
  double mean2 = static_cast<double>(winsPlayer2) / totalGames;

  double var1 = mean1 * (1 - mean1);
  double var2 = mean2 * (1 - mean2);

  double pooledVariance = sqrt(var1 / totalGames + var2 / totalGames);
  double tStatistic = (mean1 - mean2) / pooledVariance;

  return 2 * (1 - std::erf(std::abs(tStatistic) / sqrt(2.0)));
}

void reportResults(const std::map<int, int> &winnerInfo,
                   const std::vector<std::chrono::duration<double>> &gameTimes,
                   int totalGames) {
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

  // Report wins
  for (const auto &pair : winnerInfo) {
    std::cout << "Player " << pair.first << ": " << pair.second << " wins"
              << std::endl;
  }

  if (winnerInfo.size() >= 2) {
    auto it = winnerInfo.begin();
    int winsPlayer1 = it->second;
    int winsPlayer2 = (++it)->second;

    double pValue = calculatePValue(winsPlayer1, winsPlayer2, totalGames);
    std::cout << "P-value for victory margin between Player 0 and Player 1: "
              << pValue << std::endl;
  }
}

void reportAverageTurnLengths(
    const std::map<int, TurnDurationInfo> &turnDurationInfo) {
  for (const auto &pair : turnDurationInfo) {
    int playerId = pair.first;
    const TurnDurationInfo &info = pair.second;
    if (info.turnCount > 0) {
      double averageTurnLength = info.totalTurnLength / info.turnCount;
      std::cout << "Average turn length for Player " << playerId << ": "
                << averageTurnLength << " seconds" << std::endl;
    } else {
      std::cout << "Player " << playerId << " had no turns." << std::endl;
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

  int gameCnt = 100;
  if (argc > 1) {
    gameCnt = std::max(1, std::atoi(argv[1]));
  }

  const int numThreads = 16;

  std::map<int, int> winnerInfo;
  std::vector<std::chrono::duration<double>> gameTimes;
  std::map<int, TurnDurationInfo> turnDurationInfo;

  std::vector<std::thread> threads;

  for (int t = 0; t < numThreads; ++t) {
    int startGameIndex = t * (gameCnt / numThreads);
    threads.push_back(std::thread(simulateGames, startGameIndex,
                                  gameCnt / numThreads, std::ref(dataManager),
                                  std::ref(winnerInfo), std::ref(gameTimes),
                                  std::ref(turnDurationInfo)));
  }

  for (auto &th : threads) {
    th.join();
  }

  reportResults(winnerInfo, gameTimes, gameCnt);
  reportAverageTurnLengths(turnDurationInfo);
}
