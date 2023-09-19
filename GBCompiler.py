import sys

TEMP_FILE_NAME = "TEMP"
SRC_EXT = ".gbpl"
DST_EXT = ".gsm"
src_file = TEMP_FILE_NAME + SRC_EXT
dst_file = TEMP_FILE_NAME + DST_EXT

if len(sys.argv) > 1:
    src_file = sys.argv[1]
    if SRC_EXT in src_file:
        dst_file = src_file.split(".")[0]
    else:
        dst_file = src_file
    dst_file += DST_EXT
print("Parsing File :", src_file, ", Output in :", dst_file)

f = open(src_file, "r")
lexicalDict = {

}
