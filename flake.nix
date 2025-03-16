{
  description = "A chessboard simulator written in C++23 using winapi";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};

    package = pkgs.pkgsCross.mingwW64.stdenv.mkDerivation {
      name = "CHWiGrx";

      src = self;

      nativeBuildInputs = with pkgs; [cmake gnumake iconv];
      buildInputs = [pkgs.pkgsCross.mingwW64.windows.mingw_w64];

      installPhase = ''
        mkdir -p $out/bin
        cp ./CHWiGrx.exe $out/bin/
      '';
    };
  in {
    packages.${system}.default = package;

    devShells.${system}.default = pkgs.mkShell {
      stdenv = package.stdenv;
      inputsFrom = [package];
    };
  };
}
