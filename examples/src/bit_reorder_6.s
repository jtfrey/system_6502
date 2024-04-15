            .ORG $2000
                            ; bits in byte are 76543210
            LDA #$00        ; Poke the byte in $0000 before running
            LDX #$07        ; Prep for 7 iterations
LOOP:       LSR $00         ; Remove a bit from $00, leaving 0's to the left
            ROL A           ; Roll the bit into A
            DEX             ; Decrement X
            BNE LOOP        ; Loop until X == 0
            ASL A           ; Shift A left to open the 0th bit
            ORA $00         ; $00 now has just the 7th bit into 0th position, so OR that into A
            STA $00         ; Put the result back in $00
