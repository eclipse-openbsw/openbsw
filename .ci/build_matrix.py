import subprocess

platforms = ["posix", "s32k148"]
compilers = ["clang", "gcc"]
cpp_standards = ["14", "17", "20", "23"]

def main():
    for platform in platforms:
        for compiler in compilers:
            for cpp_standard in cpp_standards:
                print(f"Running build for platform={platform}, compiler={compiler}, cpp_standard={cpp_standard}")
                subprocess.run(
                    ["python3", ".ci/build.py", platform, compiler, str(cpp_standard)],
                    check=True
                )
    print("All combinations have been processed.")

if __name__ == "__main__":
    main()
