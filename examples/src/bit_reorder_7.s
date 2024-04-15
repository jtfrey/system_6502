            .ORG $2000
                            ; bits in byte are 76543210
            LDA #$00        ; Poke the byte in $0000 before running
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            ASL A           ; Shift A left to open the 0th bit as 0
            ORA $00         ; OR with $00 which now contains just bit 7 in position 0
            STA $00         ; Store the result to $00