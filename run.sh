if ! command -v brew &>/dev/null; then
    echo "Homebrew not found. Installing..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
    eval "$(/opt/homebrew/bin/brew shellenv)"
else
    echo "Homebrew is already installed."
fi

if brew list curl &>/dev/null; then
    echo "curl is already installed."
else
    echo "Installing curl..."
    brew install curl
fi

if brew list json-c &>/dev/null; then
    echo "json-c is already installed."
else
    echo "Installing json-c..."
    brew install json-c
fi

if brew list jsoncpp &>/dev/null; then
    echo "jsoncpp is already installed."
else
    echo "Installing jsoncpp..."
    brew install jsoncpp
fi

echo ""
echo "Yah my gee let’s go get it!"
"./program"