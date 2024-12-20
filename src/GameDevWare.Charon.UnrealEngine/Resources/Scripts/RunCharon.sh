#!/bin/bash
# Copyright GameDevWare, Denis Zykov 2024

# ###### DETERMINE CURRENT DIRECTORY ##########
SCRIPT_DIR=$(cd "`dirname "$0"`" && pwd)
EXECUTABLE_DIR=$SCRIPT_DIR
EXECUTABLE_NAME="Charon.exe"
EXIT_CODE=0

check_mono_version() {
    local mono_version=$(mono --version | head -n1 | grep -oE 'version [0-9]+' | awk '{print $2}')

    if [[ -z "$mono_version" ]]; then
        exit_failure_wrong_or_missing_mono
    fi

    if (( mono_version < 5 )); then
        exit_failure_wrong_or_missing_mono
    fi
}

locate_executable() {
    
    if [[ "$SKIP_CHARON_UPDATES" != "1" ]]; then
      # ###### RESTORING NUGET PACKAGE ##########
      pushd "$SCRIPT_DIR" > /dev/null
      dotnet restore --packages "." --force --ignore-failed-sources > /dev/null
      EXIT_CODE=$?
      popd > /dev/null
      
      if [[ "$EXIT_CODE" != "0" ]]; then
          exit_failure_dotnet_restore_failed
      fi
    fi
    
    # ###### LOCATING Charon.exe ##########
    FILE_NAME="Charon.exe"
    for D in "$SCRIPT_DIR"/gamedevware.charon/*/; do
        if [[ -e "$D/tools/$EXECUTABLE_NAME" ]]; then
            EXECUTABLE_DIR="$D/tools"
        fi
    done

    if [[ -e "$EXECUTABLE_DIR" ]]; then
        return
    fi
    exit_failure_no_executable
}

run_executable() {
    pushd "$EXECUTABLE_DIR" > /dev/null
    STANDALONE__APPLICATIONDATAPATH="$SCRIPT_DIR/data"
    STANDALONE__APPLICATIONTEMPPATH="$SCRIPT_DIR/temp"
    SERILOG__WRITETO__0__NAME="File"
    SERILOG__WRITETO__0__ARGS__PATH="$SCRIPT_DIR/logs/log_unrealengine_.txt"

    # Drop in configuration file before launching executable
    cp "$SCRIPT_DIR/appsettings.json" "./appsettings.json"
    
    # Run 'mono Charon.exe' with passed parameters
    mono "./$EXECUTABLE_NAME" "$@"
    EXIT_CODE=$?
    popd > /dev/null

    if [[ "$EXIT_CODE" != "0" ]]; then
        exit_failure
    else
        exit_success
    fi
}

exit_failure_wrong_or_missing_mono() {
    EXIT_CODE=-3
    echo "Wrong or missing installation of 'mono' framework. Ensure that the 'mono' v5+ is installed and available in the 'PATH'. Check https://www.mono-project.com/download/stable/ for the installer." >&2
    echo "Error: 'mono' not found in the following directories:" >&2
    echo "$PATH" | tr ':' '\n' >&2
    exit_failure
}

exit_failure_dotnet_restore_failed() {
    EXIT_CODE=-2
    echo "Failed to execute the 'dotnet restore' command to retrieve the latest package version from NuGet. Ensure that the 'dotnet' tool is installed and available in the 'PATH'. Check 'https://dotnet.microsoft.com/en-us/download' for the installer." >&2
    exit_failure
}

exit_failure_no_executable() {
    EXIT_CODE=-1
    echo "Unable to find the '$FILE_NAME' executable in './gamedevware.charon/*/tools' subfolders." >&2
    exit_failure
}

exit_failure() {
    exit $EXIT_CODE
}

exit_success() {
    exit 0
}

# Start the process
check_mono_version
locate_executable
run_executable "$@"