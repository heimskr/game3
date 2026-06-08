{
  inputs = {
    self.submodules = true;
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, self, ... }: let
    mkPkgs = system: import nixpkgs {
      inherit system;
      overlays = [ self.overlays.game3 ];
    };
  in {
    overlays = rec {
      default = game3;
      game3 = final: prev: {
        fastnoise2 = final.callPackage ./nix/fastnoise.nix { };
        game3 = final.callPackage ./nix/game3.nix { };
      };
    };

    packages.x86_64-linux = rec {
      default = game3;
      inherit (mkPkgs "x86_64-linux") fastnoise2 game3;
    };
  };
}
