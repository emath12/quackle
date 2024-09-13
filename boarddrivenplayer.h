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

#ifndef QUACKLE_BOARDDRIVENPLAYER_H
#define QUACKLE_BOARDDRIVENPLAYER_H

#include "computerplayer.h"
#include "endgame.h"

namespace Quackle {

class BoardDrivenPlayer : public ComputerPlayer {
public:
  BoardDrivenPlayer();
  virtual ~BoardDrivenPlayer();

  virtual Move move();
  virtual MoveList moves(int nmoves);
  virtual ComputerPlayer *clone() { return new BoardDrivenPlayer; }

  virtual bool isUserVisible() const;

  virtual void setDispatch(ComputerDispatch *dispatch);

private:
  Endgame m_endgame;
};

inline bool BoardDrivenPlayer::isUserVisible() const { return true; }

} // namespace Quackle

#endif
