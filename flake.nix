{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    nix.inputs.nixpkgs.follows = "nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "gitlab:grumbel/cmake-modules";
    tinycmmc.inputs.nix.follows = "nix";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";
  };

  outputs = { self, nix, nixpkgs, flake-utils, tinycmmc }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        packages = flake-utils.lib.flattenTree {
          wstsound = pkgs.stdenv.mkDerivation {
            pname = "wstsound";
            version = "0.3.0";
            src = nixpkgs.lib.cleanSource ./.;
            cmakeFlags = [ "-DBUILD_EXTRA=ON" ];
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.gcc
              tinycmmc.defaultPackage.${system}
            ];
            buildInputs = [
              pkgs.openal
              pkgs.libvorbis
              pkgs.libogg
              pkgs.opusfile
              pkgs.mpg123
              pkgs.libmodplug
              pkgs.gtest
            ];
           };
        };
        defaultPackage = packages.wstsound;
      });
}