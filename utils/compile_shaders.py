import os
import sys
from glob import glob

def Path(filename):
    return os.path.basename(filename).split(".")[0]

def convert_file(filename):
    hexes = [ "{:02x}".format(ord(c)) for c in open(filename, 'r').read() ]
    
    bin_str = "{"
    for number in hexes:
        bin_str += "0x{}, ".format(number)
    bin_str += "0x00 }"
    return bin_str, len(hexes) + 1

input_directory = sys.argv[1]
output_directory = sys.argv[2]
exts = ["vs", "fs"]
shaders = []

for ext in exts:
    file_list = glob(os.path.join(input_directory, "*." + ext))
    shaders.append([ Path(shader) for shader in file_list ])

shader_pairs = []
for shader in shaders[0]:
    if shader in shaders[1]:
        shader_pairs.append(shader)

file = open(os.path.join(output_directory, "shaders.h"), "w")
file.write("#ifndef RENDER_SHADERS_H\n")
file.write("#define RENDER_SHADERS_H\n\n")

for pair in shader_pairs:
    for ext in exts:
        filename = pair + "." + ext
        binary, size = convert_file(os.path.join(input_directory, filename))
        var_name = "{}_{}".format(pair, ext)
        file.write("static const char {}[] = {};\n".format(var_name, binary))
        file.write("static const int {}_size = {};\n\n".format(var_name, size))

file.write("#endif")