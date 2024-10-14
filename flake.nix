{
  description = "A neat, portable, realtime 3D rendering library";

  inputs.nixpkgs.url = github:NixOS/nixpkgs;

  outputs = { self, nixpkgs, ... }: {
    packages.x86_64-linux.default = nixpkgs.legacyPackages.x86_64-linux.callPackage ./default.nix {
      version = self.lastModifiedDate;
    };
  };
}
