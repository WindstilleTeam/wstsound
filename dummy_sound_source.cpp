//  $Id: sound_source.hpp 3462 2006-04-28 19:38:41Z sommer $
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
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
#include <config.h>

#include "dummy_sound_source.hpp"

class DummySoundSource : public SoundSource
{
public:
  DummySoundSource()
  {}
  virtual ~DummySoundSource()
  {}

  virtual void play()
  {
    is_playing = true;
  }
  
  virtual void stop()
  {
    is_playing = false;
  }
  
  virtual bool playing()
  {
    return is_playing;
  }

  virtual void set_looping(bool )
  {
  }

  virtual void set_gain(float )
  {
  }

  virtual void set_pitch(float )
  {
  }

  virtual void set_position(const Vector& )
  {
  }

  virtual void set_velocity(const Vector& )
  {
  }

  virtual void set_reference_distance(float )
  {
  }

  virtual void set_rollof_factor(float )
  {
  }

private:
  bool is_playing;
};

SoundSource* create_dummy_sound_source()
{
  return new DummySoundSource();
}

