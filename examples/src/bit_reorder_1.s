            .ORG $2000
                            ; bits in byte are 76543210
            LDA $00         ; Poke the byte in $0000 before running
            LDX #$07        ; Setup to loop 7 times
            LDY #$00        ; Starting transformed value in Y
LOOP:       ROR             ; -7654321 => C=[0]
            PHA             ; Save the value we're shifting
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[0] to enter as [0]
            TAY             ; Move the reordered version back to Y
            PLA             ; Restore the value we're shifting
            DEX             ; Decrement counter...
            BNE LOOP        ; ...and repeat 6 more times
                            ; Which leaves A having -------7
                            ; and Y having -0123456
            ROR             ; -------- => C=[7]
            TYA             ; Move the reordered version to A
            ROL             ; Shift left, allowing C=[7] to enter as [0]
            STA $00         ; Store the reordered value back to $00
