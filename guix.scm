;; Windstille - A Sci-Fi Action-Adventure Game
;; Copyright (C) 2020 Ingo Ruhnke <grumbel@gmail.com>
;;
;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

(set! %load-path
  (cons* "/ipfs/QmX6nVomBirTsHRVDiuQQgPsRWoS7HxcgFeAgwJWdGQ332/guix-cocfree_0.0.0-62-g3b27118"
         %load-path))

(use-modules (guix build-system cmake)
             ((guix licenses) #:prefix license:)
             (guix packages)
             (gnu packages audio)
             (gnu packages check)
             (gnu packages gcc)
             (gnu packages mp3)
             (gnu packages pkg-config)
             (gnu packages xiph)
             (guix-cocfree utils))

(define %source-dir (dirname (current-filename)))

(define-public wstsound
  (package
   (name "wstsound")
   (version (version-from-source %source-dir))
   (source (source-from-source %source-dir))
   (build-system cmake-build-system)
   (arguments
    `(#:configure-flags '("-DCMAKE_BUILD_TYPE=Release"
                          "-DBUILD_TESTS=ON"
                          "-DBUILD_EXTRA=ON")))
   (native-inputs
    `(("pkg-config" ,pkg-config)
      ("gcc" ,gcc-10)))
   (inputs
    `(("openal" ,openal)
      ("libvorbis" ,libvorbis)
      ("libogg" ,libogg)
      ("opusfile" ,opusfile)
      ("mpg123" ,mpg123)
      ("libmodplug" ,libmodplug)
      ("googletest" ,googletest)))
   (synopsis (synopsis-from-source %source-dir))
   (description (description-from-source %source-dir))
   (home-page (homepage-from-source %source-dir))
   (license license:gpl3+)))

wstsound

;; EOF ;;
