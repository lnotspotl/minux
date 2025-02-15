.text
.globl zfunction

zfunction:
    # Input: param in %eax
    # Output: modified value in %eax
    
    andl $0xFFFFF00F, %eax    # Clear bits 20-27
    shll $8, %eax             # Shift left by 8 bits
    orl $0xFF, %eax          # Set lowest byte to 0xFF
    ret