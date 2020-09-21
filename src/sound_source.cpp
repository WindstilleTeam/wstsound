/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
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

#include "sound_source.hpp"

namespace wstsound {

SoundSource::SoundSource() :
  m_fade_state(),
  m_fade_start_ticks(),
  m_fade_time(),
  m_total_time(0.0f)
{
}

void
SoundSource::set_fading(FadeState fade_state, float fade_time)
{
  m_fade_state       = fade_state;
  m_fade_time        = fade_time;
  m_fade_start_ticks = m_total_time;

  if (m_fade_state == FadeState::FadingOn) {
    set_gain(0.0f);
  }
}

void
SoundSource::update(float delta)
{
  m_total_time += delta;

  if (is_playing())
  {
    // handle fade-in/out
    if (m_fade_state == FadeState::FadingOn)
    {
      float time = m_fade_start_ticks - m_total_time;
      if (time >= m_fade_time)
      {
        set_gain(1.0);
        m_fade_state = FadeState::NoFading;
      }
      else
      {
        set_gain(time / m_fade_time);
      }
    }
    else if (m_fade_state == FadeState::FadingOff)
    {
      float time = m_fade_start_ticks - m_total_time;
      if (time >= m_fade_time)
      {
        stop();
        m_fade_state = FadeState::NoFading;
      }
      else
      {
        set_gain( (m_fade_time - time) / m_fade_time);
      }
    }
  }
}

} // namespace wstsound

/* EOF */
