{ pkgs ? import <nixpkgs> {} }:
let unstable = import <nixos-unstable> {}; in
pkgs.mkShell {
    nativeBuildInputs = with pkgs; [
        unstable.zig
        python3
        # python3.withPackages (python-pkgs: [
        #     # select PYthon packages here
        # ])
    ];
}
