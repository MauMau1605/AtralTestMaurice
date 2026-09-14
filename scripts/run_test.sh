#!/bin/bash
# Test runner for SREC over IPC
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

CIPHER="${1:-all}"
INPUT_FILE="${2:-sample_message.bin}"
OUTPUT_FILE="received_image.bin"
BUILD_DIR="build"

if [ ! -x "$BUILD_DIR/sender" ] || [ ! -x "$BUILD_DIR/receiver" ]; then
    echo "Error: Binaries not found in $BUILD_DIR. Please build the project first." >&2
    exit 1
fi

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: Input file '$INPUT_FILE' not found." >&2
    exit 1
fi

run_single_test() {
    local c=$1
    local name=""
    case $c in
        0) name="Cipher 0 (CIPHER_NONE - No Cipher)" ;;
        1) name="Cipher 1 (CIPHER_XOR - XOR Key)" ;;
        2) name="Cipher 2 (CIPHER_MOD - Add Modulo Key)" ;;
        *) name="Custom Cipher ($c)" ;;
    esac

    echo "============================================================"
    echo ">>> RUNNING TEST: $name"
    echo "============================================================"

    # Clean up previous sockets or outputs
    rm -f /tmp/srec_ipc.sock "$OUTPUT_FILE"

    # Launch sender in background
    ./"$BUILD_DIR"/sender "$INPUT_FILE" "$c" &
    SENDER_PID=$!

    # Brief delay to allow sender socket creation
    sleep 0.1

    # Launch receiver
    ./"$BUILD_DIR"/receiver
    wait $SENDER_PID

    # Verify integrity
    if cmp -s "$INPUT_FILE" "$OUTPUT_FILE"; then
        echo -e "\n[PASS] $name : Binary verification successful (identical to source)\n"
    else
        echo -e "\n[FAIL] $name : Binary verification FAILED!\n" >&2
        return 1
    fi
}

if [ "$CIPHER" = "all" ]; then
    echo "Starting test suite for all cipher modes..."
    run_single_test 0
    run_single_test 1
    run_single_test 2
    echo "============================================================"
    echo " ALL 3 TESTS PASSED SUCCESSFULLY! "
    echo "============================================================"
else
    run_single_test "$CIPHER"
fi
