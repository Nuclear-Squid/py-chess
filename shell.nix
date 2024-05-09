{ pkgs ? import <nixpkgs> {} }:
let
    unstable = import <nixos-unstable> {};
    python-derivation = pkgs.python3.withPackages (py-pkgs: with py-pkgs; [
        tkinter # GUI library
    ]);
in
pkgs.mkShell {
    packages = with pkgs; [
        unstable.zig # Using zig as a build system
        python-derivation
        valgrind
    ];

    shellHook = ''
        export CPYTHON_HEADER_PATH="${python-derivation}/include/python3.11"
    '';
}
