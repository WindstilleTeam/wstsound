/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sound/sound_channel.hpp"

#include <iostream>
#include <filesystem>

#include "sound/dummy_sound_source.hpp"
#include "sound/sound_file.hpp"
#include "sound/sound_manager.hpp"
#include "sound/sound_source.hpp"
#include "sound/stream_sound_source.hpp"

SoundChannel::SoundChannel(SoundManager& sound_manager) :
  m_sound_manager(sound_manager),
  m_sound_sources(),
  m_gain(1.0f)
{
}

void
SoundChannel::clear()
{
  m_sound_sources.clear();
}

SoundSourcePtr
SoundChannel::play(std::filesystem::path const& filename,
                   SoundSourceType type)
{
  SoundSourcePtr source = prepare(filename);
  source->play();
  return source;
}

SoundSourcePtr
SoundChannel::prepare(std::filesystem::path const& filename,
                      SoundSourceType type)
{
  SoundSourcePtr source = m_sound_manager.create_sound_source(filename, *this, type);
  if (!source)
  {
    std::cout << "SourceChannel::prepare: Couldn't load " << filename << std::endl;
    return SoundSourcePtr(new DummySoundSource());
  }
  else
  {
    source->update_gain();
    m_sound_sources.push_back(source);
    return source;
  }
}

void
SoundChannel::set_gain(float gain)
{
  m_gain = gain;

  for(std::vector<SoundSourcePtr>::iterator i = m_sound_sources.begin(); i != m_sound_sources.end(); ++i)
  {
    (*i)->update_gain();
  }
}

float
SoundChannel::get_gain() const
{
  return m_gain;
}

void
SoundChannel::update(float delta)
{
  for(SoundSourcePtr& sound_source : m_sound_sources)
  {
    sound_source->update(delta);
  }

  // check for finished sound sources
  std::erase_if(m_sound_sources, [](SoundSourcePtr const& source){
    return !source->is_playing();
  });
}

/* EOF */
