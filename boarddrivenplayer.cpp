// /*
//  *  Quackle -- Crossword game artificial intelligence and analysis tool
//  *  Copyright (C) 2005-2019 Jason Katz-Brown, John O'Laughlin, and John
//  Fultz.
//  *
//  *  This program is free software; you can redistribute it and/or modify
//  *  it under the terms of the GNU General Public License as published by
//  *  the Free Software Foundation; either version 3 of the License, or
//  *  (at your option) any later version.
//  *
//  *  This program is distributed in the hope that it will be useful,
//  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  *  GNU General Public License for more details.
//  *
//  *  You should have received a copy of the GNU General Public License
//  *  along with this program. If not, see <http://www.gnu.org/licenses/>.
//  */

// #include <iostream>

// #include "boarddrivenplayer.h"
// #include "datamanager.h"
// #include "strategyparameters.h"

// // #define DEBUG_COMPUTERPLAYER

// using namespace Quackle;

// BoardDrivenPlayer::BoardDrivenPlayer() {
//   m_name = MARK_UV("Board Driven Player");
//   m_id = 5000;
// }

// BoardDrivenPlayer::~BoardDrivenPlayer() {}

// Move BoardDrivenPlayer::move() { return moves(1).back(); }

// MoveList BoardDrivenPlayer::moves(int nmoves) {
//   const int zerothPrune = 33; // prune level is the top whatever moves
//   int plies = 1;

//   // 7 = rackSize()
//   if (currentPosition().bag().size() <= 7 * 2) {
//     plies = -1;
//   }

//   currentPosition().kibitz();
//   m_simulator.setIncludedMoves(m_simulator.currentPosition().moves());
//   // m_simulator.pruneTo(zerothPrune, nmoves);
//   // m_simulator.makeSureConsideredMovesAreIncluded();
//   // m_simulator.setIgnoreOppos(false);

//   // Get the moves directly from the simulator
//   MoveList staticMoves =
//       m_simulator.moves(/* prune */ true, /* sort by equity */ false);
//   m_simulator.moveConsideredMovesToBeginning(staticMoves);

//   // UVcout << "Bogo static moves: " << staticMoves << endl;
//   // UVcout << "Bogo considered moves: " << m_simulator.consideredMoves() <<
//   // endl;

//   MoveList firstMove;
//   MoveList simmedMoves;

//   MoveList::const_iterator it = staticMoves.begin();

//   firstMove.push_back(*it);

//   signalFractionDone(0);

//   m_simulator.setIncludedMoves(firstMove);
//   m_simulator.simulate(4, 4);

//   Move best =
//       *m_simulator.moves(/* prune */ false, /* sort by win */ true).begin();
//   simmedMoves.push_back(best);

//   MoveList::sort(simmedMoves, MoveList::Win);

//   MoveList ret;
//   MoveList::const_iterator simmedEnd = simmedMoves.end();
//   int i = 0;
//   for (MoveList::const_iterator simmedIt = simmedMoves.begin();
//        (simmedIt != simmedEnd); ++i, ++simmedIt) {
//     if (i < nmoves || m_simulator.isConsideredMove(*simmedIt))
//       ret.push_back(*simmedIt);
//   }

//   // UVcout << "bogo returning moves:\n" << ret << endl;
//   return ret;
// }

// void BoardDrivenPlayer::setDispatch(ComputerDispatch *dispatch) {
//   ComputerPlayer::setDispatch(dispatch);
//   m_endgame.setDispatch(dispatch);
// }

/*
 *  Quackle -- Crossword game artificial intelligence and analysis tool
 *  Copyright (C) 2005-2019 Jason Katz-Brown, John O'Laughlin, and John Fultz.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <time.h>

#include "boarddrivenplayer.h"
#include "bogowinplayer.h"
#include "endgameplayer.h"
#include "game.h"
#include "preendgame.h"
#include "resolvent.h"

using namespace Quackle;

BoardDrivenPlayer::BoardDrivenPlayer(Game game) {
  m_name = MARK_UV("Board Driven Player");
  m_id = 201;
  m_parameters.version = 0;
  m_parameters.game = game;
}

BoardDrivenPlayer::~BoardDrivenPlayer() {}

Move BoardDrivenPlayer::move() { return moves(1).back(); }

MoveList BoardDrivenPlayer::moves(int nmoves) {

  ComputerPlayer *delegatee;

  if (m_simulator.currentPosition().bag().empty()) {
    // Case 1: Straight endgame.
    delegatee = new EndgamePlayer;
  } else if (currentPosition().bag().size() <=
             Preendgame::maximumTilesInBagToEngage()) {
    // Case 2: Preendgame.
    delegatee = new Preendgame;
  } else {
    // Case 3: Beginning and middle of the game.
    delegatee = new SmartBogowin;
  }

  delegatee->setParameters(parameters());
  delegatee->setDispatch(currentPosition().nestedness() > 0 ? 0 : m_dispatch);
  delegatee->setPosition(m_simulator.currentPosition());
  delegatee->setConsideredMoves(m_simulator.consideredMoves());
  MoveList moves = delegatee->moves(nmoves);
  delete delegatee;
  return moves;
}

// bool BoardDrivenPlayer::isSlow() const { return true; }

// InferringPlayer::InferringPlayer() {
//   m_name = MARK_UV("Inferring Player");
//   m_id = 2012;
//   m_parameters.secondsPerTurn = 20;
//   m_parameters.inferring = true;
// }

// InferringPlayer::~InferringPlayer() {}

// TorontoPlayer::TorontoPlayer() {
//   m_name = MARK_UV("Championship Player");
//   m_id = 2006
//   m_parameters.secondsPerTurn = 66;
// }

// TorontoPlayer::~TorontoPlayer() {}

// FiveMinutePlayer::FiveMinutePlayer() {
//   m_name = MARK_UV("Five Minute Championship Player");
//   m_id = 5208;
//   m_parameters.secondsPerTurn = 60 * 5;
// }

// FiveMinutePlayer::~FiveMinutePlayer() {}

// TwentySecondPlayer::TwentySecondPlayer() {
//   m_name = MARK_UV("Twenty Second Championship Player");
//   m_id = 5209;
//   m_parameters.secondsPerTurn = 20;
// }

// TwentySecondPlayer::~TwentySecondPlayer() {}

void BoardDrivenPlayer::setDispatch(ComputerDispatch *dispatch) {
  ComputerPlayer::setDispatch(dispatch);
  m_endgame.setDispatch(dispatch);
}

// BoardDrivenPlayer::BoardDrivenPlayer() {
//   m_name = MARK_UV("Board Driven Player");
//   m_id = 5000;
//   m_parameters.secondsPerTurn = 60 * 5;
// }

// BoardDrivenPlayer::~BoardDrivenPlayer() {}
