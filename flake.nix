{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "git+https://github.com/grumbel/tinycmmc.git";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";

    libmodplug-win32.url = "git+https://github.com/grumnix/libmodplug-win32.git";
    libmodplug-win32.inputs.nixpkgs.follows = "nixpkgs";
    libmodplug-win32.inputs.tinycmmc.follows = "tinycmmc";

    libogg-win32.url = "git+https://github.com/grumnix/libogg-win32.git";
    libogg-win32.inputs.nixpkgs.follows = "nixpkgs";
    libogg-win32.inputs.tinycmmc.follows = "tinycmmc";

    libvorbis-win32.url = "git+https://github.com/grumnix/libvorbis-win32.git";
    libvorbis-win32.inputs.nixpkgs.follows = "nixpkgs";
    libvorbis-win32.inputs.tinycmmc.follows = "tinycmmc";
    libvorbis-win32.inputs.libogg.follows = "libogg-win32";

    mpg123-win32.url = "git+https://github.com/grumnix/mpg123-win32.git";
    mpg123-win32.inputs.nixpkgs.follows = "nixpkgs";
    mpg123-win32.inputs.tinycmmc.follows = "tinycmmc";

    openal-soft-win32.url = "git+https://github.com/grumnix/openal-soft-win32.git";
    openal-soft-win32.inputs.nixpkgs.follows = "nixpkgs";
    openal-soft-win32.inputs.tinycmmc.follows = "tinycmmc";

    opus-win32.url = "git+https://github.com/grumnix/opus-win32.git";
    opus-win32.inputs.nixpkgs.follows = "nixpkgs";
    opus-win32.inputs.tinycmmc.follows = "tinycmmc";

    opusfile-win32.url = "git+https://github.com/grumnix/opusfile-win32.git";
    opusfile-win32.inputs.nixpkgs.follows = "nixpkgs";
    opusfile-win32.inputs.tinycmmc.follows = "tinycmmc";
    opusfile-win32.inputs.libogg.follows = "libogg-win32";
    opusfile-win32.inputs.opus.follows = "opus-win32";
  };

  outputs = { self, nixpkgs, flake-utils,
              tinycmmc, libmodplug-win32, libogg-win32, libvorbis-win32,
              mpg123-win32, openal-soft-win32, opusfile-win32, opus-win32 }:
    tinycmmc.lib.eachSystemWithPkgs (pkgs:
      let
        # Shared dependency selection (Windows vs native).
        deps = {
          stdenv = pkgs.stdenv;
          tinycmmc = tinycmmc.packages.${pkgs.stdenv.hostPlatform.system}.default;

          libmodplug = if pkgs.stdenv.hostPlatform.isWindows
                       then libmodplug-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                       else pkgs.libmodplug;

          libogg = if pkgs.stdenv.hostPlatform.isWindows
                   then libogg-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                   else pkgs.libogg;

          libvorbis = if pkgs.stdenv.hostPlatform.isWindows
                      then libvorbis-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                      else pkgs.libvorbis;

          mpg123 = if pkgs.stdenv.hostPlatform.isWindows
                   then mpg123-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                   else pkgs.mpg123;

          openal = if pkgs.stdenv.hostPlatform.isWindows
                   then openal-soft-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                   else pkgs.openal;

          opusfile = if pkgs.stdenv.hostPlatform.isWindows
                     then opusfile-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                     else pkgs.opusfile;

          libopus = if pkgs.stdenv.hostPlatform.isWindows
                    then opus-win32.packages.${pkgs.stdenv.hostPlatform.system}.default
                    else pkgs.libopus;

          mcfgthreads = if pkgs.stdenv.hostPlatform.isWindows
                        then pkgs.windows.mcfgthreads
                        else null;
        };

        mkWstsound = features:
          pkgs.callPackage ./wstsound.nix (deps // features);

        # Attach .withXxx modifiers that return a new package with the
        # corresponding feature enabled.  Chaining is supported:
        #   wstsound-lean.withOpus.withVorbis
        withFeatures = baseFeatures: pkg:
          pkg // {
            withModplug = let f = baseFeatures // { withModplug = true; }; in
              withFeatures f (mkWstsound f);
            withVorbis = let f = baseFeatures // { withVorbis = true; }; in
              withFeatures f (mkWstsound f);
            withOpus = let f = baseFeatures // { withOpus = true; }; in
              withFeatures f (mkWstsound f);
            withMpg123 = let f = baseFeatures // { withMpg123 = true; }; in
              withFeatures f (mkWstsound f);
            withEfx = let f = baseFeatures // { withEfx = true; }; in
              withFeatures f (mkWstsound f);
          };

        leanFeatures = {
          withModplug = true;
          withVorbis  = false;
          withOpus    = false;
          withMpg123  = false;
          withEfx     = false;
          # Extra tools (wstsound-play, …) assume EFX; keep them off for lean.
          buildExtra  = false;
        };

        fullFeatures = {
          withModplug = true;
          withVorbis  = true;
          withOpus    = true;
          withMpg123  = true;
          withEfx     = true;
        };
      in
      rec {
        packages = rec {
          default = wstsound;

          # Full-featured build (previous default behaviour).
          wstsound = mkWstsound fullFeatures;

          # Minimal build (modplug only) with fluent .withXxx modifiers.
          # Example: packages.wstsound-lean.withOpus.withVorbis
          wstsound-lean = withFeatures leanFeatures (mkWstsound leanFeatures);
        };

        apps = rec {
          default = wstsound-play;

          # Extra tools from the full-featured package (BUILD_EXTRA=ON).
          wstsound-play = {
            type = "app";
            program = "${packages.wstsound}/bin/wstsound-play";
          };
          wstsound-openal = {
            type = "app";
            program = "${packages.wstsound}/bin/wstsound-openal";
          };
          wstsoundfile-cat = {
            type = "app";
            program = "${packages.wstsound}/bin/wstsoundfile-cat";
          };
          wstsoundfile-info = {
            type = "app";
            program = "${packages.wstsound}/bin/wstsoundfile-info";
          };
        };
      }
    );
}
