#!/bin/bash

set -e

echo ""
echo "🧰 Checking platform and installing dependencies..."

install_brew_pkg() {
    if brew list "$1" &>/dev/null; then
        echo "✅ $1 is already installed."
    else
        echo "📦 Installing $1..."
        brew install "$1" || { echo "❌ Failed to install $1"; exit 1; }
    fi
}

install_apt_pkg() {
    if dpkg -s "$1" &>/dev/null; then
        echo "✅ $1 is already installed."
    else
        echo "📦 Installing $1..."
        sudo apt install -y "$1" || { echo "❌ Failed to install $1"; exit 1; }
    fi
}

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "🖥️  macOS detected."

    if ! command -v brew &>/dev/null; then
        echo "🍺 Homebrew not found. Installing..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        echo "✅ Homebrew is already installed."
    fi

    brew update
    install_brew_pkg pkg-config
    install_brew_pkg curl
    install_brew_pkg jsoncpp
    install_brew_pkg cmake
    install_brew_pkg gcc

elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "🐧 Linux detected."

    sudo apt update
    install_apt_pkg build-essential
    install_apt_pkg g++
    install_apt_pkg cmake
    install_apt_pkg libcurl4-openssl-dev
    install_apt_pkg libjsoncpp-dev
    install_apt_pkg pkg-config

else
    echo "❌ Unsupported OS: $OSTYPE"
    exit 1
fi

echo ""
echo "✅ All dependencies installed successfully."
cd ~/Downloads
mkdir -p Sender && cd Sender
git clone https://github.com/honored/ECHOBLAST.git || { echo "❌ Failed to clone repository"; exit 1; }
cd ECHOBLAST
make clean || { echo "❌ Failed to clean previous builds"; exit 1; }
cp program ./Sender
rm "$0"
echo ""
echo "    make"
echo ""