import subprocess

all_demangled_file = open ("demangle.txt", "a+")

cmd = "c++filt"
def run_demangle_write_into_file (str):
    returned_output = subprocess.check_output (cmd + " " + str, shell=True)
    all_demangled_file.write (returned_output.decode ("utf-8"))

    return str == returned_output.decode ("utf-8").strip () # is equal to the source string

same_names = open ("same_names.txt", "a+")
source_file_lines = open ("PASS_WORK.txt").readlines ()

source_file_lines [:] = [source_file_line.partition (' ') [2] for source_file_line in source_file_lines] # erase numbers [123]
source_file_lines = [x.strip() for x in source_file_lines] # delete space symbols in end and begin of lines

source_file_lines = list (filter (("c3_ipa plugin").__ne__, source_file_lines)) # remove all 'c3_ipa plugin' strs

countSameNames = 0
for line in source_file_lines:
    is_eq = run_demangle_write_into_file (line)
    if (is_eq):
        countSameNames += 1
        same_names.write (line + "\n")

print (countSameNames / len (source_file_lines))

same_names.close ()
all_demangled_file.close ()
