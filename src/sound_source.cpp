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
  m_fade()
{
}

void
SoundSource::set_fading(FadeDirection direction, float duration)
{
  m_fade = Fade{direction, duration, 0.0f};
}

void
SoundSource::update(float delta)
{
  if (is_playing())
  {
    if (m_fade)
    {
      m_fade->time_passed += delta;
      float progress = m_fade->duration == 0.0f ? 1.0f : m_fade->duration / m_fade->duration;

      // FIXME: keep better track of gain, as this will fail when gain isn't 1.0
      if (m_fade->direction == FadeDirection::In) {
        if (progress >= 1.0f) {
          set_gain(1.0f);
          m_fade = std::nullopt;
        } else {
          set_gain(progress);
        }
      } else if (m_fade->direction == FadeDirection::Out) {
        if (progress >= 1.0f) {
          set_gain(0.0f);
          m_fade = std::nullopt;
          stop();
        } else {
          set_gain(1.0f - progress);
        }
      }
    }
  }
}

} // namespace wstsound

/* EOF */
