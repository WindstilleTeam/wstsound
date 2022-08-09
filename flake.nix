{
  description = "Windstille Sound Engine";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.05";
    flake-utils.url = "github:numtide/flake-utils";

    tinycmmc.url = "github:grumbel/tinycmmc";
    tinycmmc.inputs.nixpkgs.follows = "nixpkgs";
    tinycmmc.inputs.flake-utils.follows = "flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils, tinycmmc }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages = flake-utils.lib.flattenTree rec {
          default = wstsound;

          wstsound = pkgs.stdenv.mkDerivation {
            pname = "wstsound";
            version = "0.3.0";
            src = ./.;
            cmakeFlags = [
              "-DWARNINGS=ON"
              "-DWERROR=ON"
              "-DBUILD_TESTS=ON"
              "-DBUILD_EXTRA=ON"
            ];
            nativeBuildInputs = [
              pkgs.cmake
              tinycmmc.packages.${system}.default
            ];
            buildInputs = [
              pkgs.gtest
            ];
            propagatedBuildInputs = [
              pkgs.libmodplug
              pkgs.libogg
              pkgs.libvorbis
              pkgs.mpg123
              pkgs.openal
              pkgs.opusfile
            ];
           };
        };
      }
    );
}
