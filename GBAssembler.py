import sys
import os

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

def humanReadableSize(size):
    if size < 1024:
        return f"{size} B"
    if size < 1024*1024:
        return f"{size/1024} KB"

labelDict = {}
def ReadAllLabels(isLibrary=False):
    line = 0x150
    labelDict["ASCII_TABLE"]=line
    with open(asc_file, 'rb') as asc:
        asc.seek(0,2)
        asc_size = asc.tell()
        line+=asc_size
    labelDict["LIBRARY"]=line

    f = open(src_file, "r")
    if not isLibrary:
        with open(lib_file, 'rb') as lib:
            lib.seek(0,2)
            lib_size = lib.tell()
            line+=lib_size

        # skip title data
        temp_line = 0x134
        while temp_line < 0x150:
            f.readline()
            temp_line+=1

    for x in f:
        # print(hex(line), x)
        if x == "\n":
            continue
        if "//" in x:
            continue
        if ":" in x:
            labelDict[x.split(":")[0]]=line
            if x.split(":")[1] == '\n':
                continue
        if "CALL" in x or "JP" in x or "LD BC" in x:
            line+=3
        elif "JR" in x:
            line+=2
        elif "BIT" in x or "RES" in x or "SET" in x or "SWAP" in x:
            line+=2
        elif "PREFIX" in x:
            line+=2
        elif "DATA" in x:
            line+=1
        elif "0x" in x:
            line+=2
            x=x.split("x")[1].strip()
            if len(x)> 2:
                line+=1
        else:
            line+=1
    print(labelDict)
    f.close()

src = open(src_file, "r")
bin = open(dst_file, "wb")
MAGIC_PREFIX = 0xCB
assemblyMap = {
    "NOP":0x00,             "LD BC, 0":0x01,    "INC BC":0x03,      "INC B":0x04,       "DEC B":0x05,       "LD B, 0":0x06,         "LD A, [BC]":0x0A,  "INC C":0x0C,   "DEC C":0x0D,   "LD C, 0":0x0E,
    "LD DE, 0":0x11,        "INC DE":0x13,      "DEC D":0x15,       "LD D, 0":0x16,     "RL A":0x17,        "JR 0":0x18,            "LD A, [DE]":0x1A,  "DEC E":0x1D,   "LD E, 0":0x1E, "RR A":0x1F,
    "JR NZ, 0":0x20,        "LD HL, 0":0x21,    "LDI [HL], A":0x22, "INC HL":0x23,      "INC H":0x24,       "DAA":0x27,             "JR Z, 0":0x28,     "LD L, 0":0x2E, "CPL":0x2F,
    "JR NC, 0":0x30,        "LD SP, 0":0x31,    "LDD [HL], A":0x32, "JR C, 0":0x38,     "INC A":0x3C,       "DEC A":0x3D,           "LD A, 0":0x3E,
    "LD B, B":0x40,         "LD B, A":0x47,     "LD C, B":0x48,     "LD C, C":0x49,     "LD C, D":0x4A,     "LD C, A":0x4F,
    "LD D, B":0x50,         "LD D, A":0x57,     "LD E, A":0x5F,
    "LD H, A":0x67,         "LD L, A":0x6F,
    "HALT":0x76,            "LD [HL], A":0x77,  "LD A, B":0x78,     "LD A, C":0x79,     "LD A, D":0x7A,     "LD A, E":0x7B,         "LD A, H":0x7C,     "LD A, L":0x7D, "LD A, [HL]":0x7F,
    "ADD A, B":0x80,        "ADD A, C":0x81,    "ADD A, D":0x82,    "ADD A, E":0x83,    "ADD A, L":0x85,    "ADD A, [HL]":0x86,
    "SUB A, B":0x90,        "SUB A, C":0x91,    "SUB A, D":0x92,    "SUB A, A":0x97,
    "XOR A, A":0xAF,
    "OR B":0xB0,            "OR A":0xB7,        "CP B":0xB8,        "CP C":0xB9,        "CP D":0xBA,        "CP E":0xBB,            "CP L":0xBD,        "CP [HL]":0xBE,
    "POP BC":0xC1,          "JP 0":0xC3,        "PUSH BC":0xC5,     "ADD A, 0":0xC6,    "RET":0xC9,         "CALL 0":0xCD,          "ADC A, 0":0xCE,
    "POP DE":0xD1,          "PUSH DE":0xD5,
    "LD [HN], A, 0":0xE0,   "POP HL":0xE1,      "LD [HC], A":0xE2,  "PUSH HL":0xE5,     "AND 0":0xE6,       "LD [NN], A, 0":0xEA,
    "LD A, [HN], 0":0xF0,   "POP AF":0xF1,      "LD A, [HC]":0xF2,  "PUSH AF":0xF5,     "CP 0":0xFE,
}

specialAssemblyMap = {
    "RL C":0x11,
    "SWAP A":0x37,
    "BIT 0, A":0x47,"BIT 1, A":0x4F,
    "BIT 2, A":0x57,"BIT 3, A":0x5F,
    "BIT 4, A":0x67,"BIT 5, A":0x6F,
    "BIT 7, H":0x7C,"BIT 6, A":0x77,"BIT 7, A":0x7F,
    "RES 1, A":0x8F,
    "RES 3, A":0x9F,
    "SET 1, A":0xCF,
    "SET 3, A":0xDF,
}

def assemblerCodeGen(isLibrary=False):
    line = 0
    try:
        print("BRKP :", hex(labelDict['BRKP']))
    except:
        print("No Breakpoints")
    if isLibrary:
        line = labelDict["LIBRARY"]
    else:
        print("START :", hex(labelDict['START']))
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

        # write title data
        while line < 0x150:
            x=src.readline()
            # print(x)
            x=x.split("x")[1].strip()
            n = int(x.strip(), 16)
            # print(hex(n))
            code = [n]
            bin.write(bytearray(code))
            line+=1
        
        # write ascii data
        with open(asc_file, 'rb') as asc:
            asc.seek(0,2)
            size = asc.tell()
            asc.seek(0,0)
            bin.write(asc.read(size))
            line+=size

        # write library code
        with open(lib_file, 'rb') as lib:
            lib.seek(0,2)
            size = lib.tell()
            lib.seek(0,0)
            bin.write(lib.read(size))
            line+=size

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
                if jump < -0x80 or jump > 0x7F:
                    raise ValueError(f"Jump amount exceeds limit (-128 to 127) of JR = {jump}")
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
        elif "BIT" in x or "RES" in x or "SET" in x or "SWAP" in x:
            instruction=x.split("\n")[0]
            opcode = specialAssemblyMap[instruction]
            # print(hex(MAGIC_PREFIX),hex(opcode))
            code = [MAGIC_PREFIX, opcode]
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
        MAX_FILE_SIZE = (line // 0x100 +1)*0x100
        print("Padding with", MAX_FILE_SIZE - line, "space(s) to reach SIZE =", humanReadableSize(MAX_FILE_SIZE))
        while line < MAX_FILE_SIZE:
            bin.write(bytearray([0]))
            line+=1

isLibrary = (src_file == "library.gsm")
try:
    ReadAllLabels(isLibrary)
    assemblerCodeGen(isLibrary)
except (ValueError, FileNotFoundError):
    print("Deleting incomplete file", dst_file)
    os.remove(dst_file)