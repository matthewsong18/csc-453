import subprocess
import glob
import os

# Get all test files in the current directory
files = sorted(glob.glob("test[0-9]*"))

for file in files:
    expected_exit_code = 0  # All tests should exit with 0
    output_file = f"outputs/{file}-out"

    try:
        # Read test input
        with open(file, "r") as f:
            input_data = f.read()

        # Read expected output
        with open(output_file, "r") as f:
            expected_output = f.read().strip()

        # Run the program
        process = subprocess.run(
            ["../../compile", "--print_ast"],
            input=input_data,
            text=True,
            capture_output=True,
        )

        exit_code = process.returncode
        actual_output = process.stdout.strip()

        # Check exit code
        if exit_code != expected_exit_code:
            print(
                f"FAILED: {file} (Expected exit {expected_exit_code}, got {exit_code})"
            )
            continue  # Skip output comparison if exit code is wrong

        # Compare output
        if actual_output != expected_output:
            print(f"FAILED: {file} - Output mismatch\n")
            print(f"Expected:\n{expected_output}\n")
            print(f"Got:\n{actual_output}\n")

    except FileNotFoundError as e:
        print(f"ERROR: {e}")

    except subprocess.CalledProcessError as e:
        print(f"ERROR: {file} - Process failed: {e}")

    except Exception as e:
        print(f"CRITICAL ERROR: {file} - {e}")
