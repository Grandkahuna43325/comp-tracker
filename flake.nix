{
  description = "A Nix-flake-based C/C++ development environment";

  inputs =
    {
      flake-utils.url = "github:numtide/flake-utils";
      nixpkgs.url = "nixpkgs";
    };


  outputs = { self, nixpkgs, flake-utils  }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        libraries = with pkgs;
          [

            # libraries

          ];
        pkgs = import nixpkgs {
          system = "${system}";
        };

        pname = "pompy-mgr";

      in
      {
        inherit pname;

        devShell = pkgs.mkShell {
          shellHook = ''
            export CC=gcc
            export CXX=gcc
            export C_INCLUDE_PATH=$C_INCLUDE_PATH:${pkgs.glibc.dev}/include
            export CPLUS_INCLUDE_PATH=${pkgs.glibc.dev}/include
          '';

          nativeBuildInputs = with pkgs; [
            clang-tools
            gcc
            gdb
            cmake
            conan
            cppcheck
            doxygen
            lcov
            bear
            pkg-config
            man-pages
            man-pages-posix
          ];
        };
      }
    );

  # let
  #   pkgs = import nixpkgs { system = "x86_64-linux"; };
  # in
  # {
  #   devShell = pkgs.mkShell {
  #     nativeBuildInputs = with pkgs; [
  #       clang-tools
  #       gcc
  #       gdb
  #       cmake
  #       conan
  #       cppcheck
  #       doxygen
  #       lcov
  #       bear
  #       pkg-config
  #     ];
  #
  #     shellHook = ''
  #       # export CC=gcc
  #       # export CXX=gcc
  #       # export C_INCLUDE_PATH=$C_INCLUDE_PATH:$(pwd)/libraries/nuklear/src:${pkgs.glibc.dev}/include:${pkgs.glew.dev}/include
  #       # export CPLUS_INCLUDE_PATH=${pkgs.glibc.dev}/include
  #     '';
  #   };
  # };
}
