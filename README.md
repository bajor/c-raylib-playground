## Set things up

```
brew install raylib
brew install pkg-config 
brew install pkgconf

echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
source ~/.zprofile
```

## Setup for vscode so language server don't complain
```
brew install cmake
mkdir build
cd build
cmake ..
```

## Compile and run
```
make
```
