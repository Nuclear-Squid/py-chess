let
  nixpkgs = fetchTarball "https://github.com/NixOS/nixpkgs/tarball/nixos-23.11";
  pkgs = import nixpkgs { config = {}; overlays = []; };
in
pkgs.mkShell {
    packages = with pkgs; [
        clang
        (pkgs.python3.withPackages (py-pkgs: with py-pkgs; [
            tkinter # GUI library
        ]))
        valgrind
        git
        gnumake
    ];
}
