import os
import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d",
                    default="~",
                    dest="dir",
                    help="project root")
args = parser.parse_args()

# for subdir in os.walk(os.getcwd()):

def walk(dir):
    for file in os.scandir(dir):
        if (file.is_dir()):
            walk(file)
        elif (file.is_file() and (file.name.endswith(".hpp") or file.name.endswith(".cpp"))):
            with open(file.path, encoding="utf-8") as value:
                for num, line in enumerate(value):
                    if re.search(r'#include ["<].*[/].*[">]', line):
                        if (line.find("<gtest/gtest.h") != -1):
                            continue
                        print("> Error: found relative include")
                        print(f"> {file.path}:{num}: {line}")
                        exit(1)

os.chdir(args.dir)

print(f"> Running from {os.getcwd()}")
walk(os.getcwd())

print("> No relative inclusions were found")
print("> SUCCESS")


