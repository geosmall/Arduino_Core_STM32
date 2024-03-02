#!/bin/sh -
set -o nounset # Treat unset variables as an error

UNAME_OS="$(uname -s)"

if [ $# -lt 1 ]; then
  echo "Not enough arguments!"
  usage 2
fi

# Parse options
FILEPATH=$1

# Echo file dir path to screen - gls
DIRPATH=$(dirname "$FILEPATH")
echo "Path to ELF file:"
echo "$DIRPATH"

# Copy the string to the clipboard
if [[ "$UNAME_OS" == "Linux*" ]]; then
    # For Linux
    echo -n "$DIRPATH" | xclip -selection clipboard
elif [[ "$UNAME_OS" == "Darwin*" ]]; then
    # For macOS
    echo -n "$DIRPATH" | pbcopy
else
    # For Windows (with Git Bash, Cygwin, or WSL)
    echo -n "$DIRPATH" | clip.exe
fi

exit $?