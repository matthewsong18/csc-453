import glob
import subprocess

# Get all test and error files in the current directory
files = sorted(glob.glob("err[0-9]*") + glob.glob("test[0-9]*"))

for file in files:
    expected_exit_code = 1 if file.startswith("err") else 0  # errXX -> 1, testXX -> 0

    try:
        # Read file content
        with open(file, "r") as f:
            input_data = f.read()

        # Run the program
        process = subprocess.run(
            ["../../build/compile", "--chk_decl"],
            input=input_data,
            text=True,
            capture_output=True,
        )

        exit_code = process.returncode

        # Print only if the test fails
        if exit_code != expected_exit_code:
            print(f"FAILED: {file} (Expected {expected_exit_code}, got {exit_code})")

    except FileNotFoundError:
        print(f"ERROR: {file} not found")

    except subprocess.CalledProcessError as e:
        print(f"ERROR: {file} - Process failed: {e}")

    except Exception as e:
        print(f"CRITICAL ERROR: {file} - {e}")
