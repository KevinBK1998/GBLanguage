import sys

TEMP_FILE_NAME = "TEMP"
SRC_EXT = ".gsm"
DST_EXT = ".gb"
src_file = TEMP_FILE_NAME + SRC_EXT
dst_file = TEMP_FILE_NAME + DST_EXT
logo_file = "logo" + DST_EXT
lib_file = "library" + DST_EXT
asc_file = "ascii_tiles" + DST_EXT

if len(sys.argv) > 1:
    src_file = sys.argv[1]
    if SRC_EXT in src_file:
        dst_file = src_file.split(".")[0]
    else:
        dst_file = src_file
        src_file +=SRC_EXT
    dst_file += DST_EXT
print("Parsing File :", src_file, ", Output in :", dst_file)

labelDict = {}
def ReadAllLabels(isLibrary=False):
    line = 0x134
    if isLibrary:
        line = 0
    f = open(src_file, "r")
    for x in f:
        if x == "\n":
            continue
        if "//" in x:
            continue
        if ":" in x:
            labelDict[x.split(":")[0]]=line
            if x.split(":")[1] == '\n':
                continue
        if "CALL" in x or "JP" in x:
            line+=3
        elif "JR" in x:
            line+=2
        elif "PREFIX" in x:
            line+=2
        elif "DATA" in x:
            line+=1
        elif "0x" in x:
            pre=line
            line+=2
            x=x.split("x")[1].strip()
            if len(x)> 2:
                line+=1
        else:
            line+=1
    if not isLibrary:
        labelDict["LIBRARY"]=line
    print(labelDict)
    f.close()

src = open(src_file, "r")
bin = open(dst_file, "wb")
MAGIC_PREFIX = 0xCB
assemblyMap = { 
    "NOP":0x00,             "INC B":0x04,           "DEC B":0x05,           "LD B, 0":0x06,     "INC C":0x0C,       "DEC C":0x0D,       "LD C, 0":0x0E,
    "LD DE, 0":0x11,        "INC DE":0x13,          "DEC D":0x15,           "LD D, 0":0x16,     "RL A":0x17,        "JR 0":0x18,        "LD A, [DE]":0x1A,  "DEC E":0x1D,   "LD E, 0":0x1E,
    "JR NZ, 0":0x20,        "LD HL, 0":0x21,        "LDI [HL], A":0x22,     "INC HL":0x23,      "INC H":0x24,       "JR Z, 0":0x28,     "LD L, 0":0x2E,
    "JR NC, 0":0x30,        "LD SP, 0":0x31,        "LDD [HL], A":0x32,     "DEC A":0x3D,       "LD A, 0":0x3E,
    "LD B, B":0x40,         "LD B, A":0x47,         "LD C, B":0x48,         "LD C, C":0x49,     "LD C, D":0x4A,     "LD C, A":0x4F,
    "LD D, A":0x57,
    "LD H, A":0x67,
    "HALT":0x76,            "LD [HL], A":0x77,      "LD A, B":0x78,         "LD A, C":0x79,     "LD A, E":0x7B,     "LD A, H":0x7C,     "LD A, L":0x7D,
    "ADD A, B":0x80,        "ADD A, C":0x81,        "ADD A, [HL]":0x86,
    "SUB A, B":0x90,        "SUB A, C":0x91,        "SUB A, D":0x92,
    "XOR A, A":0xAF,
    "CP B":0xB8,            "CP [HL]":0xBE,
    "POP BC":0xC1,          "JP 0":0xC3,            "PUSH BC":0xC5,         "RET":0xC9,         "CALL 0":0xCD,      "ADC A, 0":0xCE,
    "LD [HN], A, 0":0xE0,   "LD [HC], A":0xE2,      "LD [NN], A, 0":0xEA,
    "LD A, [HN], 0":0xF0,   "POP AF":0xF1,   "LD A, [HC]":0xF2,      "PUSH AF":0xF5,      "CP 0":0xFE,
}

specialAssemblyMap = {
    "RL C":0x11,
    "BIT 7, H":0x7C,
}

def assemblerCodeGen(isLibrary=False):
    line = 0
    if not isLibrary:
        # padding for bios bytes
        while line < 0x101:
            bin.write(bytearray([0]))
            line+=1

        # jump to user program
        opcode = assemblyMap["JP 0"]
        n = labelDict["START"]
        l = n % 256
        h = n // 256
        # print(hex(l),hex(h))
        code = [opcode, l, h]
        line+=3
        bin.write(bytearray(code))

        # write logo data
        logo = open(logo_file, "rb")
        while line < 0x134:
            bin.write(logo.read(1))
            line+=1
        logo.close()

    for x in src:
        if ":" in x:
            x=x.split(":")[1]
        if x == "\n":
            continue
        if "//" in x:
            continue
        for key in labelDict:
            if key in x:
                x=x.replace(key, hex(labelDict[key]))
        if "DATA" in x:
            x=x.split("x")[1].strip()
            n = int(x.strip(), 16)
            # print(hex(n))
            code = [n]
            line+=1
        elif "0x" in x:
            instruction=x.split("x")[0]
            opcode = assemblyMap[instruction]
            x=x.split("x")[1].strip()
            n = int(x.strip(), 16)
            if "JR" in instruction:
                line+=2
                jump = n - line
                # print(n, line, jump)
                if jump < 0:
                    jump += 0x100
                # print(hex(jump))
                code = [opcode, jump]
            elif len(x)> 2 or "CALL" in instruction or "JP" in instruction:
                l = n % 256
                h = n // 256
                # print(n, hex(l),hex(h))
                code = [opcode, l, h]
                line+=3
            else: 
                # print(hex(n))
                code = [opcode, n]
                line+=2
        elif "PREFIX" in x:
            instruction=x[7:].split("\n")[0]
            opcode = specialAssemblyMap[instruction]
            # print(hex(MAGIC_PREFIX),hex(opcode))
            code = [MAGIC_PREFIX, opcode]
            line+=2
        else:
            instruction=x.split("\n")[0]
            opcode = assemblyMap[instruction]
            # print(hex(opcode))
            code = [opcode]
            line+=1
        # print(code, hex(line))
        bin.write(bytearray(code))

    if not isLibrary:
        # write library data
        with open(lib_file, 'rb') as lib:
            lib.seek(0,2)
            size = lib.tell()
            lib.seek(0,0)
            bin.write(lib.read(size))
            line+=size
        # write ascii data
        with open(asc_file, 'rb') as asc:
            asc.seek(0,2)
            size = asc.tell()
            asc.seek(0,0)
            bin.write(asc.read(size))
            line+=size
        MAX_FILE_SIZE = (line // 0x100 +1)*0x100
        print("Padding with", MAX_FILE_SIZE - line, "space(s) to reach SIZE =", MAX_FILE_SIZE)
        while line < MAX_FILE_SIZE:
            bin.write(bytearray([0]))
            line+=1

isLibrary = (src_file == "library.gsm")
ReadAllLabels(isLibrary)
assemblerCodeGen(isLibrary)
