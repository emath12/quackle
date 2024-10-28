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
#include <fstream>
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
                   std::map<int, int> &winnerCounts,
                   std::map<int, int> &gameIndexToWinner,
                   std::vector<std::chrono::duration<double>> &gameTimes,
                   std::map<int, TurnDurationInfo> &turnDurationInfo) {
  for (int gameIndex = startGameIndex; gameIndex < startGameIndex + numGames;
       ++gameIndex) {
    auto start = std::chrono::high_resolution_clock::now();

    Quackle::Game game;
    Quackle::PlayerList players;

    Quackle::Player speedyA(MARK_UV("Resolvent A"),
                            Quackle::Player::ComputerPlayerType, 110);
    speedyA.setComputerPlayer(new Quackle::Resolvent());
    players.push_back(speedyA);

    Quackle::Player boardDrivenPlayer(MARK_UV("Resolvent B"),
                                      Quackle::Player::ComputerPlayerType, 110);
    boardDrivenPlayer.setComputerPlayer(new Quackle::BoardDrivenPlayer(game));
    players.push_back(boardDrivenPlayer);

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
            winnerCounts[it->id()] += 1;
            gameIndexToWinner[gameIndex] = it->id();
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

void reportResults(const std::map<int, int> &winnerCounts,
                   const std::map<int, int> &gameIndexToWinner,
                   const std::vector<std::chrono::duration<double>> &gameTimes,
                   const std::map<int, TurnDurationInfo> &turnDurationInfo,
                   int totalGames) {

  if (gameTimes.empty()) {
    std::cout << "No games played." << std::endl;
    return;
  }

  std::chrono::duration<double> totalDuration =
      std::chrono::duration<double>::zero();

  std::ofstream gameStats("game_stats.csv");
  std::ofstream simulationSetResults("simulation_set_results.csv");

  gameStats << "Game_id,game_len,winner,avg_turn_len";
  gameStats << "\n";

  for (const auto &pair : gameIndexToWinner) {
    gameStats << pair.first << "," << gameTimes[pair.first].count() << ","
              << gameIndexToWinner.at(pair.first) << ",";
    double averageTurnLength;
    int playerId = pair.first;

    if (turnDurationInfo.find(playerId) != turnDurationInfo.end()) {
      const TurnDurationInfo &info = turnDurationInfo.at(playerId);
      if (info.turnCount > 0) {
        averageTurnLength = info.totalTurnLength / info.turnCount;
      } else {
        std::cout << "Player " << playerId << " had no turns." << std::endl;
      }
    }

    gameStats << averageTurnLength << "\n";
  }

  gameStats.close();

  for (const auto &duration : gameTimes) {
    totalDuration += duration;
  }

  double averageDuration = totalDuration.count() / gameTimes.size();

  double pValue =
      calculatePValue(winnerCounts.at(0), winnerCounts.at(1), totalGames);

  for (const auto &pair : winnerCounts) {
    std::cout << "Game ID: " << pair.first << ", Wins: " << pair.second
              << std::endl;
  }

  simulationSetResults << "num_of_games,avg_game_len,p_1_type,p_2_type,p_1_"
                          "wins,p_2_wins,p_value";
  simulationSetResults << "\n";
  simulationSetResults << gameTimes.size() << "," << averageDuration << ","
                       << "Modified" << ","
                       << "Unmodified" << "," << winnerCounts.at(0) << ","
                       << winnerCounts.at(1) << "," << pValue << "\n";

  simulationSetResults.close();
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

  const int numThreads = std::min(16, gameCnt);

  std::map<int, int> winnerCounts;
  std::map<int, int> gameIndexToWinner;
  std::vector<std::chrono::duration<double>> gameTimes;
  std::map<int, TurnDurationInfo> turnDurationInfo;

  std::vector<std::thread> threads;

  for (int t = 0; t < numThreads; ++t) {
    int startGameIndex = t * (gameCnt / numThreads);
    threads.push_back(
        std::thread(simulateGames, startGameIndex, gameCnt / numThreads,
                    std::ref(dataManager), std::ref(winnerCounts),
                    std::ref(gameIndexToWinner), std::ref(gameTimes),
                    std::ref(turnDurationInfo)));
  }

  for (auto &th : threads) {
    th.join();
  }

  reportResults(winnerCounts, gameIndexToWinner, gameTimes, turnDurationInfo,
                gameCnt);
}
