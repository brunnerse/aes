# Adapted from https://jhafranco.com/2013/05/31/aes-gcm-implementation-in-python/
import sys
from Cryptodome.Cipher import AES
from functools import reduce

hexEndChar = " "
if len(sys.argv) > 1:
    hexEndChar = ""


def printAsHex(byteArray):
    for i, b in enumerate(byteArray):
        print(format(b, "02x"), end=hexEndChar)
        if i % 16 == 15:
            print()
    print()
# define shorter function name
h = printAsHex
 
def xor(x,y):
    """Returns the exclusive or (xor) between two vectors"""
    return bytes(i^j for i,j in zip(x,y))
 
def intToList(number,listSize):
    """Convert a number into a byte list"""
    return [(number >> i) & 0xff
            for i in reversed(range(0,listSize*8,8))]
 
def listToInt(list):
    """Convert a byte list into a number"""
    return reduce(lambda x,y:(x<<8)+y,list)

def multGF2(x,y):
    """Multiply two polynomials in GF(2^m)/g(w)
        g(w) = w^128 + w^7 + w^2 + w + 1
        (operands and result bits reflected)"""     
    (x,y) = map(lambda z:listToInt(list(z)),(x,y))
    z = 0
    while y & ((1<<128)-1):
        if y & (1<<127):
            z ^= x
        y <<= 1
        if x & 1:
            x = (x>>1)^(0xe1<<120)
        else:
            x >>= 1
    return bytes(intToList(z,16))

def GHASH (hkey,aad,ctext, printSteps=False):
    """GCM's GHASH function"""
    def xorMultH (p,q):
        """Multiply (p^q) by hash key"""
        return bytes(multGF2(hkey,xor(p,q)))
     
    def gLen(s):
        """Evaluate length of input in bits and returns
           it in the LSB bytes of a 64-bit string"""
        return bytes(intToList(len(s)*8,8))  
   
    x = bytes(16)    
    # padding
    aadP = aad + bytes((16-len(aad)%16)%16) 
    ctextP = ctext + bytes((16-len(ctext)%16)%16)
    if printSteps:
        print("\nSteps of the hash:")
        printAsHex(x)
    # iterator over aad
    for i in range(0,len(aadP),16):
        if printSteps:
            print("\t Adding\t", end="")
            printAsHex(aadP[i:i+16])
        x = xorMultH(x,aadP[i:i+16])
        if printSteps:
            printAsHex(x)
    # payload phase
    for i in range(0,len(ctextP),16):
        if printSteps:
            print("\t Adding\t", end="")
            printAsHex(ctextP[i:i+16])
        x = xorMultH(x,ctextP[i:i+16])
        if printSteps:
            printAsHex(x)
    # final phase
    if printSteps:
        printAsHex(xorMultH(x,gLen(aad) + gLen(ctext)))
    return xorMultH(x,gLen(aad) + gLen(ctext))
 
def GCM_crypt(keysize,key,iv,input,aad, printSteps=False):
    """GCM's Authenticated Encryption/Decryption Operations"""
    def incr(m):
        """Increment the LSB 32 bits of input counter"""
        n = list(m)
        n12 = bytes(n[:12])
        ctr = listToInt(n[12:])
        if ctr == (1<<32)-1:
            return n12 + bytes(4)
        else:
            return n12 + bytes(intToList(ctr+1,4))
        
    obj = AES.new(key, AES.MODE_ECB) 
    h = bytes(obj.encrypt(bytes(16)))
    if printSteps:
        print("H is:")
        printAsHex(h)
    output = bytes()
    L = len(input)
    if len(iv) == 12:
        y0 = bytes(iv) + bytes(b'\x00\x00\x00\x01')
    else:
        y0 = bytes(GHASH(h,bytes(),iv, printSteps))
    y = y0
    for i in range(0,len(input),16):
        y = incr(y)
        ctextBlock = xor(bytes(obj.encrypt(y)),
                         input[i:min(i+16,L)])
        output += bytes(ctextBlock)
    g = obj.encrypt(y0)
    tag = xor(GHASH(h,aad,output, printSteps),g)
    if printSteps:
        print("Tag is finally hashed with")
        printAsHex(g)
    return output,tag,g,h
  
def GCM_encrypt(keysize,key,iv,ptext,aad, printSteps=False):
    """GCM's Authenticated Encryption Operation"""
    (ctext,tag,g,h) = GCM_crypt(keysize,key,iv,ptext,aad, printSteps)
    return ctext,tag
 
def GCM_decrypt(keysize,key,iv,ctext,aad, tag, printSteps=False):
    """GCM's Authenticated Decryption Operation"""
    (ptext,_,g,h) = GCM_crypt(keysize,key,iv,ctext,aad, printSteps)
    if printSteps:
        print("\nGHashing the plaintext for comparison...")
    calculatedTag = xor(GHASH(h,aad,ctext, printSteps),g) 
    print("Tag: ")
    printAsHex(calculatedTag)
    if tag == calculatedTag:
        return True,ptext
    else:
        return False,ptext
 

print("===========\nGCM\n===========\n")

key = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f])

data = bytes([0x00, 0x10, 0x20, 0x30, 0x01, 0x11, 0x21, 0x31, 0x02, 0x12, 0x22, 0x32, 0x03, 0x13, 0x23, 0x33])
data2 = bytes([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f])
data3 = bytes([0xaf, 0xfe, 0xde, 0xad, 0xbe, 0xef, 0xda, 0xdc, 0xab, 0xbe, 0xad, 0xbe, 0xec, 0x0c, 0xab, 0xad])

IV = bytes([0xf0, 0xe0, 0xd0, 0xc0, 0xb0, 0xa0, 0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10, 0x00])

header = data + data3
payload = data + data + data2


numChannels = 2
for channel in range(0, numChannels):
    print("=====\n CHANNEL %d\n===="%(channel))

    keyC = bytes([channel]) + key[1:]
    IVC = bytes([channel]) + IV[1:]
    headerC = bytes([channel]) + header[1:]
    payloadC = bytes([channel]) + payload[1:]

    print("IV:")
    printAsHex(IVC[:12])
    print("Key:")
    printAsHex(keyC)
    print("Header:")
    printAsHex(headerC)
    print("Payload:")
    printAsHex(payloadC)
    print("ENCRYPTION:")
    ctext, tag = GCM_encrypt(128, keyC, IVC[:12], payloadC, headerC, printSteps=True)
    print("Ciphertext:")
    printAsHex(ctext)
    print("Tag:")
    printAsHex(tag)

    print("\nDECRYPTION:")
    __, ptext = GCM_decrypt(128, keyC, IVC[:12], payloadC, headerC, tag)
    print("\nDecrypted Ciphertext:")
    printAsHex(ptext)