//  $Id: PingusSound.hh,v 1.8 2000/04/24 13:15:41 grumbel Exp $
// 
//  Pingus - A free Lemmings clone
//  Copyright (C) 2000 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef PINGUSSOUND_HH
#define PINGUSSOUND_HH

#include "audio.hh"

class PingusSound
{
private:
  static bool is_init;
  static int audio_open;
  static Mix_Music *music;
public:
  static void init(int audio_rate, Uint16 audio_format,
		   int audio_channels, int audio_buffers);
  static void play(std::string);
  static void clean_up();
  static void keep_alive();  
};

#endif

/* EOF */
