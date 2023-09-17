f = open("bios.gsm", "r")
line = 0
labelDict = {}
for x in f:
    if x == "\n":
        continue
    if ":" in x:
        labelDict[x.split(":")[0]]=line
    if "CALL" in x:
        line+=3
    elif "JR" in x:
        line+=2
    elif "PREFIX" in x:
        line+=2
    elif "0x" in x:
        pre=line
        line+=2
        x=x.split("x")[1].strip()
        if len(x)> 2:
            line+=1
    else:
        line+=1
print(labelDict)
f.close()

src = open("bios.gsm", "r")
bin = open("bios.gb", "wb")
MAGIC_PREFIX = 0xCB
assemblyMap = { 
    "NOP":0x00,             "INC B":0x04,           "DEC B":0x05,           "LD B, 0":0x06,     "INC C":0x0C,       "DEC C":0x0D,       "LD C, 0":0x0E,
    "LD DE, 0":0x11,        "INC DE":0x13,          "DEC D":0x15,           "LD D, 0":0x16,     "RL A":0x17,        "JR 0":0x18,        "LD A, [DE]":0x1A,  "DEC E":0x1D,   "LD E, 0":0x1E,
    "JR NZ, 0":0x20,        "LD HL, 0":0x21,        "LDI [HL], A":0x22,     "INC HL":0x23,      "INC H":0x24,       "JR Z, 0":0x28,     "LD L, 0":0x2E,
    "LD SP, 0":0x31,        "LDD [HL], A":0x32,     "DEC A":0x3D,           "LD A, 0":0x3E,
    "LD C, A":0x4F,
    "LD D, A":0x57,
    "LD H, A":0x67,
    "LD [HL], A":0x77,      "LD A, E":0x7B,         "LD A, H":0x7C,
    "SUB A, B":0x90,
    "XOR A, A":0xAF,
    "POP BC":0xC1,          "PUSH BC":0xC5,         "RET":0xC9,             "CALL 0":0xCD,
    "LD [HN], A, 0":0xE0,   "LD [HC], A":0xE2,      "LD [NN], A, 0":0xEA,
    "LD A, [HN], 0":0xF0,   "LD A, [HC]":0xF2,      "CP 0":0xFE,
}

specialAssemblyMap = {
    "RL C":0x11,
    "BIT 7, H":0x7C,
}

line = 0
for x in src:
    if x == "\n":
        continue
    if "//" in x:
        continue
    if ":" in x:
        x=x.split(":")[1]
    for key in labelDict:
        if key in x:
            x=x.replace(key, hex(labelDict[key]))
    if "0x" in x:
        instruction=x.split("x")[0]
        opcode = assemblyMap[instruction]
        line+=1
        x=x.split("x")[1].strip()
        n = int(x.strip(), 16)
        if len(x)> 2 or "CALL" in instruction:
            l = n % 256
            h = n // 256
            # print(hex(l),hex(h))
            code = [opcode, l, h]
            line+=2
        elif "JR" in instruction:
            jump = n - line
            # print(jump)
            if jump < 0:
                jump += 0xFF
            else:
                jump -= 1
            # print(jump)
            print(hex(jump))
            code = [opcode, jump]
            line+=1
        else: 
            # print(hex(n))
            code = [opcode, n]
            line+=1
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
    bin.write(bytearray(code))
