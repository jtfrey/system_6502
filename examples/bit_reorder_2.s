            .ORG $2000
                            ; bits in byte are 76543210
            LDA $00         ; Poke the byte in $0000 before running
            ROR             ; -7654321 => C=[0]
            BCS SET1        ; Bit 0 was 1
            LDY #$00        ; Starting bit value in Y[0] = 0
            BEQ PRELOOP     ; Skip to the loop
SET1:       LDY #$01        ; Starting bit value in Y[0] = 1
PRELOOP:    LDX #$06        ; Setup to loop 6 times
LOOP:       ROR             ; e.g. on first pass --765432 => C=[1]
            PHA             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[1] to enter as [0]
            TAY             ; Move the reordered version back to Y
            PLA             ; Restore the value we're shifting
            DEX             ; Decrement counter...
            BNE LOOP        ; ...and repeat 5 more times
                            ; Which leaves A having -------7
                            ; and Y having -0123456
            ROR             ; -------- => C=[7]
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[7] to enter as [0]
            STA $00         ; Store the reordered value back to $00
