/*
**  Windstille - A Sci-Fi Action-Adventure Game
**  Copyright (C) 2005 Matthias Braun <matze@braunis.de>
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

#include <string.h>
#include <stdint.h>
#include <stdexcept>

#include "wav_sound_file.hpp"

static inline uint32_t read32LE(PHYSFS_file* file)
{
  uint32_t result;
  if(PHYSFS_readULE32(file, &result) == 0)
    throw std::runtime_error("file too short");

  return result;
}

static inline uint16_t read16LE(PHYSFS_file* file)
{
  uint16_t result;
  if(PHYSFS_readULE16(file, &result) == 0)
    throw std::runtime_error("file too short");

  return result;
}

WavSoundFile::WavSoundFile(PHYSFS_file* file)
{
  this->file = file;

  char magic[4];
  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read file magic (not a wave file)");
  if(strncmp(magic, "RIFF", 4) != 0) {
    printf("MAGIC: %4s.\n", magic);
    throw std::runtime_error("file is not a RIFF wav file");
  }

  uint32_t wavelen = read32LE(file);
  (void) wavelen;
  
  if(PHYSFS_read(file, magic, sizeof(magic), 1) != 1)
    throw std::runtime_error("Couldn't read chunk header (not a wav file?)");
  if(strncmp(magic, "WAVE", 4) != 0)
    throw std::runtime_error("file is not a valid RIFF/WAVE file");

  char chunkmagic[4];
  uint32_t chunklen;

  // search audio data format chunk
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching format chunk");    
    chunklen = read32LE(file);
    
    if(strncmp(chunkmagic, "fmt ", 4) == 0)
      break;

    if(strncmp(chunkmagic, "fact", 4) == 0
        || strncmp(chunkmagic, "LIST", 4) == 0) {
      // skip chunk
      if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
        throw std::runtime_error("EOF while searching fmt chunk");
    } else {
      throw std::runtime_error("complex WAVE files not supported");
    }
  } while(true); 

  if(chunklen < 16)
    throw std::runtime_error("Format chunk too short");
 
  // parse format
  uint16_t encoding = read16LE(file);
  if(encoding != 1)
    throw std::runtime_error("only PCM encoding supported");
  channels = read16LE(file);
  rate = read32LE(file);
  uint32_t byterate = read32LE(file);
  (void) byterate;
  uint16_t blockalign = read16LE(file);
  (void) blockalign;
  bits_per_sample = read16LE(file);

  if(chunklen > 16) {
    if(PHYSFS_seek(file, PHYSFS_tell(file) + (chunklen-16)) == 0)
      throw std::runtime_error("EOF while reading reast of format chunk");
  }

  // set file offset to DATA chunk data
  do {
    if(PHYSFS_read(file, chunkmagic, sizeof(chunkmagic), 1) != 1)
      throw std::runtime_error("EOF while searching data chunk");    
    chunklen = read32LE(file);

    if(strncmp(chunkmagic, "data", 4) == 0)
      break;

    // skip chunk
    if(PHYSFS_seek(file, PHYSFS_tell(file) + chunklen) == 0)
      throw std::runtime_error("EOF while searching fmt chunk");
  } while(true);

  datastart = PHYSFS_tell(file);
  size = static_cast<size_t> (chunklen);
}

WavSoundFile::~WavSoundFile()
{
  PHYSFS_close(file);
}

void
WavSoundFile::reset()
{
  if(PHYSFS_seek(file, datastart) == 0)
    throw std::runtime_error("Couldn't seek to data start");
}

size_t
WavSoundFile::read(void* buffer, size_t buffer_size)
{
  PHYSFS_sint64 end = datastart + size;
  PHYSFS_sint64 cur = PHYSFS_tell(file);
  if(cur >= end)
    return 0;
  
  size_t readsize = std::min(static_cast<size_t> (end - cur), buffer_size);
  if(PHYSFS_read(file, buffer, readsize, 1) != 1)
    throw std::runtime_error("read error while reading samples");

  return readsize;
}

/* EOF */
